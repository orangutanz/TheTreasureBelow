// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemUpdated);

UENUM(BlueprintType)
enum EItemType
{
	Consumable       UMETA(DisplayName = "Consumable"),
	Equipment        UMETA(DisplayName = "Equipment"),
	Misc		     UMETA(DisplayName = "Misc")
};


USTRUCT(BlueprintType)
struct APE_INVENTORY_API FItemInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Item")
	FName ItemID = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Item", meta = (ClampMin = 0))
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Item", meta = (ClampMin = 0))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Item")
	TEnumAsByte<EItemType> ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Item")
	TArray<FName> ItemProperties;

	// Define the equality operator for FItemInfo
	bool operator==(const FItemInfo& Other) const
	{
		return ItemID == Other.ItemID
			&& MaxStack == Other.MaxStack
			&& Quantity == Other.Quantity
			&& ItemType == Other.ItemType
			&& ItemProperties == Other.ItemProperties;
	}
};


UCLASS(EditInlineNew, DefaultToInstanced)
class APE_INVENTORY_API UItemSlot : public UObject
{
	GENERATED_BODY()

public:
	// Getter
	UFUNCTION()
	bool IsEmpty() { return mItemInfo.ItemID.IsEqual(""); }

	UFUNCTION()
	FItemInfo GetItemInfo() { return mItemInfo; }

	UFUNCTION()
	FORCEINLINE FName GetItemID() const { return  mItemInfo.ItemID; }

	UFUNCTION()
	FORCEINLINE int32 GetMaxStack() const { return mItemInfo.MaxStack; }

	UFUNCTION()
	FORCEINLINE int32 GetQuantity() const { return mItemInfo.Quantity; }

	UFUNCTION()
	FORCEINLINE TEnumAsByte<EItemType> GetItemType() const { return mItemInfo.ItemType; }

	// Setter
	UFUNCTION()
	bool AddItemInfo(FItemInfo& itemInfo);

	UFUNCTION()
	void ClearItemInfo() { mItemInfo = FItemInfo(); }

	UFUNCTION()
	void SetItemInfo(FItemInfo info);

	UFUNCTION()
	bool SetQuantity(int32 num);

	/** Return null if (num > Quantity) || (MaxStack == 1) || (num == 0)	*/
	UFUNCTION()
	UItemSlot* SplitItem(int32 num);

	/** Return true if fully merged, false for partial or failed */
	UFUNCTION()
	bool MergeItem(UItemSlot* other);

	UFUNCTION()
	void SwapItemInfo(UItemSlot* other);

private:
	FItemInfo mItemInfo;
};
