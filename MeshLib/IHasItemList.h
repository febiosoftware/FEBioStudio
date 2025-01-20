#pragma once

class FSItemListBuilder;

class IHasItemLists
{
public:
	virtual int ItemLists() const = 0;
	virtual FSItemListBuilder* GetItemList(int n) = 0;
	virtual void SetItemList(FSItemListBuilder* pi, int n) = 0;

	virtual unsigned int GetMeshItemType() const = 0;
	virtual void SetMeshItemType(unsigned int meshItem) = 0;
};

class FSHasOneItemList : public IHasItemLists
{
public:
	FSHasOneItemList();
	virtual ~FSHasOneItemList();

public: // from IHasItemList
	int ItemLists() const override;
	FSItemListBuilder* GetItemList(int n = 0) override;
	void SetItemList(FSItemListBuilder* pi, int n = 0) override;

	unsigned int GetMeshItemType() const override;
	void SetMeshItemType(unsigned int meshItem) override;

private:
	unsigned int		m_itemType;	// the type of mesh item that can be assigned to this list
	FSItemListBuilder*	m_pItem;	// list of item indices to apply the BC too
};

class FSHasTwoItemLists : public IHasItemLists
{
public:
	FSHasTwoItemLists();
	virtual ~FSHasTwoItemLists();

public: // from IHasItemList
	int ItemLists() const override;
	FSItemListBuilder* GetItemList(int n) override;
	void SetItemList(FSItemListBuilder* pi, int n) override;

	unsigned int GetMeshItemType() const override;
	void SetMeshItemType(unsigned int meshItem) override;

private:
	unsigned int		m_itemType;	// the type of mesh item that can be assigned to this list
	FSItemListBuilder* m_pItem[2];	// list of item indices to apply the BC too
};
