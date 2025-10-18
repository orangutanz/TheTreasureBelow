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



/* For Object Pools */
UCLASS(Abstract, BlueprintType, Blueprintable)
class APE_STATECOMPONENT_API APooledActor : public AActor
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintPure)
	bool IsActorActive() const { return IsActive; }

	UFUNCTION(BlueprintCallable)
	void SetActorActive(bool bIsActive, float LifeTime = 0);

	UFUNCTION(BlueprintImplementableEvent)
	void OnActorToggled(bool bNewActive);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLeft;

private:
	bool IsActive;

	FTimerHandle DeactivateHandle;
};


UCLASS()
class APE_STATECOMPONENT_API UActorPool : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void InitializedPool(UWorld* World, TSubclassOf<APooledActor> PooledActorClass, int32 PoolSize = 20);

	UFUNCTION(BlueprintCallable)
	void DeinitializePool();

	// Returns an available actor (spawns new if none)
	UFUNCTION(BlueprintCallable)
	APooledActor* GetAvailableActor();

	// Returns an actor back to the pool
	UFUNCTION(BlueprintCallable)
	void ReturnActor(APooledActor* Actor);

	UFUNCTION(BlueprintCallable)
	TSubclassOf<APooledActor> GetPooledActorClass() const { return ActorClass; }

private:
	// Internal increase pool size and return a created actor
	APooledActor* IncreasePoolSize(int32 amount = 10);

protected:
	// The class of actor this pool manages (must derive from APooledActor)
	UPROPERTY(VisibleAnywhere)
	TSubclassOf<APooledActor> ActorClass;

	// Internal pool storage
	TArray<APooledActor*> Pool;

	int32 LastReturnedIndex = -1; // last checked index in the pool

private:
	bool IsInitialized = false;
};