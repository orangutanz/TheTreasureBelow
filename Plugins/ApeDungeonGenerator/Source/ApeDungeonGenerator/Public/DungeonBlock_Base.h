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
    UFUNCTION(BlueprintCallable, Category = "DungeonGenerator|Server")
    void SetConnectionPoint(int32 Index, bool bDoor, TSoftObjectPtr<ADungeonBlock_Base> Adjacent = nullptr);

    /** Blueprint hook for visual update */
    UFUNCTION(BlueprintImplementableEvent, Category = "DungeonGenerator|Visual")
    void BP_UpdateConnectionMesh(int32 Index, bool bDoor);

    UFUNCTION(BlueprintCallable, Category = "DungeonGenerator|Server")
    bool GetIsAvailableToSpawnConnections() const;

    /** Returns indices of available connection points (not doors, no adjacent block) */
    UFUNCTION(BlueprintCallable, Category = "DungeonGenerator|Server")
    TArray<int32> GetAvailableConnectionPoints() const;

    /** Blueprint function: return a TSubclassOf block and a connection point index to spawn, or nullptr/-1 */
    UFUNCTION(BlueprintImplementableEvent, Category = "DungeonGenerator|Server")
    void BP_GetRandomPossibleBlockAndIndex(TSubclassOf<ADungeonBlock_Base>& OutBlockBP, FTransform& OutTransform, int& OutConnectionIndex);

    UFUNCTION()
    bool HasAdjacentBlocks() { return AdjacentBlocks.Num() > 1; }

public:
    /** Replicated connection point data */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_ConnectionPoints, Category = "DungeonGenerator")
    TArray<FBlockConnectionPoint> ConnectionPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* CollisionQuery;

    UPROPERTY()
    TMap<int32, TSoftObjectPtr<ADungeonBlock_Base>> AdjacentBlocks;

    UPROPERTY()
    bool IsMainBranch = false;

    UPROPERTY()
    int32 BlockBranchDepth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator|Server")
    FVector CollisionBoxLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator|Server")
    FVector CollisionBoxExtent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator|Server")
    float CollisionBoxRotYaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DungeonGenerator|Server")
    int32 BlockValue = 1;

};