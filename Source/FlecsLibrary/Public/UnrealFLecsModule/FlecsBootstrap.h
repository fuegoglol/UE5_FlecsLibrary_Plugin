// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlecsLibrary/Flecs/Public/flecs.h"
#include "GameFramework/Actor.h"
#include "FlecsBootstrap.generated.h"

UCLASS()
class FLECSLIBRARY_API AFlecsBootstrap : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlecsBootstrap();

	virtual void Bootstrap(flecs::world& ecs);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
