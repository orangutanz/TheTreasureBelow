// Copyright by Yuhan Ma. All Rights Reserved.


#include "DungeonBlock_Base.h"
#include "Net/UnrealNetwork.h"

ADungeonBlock_Base::ADungeonBlock_Base()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

}

void ADungeonBlock_Base::BeginPlay()
{
	Super::BeginPlay();
}


void ADungeonBlock_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADungeonBlock_Base, ConnectionPoints);
}

void ADungeonBlock_Base::OnRep_ConnectionPoints()
{
    if (HasAuthority()) return; // server already updated visuals

    for (int32 i = 0; i < ConnectionPoints.Num(); i++)
    {
        BP_UpdateConnectionMesh(i, ConnectionPoints[i].bIsDoor);
    }
}

void ADungeonBlock_Base::SetConnectionPoint(int32 Index, bool bDoor, TSoftObjectPtr<ADungeonBlock_Base> Adjacent)
{
    if (!HasAuthority()) return;

    if (ConnectionPoints.IsValidIndex(Index))
    {
        ConnectionPoints[Index].bIsDoor = bDoor;

        if (Adjacent.IsValid())
        {
            AdjacentBlocks.Add(Index, Adjacent);
        }

        BP_UpdateConnectionMesh(Index, bDoor);
    }
}

TArray<int32> ADungeonBlock_Base::GetAvailableConnectionPoints() const
{
    TArray<int32> Available;

    for (int32 i = 0; i < ConnectionPoints.Num(); i++)
    {
        if (!ConnectionPoints[i].bIsDoor && (!AdjacentBlocks.Contains(i) || !AdjacentBlocks[i].IsValid()))
        {
            Available.Add(i);
        }
    }

    return Available;
}

int ADungeonBlock_Base::GetRandomAvailablePointIndex() const
{
    TArray<int32> Available = GetAvailableConnectionPoints();
    if (Available.Num() == 0)
    {
        return -1;
    }
    return Available[FMath::RandRange(0, Available.Num() - 1)];
}
