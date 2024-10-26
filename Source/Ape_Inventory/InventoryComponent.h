#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropInventoryItem, FItemInfo, itemInfo);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APE_INVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	//  ----- SERVER ONLY ----- //

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void Reinitialize();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void Deinitialize();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	bool AddItem(UItemSlot* item);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	bool RemoveItem(UItemSlot* item);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Server")
	void ClearInventory();

	// ----- For Client ----- //

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void TakeItemFromInventory(UInventoryComponent* takeFromInventory, const int32 itemIndex);
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void PutItemToInventory(UInventoryComponent* toInventory, const int32 itemIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_MoveItemBetweenInventory(UInventoryComponent* targetInventory, int32 itemIndex, bool isTaking);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void TakeAllFrom(UInventoryComponent* takeFromInventory);
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void TransferAllTo(UInventoryComponent* transferToInventory);
	UFUNCTION(Server, Reliable)
	void SERVER_TransferItems(UInventoryComponent* targetInventory, bool isTaking);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SwapItemByIndex(UInventoryComponent* fromInventory, UInventoryComponent* toInventory, const int32 fromA, const int32 toB);
	UFUNCTION(Server, Reliable)
	void SERVER_SwapItemByIndex(UInventoryComponent* fromInventory, UInventoryComponent* toInventory, const int32 fromA, const int32 toB);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SortItems();
	UFUNCTION(Server, Reliable)
	void SERVER_SortItems();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void DropItemAtIndex(const int32 index, bool fromEquipment = false);
	UFUNCTION(Server, Reliable)
	void SERVER_DropItemAtIndex(const int32 index, bool fromEquipment);
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void DropAllItems();
	UFUNCTION(Server, Reliable)
	void SERVER_DropAllItems();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory);
	UFUNCTION(Server, Reliable)
	void SERVER_SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory);

	// Equipment
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void UnequipItem(const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_UnequipItem(const int32 equipmentIndex);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory|Client")
	void SwapEquipmentWithInventory(UInventoryComponent* targetInventory, const int32 inventoryIndex, const int32 equipmentIndex);
	UFUNCTION(Server, Reliable)
	void SERVER_SwapEquipmentWithInventory(UInventoryComponent* targetInventory, const int32 inventoryIndex, const int32 equipmentIndex);

private:
	// Internal functions
	void UpdateItemInfos();

	void UpdateEquipmentInfos();

	// OnRep notify
	UFUNCTION()
	void OnRep_InventoryUpdate();

	UFUNCTION()
	void OnRep_EquipmentUpdate();

public:
	// Definition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Inventory|Server")
	int32 InventorySize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Inventory|Server", Replicated)
	TArray<FName> EquipmentDefinitions;

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
	FOnDropInventoryItem OnDropInventoryItem;

private:
	bool bInistialized = false;
};