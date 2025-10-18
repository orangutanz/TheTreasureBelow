// Copyright by Yuhan Ma. All Rights Reserved.


#include "ActionStateComponent.h"

void UActionStateComponent::Initialize(AActor* OwnerActor, TArray<TSubclassOf<UStateObject>> StateClasses)
{
	if (Initialized)
	{
		return;
	}
	OwnerPtr = OwnerActor;
	for (auto i : StateClasses)
	{
		UStateObject* NewState = NewObject<UStateObject>(this, i);
		NewState->InitializeState(OwnerActor);
		AllStates.Add(NewState);
	}
	Initialized = true;
}

void UActionStateComponent::Deinitialize()
{
	for (auto i : AllStates)
	{
		if (i->IsStateActive())
		{
			i->SetIsActive(false);
		}		
	}
	AllStates.Empty();
	Initialized = false;
}


bool UActionStateComponent::ReinitializeStates(TArray<TSubclassOf<UStateObject>> NewStateClasses)
{
	if (!Initialized)
	{
		return false;
	}
	AllStates.Empty();
	for (auto i : NewStateClasses)
	{
		UStateObject* NewState = NewObject<UStateObject>(this, i);
		NewState->InitializeState(OwnerPtr.Get());
		AllStates.Add(NewState);
	}
	return true;
}

void UActionStateComponent::TickStates()
{
	for (auto i : AllStates)
	{
		if (i->IsStateActive())
		{
			i->TickActivedState();
		}
	}
}

void UActionStateComponent::UpdateStates()
{
	for (auto i : AllStates)
	{
		i->UpdateState();
	}
}

TArray<FName> UActionStateComponent::GetActiveStateNames()
{
	TArray<FName> ActiveStates;
	for (auto i : AllStates)
	{
		if (i->IsStateActive())
		{
			ActiveStates.Add(i->StateName);
		}
	}
	return ActiveStates;
}

void UActorPoolComponent::InitializePools()
{
	if (IsInitialized)
		return;

	for (auto classType : PoolClassTypes)
	{
		UActorPool* NewPool = NewObject<UActorPool>(this, UActorPool::StaticClass());
		if (!NewPool)
		{
			continue; //unable to create type pool
		}
		if (Pools.AddUnique(NewPool) != -1)
		{
			NewPool->InitializedPool(GetWorld(),classType);
		}
	}
	IsInitialized = true;
}

void UActorPoolComponent::DeinitializePools()
{
	if (!IsInitialized)
		return;
	for (auto i : Pools)
	{
		i->DeinitializePool();
	}
	IsInitialized = false;
}

bool UActorPoolComponent::AddActorPoolType(TSubclassOf<APooledActor> typeActor, int32 poolSize)
{
	if (!IsInitialized || poolSize == 0 || !IsValid(typeActor))
		return false;

	return false;
}

bool UActorPoolComponent::RemoveActorPoolType(TSubclassOf<APooledActor> typeActor)
{
	if (!IsInitialized || !IsValid(typeActor))
		return false;

	return false;
}

APooledActor* UActorPoolComponent::GetAvailableActorType(TSubclassOf<APooledActor> typeActor)
{
	if (!IsInitialized || !typeActor) return nullptr;

	for (UActorPool* pool : Pools)
	{
		if (pool->GetPooledActorClass() == typeActor) 
		{
			pool->GetAvailableActor();
		}
	}
	return nullptr;
}

void UActorPoolComponent::ReturnActorType(APooledActor* poolActor)
{
	if (!IsInitialized || !poolActor)
		return;

	for (UActorPool* pool : Pools)
	{
		if (pool->GetPooledActorClass() == poolActor->GetClass())
		{
			pool->ReturnActor(poolActor);
			break;
		}
	}
}
