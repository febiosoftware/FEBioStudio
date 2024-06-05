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
#include <MeshLib/FEItemListBuilder.h>

class FSNode;
class FSEdge;
class FSFace;
class FSElement;
class FSMesh;
class GObject;
class GPart;

//-----------------------------------------------------------------------------
// Base class for all groups
//
class FSGroup : public FEItemListBuilder
{
public:
	FSGroup(GObject* po, int ntype, unsigned int flags);
	virtual ~FSGroup();

	FSMesh* GetMesh();
	void SetGObject(GObject* pm);
	GObject* GetGObject();

	int Type() { return m_ntype; }

	FSNodeList* BuildNodeList() { return 0; }
	FEEdgeList* BuildEdgeList() { return 0; }
	FEFaceList* BuildFaceList() { return 0; }
	FEElemList* BuildElemList() { return 0; }

public:
	// These functions were added when FSGroup was added to FECore. 
	// The mesh ID is the ID of the GObject that owns the mesh. In FECore
	// GObjects are not known, so a different mechanism was implemented
	// to find the parent object (and mesh) when reading the archive. Now, only the
	// mesh ID (i.e. the object ID) is written and read and a function
	// higher up the call stack needs to find the correct parent mesh. 
	// I hope one day to remove this construction.
//	void SetMeshID(int nid) { m_objID = nid; }
	int GetObjectID() const { return m_objID; }

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	int m_ntag;

protected:
	GObject*	m_pObj;		//!< pointer to the object this group belongs to
	int			m_objID;	//!< The ID of the object that owns the mesh (only used when reading the IArchive)
};

//-----------------------------------------------------------------------------
// class describing a group of elements
//
class FSElemSet : public FSGroup
{
public:
	FSElemSet(GObject* po) : FSGroup(po, FE_ELEMSET, FE_NODE_FLAG | FE_ELEM_FLAG) {}
	FSElemSet(GObject* po, const std::vector<int>&  elset);
	~FSElemSet(){}

	void CreateFromMesh();

	FEElemList* BuildElemList();
	FSNodeList* BuildNodeList();

	FEElement_* GetElement(int n);

	FEItemListBuilder* Copy();
	void Copy(FSElemSet* pg);
};

//-----------------------------------------------------------------------------
// class describing a group of elements, defined via a list of parts of an object
//
class FSPartSet : public FSGroup
{
public:
	FSPartSet(GObject* po) : FSGroup(po, FE_PARTSET, FE_PART_FLAG) {}
	FSPartSet(GObject* po, const std::vector<int>& parts);
	~FSPartSet() {}

	FEItemListBuilder* Copy();
	void Copy(FSPartSet* pg);

	GPart* GetPart(size_t n);

	std::vector<int> BuildElementIndexList();
	std::vector<int> BuildElementIndexList(const std::vector<int>& partList);
};

//-----------------------------------------------------------------------------
// CLASS: FSSurface
// class describing a group of faces
//
class FSSurface : public FSGroup
{
public:
	FSSurface(GObject* po) : FSGroup(po, FE_SURFACE, FE_NODE_FLAG | FE_FACE_FLAG) {}
	FSSurface(GObject* po, std::vector<int>& face);
	~FSSurface(){}

	FSNodeList* BuildNodeList();
	FEFaceList* BuildFaceList();

	FSFace* GetFace(int n);

	FEItemListBuilder* Copy();
	void Copy(FSSurface* pg);
};

//-----------------------------------------------------------------------------
// class describing a group of edges
class FSEdgeSet : public FSGroup
{
public:
	FSEdgeSet(GObject* po) : FSGroup(po, FE_EDGESET, FE_NODE_FLAG) {}
	FSEdgeSet(GObject* po, std::vector<int>& edge);
	~FSEdgeSet(){}

	FSNodeList* BuildNodeList();

	FEEdgeList* BuildEdgeList();

	FSEdge* Edge(FEItemListBuilder::Iterator it);

	FEItemListBuilder* Copy();
	void Copy(FSEdgeSet* pg);
};

//-----------------------------------------------------------------------------
// CLASS: FSNodeSet
// class describing a group of nodes
//
class FSNodeSet : public FSGroup
{
public:
	FSNodeSet(GObject* po) : FSGroup(po, FE_NODESET, FE_NODE_FLAG) {}
	FSNodeSet(GObject* po, const std::vector<int>& node);
	~FSNodeSet(){}

	void CreateFromMesh();

	FEItemListBuilder* Copy();
	void Copy(FSNodeSet* pg);

	FSNode* GetNode(size_t n);

	FSNodeList* BuildNodeList();
};
