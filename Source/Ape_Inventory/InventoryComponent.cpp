#include "InventoryComponent.h"
#include "Item.h"
#include "Net/UnrealNetwork.h"
#include <cstdlib>

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add the Items array to the replicated properties
	DOREPLIFETIME(UInventoryComponent, ItemInfos);
	DOREPLIFETIME(UInventoryComponent, EquipmentInfos);
	DOREPLIFETIME(UInventoryComponent, EquipmentDefinitions);
}

void UInventoryComponent::Initialize()
{
	if (bInistialized)
		return;
	for (int i = 0; i < InventorySize;++i)
	{
		Inventory.Add(NewObject<UItemSlot>());
	}
	for (auto i : EquipmentDefinitions)
	{
		auto j = NewObject<UItemSlot>();
		j->SlotName = i;
		Equipments.Add(j);
	}
	bInistialized = true;
	UpdateItemInfos();
	UpdateEquipmentInfos();
}

void UInventoryComponent::Reinitialize()
{
	if (!bInistialized)
	{
		Initialize();
		return;
	}

	int32 oldSize = Inventory.Num();
	if (oldSize < InventorySize) // Expand
	{
		for (int32 i = oldSize; i < oldSize; ++i)
		{
			Inventory.Add(NewObject<UItemSlot>());
		}
	}
	else if (oldSize > InventorySize) // Shrink
	{
		TArray<UItemSlot*> TempArray;
		for (int32 i = oldSize; i > InventorySize; --i) // Store extra
		{
			TempArray.Add(Inventory[i]);
			Inventory.RemoveAt(i);
		}
		for (auto j : TempArray) // Add extra back
		{
			if (!j->IsEmpty())
			{
				AddItem(j);
			}
		}
		for (auto k : TempArray) // Drop item if failed to add
		{
			if (!k->IsEmpty())
			{
				OnDropInventoryItem.Broadcast(k->GetItemInfo());
			}
		}		
	}

	UpdateItemInfos();
	int32 newSize = EquipmentDefinitions.Num();
	oldSize = Equipments.Num();
	if (newSize == oldSize) // No Equipment changes
	{
		return;		
	}

	TArray<UItemSlot*>  newEquipmentSlots; // Equipment slots changed
	int found = -1;
	for (auto i : EquipmentDefinitions)
	{
		auto j = NewObject<UItemSlot>();
		j->SlotName = i;
		newEquipmentSlots.Add(j);
	}
	for (auto i : Equipments)
	{
		for (auto j : newEquipmentSlots)
		{
			if (i->GetSlotName() == j->GetSlotName())
			{
				i->SwapItemInfo(j);
				break;
			}
		}
		if (!i->IsEmpty())
		{
			if (!AddItem(i))
			{
				OnDropInventoryItem.Broadcast(i->GetItemInfo());
			}
		}
	}
	Equipments.Empty();
	Equipments = newEquipmentSlots;

	UpdateItemInfos();
	UpdateEquipmentInfos(); // Update inventory and equipement
}

bool UInventoryComponent::AddItem(UItemSlot* item)
{
	if (!item || Inventory.Contains(item))
	{
		return false;
	}
	for (auto i : Inventory)
	{
		if (i->MergeItem(item))
		{
			UpdateItemInfos();
			return true;
		}
	}
	UpdateItemInfos();
	return false;
}

bool UInventoryComponent::RemoveItem(UItemSlot* slot)
{
	if (Inventory.Contains(slot))
	{
		slot->ClearItemInfo();
		UpdateItemInfos();
		return true;
	}
	return false;
	if (slot)
	{
		Inventory.RemoveSingle(slot);

		UpdateItemInfos();
		return true;
	}
	return false;
}

void UInventoryComponent::TakeItemFromInventory(UInventoryComponent* takeFromInventory, int32 itemIndex)
{
	if (!takeFromInventory || takeFromInventory->ItemInfos.Num() <= itemIndex)
		return;
	SERVER_MoveItemBetweenInventory(takeFromInventory, itemIndex,true);
}

void UInventoryComponent::PutItemToInventory(UInventoryComponent* toInventory, int32 itemIndex)
{
	if (!toInventory)
		return;
	SERVER_MoveItemBetweenInventory(toInventory, itemIndex, false);
}

void UInventoryComponent::SERVER_MoveItemBetweenInventory_Implementation(UInventoryComponent* targetInventory, int32 itemIndex, bool isTaking)
{
	if (!targetInventory)
		return;

	if (isTaking)
	{
		if (targetInventory->Inventory.Num() <= itemIndex)
		{
			return;
		}
		UItemSlot* item = targetInventory->Inventory[itemIndex];
		if (AddItem(item))
		{
			targetInventory->Inventory[itemIndex]->ClearItemInfo();
		}
	}
	else
	{
		if (Inventory.Num() <= itemIndex)
		{
			return;
		}
		UItemSlot* item = Inventory[itemIndex];
		if (targetInventory->AddItem(item))
		{
			Inventory[itemIndex]->ClearItemInfo();
		}
	}
	targetInventory->UpdateItemInfos();
	UpdateItemInfos();
}


