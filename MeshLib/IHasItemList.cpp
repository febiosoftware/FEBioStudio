#include "IHasItemList.h"
#include <MeshLib/FEItemListBuilder.h>

FSHasOneItemList::FSHasOneItemList()
{
	m_itemType = 0;
	m_pItem = nullptr;
}

FSHasOneItemList::~FSHasOneItemList()
{
	m_pItem = nullptr;
}

int FSHasOneItemList::ItemLists() const
{
	return 1;
}

FEItemListBuilder* FSHasOneItemList::GetItemList(int n)
{
	assert(n == 0);
	return m_pItem;
}

void FSHasOneItemList::SetItemList(FEItemListBuilder* pi, int n)
{
	assert(n == 0);
	if (m_pItem) m_pItem->DecRef();
	m_pItem = pi;
	if (m_pItem) m_pItem->IncRef();
}

unsigned int FSHasOneItemList::GetMeshItemType() const { return m_itemType; }
void FSHasOneItemList::SetMeshItemType(unsigned int meshItem) { m_itemType = meshItem; }

//=============================================================================
FSHasTwoItemLists::FSHasTwoItemLists()
{
	m_itemType = 0;
	m_pItem[0] = nullptr;
	m_pItem[1] = nullptr;
}

FSHasTwoItemLists::~FSHasTwoItemLists()
{
	m_pItem[0] = nullptr;
	m_pItem[1] = nullptr;
}

int FSHasTwoItemLists::ItemLists() const
{
	return 2;
}

FEItemListBuilder* FSHasTwoItemLists::GetItemList(int n)
{
	switch (n)
	{
	case 0: return m_pItem[0]; break;
	case 1: return m_pItem[1]; break;
	}
	assert(false);
	return nullptr;
}

void FSHasTwoItemLists::SetItemList(FEItemListBuilder* pi, int n)
{
	assert((n == 0) || (n == 1));
	if ((n == 0) || (n == 1))
	{
		if (m_pItem[n]) m_pItem[n]->DecRef();
		m_pItem[n] = pi;
		if (m_pItem[n]) m_pItem[n]->IncRef();
	}
}

unsigned int FSHasTwoItemLists::GetMeshItemType() const { return m_itemType; }
void FSHasTwoItemLists::SetMeshItemType(unsigned int meshItem) { m_itemType = meshItem; }
