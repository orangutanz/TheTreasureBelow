// Copyright by Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActionStateObjects.generated.h"

/* Enter, Exit, and Tick. Tick determine Actions */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class APE_STATECOMPONENT_API UStateObject : public UObject
{
	GENERATED_BODY()

public:
	// C++ Internal 
	/** Check should enter/exit state. Return is State Active */
	UFUNCTION()
	void InitializeState(AActor* Owner);

	UFUNCTION()
	bool UpdateState();

	UFUNCTION()
	void SetIsActive(bool isActive);

	// ========== Blueprint Implementations ==========
	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	void OnInitialize();
	virtual void OnInitialize_Implementation() { return; }

	/* Not called every frame. To Check & Update state's can enter or exit.*/
	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	void UpdateActivedState();
	virtual void UpdateActivedState_Implementation() { return ; }

	/* Call every frame, only tick when state is active */
	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	void TickActivedState();
	virtual void TickActivedState_Implementation() { return; }

	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	bool CheckCanEnter();
	virtual bool CheckCanEnter_Implementation() { return false; }

	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	bool CheckCanExit();
	virtual bool CheckCanExit_Implementation() { return false; }

	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	void OnEnter();
	virtual bool OnEnter_Implementation() { return false; }

	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	void OnExit();
	virtual bool OnExit_Implementation() { return false; }


	// ========== Blueprint Callables ==========

	UFUNCTION(BlueprintPure, Category = "Ape_ActionState|State")
	bool IsStateActive() { return IsActive; }

	UFUNCTION(BlueprintCallable, Category = "Ape_ActionState|Get")
	AActor* GetOwnerActor() const
	{
		return OwnerPtr.Get();
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_ActionState|State")
	FName StateName = "";

private:
	UPROPERTY()
	bool IsInitialized;
	UPROPERTY()
	bool IsActive;
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerPtr;
};

