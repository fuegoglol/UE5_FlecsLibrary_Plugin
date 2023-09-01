// Fill out your copyright notice in the Description page of Project Settings.

// Fill out your copyright notice in the Description page of Project Settings.

#include "Multithreading/FMyWorker.h"

FMyWorker::FMyWorker()
{
	//Constructeur
}

FMyWorker::FMyWorker(flecs::world* w, ecs_entity_t f, uint32 off, uint32 l) : world(w), function(f), offset(off), limit(l)
{
}

void FMyWorker::DoThreadedWork()
{
	UE_LOG(LogTemp, Warning, TEXT("Je suis le thread qui exécute"));
	
	if (world != nullptr && function != NULL)
	{
		ecs_run_w_filter(*world, function, 0, offset, limit, NULL);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Tout va bien"));
}

void FMyWorker::Abandon()
{
	//Il a fini sa tache
	UE_LOG(LogTemp, Warning, TEXT("Tout va mal"));
}


