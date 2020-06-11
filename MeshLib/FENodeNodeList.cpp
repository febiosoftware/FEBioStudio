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

#include "FEMesh.h"
#include "FESurfaceMesh.h"
#include "FENodeNodeList.h"
#include "FENodeElementList.h"
#include "FENodeFaceList.h"

FENodeNodeList::FENodeNodeList(FEMesh* pm)
{
	Build(pm);
}

FENodeNodeList::FENodeNodeList(FESurfaceMesh* pm)
{
	Build(pm);
}

FENodeNodeList::~FENodeNodeList()
{
}

void FENodeNodeList::Build(FEMesh* pm)
{
	assert(pm);
	if (pm == 0) return;

	FENodeElementList NEL;
	NEL.Build(pm);

	int i, j, k, n;
	int NN = pm->Nodes();
	vector<int> tag; tag.assign(NN, -1);

	m_val.resize(NN);
	int nsize = 0;

	for (i=0; i<NN; ++i)
	{
		int nv = NEL.Valence(i);
		for (j=n=0; j<nv; ++j)
		{
			FEElement_* pe = NEL.Element(i, j);
			int ne = pe->Nodes();
			for (k=0; k<ne; ++k) 
			{
				int nn = pe->m_node[k];
				if ((nn != i) && (tag[nn] != i))
				{
					tag[nn] = i;
					n++;
				}
			}

			m_val[i] = n;
			nsize += n;
		}
	}

	m_off.resize(NN);
	m_off[0] = 0;
	for (i=1; i<NN; ++i) m_off[i] = m_off[i-1] + m_val[i-1];

	for (i=0; i<NN; ++i) tag[i] = -1;

	m_node.resize(nsize);

	for (i=0; i<NN; ++i)
	{
		int nv = NEL.Valence(i);
		int noff = m_off[i];
		for (j=n=0; j<nv; ++j)
		{
			FEElement_* pe = NEL.Element(i, j);
			int ne = pe->Nodes();
			for (k=0; k<ne; ++k) 
			{
				int nn = pe->m_node[k];
				if ((nn != i) && (tag[nn] != i))
				{
					tag[nn] = i;
					m_node[noff + n] = nn;
					n++;
				}
			}
		}
	}
}

void FENodeNodeList::Build(FESurfaceMesh* pm)
{
	assert(pm);
	if (pm == 0) return;

	FENodeFaceList NFL;
	NFL.Build(pm);

	int i, j, k, n;
	int NN = pm->Nodes();
	vector<int> tag; tag.assign(NN, -1);

	m_val.resize(NN);
	int nsize = 0;

	for (i = 0; i<NN; ++i)
	{
		int nv = NFL.Valence(i);
		for (j = n = 0; j<nv; ++j)
		{
			FEFace* pf = NFL.Face(i, j);
			int nf = pf->Nodes();
			for (k = 0; k<nf; ++k)
			{
				int nn = pf->n[k];
				if ((nn != i) && (tag[nn] != i))
				{
					tag[nn] = i;
					n++;
				}
			}

			m_val[i] = n;
			nsize += n;
		}
	}

	m_off.resize(NN);
	m_off[0] = 0;
	for (i = 1; i<NN; ++i) m_off[i] = m_off[i - 1] + m_val[i - 1];

	for (i = 0; i<NN; ++i) tag[i] = -1;

	m_node.resize(nsize);

	for (i = 0; i<NN; ++i)
	{
		int nv = NFL.Valence(i);
		int noff = m_off[i];
		for (j = n = 0; j<nv; ++j)
		{
			FEFace* pf = NFL.Face(i, j);
			int nf = pf->Nodes();
			for (k = 0; k<nf; ++k)
			{
				int nn = pf->n[k];
				if ((nn != i) && (tag[nn] != i))
				{
					tag[nn] = i;
					m_node[noff + n] = nn;
					n++;
				}
			}
		}
	}
}

void FENodeNodeList::InitValues(double v)
{
	m_data.assign(m_node.size(), v);
}
