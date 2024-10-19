#include "InventoryComponent.h"
#include "Item.h"
#include "Net/UnrealNetwork.h"
#include <cstdlib>

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add the Items array to the replicated properties
	DOREPLIFETIME(UInventoryComponent, ItemInfos);
	DOREPLIFETIME(UInventoryComponent, MaxSize);
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
		if (targetInventory->Items.Num() <= itemIndex)
		{
			return;
		}
		UItem* item = targetInventory->Items[itemIndex];
		if (AddItem(item))
		{
			targetInventory->Items.RemoveAt(itemIndex);
		}
	}
	else
	{
		if (Items.Num() <= itemIndex)
		{
			return;
		}
		UItem* item = Items[itemIndex];
		if (targetInventory->AddItem(item))
		{
			Items.RemoveAt(itemIndex);
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

	TArray<UItem*> tempTransferedItems; // Fully transfered items
	if (isTaking)
	{
		for (auto i : targetInventory->Items)
		{
			if (AddItem(i))
			{
				tempTransferedItems.Add(i);
			}
		}
		for (auto j : tempTransferedItems)
		{
			targetInventory->Items.RemoveSingleSwap(j);
		}
		targetInventory->UpdateItemInfos();
	}
	else
	{
		for (auto i : Items)
		{
			if (targetInventory->AddItem(i))
			{
				tempTransferedItems.Add(i);
			}
		}
		for (auto j : tempTransferedItems)
		{
			Items.RemoveSingleSwap(j);
		}
	}
	UpdateItemInfos();
}

void UInventoryComponent::SwapItemByIndex(const int32 a, const int32 b)
{
	if ((a >= 0 && a < ItemInfos.Num()) && (b >= 0 && b < ItemInfos.Num()) && (a != b))
	{
		SERVER_SwapItemByIndex(a, b);
	}
}

void UInventoryComponent::SERVER_SwapItemByIndex_Implementation(const int32 a, const int32 b)
{
	if ((a >= 0 && a < Items.Num()) && (b >= 0 && b < Items.Num()) && (a != b))
	{
		Items.Swap(a, b);
		UpdateItemInfos();
	}
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
	Items.Sort([](const UItem& a, const UItem& b) { return a.GetItemID().FastLess(b.GetItemID()); });

	//Type sort
	//Items.Sort([](const UItem& a, const UItem& b) { return a.GetItemType() <= b.GetItemType(); });

	UpdateItemInfos();
	//OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::DropItemAtIndex(const int32 index)
{
	if (ItemInfos.Num() <= index)
		return;
	SERVER_DropItemAtIndex(index);
}

void UInventoryComponent::SERVER_DropItemAtIndex_Implementation(const int32 index)
{
	if (ItemInfos.Num() <= index) // do I have to check this part?
		return;
	FItemInfo RemovedItemInfo = Items[index]->GetItemInfo();
	Items.RemoveAt(index);
	UpdateItemInfos();
	OnInventoryDropItem.Broadcast(RemovedItemInfo);
}

void UInventoryComponent::SplitItem(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory)
{
	if (ItemInfos.Num() <= index || splitAmount <= 0)
		return;
	SERVER_SplitItem(index, splitAmount, isToInventory);
}

void UInventoryComponent::SERVER_SplitItem_Implementation(const int32 index, int32 splitAmount, UInventoryComponent* isToInventory)
{
	if (Items.Num() <= index)
		return;
	UItem* item = Items[index];
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
			Items.RemoveAt(index); // remove once fully added
		}
	}
	else
	{
		OnInventoryDropItem.Broadcast(splitItem->GetItemInfo()); // if not into inventory, drop the split item
		Items.RemoveAt(index); // remove from inventory
	}
	UpdateItemInfos();
	return ;
}

