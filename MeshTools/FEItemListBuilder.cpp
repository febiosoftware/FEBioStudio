#include "FEItemListBuilder.h"

int FEItemListBuilder::m_ncount = 1;

FEItemListBuilder::FEItemListBuilder(int ntype)
{
	// set the unique group ID
	m_nID = m_ncount++;

	m_ntype = ntype;
}

void FEItemListBuilder::SetID(int nid)
{ 
	m_nID = nid; 
	m_ncount = (nid >= m_ncount? nid+1: m_ncount);
}

bool FEItemListBuilder::IsValid() const
{ 
	return (m_Item.empty() == false); 
}

void FEItemListBuilder::Save(OArchive& ar)
{
	int N = (int)m_Item.size();
	ar.WriteChunk(ID, m_nID);
	ar.WriteChunk(NAME, GetName());
	ar.WriteChunk(SIZE, N);

	Iterator it = m_Item.begin();
	for (int i=0; i<N; ++i, ++it)
	{
		ar.WriteChunk(ITEM, (*it));
	}
}

void FEItemListBuilder::Load(IArchive &ar)
{
	TRACE("FEImteListBuilder::Load");

	m_Item.clear();

	int N, n;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case ID: ar.read(n); SetID(n); break;
		case NAME: { char sz[256]; ar.read(sz); SetName(sz); } break;
		case MESHID: break;	//--> obsolete
		case SIZE: ar.read(N); break;
		case ITEM: ar.read(n); m_Item.push_back(n); break;
		default:
			throw ReadError("unknown CID in FEItemListBuilder::Load");
		}
		ar.CloseChunk();
	}
	assert((int) m_Item.size() == N);
}

void FEItemListBuilder::remove(int n)
{
	if (m_Item.empty()) return;
	Iterator pi = m_Item.begin();
	for (int i=0; i<n; ++i) pi++;
	m_Item.erase(pi);
}

void FEItemListBuilder::Merge(list<int>& o)
{
	m_Item.insert(m_Item.end(), o.begin(), o.end());

	if (m_Item.empty() == false)
	{
		// sort the items
		m_Item.sort();

		// remove duplicates
		Iterator it1 = m_Item.begin(); 
		Iterator it2 = it1; it2++;

		while (it2 != m_Item.end())
		{
			if (*it1 == *it2) it2 = m_Item.erase(it2);
			else { it1 = it2; it2++; }
		}
	}
}

void FEItemListBuilder::Subtract(list<int>& o)
{
	// NOTE: This algorithm assumes that both lists are sorted
	Iterator it  = m_Item.begin();
	Iterator it2 = o.begin();
	while ((it != m_Item.end()) && (it2 != o.end()))
	{
		if (*it == *it2)
		{
			it = m_Item.erase(it);
			++it2;
		}
		else ++it;
	}
}
