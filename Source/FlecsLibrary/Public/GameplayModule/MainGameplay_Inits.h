// Copyright 2021 Red J

#pragma once

#include "CoreMinimal.h"
#include "GameplayModule/MainGameplayBootstrap.h"
#include "UnrealFlecsModule/FlecsModuleBase.h"
#include "MainGameplay_Inits.generated.h"

/**
 * 
 */
UCLASS()
class UMainGameplay_Inits : public UFlecsModuleBase, public IGameplayConfigSet, public IWorldSet
{
	GENERATED_BODY()

	virtual void Initialize(flecs::world& ecs) override;
};
