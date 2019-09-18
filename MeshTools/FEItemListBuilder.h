#pragma once
#include "FEItemList.h"
#include <FSCore/FSObject.h>
#include <FEMLib/FECoreModel.h>
#include <list>

//-----------------------------------------------------------------------------
enum ITEMLIST_TYPE {
	GO_NODE,
	GO_EDGE,
	GO_FACE,
	GO_PART,
	FE_NODESET,
	FE_EDGESET,
	FE_SURFACE,
	FE_PART,
};

//-----------------------------------------------------------------------------
enum DOMAIN_TYPE
{
	DOMAIN_MESH,
	DOMAIN_PART,
	DOMAIN_SURFACE,
	DOMAIN_EDGE,
	DOMAIN_NODESET
};

//-----------------------------------------------------------------------------
// This class is an abstract base class for any class that can build FEItem lists.
// Currently this is the GItem class for geometry objects and FEGroup class for
// FE meshes. Each derived class must be able to define how to build FEItem lists.
//
class FEItemListBuilder : public FSObject
{
public:
	enum {ID, NAME, MESHID, SIZE, ITEM};

public:
	FEItemListBuilder(int ntype);

	virtual FENodeList*	BuildNodeList() = 0;
	virtual FEFaceList*	BuildFaceList() = 0;
	virtual FEElemList*	BuildElemList() = 0;
	
	virtual FEItemListBuilder* Copy() = 0;

	virtual bool IsValid() const;

	void clear() { m_Item.clear(); }
	void add(int n) { m_Item.push_back(n); }
	void remove(int i);
	int size() const { return (int)m_Item.size(); }
	list<int>::iterator begin() { return m_Item.begin(); }
	list<int>::iterator end() { return m_Item.end(); }
	list<int>::iterator erase(list<int>::iterator p) { return m_Item.erase(p); }

	list<int>::const_iterator begin() const { return m_Item.begin(); }
	list<int>::const_iterator end() const { return m_Item.end(); }

	int GetID() { return m_nID; }
	void SetID(int nid);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	int Type() { return m_ntype; }

	void Merge(list<int>& o);
	void Subtract(list<int>& o);

	list<int> CopyItems() { return m_Item; }

protected:
	list<int>	m_Item;

	int	m_ntype;

	int	m_nID;	// the unique group ID
	static int m_ncount;
};
