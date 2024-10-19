#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class APE_INVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// for client to call RPC on Server

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void AddItemFromInventory(UInventoryComponent* fromInventory, int32 itemIndex);

	UFUNCTION(Server, Reliable, Category = "Server")
	void SERVER_AddItemFromInventory(UInventoryComponent* fromInventory, int32 itemIndex);


public:
	// SERVER ONLY //
	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool AddItem(UItem* item);


	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool RemoveItem(UItem* item);
	//UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	//bool RemoveItemAtIndex(int32 item);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool SwapItemByIndex(const int32 a, const int32 b);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool MergeItemByIndex(const int32 from, const int32 to);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool SplitItemInInventory(UItem* item, int32 splitAmount);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	void SortItems();

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	void TransferItems(UInventoryComponent* toInventory);
	UFUNCTION(Server, Reliable, Category = "Ape_Inventory_Server")
	void SERVER_TransferItems(UInventoryComponent* toInventory);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool Contains(UItem* item, int32& index);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory_Server")
	bool ContainsItem(FItemInfo itemInfo, int32& index);

	UFUNCTION(BlueprintCallable, Category = "Ape_Inventory")
	TArray<FItemInfo> GetItemInfos();
	

private:
	//Internal functions
	UItem* FindItemID(FName name, int32& index);

	void UpdateItemInfos();

	bool IsInventoryFull() { return Items.Num() == MaxSize; }

	UFUNCTION()
	void OnRep_InventoryUpdate();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ape_Inventory", ReplicatedUsing = OnRep_InventoryUpdate)
	int32 MaxSize = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ape_Inventory", ReplicatedUsing = OnRep_InventoryUpdate)
	TArray<FItemInfo> ItemInfos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ape_Inventory")
	TArray<UItem*> Items;

	UPROPERTY(BlueprintAssignable, Category = "Ape_Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

		
};