bool UInventoryComponent::AddItem(UItem* item)
{
	if (!item)
	{
		return false;
	}
	// Item can't stack
	if (item->GetMaxStack() == 1)
	{
		if (Items.Num() >= MaxSize)
		{
			return false;
		}
		else 
		{
			Items.Add(item);
			UpdateItemInfos();
			//OnInventoryUpdated.Broadcast(); //Add new slot
			return true;
		}
	}
	int32 foundIndex = 0;
	
	UItem* itemFound = FindItemID(item->GetItemID(), foundIndex);

	// No item for stacking, try add to new slot
	if (!itemFound)
	{
		if (Items.Num() >= MaxSize)
		{
			return false; // No space
		}
		else
		{
			Items.Add(item);//Add new

			UpdateItemInfos();
			return true; // Fully added
		}
	}
	else // Found item for stacking
	{
		while (itemFound && (item->GetQuantity() > 0))
		{
			if (itemFound->MergeItem(item))
			{
				UpdateItemInfos();
				return true;
			}
			itemFound = FindItemID(item->GetItemID(), ++foundIndex);
		}
	}
	if (Items.Num() < MaxSize)
	{
		Items.Add(item);//Add new
		UpdateItemInfos();
		return true;
	}
	return false; // Only partially added
}

bool UInventoryComponent::RemoveItem(UItem* item)
{
	if (item)
	{
		Items.RemoveSingle(item);

		UpdateItemInfos();
		//OnInventoryUpdated.Broadcast();
		return true;
	}
	return false;
}


bool UInventoryComponent::MergeItemByIndex(const int32 from, const int32 to)
{
	if (from > Items.Num() || to > Items.Num() || (from == to))
	{
		return false;
	}
	auto fromItem = Items[from]->GetItemInfo();
	auto toItem = Items[to]->GetItemInfo();
	if (fromItem.MaxStack <= 1 || toItem.MaxStack <= 1|| (fromItem.ItemID != toItem.ItemID))
	{
		return false;
	}
	int32 combinedQuantity = fromItem.Quantity + toItem.Quantity;
	if (combinedQuantity > toItem.MaxStack) // partially merge
	{
		toItem.Quantity = toItem.MaxStack;
		fromItem.Quantity = combinedQuantity - toItem.MaxStack;
		Items[from]->SetItemInfo(fromItem);
		Items[to]->SetItemInfo(toItem);
	}
	else // fully merge
	{
		toItem.Quantity = combinedQuantity;
		Items[to]->SetItemInfo(toItem);
		Items.RemoveAt(from);
	}

	UpdateItemInfos();
	//OnInventoryUpdated.Broadcast();
	return true;
}

bool UInventoryComponent::SplitItemInInventory(UItem* item, int32 splitAmount)
{
	if (IsInventoryFull() || item->GetQuantity()<2 || item->GetMaxStack() == 1 || !Items.Contains(item) || item->GetQuantity() <= splitAmount)
	{
		return false;
	}	
	auto newItem = item->SplitItem(splitAmount);
	if (!newItem)
		return false;
	Items.Add(newItem);

	UpdateItemInfos();
	//OnInventoryUpdated.Broadcast();
	return true;
}

UItem* UInventoryComponent::FindItemID(FName name, int32& index)
{
	if (name.IsNone() && (index < 0 || index >= MaxSize))
	{
		return nullptr;
	}

	for (index; index < Items.Num(); index++)
	{
		if (name == Items[index]->GetItemID())
		{
			return  Items[index];
		}
	}
	return nullptr;
}

bool UInventoryComponent::Contains(UItem* item, int32& index)
{
	index = Items.Find(item);
	return false;
}

bool UInventoryComponent::ContainsItem(FItemInfo itemInfo, int32& index)
{
	index = ItemInfos.Find(itemInfo);
	return false;
}

void UInventoryComponent::OnRep_InventoryUpdate()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::UpdateItemInfos()
{
	ItemInfos.Empty();
	for (auto i : Items)
	{
		ItemInfos.Add(i->GetItemInfo());
	}
	OnRep_InventoryUpdate();
}