void UInventoryComponent::TakeAllFrom(UInventoryComponent* takeFromInventory)
{
	if (!takeFromInventory)
		return;
	SERVER_TransferItems(takeFromInventory, true); //to this
}


void UInventoryComponent::TransferAllTo(UInventoryComponent* transferToInventory)
{
	if (!transferToInventory)
		return;
	SERVER_TransferItems(transferToInventory, false); // to other
}

void UInventoryComponent::SERVER_TransferItems_Implementation(UInventoryComponent* targetInventory, bool isTaking)
{
	if (!targetInventory)
		return;

	TArray<UItemSlot*> tempTransferedItems; // Fully transfered items
	if (isTaking)
	{
		for (auto i : targetInventory->Inventory)
		{
			if (AddItem(i))
			{
				tempTransferedItems.Add(i);
			}
		}
		for (auto j : tempTransferedItems)
		{
			j->ClearItemInfo();
		}
		targetInventory->UpdateItemInfos();
	}
	else
	{
		for (auto i : Inventory)
		{
			if (targetInventory->AddItem(i))
			{
				tempTransferedItems.Add(i);
			}
		}
		for (auto j : tempTransferedItems)
		{
			j->ClearItemInfo();
		}
	}
	UpdateItemInfos();
}

void UInventoryComponent::SwapItemByIndex(UInventoryComponent* fromInventory, UInventoryComponent* toInventory, const int32 fromA, const int32 toB)
{
	if (!fromInventory || !toInventory || fromInventory->InventorySize <= fromA || toInventory->InventorySize <= toB)
	{
		return;
	}

	SERVER_SwapItemByIndex(fromInventory, toInventory, fromA, toB);
}

void UInventoryComponent::SERVER_SwapItemByIndex_Implementation(UInventoryComponent* fromInventory, UInventoryComponent* toInventory, const int32 fromA, const int32 toB)
{
	if (!fromInventory || !toInventory || fromInventory->InventorySize <= fromA || toInventory->InventorySize <= toB)
	{
		return;
	}
	if (fromInventory->Inventory[fromA]->GetItemID() == toInventory->Inventory[toB]->GetItemID()) // Is same item?
	{
		if (fromInventory->Inventory[fromA]->IsFull())
		{
			fromInventory->Inventory[fromA]->SwapItemInfo(toInventory->Inventory[toB]); // Do swap if full
		}
		else
		{
			if (!toInventory->Inventory[toB]->MergeItem(fromInventory->Inventory[fromA])) // Do merge
			{
				fromInventory->Inventory[fromA]->SwapItemInfo(toInventory->Inventory[toB]); // Swap if merge fail
			}
		}
	}
	else
	{
		fromInventory->Inventory[fromA]->SwapItemInfo(toInventory->Inventory[toB]); // Swap if different
	}
	fromInventory->UpdateItemInfos();
	toInventory->UpdateItemInfos();
}

void UInventoryComponent::SortItems()
{
	if (ItemInfos.IsEmpty())
		return;

	SERVER_SortItems();
}

void UInventoryComponent::SERVER_SortItems_Implementation()
{
	//ID sort
	Inventory.Sort([](const UItemSlot& a, const UItemSlot& b) { return a.GetItemID().FastLess(b.GetItemID()); });

	//Type sort
	//Items.Sort([](const UItemSlot& a, const UItemSlot& b) { return a.GetItemType() <= b.GetItemType(); });

	UpdateItemInfos();
	//OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::DropItemAtIndex(const int32 index, bool fromEquipment)
{
	if ((!fromEquipment && ItemInfos.Num() <= index) || (fromEquipment && EquipmentInfos.Num() <= index))
		return;
	SERVER_DropItemAtIndex(index, fromEquipment);
}

void UInventoryComponent::SERVER_DropItemAtIndex_Implementation(const int32 index, bool fromEquipment)
{
	if ((!fromEquipment && Inventory.Num() <= index) || (fromEquipment && Equipments.Num() <= index))
		return;
	FItemInfo RemovedItemInfo;
	if (fromEquipment)
	{
		RemovedItemInfo = Equipments[index]->GetItemInfo();
		Equipments[index]->ClearItemInfo();
		UpdateEquipmentInfos();
	}
	else
	{
		RemovedItemInfo	= Inventory[index]->GetItemInfo();
		Inventory[index]->ClearItemInfo();
		UpdateItemInfos();
	}
	OnDropInventoryItem.Broadcast(RemovedItemInfo);
}

void UInventoryComponent::DropAllItems()
{
	if (ItemInfos.IsEmpty())
		return;
	SERVER_DropAllItems();
}

void UInventoryComponent::SERVER_DropAllItems_Implementation()
{
	for (auto i : Inventory)
	{
		if (i->IsEmpty())
			continue;
		OnDropInventoryItem.Broadcast(i->GetItemInfo());
		i->ClearItemInfo();
	}
	UpdateItemInfos();
}

void UInventoryComponent::SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory)
{
	if (ItemInfos.Num() <= index || splitAmount <= 0)
		return;
	SERVER_SplitItem(index, splitAmount, isToInventory);
}

