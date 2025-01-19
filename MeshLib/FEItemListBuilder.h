/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include <FSCore/FSObject.h>
#include "FEItemList.h"
#include <vector>

//-----------------------------------------------------------------------------
enum ITEMLIST_TYPE {
	GO_NODE,
	GO_EDGE,
	GO_FACE,
	GO_PART,
	FE_NODESET,
	FE_EDGESET,
	FE_SURFACE,
	FE_ELEMSET,
	FE_PARTSET
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
enum MESH_ITEM_FLAGS
{
	FE_NODE_FLAG = 0x01,
	FE_EDGE_FLAG = 0x02,
	FE_FACE_FLAG = 0x04,
	FE_ELEM_FLAG = 0x08,
	FE_PART_FLAG = 0x10,

	FE_ALL_FLAGS = 0xFF
};

//-----------------------------------------------------------------------------
// This class is an abstract base class for any class that can build MeshItem lists.
// Currently this is the GItem class for geometry objects and FSGroup class for
// FE meshes. Each derived class must be able to define how to build MeshItem lists.
//
class FEItemListBuilder : public FSObject
{
public:
	enum { ID, NAME, MESHID, SIZE, ITEM };

	typedef std::vector<int>::iterator Iterator;
	typedef std::vector<int>::const_iterator ConstIterator;

public:
	FEItemListBuilder(int ntype, unsigned int flags);
	~FEItemListBuilder();

	virtual FSNodeList* BuildNodeList() = 0;
	virtual FEEdgeList* BuildEdgeList() = 0;
	virtual FEFaceList* BuildFaceList() = 0;
	virtual FEElemList* BuildElemList() = 0;

	virtual FEItemListBuilder* Copy() = 0;

	virtual bool IsValid() const;

	bool Supports(unsigned int itemFlag) const;

	void clear() { m_Item.clear(); }
	void add(int n) { m_Item.push_back(n); }
	void add(const std::vector<int>& nodeList);
	void remove(int i);
	void reserve(size_t n) { m_Item.reserve(n); }
	int size() const { return (int)m_Item.size(); }
	Iterator begin() { return m_Item.begin(); }
	Iterator end() { return m_Item.end(); }
	Iterator erase(Iterator p) { return m_Item.erase(p); }

	ConstIterator begin() const { return m_Item.begin(); }
	ConstIterator end() const { return m_Item.end(); }

	int GetID() { return m_nID; }
	void SetID(int nid);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	int Type() { return m_ntype; }

	void Merge(std::vector<int>& o);
	void Subtract(std::vector<int>& o);

	std::vector<int> CopyItems() { return m_Item; }

	int operator [] (size_t n) const { return m_Item[n]; }

	bool HasItem(int n) const;

public:
	int GetReferenceCount() const;
	void IncRef();
	void DecRef();

protected:
	std::vector<int>	m_Item;

	int	m_ntype;

	int m_refs;	// reference count

	unsigned int	m_flags;

	int	m_nID;	// the unique group ID
	static int m_ncount;
};
