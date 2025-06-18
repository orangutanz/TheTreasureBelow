// Copyright by Yuhan Ma. All Rights Reserved.


#include "ActionStateComponent.h"

void UActionStateComponent::Initialize()
{
	if (Initialized)
	{
		return;
	}

	for (TSubclassOf<UStateObject> StateClass : UsingStateClasses)
	{
		if (StateClass)
		{
			UStateObject* NewState = NewObject<UStateObject>(this, StateClass);
			NewState->InitializeState(GetOwner());
			AllStates.Add(NewState);
		}
	}
	Initialized = true;
}

void UActionStateComponent::TickStates()
{
	for (auto i : AllStates)
	{
		if (i->IsStateActive())
		{
			i->TickActiveState();
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