void UInventoryComponent::SERVER_SplitItem_Implementation(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory)
{
	if (Inventory.Num() <= index)
		return;
	UItemSlot* item = Inventory[index];
	if (item->GetQuantity() < 2 || item->GetMaxStack() == 1 || item->GetQuantity() <= splitAmount)
	{
		return ;
	}
	auto splitItem = item->SplitItem(splitAmount);
	if (!splitItem) // valid splitting?
		return ;
	if (isToInventory)
	{
		if (isToInventory->AddItem(splitItem)) // is fully added?
		{
			Inventory[index]->ClearItemInfo(); // remove once fully added
		}
		isToInventory->UpdateItemInfos();
	}
	else
	{
		OnDropInventoryItem.Broadcast(splitItem->GetItemInfo()); // if not into inventory, drop the split item
		Inventory[index]->ClearItemInfo(); // remove from inventory
	}
	UpdateItemInfos();
}

void UInventoryComponent::EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex)
{
	if (!fromInventory || fromInventory->ItemInfos.Num() <= inventoryIndex || equipmentIndex >= EquipmentInfos.Num())
		return;
	SERVER_EquipItem(fromInventory, inventoryIndex, equipmentIndex);
}

void UInventoryComponent::SERVER_EquipItem_Implementation(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex)
{
	if (!fromInventory || fromInventory->ItemInfos.Num() <= inventoryIndex || equipmentIndex >= Equipments.Num())
		return;
	fromInventory->Inventory[inventoryIndex]->SwapItemInfo(Equipments[equipmentIndex]);
	fromInventory->UpdateItemInfos();
	UpdateEquipmentInfos();
}

void UInventoryComponent::UnequipItem(const int32 equipmentIndex)
{
	if (equipmentIndex >= EquipmentDefinitions.Num())
		return;
	SERVER_UnequipItem(equipmentIndex);
}

void UInventoryComponent::SERVER_UnequipItem_Implementation(const int32 equipmentIndex)
{
	if (equipmentIndex >= Equipments.Num())
		return;
	bool found = false;
	for (auto i : Inventory)
	{
		if (i->IsEmpty())
		{
			i->SwapItemInfo(Equipments[equipmentIndex]);
			found = true;
			break;
		}
	}
	if (!found)
		return;
	UpdateItemInfos();
	UpdateEquipmentInfos();
}

void UInventoryComponent::SwapEquipmentWithInventory(UInventoryComponent* targetInventory, const int32 inventoryIndex, const int32 equipmentIndex)
{
	if (inventoryIndex >= ItemInfos.Num() || equipmentIndex >= EquipmentInfos.Num())
	{
		return;
	}
	SERVER_SwapEquipmentWithInventory(targetInventory, inventoryIndex, equipmentIndex);
}

void UInventoryComponent::SERVER_SwapEquipmentWithInventory_Implementation(UInventoryComponent* targetInventory, const int32 inventoryIndex, const int32 equipmentIndex)
{
	if (inventoryIndex >= Inventory.Num() || equipmentIndex >= Equipments.Num())
	{
		return;
	}
	targetInventory->Inventory[inventoryIndex]->SwapItemInfo(Equipments[equipmentIndex]);
	targetInventory->UpdateItemInfos();
	UpdateEquipmentInfos();
}

void UInventoryComponent::UpdateItemInfos()
{
	ItemInfos.Empty();
	for (auto i : Inventory)
	{
		ItemInfos.Add(i->GetItemInfo());
	}
	OnRep_InventoryUpdate();
}

void UInventoryComponent::UpdateEquipmentInfos()
{
	EquipmentInfos.Empty();
	for (auto i : Equipments)
	{
		EquipmentInfos.Add(i->GetItemInfo());
	}
	OnRep_EquipmentUpdate();
}

void UInventoryComponent::OnRep_InventoryUpdate()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::OnRep_EquipmentUpdate()
{
	OnEquipmentUpdated.Broadcast();
}

