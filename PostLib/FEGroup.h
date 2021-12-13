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
#include <MeshLib/FEElement.h>
#include <MeshTools/FEGroup.h>
#include <vector>
//using namespace std;

class FSCoreMesh;

namespace Post {
//-----------------------------------------------------------------------------
// forward declaration of the mesh class
class FEPostMesh;

//-----------------------------------------------------------------------------
// Base class that describes a list of mesh items. 
class MeshItemList : public FSObject
{
public:
	MeshItemList(FSCoreMesh* pm, int ntype) { m_pm = pm; m_type = ntype; }
	virtual ~MeshItemList(void) {}

	FSCoreMesh* GetMesh() const { return m_pm; }

private:
	MeshItemList(const MeshItemList&) {}
	void operator = (const MeshItemList&){}

protected:
	int			m_type;
	FSCoreMesh*	m_pm;	// pointer to the parent mesh
};

//-----------------------------------------------------------------------------
// A doman is an internal organization of elements. A domain is created for each material.
class MeshDomain
{
public:
	MeshDomain(FEPostMesh* pm);

	void SetMatID(int matid);
	int GetMatID() const { return m_nmat; }

	int Type() const { return m_ntype; }

	int Faces() { return (int) m_Face.size(); }
	FSFace& Face(int n);

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
class FSPart : public Post::MeshItemList
{
public:
	FSPart(FSCoreMesh* pm) : MeshItemList(pm, FE_PART) {}

	int Size() const { return (int) m_Elem.size(); }

	void GetNodeList(vector<int>& node, vector<int>& lnode);

	vector<int> GetElementList() const { return m_Elem; }

public:
	vector<int>	m_Elem;	// element indices
};

//-------------------------------------------------------------------------
// Class that describes a group of faces
class FSSurface : public MeshItemList
{
public:
	FSSurface(FSCoreMesh* pm) : MeshItemList(pm, FE_SURFACE) {}

	int Size() const { return (int) m_Face.size(); }

	void GetNodeList(vector<int>& node, vector<int>& lnode);

public:
	vector<int>	m_Face;	// face indices
};

//-------------------------------------------------------------------------
//! Class that defines a node set
class FSNodeSet : public MeshItemList
{
public:
	FSNodeSet(FSCoreMesh* pm) : MeshItemList(pm, FE_NODESET){}

public:
	vector<int>	m_Node;
};
}
