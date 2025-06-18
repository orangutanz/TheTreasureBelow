// Copyright by Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActionStateObjects.generated.h"

/* Execute or Stop, */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class APE_STATECOMPONENT_API UActionObject : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|Action")
	void OnExecute();
	virtual void OnExecute_Implementation() { return; }

	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|Action")
	void OnStop();
	virtual void OnStop_Implementation() { return; }

	UFUNCTION(BlueprintCallable, Category = "Ape_ActionState|Action")
	AActor* GetOwnerActor() const
	{
		return OwnerPtr.Get();
	}
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_ActionState|Action")
	FName ActionName = "";

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerPtr;
};


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

	// Blueprint implementations
	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	void UpdateStateActions();
	virtual void UpdateStateActions_Implementation() { return ; }

	UFUNCTION(BlueprintNativeEvent, Category = "Ape_ActionState|State")
	void TickActiveState();
	virtual void TickActiveState_Implementation() { return; }


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

	// Blueprint Call Functions

	UFUNCTION(BlueprintPure, Category = "Ape_ActionState|State")
	bool IsStateActive() { return IsActive; }

	UFUNCTION(BlueprintCallable, Category = "Ape_ActionState|AcStatetion")
	AActor* GetOwnerActor() const
	{
		return OwnerPtr.Get();
	}
	/* Try Fire or Stop Action */
	UFUNCTION(BlueprintCallable, Category = "Ape_ActionState|State")
	void ExecuteAction(FName ActionName);
	UFUNCTION(BlueprintCallable, Category = "Ape_ActionState|State")
	void StopAction(FName ActionName);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_ActionState|State")
	FName StateName = "";

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_ActionState|State")
	TArray<TSubclassOf<UActionObject>> UsingActionClasses;
	UPROPERTY(BlueprintReadOnly, Category = "Ape_ActionState|State")
	TArray<UActionObject*> AllActions;

private:
	UPROPERTY()
	bool IsInitialized;
	UPROPERTY()
	bool IsActive;
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerPtr;
};
