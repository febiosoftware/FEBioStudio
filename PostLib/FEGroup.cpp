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

#include "stdafx.h"
#include "FEGroup.h"
#include "FEPostMesh.h"
#include <string.h>
using namespace std;

//-----------------------------------------------------------------------------
// FSDomain constructor
Post::MeshDomain::MeshDomain(Post::FEPostMesh *pm)
{
	m_pm = pm;
	m_nmat = -1;
}

//-----------------------------------------------------------------------------
void Post::MeshDomain::Reserve(int nelems, int nfaces)
{
	m_Elem.reserve(nelems);
	m_Face.reserve(nfaces);
}

//-----------------------------------------------------------------------------
void Post::MeshDomain::SetMatID(int matid)
{
	m_nmat = matid;
}

//-----------------------------------------------------------------------------
FSFace& Post::MeshDomain::Face(int n)
{ 
	return m_pm->Face(m_Face[n]); 
}

//-----------------------------------------------------------------------------
FEElement_& Post::MeshDomain::Element(int n)
{
	return m_pm->ElementRef(m_Elem[n]);
}

void Post::FSElemSet::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FSCoreMesh& mesh = *GetMesh();
	int NN = mesh.Nodes();
	int NE = Size();

	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = -1;

	int n = 0, nne = 0;
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(m_Elem[i]);
		int ne = el.Nodes();
		nne += ne;
		for (int j=0; j<ne; ++j)
		{
			if (mesh.Node(el.m_node[j]).m_ntag == -1) mesh.Node(el.m_node[j]).m_ntag = n++;
		}
	}

	node.resize(n);
	for (int i=0; i<NN; ++i)
		if (mesh.Node(i).m_ntag >= 0) node[mesh.Node(i).m_ntag] = i;

	lnode.resize(nne); nne = 0;
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(m_Elem[i]);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			int lid = mesh.Node(el.m_node[j]).m_ntag; assert(lid >= 0);
			lnode[nne + j] = lid;
		}
		nne += ne;
	}
}

void Post::FSSurface::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FSCoreMesh& mesh = *GetMesh();
	int NN = mesh.Nodes();
	int NF = Size();

	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = -1;

	int n = 0, nnf = 0;
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = mesh.Face(m_Face[i]);
		int nf = face.Nodes();
		nnf += nf;
		for (int j=0; j<nf; ++j)
		{
			if (mesh.Node(face.n[j]).m_ntag == -1) mesh.Node(face.n[j]).m_ntag = n++;
		}
	}

	node.resize(n);
	for (int i=0; i<NN; ++i)
		if (mesh.Node(i).m_ntag >= 0) node[mesh.Node(i).m_ntag] = i;

	lnode.resize(nnf); nnf = 0;
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = mesh.Face(m_Face[i]);
		int nf = face.Nodes();
		for (int j=0; j<nf; ++j)
		{
			int lid = mesh.Node(face.n[j]).m_ntag; assert(lid >= 0);
			lnode[nnf + j] = lid;
		}
		nnf += nf;
	}
}
