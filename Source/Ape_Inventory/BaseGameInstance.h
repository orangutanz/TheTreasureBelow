// Copyright Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "BaseGameInstance.generated.h"

USTRUCT(BlueprintType)
struct APE_INVENTORY_API FWorldReferenceStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Soft reference to a level (UWorld)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> LevelReference;
};

UCLASS(BlueprintType)
class APE_INVENTORY_API UTTBConfig : public UObject
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void DisableTabNaviation();
};