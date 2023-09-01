// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RektProtocol/RPSubsystem.h"
#include "RektProtocol/Struct/RPConnectState.h"
#include "RektProtocol/Utils/DefaultHasher.h"
#include "Scene/ISMController.h"
#include "FIAPawn.generated.h"

UCLASS()
class FLECSLIBRARY_API AFIAPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFIAPawn();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void InitPositionAndId(int id);

	// Global function to send requests
	UFUNCTION(BlueprintCallable)
	void ChangeColor(int32 colorID);

	UFUNCTION()
	void GetCurrentPlayers();

	UFUNCTION()
	void SendPosition();

	UFUNCTION(BlueprintCallable)
	void StartSendingPosition();

	UFUNCTION(BlueprintCallable)
	void StopSendingPosition();

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle TimerHandlerMove;


private:

	
	// Function request handle
	UFUNCTION()
	void OnRequestPlayersReceived();

	UFUNCTION()
	void OnTimerElapsed();

	UFUNCTION()
	void OnConnect(ERPConnectState ErpConnectState);

	UFUNCTION()
	void SearchForIsmController();

	UPROPERTY()
	FTimerHandle CheckForPlayersHandle;

	UPROPERTY(EditDefaultsOnly)
	float TimeForSearching = 1;

	UPROPERTY()
	bool bIsSearching = false;
	
	UPROPERTY(EditAnywhere)
	int32 PlayerID = 0;

	UPROPERTY()
	URPSubsystem* RPSystem;

	UPROPERTY()
	TObjectPtr<AISMController> AISMCont;

	UPROPERTY(EditAnywhere, Category = "Replication")
	float SendPositionRate = 0.1;

	UPROPERTY(EditDefaultsOnly)
	float RenderDistance = 10000;

	bool FirstPacketReceived = false;
	
	float LastTime = 0.f;
	
	//Topics name
	const FString PlayerPositionString = "/Players/Positions";
	const FString PlayerMaterialString = "/Players/Materials";
	const FString PlayerListString = "/Players/Names";
	
	// Topics Id
	const uint64 PlayerPositionTopicId = DefaultHasher::StringToU64(TCHAR_TO_UTF8(*PlayerPositionString));
	const uint64 PlayerMaterialTopicId = DefaultHasher::StringToU64(TCHAR_TO_UTF8(*PlayerMaterialString));
	const uint64 PlayersList = DefaultHasher::StringToU64(TCHAR_TO_UTF8(*PlayerListString));
	
};
