/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <MeshLib/FEElement.h>
#include <MeshTools/FEGroup.h>
#include <vector>
using namespace std;

class FECoreMesh;

namespace Post {
//-----------------------------------------------------------------------------
// forward declaration of the mesh class
class FEPostMesh;

//-----------------------------------------------------------------------------
// Base class that describes a group of mesh items. 
class FEGroup : public ::FEGroup
{
public:
	FEGroup(FECoreMesh* pm, int ntype) :  ::FEGroup(nullptr, ntype) { m_pm = pm; }
	virtual ~FEGroup(void) {}

	FECoreMesh* GetMesh() const { return m_pm; }

protected:
	FECoreMesh*	m_pm;	// pointer to the parent mesh
};

//-----------------------------------------------------------------------------
// A doman is an internal organization of elements. A domain is created for each material.
class FEDomain
{
public:
	FEDomain(FEPostMesh* pm);

	void SetMatID(int matid);
	int GetMatID() const { return m_nmat; }

	int Type() const { return m_ntype; }

	int Faces() { return (int) m_Face.size(); }
	FEFace& Face(int n);

	int Elements() { return (int) m_Elem.size(); }
	FEElement_& Element(int n);

	void Reserve(int nelems, int nfaces);

	void AddElement(int n) { m_Elem.push_back(n); }
	void AddFace   (int n) { m_Face.push_back(n); }

protected:
	FEPostMesh*	m_pm;
	int			m_nmat;	// material index
	int			m_ntype;
	vector<int>	m_Face;	// face indices 
	vector<int>	m_Elem;	// element indices
};

//-----------------------------------------------------------------------------
// Class that describes a group of elements
class FEPart : public Post::FEGroup
{
public:
	FEPart(FECoreMesh* pm) : Post::FEGroup(pm, FE_PART) {}

	int Size() const { return (int) m_Elem.size(); }

	void GetNodeList(vector<int>& node, vector<int>& lnode);

	vector<int> GetElementList() const { return m_Elem; }

	FEItemListBuilder* Copy() override { return nullptr; }

public:
	vector<int>	m_Elem;	// element indices
};

//-------------------------------------------------------------------------
// Class that describes a group of faces
class FESurface : public Post::FEGroup
{
public:
	FESurface(FECoreMesh* pm) : Post::FEGroup(pm, FE_SURFACE) {}

	int Size() const { return (int) m_Face.size(); }

	void GetNodeList(vector<int>& node, vector<int>& lnode);

	FEItemListBuilder* Copy() override { return nullptr; }

public:
	vector<int>	m_Face;	// face indices
};

//-------------------------------------------------------------------------
//! Class that defines a node set
class FENodeSet : public Post::FEGroup
{
public:
	FENodeSet(FECoreMesh* pm) : Post::FEGroup(pm, FE_NODESET){}

	FEItemListBuilder* Copy() override { return nullptr; }

public:
	vector<int>	m_Node;
};
}
