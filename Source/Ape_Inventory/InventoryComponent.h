#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryDropItem, FItemInfo, itemInfo);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APE_INVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
public:
	// ----- For Client ----- //
	// Inventory
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
	void DropAllItems();
	UFUNCTION(Server, Reliable)
	void SERVER_DropAllItems();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory);
	UFUNCTION(Server, Reliable)
	void SERVER_SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory);

	// Equipment
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void UnequipItem(const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_UnequipItem(const int32 equipmentIndex);


	//  ----- SERVER ONLY ----- //

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool AddItemInfo(FItemInfo& itemInfo);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool AddItem(UItemSlot* item);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool RemoveItem(UItemSlot* item);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool MergeItemByIndex(const int32 from, const int32 to);

private:
	// Internal functions
	UItemSlot* FindItemID(FName name, int32& index);

	void UpdateItemInfos();

	//bool IsInventoryFull() { return Items.Num() == MaxSize; }

	// OnRep notify
	UFUNCTION()
	void OnRep_InventoryUpdate();

	UFUNCTION()
	void OnRep_EquipmentUpdate();

public:
	// Definition
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 InventorySize = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 EquipmentSize = 1;

	// Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ape_Inventory", ReplicatedUsing = OnRep_InventoryUpdate)
	TArray<FItemInfo> ItemInfos;

	UPROPERTY()
	TArray<UItemSlot*> Inventory;

	// Equipment
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ape_Inventory", ReplicatedUsing = OnRep_EquipmentUpdate)
	TArray<FItemInfo> EquipmentInfos;

	UPROPERTY()
	TArray<UItemSlot*> Equipments;


	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnEquipmentUpdated OnEquipmentUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnInventoryDropItem OnInventoryDropItem;	
};
