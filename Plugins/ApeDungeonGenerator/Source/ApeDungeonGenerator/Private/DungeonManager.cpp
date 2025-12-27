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
    if (compareValue > 268)
    {
        return 90;
    }
    if (compareValue > 178)
    {
        return 0;
    }
    if (compareValue < -268)
    {
        return -90;
    }
    if (compareValue < -178)
    {
        return 0;
    }
    if (compareValue < -88)
    {
        return 90;
    }
    if (compareValue >  88)
    {
        return -90;
    }

    return 0;
}

void ADungeonManager::GenerationDungeon(FVector InitialLocation, FRotator InitialRotation, int32 MainBlockValue, int32 SubBlockValue, int32 MaxSubBranchDepth)
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
    FirstBlock->SetBranch(true, 0);
	SpawnedBlocks.Add(FirstBlock);

	int32 UsedBlockValues = FirstBlock->BlockValue;
    ADungeonBlock_Base* LatestMainBlock = FirstBlock;
	// Main branch
	while (UsedBlockValues < MainBlockValue)
	{
        // Spawn and connect a new block
        ADungeonBlock_Base* NewBlock = SpawnDungeonBlock(LatestMainBlock);
        if (!NewBlock)
        {
            break;
        }
        // Set MainBranch
        NewBlock->SetBranch(true, LatestMainBlock->BranchDepth + 1);
        LatestMainBlock = NewBlock;
        UsedBlockValues += NewBlock->BlockValue;
	}

    // Collect blocks with free connections
    TArray<ADungeonBlock_Base*> BlocksWithFreeConnections;

    //Blocks for sub branch
    UsedBlockValues = 0;
    while (UsedBlockValues < SubBlockValue)
    {
        BlocksWithFreeConnections.Empty();
        for (ADungeonBlock_Base* Block : SpawnedBlocks)
        {
            //Spawn conditions
            if (Block->IsMainBranch)
            {
                if (Block != LatestMainBlock && Block->GetIsAvailableToSpawnConnections())
                {
                    BlocksWithFreeConnections.Add(Block);
                }
            }
            else if(Block->BranchDepth <= MaxSubBranchDepth && Block->GetIsAvailableToSpawnConnections())
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

        // Set SubBranch
        if (ParentBlock->IsMainBranch)
        {
            NewBlock->SetBranch(false, 1);
        }
        else
        {
            NewBlock->SetBranch(false, ParentBlock->BranchDepth + 1);
        }
        UsedBlockValues += NewBlock->BlockValue;
    }
}


ADungeonBlock_Base* ADungeonManager::SpawnDungeonBlock(ADungeonBlock_Base* ConnectedToBlock)
{
    if (!HasAuthority() || !ConnectedToBlock)
        return nullptr;

    TSubclassOf<ADungeonBlock_Base> BlockBP = nullptr;
    int32 ParentConnectionIndex = -1;
    FTransform SpawnTransform;

    ConnectedToBlock->BP_GetRandomPossibleBlockAndIndex(BlockBP, SpawnTransform, ParentConnectionIndex);
    if (!BlockBP || ParentConnectionIndex < 0)
        return nullptr;

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ADungeonBlock_Base* NewBlock = GetWorld()->SpawnActor<ADungeonBlock_Base>(BlockBP, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), Params);

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