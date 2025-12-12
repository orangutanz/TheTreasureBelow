// Copyright by Yuhan Ma. All Rights Reserved.


#include "DungeonManager.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

ADungeonManager::ADungeonManager()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = false;
}

void ADungeonManager::BeginPlay()
{
	Super::BeginPlay();
}

float ADungeonManager::FindOppositeYaw(float InValue)
{
    float compareValue = InValue;
    if (compareValue > 179)
        compareValue -= 180;
    if (compareValue < -179)
        compareValue += 180;
    if (compareValue > 179 || compareValue < -179)
        return 0;
    if (compareValue > 89)
        return 90;
    if (compareValue < -89)
        return -90;
    return 179.998;
}

void ADungeonManager::GenerationDungeon(FVector InitialLocation, FRotator InitialRotation, int32 MaxBlocksValue)
{
	if (!HasAuthority()) return;

	SpawnedBlocks.Empty();

    // Spawn first block
    TSubclassOf<ADungeonBlock_Base> FirstBlockBP = BP_GetRandomInitialBlockType(); // Get initial block from BP function
    if (!FirstBlockBP) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADungeonBlock_Base* FirstBlock = GetWorld()->SpawnActor<ADungeonBlock_Base>(FirstBlockBP, InitialLocation, InitialRotation, Params);
	if (!FirstBlock) return;

    FirstBlock->SetConnectionPoint(0, true, nullptr);
	SpawnedBlocks.Add(FirstBlock);

	int32 UsedBlockValues = FirstBlock->BlockValue;

	// Main loop
	while (UsedBlockValues < MaxBlocksValue)
	{
		// Collect blocks with free connections
		TArray<ADungeonBlock_Base*> BlocksWithFreeConnections;
		for (ADungeonBlock_Base* Block : SpawnedBlocks)
		{
			if (Block && Block->GetAvailableConnectionPoints().Num() > 0)
			{
				BlocksWithFreeConnections.Add(Block);
			}
		}

		if (BlocksWithFreeConnections.Num() == 0) break;

		// Pick random parent
		ADungeonBlock_Base* ParentBlock = BlocksWithFreeConnections[FMath::RandRange(0, BlocksWithFreeConnections.Num() - 1)];

		// Spawn and connect a new block
		ADungeonBlock_Base* NewBlock = SpawnDungeonBlock(ParentBlock);
		if (!NewBlock) break;

		UsedBlockValues += NewBlock->BlockValue;
	}
}


ADungeonBlock_Base* ADungeonManager::SpawnDungeonBlock(ADungeonBlock_Base* ConnectedToBlock)
{
    if (!HasAuthority() || !ConnectedToBlock)
        return nullptr;

    TSubclassOf<ADungeonBlock_Base> BlockBP = nullptr;
    int32 ParentConnectionIndex = -1;

    ConnectedToBlock->BP_GetRandomPossibleBlockAndIndex(BlockBP, ParentConnectionIndex);
    if (!BlockBP || ParentConnectionIndex < 0)
        return nullptr;

    float PointRelativeYaw = ConnectedToBlock->ConnectionPoints[ParentConnectionIndex].RelativeYaw;
    float NewBlockFacingYaw = FindOppositeYaw(PointRelativeYaw + ConnectedToBlock->GetActorRotation().Yaw);

    FRotator NewBlockRotator(0.f, NewBlockFacingYaw, 0.f);

    // World location of parent connection point
    const FVector ParentConnectionWorldPos = ConnectedToBlock->GetActorLocation() +
        ConnectedToBlock->GetActorRotation().RotateVector(
            ConnectedToBlock->ConnectionPoints[ParentConnectionIndex].RelativeLocation
        );

    // New block connection offset (using index 0 on new block)
    const int32 NewBlockConnectionIndex = 0;
    const FVector NewBlockRelativePos = BlockBP->GetDefaultObject<ADungeonBlock_Base>()
        ->ConnectionPoints[NewBlockConnectionIndex].RelativeLocation;

    // Rotate by new block rotation
    const FVector NewBlockConnectionWorldOffset = NewBlockRotator.RotateVector(NewBlockRelativePos);

    // Final spawn location
    FVector NewBlockLocation = ParentConnectionWorldPos - NewBlockConnectionWorldOffset;

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ADungeonBlock_Base* NewBlock = GetWorld()->SpawnActor<ADungeonBlock_Base>(BlockBP, NewBlockLocation, NewBlockRotator, Params  );

    if (!NewBlock)
        return nullptr;


    ConnectedToBlock->SetConnectionPoint(
        ParentConnectionIndex, true, NewBlock
    );

    NewBlock->SetConnectionPoint(
        0, true, ConnectedToBlock
    );

    SpawnedBlocks.Add(NewBlock);
    return NewBlock;
}