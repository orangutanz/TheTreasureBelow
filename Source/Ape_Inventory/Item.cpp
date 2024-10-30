// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

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

bool UItemSlot::CanSplit(int32 amount)
{
	if (mItemInfo.Quantity < amount || mItemInfo.MaxStack == 1 || amount == 0 || IsEmpty())
	{
		return false;
	}
	return true;
}

FItemInfo UItemSlot::SplitQuantity(int32 amount)
{
	if (amount > mItemInfo.Quantity)
		return FItemInfo();
	FItemInfo outValue = mItemInfo;
	if (amount == mItemInfo.Quantity)
	{
		ClearItemInfo();
	}
	else
	{
		mItemInfo.Quantity -= amount;
		outValue.Quantity = amount;
	}
	return outValue;
}

bool UItemSlot::MergeItem(UItemSlot* other)
{
	if (!other || other == this)
	{
		return false;
	}

	if (IsEmpty())
	{
		SetItemInfo(other->GetItemInfo());
		return true;
	}
	else
	{
		if (other->GetItemID() != mItemInfo.ItemID || mItemInfo.MaxStack <= mItemInfo.Quantity)
		{
			return false;
		}
		if ((mItemInfo.Quantity + other->GetQuantity()) > mItemInfo.MaxStack)
		{
			int amount = mItemInfo.MaxStack - mItemInfo.Quantity;
			mItemInfo.Quantity = mItemInfo.MaxStack;
			other->SetQuantity(other->GetQuantity() - amount);
			return false;
		}
		mItemInfo.Quantity += other->GetQuantity();
		other->ClearItemInfo();
		return true;
	}
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
	FItemInfo tempInfo = mItemInfo;
	mItemInfo = other->GetItemInfo();
	other->SetItemInfo(tempInfo);
}
