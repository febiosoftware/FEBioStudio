#include "FENodeElementList.h"

FENodeElementList::FENodeElementList()
{
	m_pm = nullptr;
}

FENodeElementList::~FENodeElementList()
{
}

void FENodeElementList::Build(FECoreMesh* pm)
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
		vector<NodeElemRef>& li = m_elem[i];
		FEElement_& el = m_pm->ElementRef(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j) 
		{
			int n = el.m_node[j];

			NodeElemRef ref;
			ref.eid = i;
			ref.nid = j;
			ref.pe = &el;

			li.push_back(ref);
		}
	}
}

void FENodeElementList::Clear()
{
	m_elem.clear();
}

bool FENodeElementList::IsEmpty() const
{
	return m_elem.empty();
}

bool FENodeElementList::HasElement(int node, int iel) const
{
	int nval = Valence(node);
	for (int i=0; i<nval; ++i) 
		if (ElementIndex(node, i) == iel) return true;

	return false;
}

vector<int> FENodeElementList::ElementIndexList(int n) const
{
	vector<int> l;
	int nval = Valence(n);
	for (int i=0; i<nval; ++i)
	{
		l.push_back(ElementIndex(n, i));
	}
	return l;
}
