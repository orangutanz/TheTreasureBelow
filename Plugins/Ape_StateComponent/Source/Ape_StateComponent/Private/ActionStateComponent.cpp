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
