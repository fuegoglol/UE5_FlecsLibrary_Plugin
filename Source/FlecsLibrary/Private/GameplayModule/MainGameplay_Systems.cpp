// Copyright 2021 Red J

#include "GameplayModule/MainGameplay_Systems.h"
#include "GameplayModule/MainGameplay_Components.h"
#include <thread>

#include "Multithreading/JobSystem.h"
#include "Serialization/JsonTypes.h"

void SystemSpawnInstancesInRadius(flecs::iter& It)
{
	
	auto ecs = It.world();
	auto cBatch = It.field<BatchInstanceAdding>(1);
	auto cMap = It.field<ISM_Map>(2);
	auto cGameSettings = It.field<GameSettings>(3);
	auto cVelocity = It.field<SpaceshipVelocity>(4);

	for (auto i : It)
	{
		auto batch = cBatch[i];
		auto controller = *(cMap->ISMs.Find(batch.Hash));
		if (controller != nullptr)
		{
			for (auto j = 0; j < batch.Num; j++)
			{
				auto instanceIndex = controller->AddInstance();
				float spawnRadius = FMath::RandRange(cGameSettings->SpawnRange.X, cGameSettings->SpawnRange.Y);
				FVector direction = FMath::VRand();
				auto pos = direction * spawnRadius;
				FTransform transformValue = FTransform{FVector{pos.X, pos.Y, pos.Z}};
				ecs.entity()
				   .add(batch.Prefab)
				   .set<ISM_ControllerRef>({controller})
				   .set<ISM_Index>({instanceIndex})
				   .set<ISM_Hash>({batch.Hash})
				   .set<Transform>({transformValue})
				   .set<SpaceshipVelocity>({cVelocity->Velocity});
			}
			controller->CreateOrExpandTransformArray();
		}
		It.entity(i).destruct();
	}
}

void SystemCopyInstanceTransforms(flecs::iter& It)
{
	auto cTransform = It.field<Transform>(1);
	auto cISMIndex = It.field<ISM_Index>(2);
	auto cISMController = It.field<ISM_ControllerRef>(3);

	for (auto i : It)
	{
		auto index = cISMIndex[i].Value;
		cISMController[i].Value->SetTransform(index, cTransform[i].Value);
	}
}

void SystemUpdateTransformsInBatch(flecs::iter& It)
{

	//Datas
	auto cMap = It.field<ISM_Map>(1);

	//Boucle iter
	for (auto& data : cMap->ISMs)
	{
		data.Value->BatchUpdateTransform();
	}
	
}

void SystemUpdateBoids(flecs::iter& It)
{
	
	auto cTransform = It.field<Transform>(1);
	auto cBoidSettings = It.field<BoidSettings>(2);
	auto cSpeed = It.field<Speed>(3);

	TMap<FIntVector, TArray<int>> hashMap;
	TArray<FVector> cellPositions;
	cellPositions.SetNumUninitialized(It.count());
	TArray<FVector> cellAlignment;
	cellAlignment.SetNumUninitialized(It.count());
	TArray<int> cellBoidCount;
	cellBoidCount.SetNumZeroed(It.count());
	TArray<int> cellIndices;
	cellIndices.SetNumUninitialized(It.count());


	for (auto i : It)
	{
		auto location = cTransform[i].Value.GetLocation();
		FIntVector hashedVector = FIntVector(location / cBoidSettings->CellSize);

		auto entityIndices = hashMap.Find(hashedVector);
		if (!entityIndices)
		{
			TArray<int> newEntityIndices;

			newEntityIndices.Add(i);
			hashMap.Emplace(hashedVector, std::move(newEntityIndices));
		}
		else
		{
			entityIndices->Add(i);
		}


		cellPositions[i] = location;
		cellAlignment[i] = cTransform[i].Value.GetRotation().GetForwardVector();
	}

	// Merge Cells
	for (auto& hashedData : hashMap)
	{
		if (hashedData.Value.Num() > 0)
		{
			auto cellIndex = hashedData.Value[0];
			cellIndices[cellIndex] = cellIndex;
			cellBoidCount[cellIndex] = 1;

			for (auto i = 1; i < hashedData.Value.Num(); i++)
			{
				auto index = hashedData.Value[i];
				cellIndices[index] = cellIndex;
				cellBoidCount[cellIndex] += 1;
				cellPositions[cellIndex] += cellPositions[index];
				cellAlignment[cellIndex] += cellAlignment[index];
			}
		}
	}


	for (auto boidIndex : It)
	{
		auto transform = cTransform[boidIndex].Value;
		auto boidPosition = transform.GetLocation();
		auto boidForward = transform.GetRotation().GetForwardVector();
		int cellIndex = cellIndices[boidIndex];

		int nearbyBoidCount = cellBoidCount[cellIndex] - 1;

		FVector force = FVector::ZeroVector;

		if (nearbyBoidCount > 0)
		{
			auto positionSum = cellPositions[cellIndex] - boidPosition;
			auto alignmentSum = cellAlignment[cellIndex] - boidForward;

			auto averagePosition = positionSum / nearbyBoidCount;

			float distToAveragePositionSq = FVector::DistSquared(averagePosition, boidPosition);
			float maxDistToAveragePositionSq = cBoidSettings->CellSize * cBoidSettings->CellSize;

			float distanceNormalized = distToAveragePositionSq / maxDistToAveragePositionSq;
			float needToLeave = FMath::Max(1 - distanceNormalized, 0.f);

			FVector toAveragePosition = (averagePosition - boidPosition).GetSafeNormal();
			auto averageHeading = alignmentSum / nearbyBoidCount;

			force += -toAveragePosition * cBoidSettings->SeparationWeight * needToLeave;
			force += toAveragePosition * cBoidSettings->CohesionWeight;
			force += averageHeading * cBoidSettings->AlignmentWeight;
		}

		if (FMath::Min(FMath::Min(
			               (cBoidSettings->CageSize / 2.f) - FMath::Abs(boidPosition.X),
			               (cBoidSettings->CageSize / 2.f) - FMath::Abs(boidPosition.Y)),
		               (cBoidSettings->CageSize / 2.f) - FMath::Abs(boidPosition.Z))
			< cBoidSettings->CageAvoidDistance)
		{
			force += -boidPosition.GetSafeNormal() * cBoidSettings->CageAvoidWeight;
		}

		FVector velocity = transform.GetRotation().GetForwardVector();
		velocity += force * It.delta_time();
		velocity = velocity.GetSafeNormal() * cSpeed->Value;
		transform.SetLocation(transform.GetLocation() + velocity * It.delta_time());

		auto rotator = FRotationMatrix::MakeFromX(velocity.GetSafeNormal()).Rotator();
		transform.SetRotation(rotator.Quaternion());

		cTransform[boidIndex].Value = transform;
	}
}

