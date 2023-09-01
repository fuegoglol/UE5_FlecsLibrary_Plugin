// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlecsLibrary/Flecs/Public/flecs.h"
#include "Multithreading/FMyWorker.h"
#include "UnrealFlecsSubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class FLECSLIBRARY_API UUnrealFlecsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	FMyWorker Mworker;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Getting the Flecs world if we need it
	flecs::world* GetEcsWorld() const;
protected:
	// UnrealSubsystem doesn't have a Tick function; instead
	// we use `FTickerDelegate` 
	FTickerDelegate OnTickDelegate;
	FDelegateHandle OnTickHandle;


	// UUnrealFlecsSubsystem contains a pointer to the current Flecs world
	flecs::world* ECSWorld = nullptr;

	bool bIsPaused = false;

	
private:
	//Our custom Tick function used by `FTickerDelegate`
	bool Tick(float DeltaTime);
};

