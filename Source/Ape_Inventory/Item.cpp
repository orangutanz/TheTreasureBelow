// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

bool UItemSlot::AddItemInfo(FItemInfo& itemInfo)
{
	if (itemInfo.ItemID != mItemInfo.ItemID || mItemInfo.MaxStack <= mItemInfo.Quantity)
	{
		return false;
	}
	mItemInfo.Quantity += itemInfo.Quantity;
	if (mItemInfo.Quantity > mItemInfo.MaxStack)
	{
		itemInfo.Quantity = mItemInfo.Quantity - mItemInfo.MaxStack; // not fully added
		mItemInfo.Quantity = mItemInfo.MaxStack;
		return false;
	}
	return true;
}

void UItemSlot::SetItemInfo(FItemInfo info)
{
	mItemInfo = info;
}

bool UItemSlot::SetQuantity(int32 num)
{
	if (num == 0 || (num > mItemInfo.MaxStack) && (mItemInfo.ItemType != EItemType::Equipment))
		return false;

	mItemInfo.Quantity = num;
	return true;
}

UItemSlot* UItemSlot::SplitItem(int32 num)
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
		auto newItem = NewObject<UItemSlot>();
		newItem->SetItemInfo(newInfo);

		return newItem;
	}
	else if (mItemInfo.Quantity == num)
	{
		return this;
	}


	return nullptr;
}

bool UItemSlot::MergeItem(UItemSlot* other)
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

void UItemSlot::SwapItemInfo(UItemSlot* other)
{
	auto tempInfo = mItemInfo;
	mItemInfo = other->mItemInfo;
	other->mItemInfo = tempInfo;
}
