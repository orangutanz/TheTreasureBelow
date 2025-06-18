// Copyright by Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionStateObjects.h"
#include "ActionStateComponent.generated.h"


UCLASS(Blueprintable, BlueprintType, ClassGroup = (ActionState), meta = (BlueprintSpawnableComponent))
class APE_STATECOMPONENT_API UActionStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Initialize();

	/* Call every frame, only tick active states */
	UFUNCTION(BlueprintCallable)
	void TickStates();

	/* Don't call every frame. Check & Update all states.*/
	UFUNCTION(BlueprintCallable)
	void UpdateStates();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_ActionState")
	TArray<TSubclassOf<UStateObject>> UsingStateClasses;

private:
	UPROPERTY()
	TArray<UStateObject*> AllStates;

	UPROPERTY()
	bool Initialized;
};
