// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/FIAPawn.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Multithreading/JobSystem.h"
#include "RektProtocol/Utils/FSerializer.h"

AFIAPawn::AFIAPawn()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AFIAPawn::BeginPlay()
{
	Super::BeginPlay();
	const auto* GameInstance = GetGameInstance();
	RPSystem = GameInstance->GetSubsystem<URPSubsystem>();
	RPSystem->Init();
	RPSystem->OnConnectResponseReceivedDelegate.AddUObject(this,&ThisClass::OnConnect);
	RPSystem->Connect();
	
	FTimerHandle TimerHandler;
	GetWorldTimerManager().SetTimer(TimerHandler,this,&ThisClass::SearchForIsmController,0.2);

	
	
}

void AFIAPawn::InitPositionAndId(int id)
{
	PlayerID = id;
	
	auto* MySerialized = JobSystem::GetInstance();
	if(MySerialized->SerializedTransforms.IsEmpty()) return;

	FVector3d Loc = this->GetActorLocation();

	MySerialized->SerializedTransforms[PlayerID]->SetLocation(FVector(Loc.X, Loc.Y, Loc.Z));

	StartSendingPosition();
}

void AFIAPawn::ChangeColor(int32 colorID)
{
	if(!AISMCont) return;
	if(!AISMCont->InstancedStaticMeshComponent) return;
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

void AFIAPawn::GetCurrentPlayers()
{
	bIsSearching = true;
	GetWorldTimerManager().SetTimer(CheckForPlayersHandle,this,&ThisClass::OnRequestPlayersReceived,TimeForSearching,false);
	
}

void AFIAPawn::SendPosition()
{
	auto* MySerialized = JobSystem::GetInstance();
	if(MySerialized->SerializedTransforms.IsEmpty()) return;
	
	const FVector3d Loc = this->GetActorLocation();
	const FRotator Rot = this->GetActorRotation();

	if(!MySerialized->SerializedTransforms.IsValidIndex(PlayerID))
	{
		UE_LOG(LogTemp, Warning, TEXT("The Id %d is unkown in the SerializedTransforms Array"),PlayerID);
		return;
	}
	
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

void AFIAPawn::StartSendingPosition()
{
	GetWorldTimerManager().SetTimer(TimerHandlerMove,this,&ThisClass::SendPosition,SendPositionRate, true);
}

void AFIAPawn::StopSendingPosition()
{
	TimerHandlerMove.Invalidate();
}

void AFIAPawn::OnConnect(ERPConnectState ErpConnectState)
{
	SendPosition();
}

void AFIAPawn::OnRequestPlayersReceived()
{
	if(!bIsSearching) return;
		
		
}

void AFIAPawn::OnTimerElapsed()
{
	bIsSearching = false;
}


void AFIAPawn::SearchForIsmController()
{
	auto* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AISMController::StaticClass());
	checkf(Actor, TEXT("AISMController pas recup dans FPlayerPawn"));
	AISMCont = Cast<AISMController>(Actor);
	AISMCont->InstancedStaticMeshComponent->SetCustomDataValue(PlayerID,1,1);
}


