#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryDropItem, FItemInfo, itemInfo);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APE_INVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// ----- For Client ----- //

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void TakeItemFromInventory(UInventoryComponent* takeFromInventory, const int32 itemIndex);
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void PutItemToInventory(UInventoryComponent* toInventory, const int32 itemIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_MoveItemBetweenInventory(UInventoryComponent* targetInventory, int32 itemIndex, bool isTaking);


	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void TakeAllFrom(UInventoryComponent* takeFromInventory);
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void TransferAllTo(UInventoryComponent* transferToInventory);
	UFUNCTION(Server, Reliable)
	void SERVER_TransferItems(UInventoryComponent* targetInventory, bool isTaking);


	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void SwapItemByIndex(const int32 a, const int32 b);
	UFUNCTION(Server, Reliable)
	void SERVER_SwapItemByIndex(const int32 a, const int32 b);


	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void SortItems();
	UFUNCTION(Server, Reliable)
	void SERVER_SortItems();


	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void DropItemAtIndex(const int32 index);
	UFUNCTION(Server, Reliable)
	void SERVER_DropItemAtIndex(const int32 index);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory);
	UFUNCTION(Server, Reliable)
	void SERVER_SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory);

	//  ----- SERVER ONLY ----- //

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool AddItem(UItem* item);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool RemoveItem(UItem* item);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool MergeItemByIndex(const int32 from, const int32 to);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool SplitItemInInventory(UItem* item, int32 splitAmount);


	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool Contains(UItem* item, int32& index);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool ContainsItem(FItemInfo itemInfo, int32& index);
		

private:
	// Internal functions
	UItem* FindItemID(FName name, int32& index);

	void UpdateItemInfos();

	bool IsInventoryFull() { return Items.Num() == MaxSize; }

	// OnRep notify
	UFUNCTION()
	void OnRep_InventoryUpdate();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Inventory", ReplicatedUsing = OnRep_InventoryUpdate)
	int32 MaxSize = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ape_Inventory", ReplicatedUsing = OnRep_InventoryUpdate)
	TArray<FItemInfo> ItemInfos;

	UPROPERTY()
	TArray<UItem*> Items;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnInventoryDropItem OnInventoryDropItem;	
};
