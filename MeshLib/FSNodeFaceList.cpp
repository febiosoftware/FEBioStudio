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

#include "FSNodeFaceList.h"
#include "FSMeshBase.h"
#include "FSFace.h"
#include <assert.h>
#include <FSCore/FunctionTimer.h>
using namespace std;

//-----------------------------------------------------------------------------
FSNodeFaceList::FSNodeFaceList()
{
	m_pm = nullptr;
}

//-----------------------------------------------------------------------------
FSNodeFaceList::~FSNodeFaceList(void)
{
}

//-----------------------------------------------------------------------------
void FSNodeFaceList::Clear()
{
	m_face.clear();
}

//-----------------------------------------------------------------------------
bool FSNodeFaceList::IsEmpty() const
{
	return m_face.empty();
}

//-----------------------------------------------------------------------------
// Builds a sorted node-facet list. That is, the facets form a star around the node.
// Note that for non-manifold topologies this may fail, so make sure to check the return value.
bool FSNodeFaceList::BuildSorted(FSMeshBase* pm)
{
	Build(pm);

	// sort the faces
	int N = m_pm->Nodes();
	for (int i=0; i<N; ++i)
	{
		if (Sort(i) == false) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void FSNodeFaceList::Build(FSMeshBase* pm)
{
	m_pm = pm;
	assert(m_pm);
	FSMeshBase& m = *m_pm;

	int NN = m.Nodes();
	int NF = m.Faces();

	std::vector<int> val(NN, 0);
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = m.Face(i);
		int nf = f.Nodes();
		for (int j = 0; j < nf; ++j)
		{
			int n = f.n[j];
			val[n]++;
		}
	}

	m_face.resize(val); // this also resets all vals to zero

	for (int i=0; i<NF; ++i)
	{
		FSFace& f = m.Face(i);
		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int n = f.n[j];
			NodeFaceRef& ref = m_face.item(n, val[n]);
			ref.fid = i;
			ref.nid = j;
			ref.pf = &f;
			val[n]++;
		}
	}
}

//-----------------------------------------------------------------------------
bool FSNodeFaceList::HasFace(int n, FSFace* pf) const
{
	int nval = Valence(n);
	for (int i=0; i<nval; ++i) if (Face(n, i) == pf) return true;
	return false;
}

//-----------------------------------------------------------------------------
int FSNodeFaceList::FindFace(const FSFace& f)
{
	int n = f.n[0];
	int nval = Valence(n);
	for (int i=0; i<nval; ++i)
	{
		FSFace* pf = Face(n, i);
		if (*pf == f) return FaceIndex(n, i);
	}
	return -1;
}

//-----------------------------------------------------------------------------
bool FSNodeFaceList::Sort(int node)
{
	int nval = Valence(node);
	vector<NodeFaceRef> fl; fl.reserve(nval);

	for (int i=0; i<nval; ++i) Face(node, i)->m_ntag = 0;

	NodeFaceRef ref = m_face.item(node, 0);
	ref.pf->m_ntag = 1;
	fl.push_back(ref);
	bool bdone = false;
	do
	{
		bdone = true;

		int m = -1;
		if      (ref.pf->n[0] == node) m = 0;
		else if (ref.pf->n[1] == node) m = 1;
		else if (ref.pf->n[2] == node) m = 2;

		int nj = ref.pf->m_nbr[(m+2)%3];
		if (nj >= 0)
		{
			FSFace* pf2 = &m_pm->Face(nj);
			assert(HasFace(node, pf2));
			if (pf2->m_ntag == 0)
			{
				pf2->m_ntag = 1;

				int k = 0;
				for (; k < nval; ++k)
				{
					if (Face(node, k) == pf2)
					{
						break;
					}
				}
				assert(k < nval);

				fl.push_back(m_face.item(node,k));
				ref = m_face.item(node,k);
				bdone = false;
			}
		}
	}
	while (bdone == false);

	// for non-manifold topologies this algorithm
	// can fail. In that case, we return false
	if ((int)fl.size() != nval) return false;

	for (int i =0; i<fl.size(); ++i)
	m_face.item(node, i) = fl[i];

	return true;
}

/*
const vector<NodeFaceRef>& FSNodeFaceList::FaceList(int n) const
{ 
	return m_face[n]; 
}
*/

//-----------------------------------------------------------------------------
// This function will fail if the facet could not be found. 
// The most likely cause would be if the facet is an interal fact, since PostView does not
// process internal facets (e.g. facets between two materials).
// \todo perhaps I should modify PostView so that it stores internal facets as well.
int FSNodeFaceList::FindFace(int inode, int n[10], int m)
{
	FSFace ft;
	for (int i = 0; i<m; ++i) ft.n[i] = n[i];
	switch (m)
	{
	case 3: ft.m_type = FE_FACE_TRI3; break;
	case 4: ft.m_type = FE_FACE_QUAD4; break;
	case 6: ft.m_type = FE_FACE_TRI6; break;
	case 7: ft.m_type = FE_FACE_TRI7; break;
	case 8: ft.m_type = FE_FACE_QUAD8; break;
	case 9: ft.m_type = FE_FACE_QUAD9; break;
	case 10: ft.m_type = FE_FACE_TRI10; break;
	default:
		assert(false);
	};

	int nf = (int)m_face.items(inode);
	for (int i = 0; i<nf; ++i)
	{
		NodeFaceRef& ref = m_face.item(inode, i);
		FSFace& f = m_pm->Face(ref.fid);
		if (f == ft) return ref.fid;
	}
	return -1;
}
