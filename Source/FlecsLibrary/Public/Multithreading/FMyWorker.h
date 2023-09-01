// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlecsLibrary/Flecs/Public/flecs.h"

/**
 * 
 */
class FLECSLIBRARY_API FMyWorker : public IQueuedWork
{
private:
	flecs::world* world;
	ecs_entity_t function;
	uint32 offset;
	uint32 limit;
	
public:	
	FMyWorker();
	FMyWorker(flecs::world* w, ecs_entity_t f, uint32 off, uint32 l );
	void DoThreadedWork() override;
	virtual  void Abandon() override;
};