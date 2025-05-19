#include "InventoryComponent.h"
#include "Item.h"
#include "Net/UnrealNetwork.h"
#include <cstdlib>

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add the Items array to the replicated properties
	DOREPLIFETIME(UInventoryComponent, InventoryInfos);
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
	UpdateInventoryInfos();
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

	UpdateInventoryInfos();
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

	UpdateInventoryInfos();
	UpdateEquipmentInfos(); // Update inventory and equipement
}

void UInventoryComponent::Deinitialize()
{
	OnInventoryUpdated.Clear();
	OnEquipmentUpdated.Clear();
	OnDropInventoryItem.Clear();
	OnUseInventoryItem.Clear();
	Inventory.Empty();
	Equipments.Empty();
	bInistialized = false;
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
			UpdateInventoryInfos();
			return true;
		}
	}
	UpdateInventoryInfos();
	return false;
}

bool UInventoryComponent::RemoveItemByName(FName ItemID, int32 amount)
{
	TArray<UItemSlot*> tempArray;
	int32 tempTotalAmount = 0;
	for (auto i : Inventory)
	{
		if (i->GetItemID() == ItemID)
		{
			if (i->GetQuantity() == amount)
			{
				i->ClearItemInfo();
				UpdateInventoryInfos();
				return true;
			}
			else if (i->GetQuantity() > amount)
			{
				i->SetQuantity(i->GetQuantity() - amount);
				UpdateInventoryInfos();
				return true;
			}
			else
			{
				tempArray.Add(i);
				if (tempTotalAmount >= amount)
				{
					for (auto j : tempArray)
					{
						if (j->GetQuantity() < tempTotalAmount)
						{
							tempTotalAmount -= j->GetQuantity();
							j->ClearItemInfo();
						}
						else if (i->GetQuantity() > tempTotalAmount)
						{
							j->SetQuantity(j->GetQuantity() - tempTotalAmount);
							UpdateInventoryInfos();
							return true;
						}
						else
						{
							j->ClearItemInfo();
							UpdateInventoryInfos();
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

bool UInventoryComponent::RemoveItemByIndex(int32 index, int32 Amount)
{
	if (Inventory.Num() <= index)
	{
		return false;
	}

	int itemAmount = Inventory[index]->GetQuantity();
	if(itemAmount <= 0)
		return false;
	itemAmount -= Amount;

	if (itemAmount > 0)
	{
		Inventory[index]->SetQuantity(itemAmount);
		UpdateInventoryInfos();
		return true;
	}
	else if (itemAmount == 0)
	{
		Inventory[index]->ClearItemInfo();
		UpdateInventoryInfos();
		return true;
	}

	return false;
}

void UInventoryComponent::ClearInventory()
{
	for (auto i : Inventory)
	{
		i->ClearItemInfo();
	}
	UpdateInventoryInfos();
}

int UInventoryComponent::FindItemQuantity(FName ItemID)
{
	int totalAmount = 0;
	for (auto i : InventoryInfos)
	{
		if(i.ItemID == ItemID)
		{
			totalAmount += i.Quantity;
		}
	}
	return totalAmount;
}

void UInventoryComponent::TakeItemFromInventory(UInventoryComponent* takeFromInventory, int32 itemIndex)
{
	if (!takeFromInventory || takeFromInventory->InventoryInfos.Num() <= itemIndex)
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
	targetInventory->UpdateInventoryInfos();
	UpdateInventoryInfos();
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
		targetInventory->UpdateInventoryInfos();
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
	UpdateInventoryInfos();
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
	fromInventory->UpdateInventoryInfos();
	toInventory->UpdateInventoryInfos();
}

void UInventoryComponent::SortItems()
{
	if (InventoryInfos.IsEmpty())
		return;

	SERVER_SortItems();
}

void UInventoryComponent::SERVER_SortItems_Implementation()
{
	//ID sort
	Inventory.Sort([](const UItemSlot& a, const UItemSlot& b) { return b.GetItemID().FastLess(a.GetItemID()); });
	

	//Type sort
	//Items.Sort([](const UItemSlot& a, const UItemSlot& b) { return a.GetItemType() <= b.GetItemType(); });

	UpdateInventoryInfos();
	//OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::DropItemAtIndex(const int32 index, bool fromEquipment)
{
	if ((!fromEquipment && InventoryInfos.Num() <= index) || (fromEquipment && EquipmentInfos.Num() <= index))
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
		UpdateInventoryInfos();
	}
	OnDropInventoryItem.Broadcast(RemovedItemInfo);
}

void UInventoryComponent::DropAllItems()
{
	if (InventoryInfos.IsEmpty())
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
	UpdateInventoryInfos();
}

void UInventoryComponent::SplitItem(const int32 fromIndex, const int32 toIndex, const int32 splitAmount, UInventoryComponent* fromInventroy, UInventoryComponent* toInventory)
{
	if (InventoryInfos.Num() <= fromIndex || splitAmount <= 0)
		return;
	SERVER_SplitItem(fromIndex, toIndex, splitAmount, fromInventroy, toInventory);
}

void UInventoryComponent::SERVER_SplitItem_Implementation(const int32 fromIndex, const int32 toIndex, const int32 splitAmount, UInventoryComponent* fromInventroy, UInventoryComponent* toInventory)
{
	if (!fromInventroy || splitAmount == 0) // Check valid
	{
		return;
	}
	if (fromInventroy->Inventory.Num() <= fromIndex) // Check index
	{
		return;
	}
	else if (!fromInventroy->Inventory[fromIndex]->CanSplit(splitAmount)) // Check can split from?
	{
		return;
	}
	if (toInventory) // Split to inventory
	{
		if (toInventory->Inventory.Num() <= toIndex)
		{
			return; // Not valid index
		}
		else if (!toInventory->Inventory[toIndex]->IsEmpty())
		{
			return; // Target slot is not empty
		}
		UItemSlot* fromSlot = fromInventroy->Inventory[fromIndex]; // split target
		UItemSlot* toSlot = toInventory->Inventory[toIndex]; // empty slot
		FItemInfo splitInfo = fromSlot->SplitQuantity(splitAmount);
		toSlot->SetItemInfo(splitInfo);
	}
	else // Split to world
	{
		UItemSlot* fromSlot = fromInventroy->Inventory[fromIndex]; // split target
		FItemInfo splitInfo = fromSlot->SplitQuantity(splitAmount);
		fromInventroy->OnDropInventoryItem.Broadcast(splitInfo);
	}

	fromInventroy->UpdateInventoryInfos();
	if (toInventory)
	{
		toInventory->UpdateInventoryInfos();
	}
}


void UInventoryComponent::UseInventoryItem(UInventoryComponent* fromInventory, const int32 inventoryIndex)
{
	if (!fromInventory || fromInventory->InventoryInfos.Num() <= inventoryIndex)
		return;
	SERVER_UseInventoryItem(fromInventory, inventoryIndex);
}

void UInventoryComponent::UseInventoryItemByName(UInventoryComponent* fromInventory, FName itemName)
{
	if (!fromInventory)
		return;

	for (int i = 0; i < InventoryInfos.Num(); ++i)
	{
		if (fromInventory->InventoryInfos[i].ItemID == itemName)
		{
			SERVER_UseInventoryItem(fromInventory, i);
			return;
		}
	}
}


void UInventoryComponent::SERVER_UseInventoryItem_Implementation(UInventoryComponent* fromInventory, const int32 inventoryIndex)
{
	if (!fromInventory || fromInventory->InventoryInfos.Num() <= inventoryIndex)
		return;
	OnUseInventoryItem.Broadcast(fromInventory->InventoryInfos[inventoryIndex], inventoryIndex);
}


void UInventoryComponent::EquipItem(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex)
{
	if (!fromInventory || fromInventory->InventoryInfos.Num() <= inventoryIndex || equipmentIndex >= EquipmentInfos.Num())
		return;
	SERVER_EquipItem(fromInventory, inventoryIndex, equipmentIndex);
}

void UInventoryComponent::SERVER_EquipItem_Implementation(UInventoryComponent* fromInventory, const int32 inventoryIndex, const int32 equipmentIndex)
{
	if (!fromInventory || fromInventory->InventoryInfos.Num() <= inventoryIndex || equipmentIndex >= Equipments.Num())
		return;
	fromInventory->Inventory[inventoryIndex]->SwapItemInfo(Equipments[equipmentIndex]);
	fromInventory->UpdateInventoryInfos();
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
	UpdateInventoryInfos();
	UpdateEquipmentInfos();
}

void UInventoryComponent::SwapEquipmentWithInventory(UInventoryComponent* targetInventory, const int32 inventoryIndex, const int32 equipmentIndex)
{
	if (inventoryIndex >= InventoryInfos.Num() || equipmentIndex >= EquipmentInfos.Num())
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
	targetInventory->UpdateInventoryInfos();
	UpdateEquipmentInfos();
}


void UInventoryComponent::SwapEquipmentPosition(const int32 fromIndex, const int32 toIndex)
{
	if (fromIndex >= EquipmentInfos.Num() || toIndex >= EquipmentInfos.Num() || fromIndex == toIndex)
	{
		return;
	}
	SERVER_SwapEquipmentPosition(fromIndex, toIndex);
}

void UInventoryComponent::SERVER_SwapEquipmentPosition_Implementation(const int32 fromIndex, const int32 toIndex)
{
	if (fromIndex >= Equipments.Num() || toIndex >= Equipments.Num())
	{
		return;
	}
	Equipments[fromIndex]->SwapItemInfo(Equipments[toIndex]);
	UpdateEquipmentInfos();
}

void UInventoryComponent::CallEquipmentUpdate()
{
	OnEquipmentUpdated.Broadcast();
}

void UInventoryComponent::CallInventoryUpdate()
{
	OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::IsInventoryEmpty()
{
	for (auto i : InventoryInfos)
	{
		if (!i.ItemID.IsNone())
		{
			return false;
		}
	}	
	return true;
}


void UInventoryComponent::UpdateInventoryInfos()
{
	InventoryInfos.Empty();
	for (auto i : Inventory)
	{
		InventoryInfos.Add(i->GetItemInfo());
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

