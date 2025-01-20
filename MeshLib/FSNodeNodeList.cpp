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

#include "FSMesh.h"
#include "FSSurfaceMesh.h"
#include "FSNodeNodeList.h"
#include "FSNodeElementList.h"
#include "FSNodeFaceList.h"

FSNodeNodeList::FSNodeNodeList(FSMesh* pm, bool preservePartitions)
{
	Build(pm, preservePartitions);
}

FSNodeNodeList::FSNodeNodeList(FSSurfaceMesh* pm)
{
	Build(pm);
}

FSNodeNodeList::~FSNodeNodeList()
{
}

void FSNodeNodeList::Build(FSMesh* pm, bool preservePartitions)
{
	assert(pm);
	if (pm == 0) return;

	FSNodeElementList NEL;
	NEL.Build(pm);

	int NN = pm->Nodes();
	std::vector<int> tag; tag.assign(NN, -1);

	std::vector<int> P(NN, 0), D(NN, -1);
	if (preservePartitions)
	{
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			if (node.m_gid >= 0)
			{
				P[i] = 3;
				D[i] = node.m_gid;
			}
		}

		for (int i = 0; i < pm->Edges(); ++i)
		{
			FSEdge& e = pm->Edge(i);
			if (e.m_gid >= 0)
			{
				int nn = e.Nodes();
				for (int j = 0; j < nn; ++j)
				{
					int nj = e.n[j];
					if (P[nj] == 0)
					{
						P[nj] = 2;
						D[nj] = e.m_gid;
					}
				}
			}
		}

		for (int i = 0; i < pm->Faces(); ++i)
		{
			FSFace& f = pm->Face(i);
			if (f.m_gid >= 0)
			{
				int nn = f.Nodes();
				for (int j = 0; j < nn; ++j)
				{
					int nj = f.n[j];
					if (P[nj] == 0)
					{
						P[nj] = 1;
						D[nj] = f.m_gid;
					}
				}
			}
		}

	}

	m_val.resize(NN);
	int nsize = 0;

	for (int i=0; i<NN; ++i)
	{
		int Pi = P[i];
		double Di = D[i];
		int n = 0;
		int nv = NEL.Valence(i);
		for (int j = 0; j < nv; ++j)
		{
			FSElement_* pe = NEL.Element(i, j);
			int ne = pe->Nodes();
			for (int k = 0; k < ne; ++k)
			{
				int nn = pe->m_node[k];
				if ((nn != i) && (tag[nn] != i))
				{
					int Pn = P[nn], Dn = D[nn];
					if ((preservePartitions == false) || 
						(Pi < Pn) ||
						((Pi == Pn) && (Di == Dn)))
					{
						tag[nn] = i;
						n++;
					}
				}
			}
		}
		m_val[i] = n;
		nsize += n;
	}

	m_off.resize(NN);
	m_off[0] = 0;
	for (int i=1; i<NN; ++i) m_off[i] = m_off[i-1] + m_val[i-1];

	for (int i=0; i<NN; ++i) tag[i] = -1;

	m_node.resize(nsize);

	for (int i=0; i<NN; ++i)
	{
		int Pi = P[i], Di = D[i];
		int n = 0;
		int nv = NEL.Valence(i);
		int noff = m_off[i];
		for (int j = 0, n = 0; j < nv; ++j)
		{
			FSElement_* pe = NEL.Element(i, j);
			int ne = pe->Nodes();
			for (int k = 0; k < ne; ++k)
			{
				int nn = pe->m_node[k];
				if ((nn != i) && (tag[nn] != i))
				{
					int Pn = P[nn], Dn = D[nn];
					if ((preservePartitions == false) || 
						(Pi < Pn) ||
						((Pi == Pn) && (Di == Dn)))
					{
						tag[nn] = i;
						m_node[noff + n] = nn;
						n++;
					}
				}
			}
		}
	}
}

void FSNodeNodeList::Build(FSSurfaceMesh* pm)
{
	assert(pm);
	if (pm == 0) return;

	FSNodeFaceList NFL;
	NFL.Build(pm);

	int i, j, k, n;
	int NN = pm->Nodes();
	std::vector<int> tag; tag.assign(NN, -1);

	m_val.resize(NN);
	int nsize = 0;

	for (i = 0; i<NN; ++i)
	{
		int nv = NFL.Valence(i);
		for (j = n = 0; j<nv; ++j)
		{
			FSFace* pf = NFL.Face(i, j);
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
			FSFace* pf = NFL.Face(i, j);
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

void FSNodeNodeList::InitValues(double v)
{
	m_data.assign(m_node.size(), v);
}
