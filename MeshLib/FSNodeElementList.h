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
#include <vector>
#include "FSCoreMesh.h"
#include <FSCore/NestedArray.h>

//-----------------------------------------------------------------------------
// the first index is the element number
// the second index is the local node index of the element
struct NodeElemRef {
	int		eid;	// element index in mesh
	int		nid;	// local node index of the element
	FSElement_*	pe;	// pointer to element
};

class FSNodeElementList
{
public:
	FSNodeElementList();
	~FSNodeElementList();

	void Build(FSCoreMesh* pm);

	void Clear();

	bool IsEmpty() const;

	int Valence(int n) const { return (int)m_elem.items(n); }
	FSElement_* Element(int n, int j) { return m_elem.item(n,j).pe; }
	int ElementIndex(int n, int j) const { return m_elem.item(n,j).eid; }
	int ElementNodeIndex(int n, int j) const { return m_elem.item(n,j).nid; }

	bool HasElement(int node, int iel) const;

protected:
	FSCoreMesh*	m_pm;
	NestedArray<NodeElemRef> m_elem;
};
