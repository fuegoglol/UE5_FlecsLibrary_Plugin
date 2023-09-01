// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlecsLibrary/Public/Scene/ISMController.h"
#include "GameFramework/Character.h"
#include "RektProtocol/RPSubsystem.h"
#include "RektProtocol/Utils/DefaultHasher.h"

#include "FPlayerPawn.generated.h"

UCLASS()
class FLECSLIBRARY_API AFPlayerPawn : public ACharacter
{
	GENERATED_BODY()


protected:

	AFPlayerPawn();
	virtual void BeginPlay() override;

	// Global function to send requests
	UFUNCTION(BlueprintCallable)
	void ChangeColor(int32 colorID);

	UFUNCTION(BlueprintCallable)
	void InputMove(const FVector2D &Vector2d);

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
	void OnReceivedData(const uint64& Id, const TArray<uint8>& Value);

	UFUNCTION()
	void OnPositionReceived(const TArray<uint8>& Value);

	UFUNCTION()
	void OnMaterialReceived(const TArray<uint8>& Value);

	UFUNCTION()
	void OnConnect(ERPConnectState ErpConnectState);

	UFUNCTION()
	void SearchForIsmController();

public:
	virtual void Tick(float DeltaSeconds) override;

private:
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

	TArray<float> ElapsedTimes;

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

	UPROPERTY(EditDefaultsOnly)
	int32 BufferSize = 5;
	
	TMap<int32,TArray<TTuple<FVector,FRotator>>> Buffer; // Players[Buffer[Infos[]]] 


	UPROPERTY()
	TArray<float> t;

	UPROPERTY(EditDefaultsOnly)
	int32 NumberOfPlayers = 2;
	
};


