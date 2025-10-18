// Copyright by Yuhan Ma. All Rights Reserved.


#include "ActionStateObjects.h"

void UStateObject::InitializeState(AActor* Owner)
{
	if (IsInitialized)
		return;

	OwnerPtr = Owner;
	OnInitialize(); //Call BP Initialize
	IsInitialized = true;
}

bool UStateObject::UpdateState()
{
	if (!IsActive)
	{
		if (CheckCanEnter())
		{
			IsActive = true;
			OnEnter();
			UpdateActivedState(); //Update only active state
		}
	}
	else
	{		
		if (CheckCanExit())
		{
			IsActive = false;
			OnExit();
		}
		else
		{
			UpdateActivedState(); //Update only active state
		}
	}
	return IsActive;
}

void UStateObject::SetIsActive(bool isActive)
{
	IsActive = isActive;
	OnExit();
}

void APooledActor::SetActorActive(bool bIsActive, float LifeTime)
{
	IsActive = bIsActive;
	TimeLeft = LifeTime;
	SetActorHiddenInGame(!IsActive); //Hide Actor
	SetActorTickEnabled(IsActive); //Stop Updating
	SetActorEnableCollision(IsActive); //Stop physics
	if (IsActive && TimeLeft > 0)
	{
		GetWorldTimerManager().ClearTimer(DeactivateHandle);

		// Set timer to deactivate this actor after TimeLeft seconds
		GetWorldTimerManager().SetTimer(DeactivateHandle,[this](){SetActorActive(false);},TimeLeft,	false);
	}
	else if (!IsActive)
	{
		GetWorldTimerManager().ClearTimer(DeactivateHandle);
	}
	OnActorToggled(IsActive);
}

void UActorPool ::InitializedPool(UWorld* World, TSubclassOf<APooledActor> PooledActorClass, int32 PoolSize)
{
	if (IsInitialized || !PooledActorClass || PoolSize == 0)
	{
		return;
	}

	ActorClass = PooledActorClass;
	Pool.Reserve(PoolSize);
	for (int i = 0 ; i < PoolSize; ++i)
	{
		APooledActor* NewActor = World->SpawnActor<APooledActor>(PooledActorClass, FVector::ZeroVector, FRotator::ZeroRotator);

		if (NewActor)
		{
			NewActor->SetActorActive(false);
			Pool.Add(NewActor);
		}
	}
	IsInitialized = true;
}

void UActorPool ::DeinitializePool()
{
	if (!IsInitialized)
	{
		return;
	}
	for (auto i : Pool)
	{
		if (IsValid(i))
		{
			i->SetActorActive(false);
			i->Destroy();
		}
	}
	Pool.Empty();
	ActorClass = NULL;
	IsInitialized = false;
}

APooledActor* UActorPool ::GetAvailableActor()
{
	if (!IsInitialized)
		return nullptr;

	int32 PoolSize = Pool.Num();
	APooledActor* Actor = NULL;
	for (int32 i = 1; i <= PoolSize; ++i)
	{
		int32 Index = (LastReturnedIndex + i) % PoolSize;
		Actor = Pool[Index];

		if (Actor && !Actor->IsActorActive())
		{
			LastReturnedIndex = Index;
			return Actor;     
		}
	}

	return IncreasePoolSize(10);
}

void UActorPool ::ReturnActor(APooledActor* Actor)
{
	if (!IsInitialized || !IsValid(Actor))
		return;
	if (!Actor->IsActorActive())
		return;
	for (auto i : Pool)
	{
		if (Actor == i)
		{
			Actor->SetActorActive(false);
			return;
		}
	}
}

APooledActor* UActorPool ::IncreasePoolSize(int32 amount)
{
	Pool.Reserve(Pool.Num()+amount);
	LastReturnedIndex = Pool.Num();
	APooledActor* NewActor = NULL;
	for (int i = 0; i < amount; ++i)
	{
		NewActor = GetWorld()->SpawnActor<APooledActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator);

		if (NewActor)
		{
			NewActor->SetActorActive(false);
			Pool.Add(NewActor);
		}
	}
	return Pool[LastReturnedIndex];
}