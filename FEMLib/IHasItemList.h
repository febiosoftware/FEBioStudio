#pragma once

class FEItemListBuilder;

class IHasItemList
{
public:
	virtual FEItemListBuilder* GetItemList() = 0;
	virtual void SetItemList(FEItemListBuilder* pi) = 0;

	virtual unsigned int GetMeshItemType() const = 0;
	virtual void SetMeshItemType(unsigned int meshItem) = 0;
};
