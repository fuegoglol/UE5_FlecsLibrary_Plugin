// Fill out your copyright notice in the Description page of Project Settings.


#include "UnrealFLecsModule/FlecsBootstrap.h"

#include "Kismet/GameplayStatics.h"
#include "Multithreading/FMyWorker.h"
#include "UnrealFLecsModule/UnrealFlecsSubsystem.h"


// Sets default values
AFlecsBootstrap::AFlecsBootstrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AFlecsBootstrap::BeginPlay()
{	
	UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	UUnrealFlecsSubsystem* subSystem = gameInstance->GetSubsystem<UUnrealFlecsSubsystem>();

	Bootstrap(*subSystem->GetEcsWorld());
	
	Super::BeginPlay();
	
}

void AFlecsBootstrap::Bootstrap(flecs::world& ecs)
{
	// Here we can initialize flecs systems and components, if we'd like to
}

