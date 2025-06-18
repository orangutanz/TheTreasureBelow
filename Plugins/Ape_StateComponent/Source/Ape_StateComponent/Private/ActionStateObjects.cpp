// Copyright by Yuhan Ma. All Rights Reserved.


#include "ActionStateObjects.h"

void UStateObject::InitializeState(AActor* Owner)
{
	if (IsInitialized)
		return;

	OwnerPtr = Owner;
	for (auto ActionClass : UsingActionClasses)
	{
		if (ActionClass)
		{
			auto NewAction = NewObject<UActionObject>(this, ActionClass);
			NewAction->OwnerPtr = Owner;
			AllActions.Add(NewAction);
		}
	}
	IsInitialized = true;
}

bool UStateObject::UpdateState()
{
	if (!IsActive)
	{
		IsActive = CheckCanEnter();
		OnEnter();
	}
	else
	{
		IsActive = CheckCanExit();
		OnExit();
	}
	return IsActive;
}

void UStateObject::ExecuteAction(FName ActionName)
{
	for (auto i : AllActions)
	{
		if (i->ActionName == ActionName)
		{
			i->OnExecute();
			return;
		}
	}
	return;
}

void UStateObject::StopAction(FName ActionName)
{
	for (auto i : AllActions)
	{
		if (i->ActionName == ActionName)
		{
			i->OnStop();
			return;
		}
	}
	return;
}
