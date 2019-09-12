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
