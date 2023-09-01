// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMyWorker.h"

//La tache que je vais donner à mes worker
using Task = std::function<void()>;

// JobSystem* JobSystem::JobSystem_= nullptr;

/**
 * 
 */
class FLECSLIBRARY_API JobSystem
{
	
protected:
	
	JobSystem();
	
	static JobSystem* JobSystem_ ;
	
public:
	
	//Constructeur / Destructeur
	explicit JobSystem(uint16 threads, flecs::world w);
	~JobSystem();

	uint16 nbThreads;
	flecs::world* world;
	TArray<FMyWorker> MyWorkers;

	//Singletons should not be cloneable
	JobSystem(JobSystem &other) = delete;
	
	//Singletons should not be assignable.
	void operator=(const JobSystem &) = delete;
	
	static JobSystem *GetInstance();

	TArray<FTransform*> SerializedTransforms;
};