void UpdateActorsPositions(flecs::iter& It)
{
	auto* MySerialized = JobSystem::GetInstance();
	
	auto cTransform = It.field<Transform>(1);

	for (auto i : It)
	{
		if(!MySerialized->SerializedTransforms[i]) continue;
		
		auto cTransformX = MySerialized->SerializedTransforms[i]->GetLocation().X;
		auto cTransformY = MySerialized->SerializedTransforms[i]->GetLocation().Y;
		auto cTransformZ = MySerialized->SerializedTransforms[i]->GetLocation().Z;

		UE::Math::TVector newTransform = UE::Math::TVector(cTransformX, cTransformY, cTransformZ);
		
		cTransform[i].Value.SetLocation(newTransform);
	}
	

	
}
void SimulateFakeConnectionSystem(flecs::iter& It)
{
	// auto* MySerialized = JobSystem::GetInstance();
	// if(MySerialized->SerializedTransforms.IsEmpty()) return;
	// MySerialized->SerializedTransforms[0]->SetLocation(FVector(FMath::Rand(), FMath::Rand(), FMath::Rand()));
}

void SerializeTransform(flecs::iter& It)
{
	auto cTransform = It.field<Transform>(1);

	//UE_LOG(LogTemp, Warning, TEXT("%i"), It.count());
	
	//Recupération de notre singleton
	auto* MySerialized = JobSystem::GetInstance();
	//MySerialized->SerializedTransforms = TArray<FTransform*>();
	FTransform* Element = nullptr;
	MySerialized->SerializedTransforms.Init(Element, It.count()); 

	for (auto i : It)
	{
		MySerialized->SerializedTransforms[i] = &cTransform[i].Value;
	}
	UE_LOG(LogTemp, Warning, TEXT("%d elements in the SerializedTransforms Array"),MySerialized->SerializedTransforms.Num());
}
	

void UMainGameplay_Systems::Initialize(flecs::world& ecs)
{
	
	//world.thread_init(2);
	
	//SystemSpawnInstancesInRadius
	ecs.system()
	  .with<BatchInstanceAdding>()
	  .with<ISM_Map>().src(ecs.lookup("Game"))
	  .with<GameSettings>().src(ecs.lookup("Game"))
	  .with<SpaceshipVelocity>().src(ecs.lookup("Game"))
	  .iter(SystemSpawnInstancesInRadius)
	  .remove(EcsOnUpdate)
	  .add(EcsOnStart)
	;

	//SystemCopyInstanceTransforms
	ecs.system()
	  .with<Transform>()
	  .with<ISM_Index>()
	  .with<ISM_ControllerRef>()
	  .multi_threaded(true)
	  .iter(SystemCopyInstanceTransforms);
	
	//SystemUpdateTransformsInBatch
	ecs.system()
	  .with<ISM_Map>()
	  .multi_threaded(true)
	  .iter(SystemUpdateTransformsInBatch);

	
	//SystemUpdateBoids
	ecs.system()
	  .with<Transform>()
	  .with<BoidSettings>().src(ecs.lookup("Game"))
	  .with<Speed>().src(ecs.lookup("SHARED"))
	  .with<BoidInstance>()
	  .multi_threaded(true)
	  .iter(SystemUpdateBoids);
	
	//MoveActor
	ecs.system()
	  .with<Transform>()
	  .with<SpaceshipVelocity>()
	  .iter(SerializeTransform)
	  .remove(EcsOnUpdate)
	  .add(EcsOnStart)
	;
	/*
	//UpdateActorsPositions
	ecs.system()
	  .with<Transform>()
	  .with<SpaceshipVelocity>()
	  .multi_threaded(true)
	  .iter(UpdateActorsPositions)
	  ;*/

	//FakeInternetInputs
	ecs.system()
	  .with<Transform>()
	  .with<SpaceshipVelocity>()
	  //.multi_threaded(true)
	  .iter(SimulateFakeConnectionSystem)
	  ;
}




