#include "FENodeFaceList.h"

//-----------------------------------------------------------------------------
FENodeFaceList::FENodeFaceList(FEMeshBase* pm)
{
	m_pm = pm;
}

//-----------------------------------------------------------------------------
FENodeFaceList::~FENodeFaceList(void)
{
}

//-----------------------------------------------------------------------------
// Builds a sorted node-facet list. That is, the facets form a star around the node.
// Note that for non-manifold topologies this may fail, so make sure to check the return value.
bool FENodeFaceList::BuildSorted()
{
	Build();

	// sort the faces
	int N = m_pm->Nodes();
	for (int i=0; i<N; ++i)
	{
		if (Sort(i) == false) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void FENodeFaceList::Build()
{
	assert(m_pm);
	FEMeshBase& m = *m_pm;

	int NN = m.Nodes();
	int NF = m.Faces();
	m_val.assign(NN, 0);
	int nsize = 0;
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = m_pm->Face(i);
		int nf = face.Nodes();
		for (int j = 0; j<nf; ++j) m_val[face.n[j]]++;
		nsize += nf;
	}

	m_off.assign(NN, 0);
	m_off[0] = 0;
	for (int i = 1; i<NN; ++i) m_off[i] = m_off[i - 1] + m_val[i - 1];

	for (int i = 0; i<NN; ++i) m_val[i] = 0;

	m_pface.resize(nsize);
	m_nface.resize(nsize);
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = m.Face(i);
		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int n = f.n[j];
			int noff = m_off[n] + m_val[n];
			m_pface[noff] = &f;
			m_nface[noff] = i;
			m_val[n]++;
		}
	}
}

//-----------------------------------------------------------------------------
bool FENodeFaceList::HasFace(int n, FEFace* pf)
{
	int nval = Valence(n);
	for (int i=0; i<nval; ++i) if (Face(n, i) == pf) return true;
	return false;
}

//-----------------------------------------------------------------------------
int FENodeFaceList::FindFace(const FEFace& f)
{
	int n = f.n[0];
	int nval = Valence(n);
	for (int i=0; i<nval; ++i)
	{
		FEFace* pf = Face(n, i);
		if (*pf == f) return FaceIndex(n, i);
	}
	return -1;
}

//-----------------------------------------------------------------------------
bool FENodeFaceList::Sort(int node)
{
	int nval = Valence(node);
	vector<FEFace*> fl; fl.reserve(nval);

	for (int i=0; i<nval; ++i) Face(node, i)->m_ntag = 0;

	FEFace* pf = Face(node, 0);
	pf->m_ntag = 1;
	fl.push_back(pf);
	bool bdone = false;
	do
	{
		bdone = true;

		int m = -1;
		if (pf->n[0] == node) m = 0;
		else if (pf->n[1] == node) m = 1;
		else if (pf->n[2] == node) m = 2;

		int nj = pf->m_nbr[(m+2)%3];
		if (nj >= 0)
		{
			FEFace* pf2 = &m_pm->Face(nj);
			assert(HasFace(node, pf2));
			if (pf2->m_ntag == 0)
			{
				pf2->m_ntag = 1;
				fl.push_back(pf2);
				pf = pf2;
				bdone = false;
			}
		}
	}
	while (bdone == false);

	// for non-manifold topologies this algorithm
	// can fail. In that case, we return false
	if ((int)fl.size() != nval) return false;

	for (int i=0; i<nval; ++i)
	{
		m_pface[m_off[node] + i] = fl[i];
	}

	return true;
}
