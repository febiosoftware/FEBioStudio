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

class FSHasItemList : public IHasItemList
{
public:
	FSHasItemList();
	virtual ~FSHasItemList();

public: // from IHasItemList
	FEItemListBuilder* GetItemList() override;
	void SetItemList(FEItemListBuilder* pi) override;

	unsigned int GetMeshItemType() const override;
	void SetMeshItemType(unsigned int meshItem) override;

private:
	unsigned int		m_itemType;	// the type of mesh item that can be assigned to this list
	FEItemListBuilder*	m_pItem;	// list of item indices to apply the BC too
};
