#include "stdafx.h"
#include "IHasItemList.h"
#include <MeshLib/FEItemListBuilder.h>

FSHasItemList::FSHasItemList()
{
	m_itemType = 0;
	m_pItem = nullptr;
}

FSHasItemList::~FSHasItemList()
{
	m_pItem = nullptr;
}

FEItemListBuilder* FSHasItemList::GetItemList()
{
	return m_pItem;
}

void FSHasItemList::SetItemList(FEItemListBuilder* pi)
{
	if (m_pItem) m_pItem->DecRef();
	m_pItem = pi;
	if (m_pItem) m_pItem->IncRef();
}

unsigned int FSHasItemList::GetMeshItemType() const { return m_itemType; }
void FSHasItemList::SetMeshItemType(unsigned int meshItem) { m_itemType = meshItem; }
