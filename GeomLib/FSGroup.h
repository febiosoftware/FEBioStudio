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
#include <MeshLib/FSItemListBuilder.h>

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
class FSGroup : public FSItemListBuilder
{
public:
	FSGroup(FSMesh* pm, int ntype, unsigned int flags);
	virtual ~FSGroup();

	FSMesh* GetMesh();
	GObject* GetGObject();

	int Type() { return m_ntype; }

	FSNodeList* BuildNodeList() { return nullptr; }
	FSEdgeList* BuildEdgeList() { return nullptr; }
	FSFaceList* BuildFaceList() { return nullptr; }
	FSElemList* BuildElemList() { return nullptr; }

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// This is only used to process some older fsm files.
	int GetMeshID() const { return m_meshID; }
	void SetMesh(FSMesh* mesh) { assert(m_mesh == nullptr); m_mesh = mesh; }

public:
	int m_ntag;

protected:
	FSMesh* m_mesh;

	int m_meshID = -1; // this is obsolete, but we still need to read some older files.
};

//-----------------------------------------------------------------------------
// class describing a group of elements
//
class FSElemSet : public FSGroup
{
public:
	FSElemSet(FSMesh* pm) : FSGroup(pm, FE_ELEMSET, FE_NODE_FLAG | FE_ELEM_FLAG) {}
	FSElemSet(FSMesh* pm, const std::vector<int>& elset);
	~FSElemSet(){}

	void CreateFromMesh();

	FSElemList* BuildElemList();
	FSNodeList* BuildNodeList();

	// TODO: Replace this will call to BuildNodeList
	void GetNodeList(std::vector<int>& node, std::vector<int>& lnode);

	FSElement_* GetElement(int n);

	FSItemListBuilder* Copy() override;
};

//-----------------------------------------------------------------------------
// class describing a group of elements, defined via a list of parts of an object
//
class FSPartSet : public FSGroup
{
public:
	FSPartSet(FSMesh* pm) : FSGroup(pm, FE_PARTSET, FE_PART_FLAG) {}
	~FSPartSet() {}

	FSItemListBuilder* Copy() override;

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
	FSSurface(FSMesh* pm) : FSGroup(pm, FE_SURFACE, FE_NODE_FLAG | FE_FACE_FLAG) {}
	FSSurface(FSMesh* pm, std::vector<int>& face);
	~FSSurface(){}

	FSNodeList* BuildNodeList();
	FSFaceList* BuildFaceList();

	// TODO: Replace with call to BuildNodeList
	void GetNodeList(std::vector<int>& node, std::vector<int>& lnode);

	FSFace* GetFace(int n);

	FSItemListBuilder* Copy() override;
};

//-----------------------------------------------------------------------------
// class describing a group of edges
class FSEdgeSet : public FSGroup
{
public:
	FSEdgeSet(FSMesh* pm) : FSGroup(pm, FE_EDGESET, FE_NODE_FLAG) {}
	FSEdgeSet(FSMesh* pm, std::vector<int>& edge);
	~FSEdgeSet(){}

	FSNodeList* BuildNodeList();

	FSEdgeList* BuildEdgeList();

	FSEdge* Edge(FSItemListBuilder::Iterator it);

	FSItemListBuilder* Copy() override;
};

//-----------------------------------------------------------------------------
// CLASS: FSNodeSet
// class describing a group of nodes
//
class FSNodeSet : public FSGroup
{
public:
	FSNodeSet(FSMesh* pm) : FSGroup(pm, FE_NODESET, FE_NODE_FLAG) {}
	FSNodeSet(FSMesh* pm, const std::vector<int>& node);
	~FSNodeSet(){}

	void CreateFromMesh();

	FSItemListBuilder* Copy() override;

	FSNode* GetNode(size_t n);

	FSNodeList* BuildNodeList();
};
