// Copyright 2021 Red J

#pragma once

#include "CoreMinimal.h"
#include "Multithreading/JobSystem.h"

#include "UnrealFlecsModule/FlecsModuleBase.h"
#include "MainGameplay_Systems.generated.h"



/**
 * 
 */
UCLASS()
class UMainGameplay_Systems : public UFlecsModuleBase
{
	GENERATED_BODY()

	virtual void Initialize(flecs::world& ecs) override;
};
