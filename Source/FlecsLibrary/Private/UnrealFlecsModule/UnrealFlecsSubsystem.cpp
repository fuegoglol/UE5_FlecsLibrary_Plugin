// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealFLecsModule/UnrealFlecsSubsystem.h"

#include "Editor.h"
#include "FlecsLibrary/Flecs/Public/flecs.h"
#include "Multithreading/FMyWorker.h"

void UUnrealFlecsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	OnTickDelegate = FTickerDelegate::CreateUObject(this, &UUnrealFlecsSubsystem::Tick);
	OnTickHandle = FTicker::GetCoreTicker().AddTicker(OnTickDelegate);

	//Create the Flecs world immediately after initialization of Game Instance
	ECSWorld = new flecs::world();
	ECSWorld->set_threads(12);
	
	
	FEditorDelegates::PausePIE.AddLambda([&](bool bNewValue)
	{
		bIsPaused = bNewValue;
	});
	
	UE_LOG(LogTemp, Warning, TEXT("UUnrealFlecsSubsystem has started!"));
	
	Super::Initialize(Collection);
}

void UUnrealFlecsSubsystem::Deinitialize()
{
	FTicker::GetCoreTicker().RemoveTicker(OnTickHandle);

	//Destroying the Flecs world as part of the deinitialization of Game Instance
	if(!ECSWorld) delete(ECSWorld);

	UE_LOG(LogTemp, Warning, TEXT("UUnrealFlecsSubsystem has shut down!"));
	
	Super::Deinitialize();
}

flecs::world* UUnrealFlecsSubsystem::GetEcsWorld() const
{
	return ECSWorld;
}

bool UUnrealFlecsSubsystem::Tick(float DeltaTime)
{
	if(ECSWorld && !GetWorld()->IsPaused())
	{
		//ECSWorld->set_target_fps(60);
		//ECSWorld->set_threads(12);
		ECSWorld->progress(DeltaTime);
	}
	return true;
}