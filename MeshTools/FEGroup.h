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
#include "FEItemListBuilder.h"

class FENode;
class FEEdge;
class FEFace;
class FEElement;
class FEMesh;
class GObject;

//-----------------------------------------------------------------------------
// Base class for all groups
//
class FEGroup : public FEItemListBuilder
{
public:
	FEGroup(GObject* po, int ntype);
	virtual ~FEGroup();

	FEMesh* GetMesh();
	void SetGObject(GObject* pm);
	GObject* GetGObject();

	int Type() { return m_ntype; }

	FENodeList* BuildNodeList() { return 0; }
	FEFaceList* BuildFaceList() { return 0; }
	FEElemList* BuildElemList() { return 0; }

public:
	// These functions were added when FEGroup was added to FECore. 
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
class FEPart : public FEGroup
{
public:
	FEPart(GObject* po) : FEGroup(po, FE_PART) {}
	FEPart(GObject* po, const vector<int>&  elset);
	~FEPart(){}

	void CreateFromMesh();

	FEElemList* BuildElemList();
	FENodeList* BuildNodeList();

	FEItemListBuilder* Copy();
	void Copy(FEPart* pg);
};

//-----------------------------------------------------------------------------
// CLASS: FESurface
// class describing a group of faces
//
class FESurface : public FEGroup
{
public:
	FESurface(GObject* po) : FEGroup(po, FE_SURFACE) {}
	FESurface(GObject* po, vector<int>& face);
	~FESurface(){}

	FENodeList* BuildNodeList();
	FEFaceList* BuildFaceList();

	FEItemListBuilder* Copy();
	void Copy(FESurface* pg);
};

//-----------------------------------------------------------------------------
// class describing a group of edges
class FEEdgeSet : public FEGroup
{
public:
	FEEdgeSet(GObject* po) : FEGroup(po, FE_EDGESET) {}
	FEEdgeSet(GObject* po, vector<int>& edge);
	~FEEdgeSet(){}

	FENodeList* BuildNodeList();

	FEEdge* Edge(FEItemListBuilder::Iterator it);

	FEItemListBuilder* Copy();
	void Copy(FEEdgeSet* pg);
};

//-----------------------------------------------------------------------------
// CLASS: FENodeSet
// class describing a group of nodes
//
class FENodeSet : public FEGroup
{
public:
	FENodeSet(GObject* po) : FEGroup(po, FE_NODESET) {}
	FENodeSet(GObject* po, const vector<int>& node);
	~FENodeSet(){}

	void CreateFromMesh();

	FEItemListBuilder* Copy();
	void Copy(FENodeSet* pg);

	FENodeList* BuildNodeList();
};
