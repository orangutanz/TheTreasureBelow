// Copyright by Yuhan Ma. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DungeonBlock_Base.generated.h"


USTRUCT(BlueprintType)
struct FBlockConnectionPoint
{
	GENERATED_BODY()

public:
	/** Local-space position where another block can attach */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator")
	FVector RelativeLocation = FVector::ZeroVector;

	/** Local-space direction for alignment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator")
	float RelativeYaw = 0;

	/** Connection state: false = wall, true = door */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator")
	bool bIsDoor = false;
};


UCLASS()
class APEDUNGEONGENERATOR_API ADungeonBlock_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	ADungeonBlock_Base();

protected:
	virtual void BeginPlay() override;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Called on clients when ConnectionPoints replicate */
    UFUNCTION()
    void OnRep_ConnectionPoints();

    /** Server function to set a connection point state */
    UFUNCTION(BlueprintCallable, Category = "DungeonGenerator")
    void SetConnectionPoint(int32 Index, bool bDoor, TSoftObjectPtr<ADungeonBlock_Base> Adjacent = nullptr);

    /** Blueprint hook for visual update */
    UFUNCTION(BlueprintImplementableEvent, Category = "DungeonGenerator")
    void BP_UpdateConnectionMesh(int32 Index, bool bDoor);

    UFUNCTION(BlueprintCallable, Category = "DungeonGenerator")
    bool GetIsAvailableToSpawnConnections() const;

    /** Returns indices of available connection points (not doors, no adjacent block) */
    UFUNCTION(BlueprintCallable, Category = "DungeonGenerator")
    TArray<int32> GetAvailableConnectionPoints() const;

    /** Blueprint function: return a TSubclassOf block and a connection point index to spawn, or nullptr/-1 */
    UFUNCTION(BlueprintImplementableEvent, Category = "DungeonGenerator")
    void BP_GetRandomPossibleBlockAndIndex(TSubclassOf<ADungeonBlock_Base>& OutBlockBP, FTransform& OutTransform, int& OutConnectionIndex);

    UFUNCTION()
    bool HasAdjacentBlocks() { return AdjacentBlocks.Num() > 1; }


    UFUNCTION()
    void SetBranch(bool isMainBranch, int32 branchDepth) { IsMainBranch = isMainBranch; BranchDepth = branchDepth; }
public:
    UPROPERTY()
    TMap<int32, TSoftObjectPtr<ADungeonBlock_Base>> AdjacentBlocks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DungeonGenerator")
    bool IsMainBranch = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DungeonGenerator")
    int32 BranchDepth = 0;

    /** Replicated connection point data */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_ConnectionPoints, Category = "DungeonGenerator")
    TArray<FBlockConnectionPoint> ConnectionPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* CollisionQuery;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DungeonGenerator")
    int32 MaxConnection = 999;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator")
    FVector CollisionBoxLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator")
    FVector CollisionBoxExtent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator")
    float CollisionBoxRotYaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator")
    int32 BlockValue = 1;

};