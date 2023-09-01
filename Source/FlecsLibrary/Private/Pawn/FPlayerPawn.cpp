// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/FPlayerPawn.h"

#include "Async/Async.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Multithreading/JobSystem.h"
#include "RektProtocol/RPSubsystem.h"
#include "RektProtocol/Utils/FSerializer.h"
#include "RektProtocol/Utils/MyMathLib.h"
#include "Scene/ISMController.h"


AFPlayerPawn::AFPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	const auto* GameInstance = GetGameInstance();
	RPSystem = GameInstance->GetSubsystem<URPSubsystem>();
	RPSystem->Init();
	RPSystem->OnConnectResponseReceivedDelegate.AddUObject(this,&ThisClass::OnConnect);
	RPSystem->Connect();
	RPSystem->OnDataReceivedDelegate.AddUObject(this,&ThisClass::OnReceivedData);
	
	const FVector &Location = FVector();
	
	ElapsedTimes.Init(0,NumberOfPlayers);
	FTimerHandle TimerHandler;
	GetWorldTimerManager().SetTimer(TimerHandler,this,&ThisClass::SearchForIsmController,0.2);

	///////////////////

	auto* MySerialized = JobSystem::GetInstance();
	if(MySerialized->SerializedTransforms.IsEmpty()) return;

	FVector3d Loc = this->GetActorLocation();

	MySerialized->SerializedTransforms[PlayerID]->SetLocation(FVector(Loc.X, Loc.Y, Loc.Z));

	
	
}

void AFPlayerPawn::ChangeColor(int32 colorID)
{
	AISMCont->InstancedStaticMeshComponent->SetCustomDataValue(PlayerID,0,colorID);

	TArray<uint8> DataToSend;

	TArray<uint8> SerializedId;
	TArray<uint8> SerializedMaterial;

	FSerializer::SerializeU16(SerializedId,PlayerID);

	FSerializer::SerializeU16(SerializedMaterial,colorID);

	DataToSend.Append(SerializedId);
	DataToSend.Append(SerializedMaterial);
	
	RPSystem->SendData("/Players/Materials",DataToSend);
}

void AFPlayerPawn::InputMove(const FVector2D &Vector2d)
{
	auto* MySerialized = JobSystem::GetInstance();
	if(MySerialized->SerializedTransforms.IsEmpty()) return;
	const FVector &Location = MySerialized->SerializedTransforms[PlayerID]->GetLocation();

	GetMovementComponent()->AddInputVector(FVector(Vector2d.X, Vector2d.Y, 0));

}

void AFPlayerPawn::OnReceivedData(const uint64& Id, const TArray<uint8>& Value)
{
	if(Id == PlayerPositionTopicId)
	{
		OnPositionReceived(Value);
	}
	else if(Id == PlayerMaterialTopicId)
	{
		OnMaterialReceived(Value);
	}
	else if(Id == PlayersList)
	{
		OnRequestPlayersReceived();
	}
		
}

void AFPlayerPawn::OnPositionReceived(const TArray<uint8>& Value)
{
	
	TArray<uint8> SerializedId;
	MyMathLib::Subarray(0,1,Value,SerializedId);

	TArray<uint8> SerializedLocation;
	MyMathLib::Subarray(2,25,Value,SerializedLocation);

	TArray<uint8> SerializedYaw;
	MyMathLib::Subarray(26, Value.Num()-1,Value,SerializedYaw);

	uint16 Id_;
	FVector Location;
	uint16 Yaw; 

	FSerializer::DeserializeU16(SerializedId,Id_);
	FSerializer::Deserialize(SerializedLocation,Location);
	FSerializer::DeserializeU16(SerializedYaw,Yaw);
	
	auto* MySerialized = JobSystem::GetInstance();
	if(MySerialized->SerializedTransforms[Id_]->GetLocation().Equals(Location,0.01)) return;
	if(Buffer.Find(Id_) == nullptr)
	{
		TArray<TTuple<FVector,FRotator>> Array;
		Array.Init(TTuple<FVector,FRotator>(FVector(),FRotator()),BufferSize);
		Buffer.Add(Id_,Array);
	}
	FirstPacketReceived = true;
	auto & CurrentBuffer = Buffer[Id_];
	for(int i = 0; i < BufferSize - 1; i++)
	{
		CurrentBuffer[i] = CurrentBuffer[i+1];
	}
	CurrentBuffer[BufferSize-1] = TTuple<FVector,FRotator>(Location,FRotator(0,Yaw-90,0).Quaternion());
	ElapsedTimes[Id_] = 0;
	
	

	/*
	if ((Location - this->GetActorLocation()).Length() <= RenderDistance)
	{
		AISMCont->InstancedStaticMeshComponent->SetCustomDataValue(Id_,1,1);
		
	}
	else
	{
		AISMCont->InstancedStaticMeshComponent->SetCustomDataValue(Id_,1,0);
	}
	*/
}


