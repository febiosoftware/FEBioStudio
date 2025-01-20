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
#include <list>

//-----------------------------------------------------------------------------
// Forward declaration of the FSCoreMesh
class FSCoreMesh;

// The FSItemList stores a list of pointers to FSMeshItems (nodes, faces, elements)
template <class T> class FSItemList_T
{
public:
	struct ITEM
	{
		ITEM(FSCoreMesh* pm, T* pi, int lid = -1) { m_pm = pm; m_pi = pi; m_lid = lid; }
		FSCoreMesh*	m_pm;
		T*		m_pi;
		int		m_lid;
	};

	typedef typename std::list<ITEM>::iterator Iterator;

public:
	FSItemList_T(){}

	void Add(FSCoreMesh* pm, T* pn, int lid = -1) { m_Item.push_back(ITEM(pm, pn, lid)); }
	int Size() { return (int)m_Item.size(); }
	Iterator First() { return m_Item.begin(); }
	Iterator End() { return m_Item.end(); }

protected:
	std::list<ITEM>	m_Item;
};

//-----------------------------------------------------------------------------
// Specialization for nodes
class FSNode;
typedef FSItemList_T<FSNode> FSNodeList;

//-----------------------------------------------------------------------------
// Specialization for edges
class FSEdge;
typedef FSItemList_T<FSEdge> FSEdgeList;

//-----------------------------------------------------------------------------
// Specialization for faces
class FSFace;
typedef FSItemList_T<FSFace> FSFaceList;

//-----------------------------------------------------------------------------
// Specialization for elements
class FSElement_;
typedef FSItemList_T<FSElement_> FSElemList;
