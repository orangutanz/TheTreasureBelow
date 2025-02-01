// Copyright Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class APE_INVENTORY_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()
	

	UFUNCTION(BlueprintCallable, Category = "GameInstance")
	void Initialize();

};
