// Copyright by Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonBlock_Base.h"
#include "DungeonManager.generated.h"


UCLASS()
class APEDUNGEONGENERATOR_API ADungeonManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "DungeonGenerator|Server")
	TSubclassOf<ADungeonBlock_Base> BP_GetRandomInitialBlockType() const;

	/** The server keeps track of spawned blocks */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DungeonGenerator|Server")
	TArray<ADungeonBlock_Base*> SpawnedBlocks;

	/** The main server function to generate the dungeon */
	UFUNCTION(BlueprintCallable, Category = "DungeonGenerator|Server")
	void GenerationDungeon(FVector InitialLocation, FRotator InitialRotation, int32 MaxBlocksValue);


	float FindOppositeYaw(float InValue);

private:
	/** Helper function to spawn and connect both blocks */
	ADungeonBlock_Base* SpawnDungeonBlock(ADungeonBlock_Base* ConnectedToBlock);

};
