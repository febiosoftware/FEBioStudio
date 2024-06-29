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

#include "FENodeElementList.h"

FSNodeElementList::FSNodeElementList()
{
	m_pm = nullptr;
}

FSNodeElementList::~FSNodeElementList()
{
}

void FSNodeElementList::Build(FSCoreMesh* pm)
{
	m_pm = pm;
	assert(m_pm);
	m_elem.clear();

	int NN = m_pm->Nodes();
	int NE = m_pm->Elements();
	if ((NE == 0) || (NN == 0)) return;

	m_elem.resize(NN);
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = m_pm->ElementRef(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j) 
		{
			int n = el.m_node[j];

			std::vector<NodeElemRef>& lj = m_elem[n];

			NodeElemRef ref;
			ref.eid = i;
			ref.nid = j;
			ref.pe = &el;

			lj.push_back(ref);
		}
	}
}

void FSNodeElementList::Clear()
{
	m_pm = nullptr;
	m_elem.clear();
}

bool FSNodeElementList::IsEmpty() const
{
	return m_elem.empty();
}

bool FSNodeElementList::HasElement(int node, int iel) const
{
	int nval = Valence(node);
	for (int i=0; i<nval; ++i) 
		if (ElementIndex(node, i) == iel) return true;

	return false;
}

std::vector<int> FSNodeElementList::ElementIndexList(int n) const
{
	std::vector<int> l;
	int nval = Valence(n);
	for (int i=0; i<nval; ++i)
	{
		l.push_back(ElementIndex(n, i));
	}
	return l;
}