void AFPlayerPawn::OnMaterialReceived(const TArray<uint8>& Value)
{
	
	TArray<uint8> SerializedId;
	MyMathLib::Subarray(0,1,Value,SerializedId);

	TArray<uint8> SerializedColorId;
	MyMathLib::Subarray(2,Value.Num()-1,Value,SerializedColorId);

	uint16 Id_;
	uint16 ColorId;

	FSerializer::DeserializeU16(SerializedId,Id_);
	FSerializer::DeserializeU16(SerializedColorId,ColorId);

	AISMCont->InstancedStaticMeshComponent->SetCustomDataValue(Id_,0,ColorId);
	
}

void AFPlayerPawn::GetCurrentPlayers()
{
	bIsSearching = true;
	GetWorldTimerManager().SetTimer(CheckForPlayersHandle,this,&ThisClass::OnRequestPlayersReceived,TimeForSearching,false);
	
}

void AFPlayerPawn::SendPosition()
{
	auto* MySerialized = JobSystem::GetInstance();
	if(MySerialized->SerializedTransforms.IsEmpty()) return;
	
	const FVector3d Loc = this->GetActorLocation();
	const FRotator Rot = this->GetActorRotation();

	MySerialized->SerializedTransforms[PlayerID]->SetLocation(Loc);
	MySerialized->SerializedTransforms[PlayerID]->SetRotation(FRotator(0,Rot.Yaw,0).Quaternion());

	TArray<uint8> DataToSend;

	TArray<uint8> SerializedId;
	TArray<uint8> SerializedLocation;
	TArray<uint8> SerializedYaw;

	//FSerializer::SerializeU16(SerializedId,PlayerID);
	FSerializer::SerializeU16(SerializedId,PlayerID);

	FSerializer::Serialize(SerializedLocation,MySerialized->SerializedTransforms[PlayerID]->GetLocation());
	FSerializer::SerializeU16(SerializedYaw,MySerialized->SerializedTransforms[PlayerID]->GetRotation().Rotator().Yaw);

	DataToSend.Append(SerializedId);
	DataToSend.Append(SerializedLocation);
	DataToSend.Append(SerializedYaw);
	
	RPSystem->SendData("/Players/Positions",DataToSend);

	

}

void AFPlayerPawn::StartSendingPosition()
{
	GetWorldTimerManager().SetTimer(TimerHandlerMove,this,&ThisClass::SendPosition,SendPositionRate, true);
}

void AFPlayerPawn::StopSendingPosition()
{
	TimerHandlerMove.Invalidate();
}

void AFPlayerPawn::OnConnect(ERPConnectState ErpConnectState)
{
	RPSystem->TopicRequest(ERPTopicRequestFlag::TopicSubscribe,"/Players/Positions");
	RPSystem->TopicRequest(ERPTopicRequestFlag::TopicSubscribe,"/Players/Materials");
	//RPSystem->TopicRequest(ERPTopicRequestFlag::TopicSubscribe,"/Players/ConnectBroadcast");
	SendPosition();

}

void AFPlayerPawn::OnRequestPlayersReceived()
{
	if(!bIsSearching) return;
		
		
}

void AFPlayerPawn::OnTimerElapsed()
{
	bIsSearching = false;
}


void AFPlayerPawn::SearchForIsmController()
{
	auto* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AISMController::StaticClass());
	checkf(Actor, TEXT("AISMController pas recup dans FPlayerPawn"));
	AISMCont = Cast<AISMController>(Actor);
	AISMCont->InstancedStaticMeshComponent->SetCustomDataValue(PlayerID,1,1);
}

void AFPlayerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!FirstPacketReceived) return;
	auto* MySerialized = JobSystem::GetInstance();
	if(MySerialized->SerializedTransforms.IsEmpty()) return;
	for(int i = 0; i<Buffer.Num(); i++)
	{
		
		if(i == PlayerID || Buffer[i].Num() < BufferSize)
			continue;
		auto & CurrentBuffer = Buffer[i];
		
			
		ElapsedTimes[i] += DeltaSeconds;
		float Alpha = FMath::Clamp(ElapsedTimes[i] / SendPositionRate, 0.f, 1.f);
		
		FVector InterpPosition = FMath::Lerp(CurrentBuffer[0].Key, CurrentBuffer[1].Key, Alpha);
		FRotator InterpRotation = FMath::Lerp(CurrentBuffer[0].Value, CurrentBuffer[1].Value, Alpha);
		UE_LOG(LogTemp, Warning, TEXT("Alpha is : %f"),Alpha);
		MySerialized->SerializedTransforms[i]->SetLocation(InterpPosition);
		MySerialized->SerializedTransforms[i]->SetRotation(InterpRotation.Quaternion());
	}
}

