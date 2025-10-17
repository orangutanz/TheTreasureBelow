// Copyright by Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionStateObjects.h"
#include "ActionStateComponent.generated.h"

/* AIController Component */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (ActionState), meta = (BlueprintSpawnableComponent))
class APE_STATECOMPONENT_API UActionStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Initialize(AActor* OwnerActor, TArray<TSubclassOf<UStateObject>> StateClasses);

	UFUNCTION(BlueprintCallable)
	void Deinitialize();

	/* A set of new states to replace current ones */
	UFUNCTION(BlueprintCallable)
	bool ReinitializeStates(TArray<TSubclassOf<UStateObject>> NewStateClasses);

	/* Call every frame, only tick active states */
	UFUNCTION(BlueprintCallable)
	void TickStates();

	/* Don't call every frame. Check & Update all states.*/
	UFUNCTION(BlueprintCallable)
	void UpdateStates();

	UFUNCTION(BlueprintCallable)
	TArray<FName> GetActiveStateNames();

private:
	UPROPERTY()
	TArray<UStateObject*> AllStates;
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerPtr;
	UPROPERTY()
	bool Initialized;
};
