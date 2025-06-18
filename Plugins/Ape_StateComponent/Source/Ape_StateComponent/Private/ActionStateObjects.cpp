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
