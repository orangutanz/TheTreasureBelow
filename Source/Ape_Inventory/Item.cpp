// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

void UItem::SetItemInfo(FItemInfo info)
{
	mItemInfo = info;
}

bool UItem::SetQuantity(int32 num)
{
	if (num == 0 || (num > mItemInfo.MaxStack) && (mItemInfo.ItemType != EItemType::Equipment))
		return false;

	mItemInfo.Quantity = num;
	FOnItemUpdated.Broadcast();
	return true;
}

UItem* UItem::SplitItem(int32 num)
{
	if (mItemInfo.MaxStack == 1 || num == 0)
	{
		return nullptr;
	}

	if (mItemInfo.Quantity > num)
	{
		FItemInfo newInfo = mItemInfo;
		newInfo.Quantity = num;
		mItemInfo.Quantity -= num;
		auto newItem = NewObject<UItem>();
		newItem->SetItemInfo(newInfo);

		FOnItemUpdated.Broadcast();
		return newItem;
	}
	else if (mItemInfo.Quantity == num)
	{
		return this;
	}


	return nullptr;
}

bool UItem::MergeItem(UItem* other)
{	
	if (other == this || mItemInfo.Quantity >= mItemInfo.MaxStack || other->GetItemID() != GetItemID())
	{
		return false;
	}

	int32 combiedAmount = mItemInfo.Quantity + other->mItemInfo.Quantity;
	if (combiedAmount <= mItemInfo.MaxStack)
	{
		mItemInfo.Quantity = combiedAmount; // Fully merged
		other->mItemInfo.Quantity = 0;
		return true;
	}
	mItemInfo.Quantity = mItemInfo.MaxStack;
	other->mItemInfo.Quantity = combiedAmount - mItemInfo.MaxStack;
	return false; // Partial merged
}
