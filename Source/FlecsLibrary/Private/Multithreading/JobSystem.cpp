// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/Multithreading/JobSystem.h"
#include <thread>

JobSystem* JobSystem::JobSystem_ = nullptr;


JobSystem::JobSystem()
{
}

JobSystem::JobSystem(uint16 threads, flecs::world w) : nbThreads(threads), world(&w) 
{
}

JobSystem::~JobSystem()
{
}

JobSystem* JobSystem::GetInstance()
{
	if(!JobSystem_)
	{
		JobSystem_ = new JobSystem();
	}
	return JobSystem_;
}
