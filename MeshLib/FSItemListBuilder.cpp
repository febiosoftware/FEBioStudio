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

#include "FSItemListBuilder.h"
#include <algorithm>

int FSItemListBuilder::m_ncount = 1;

FSItemListBuilder::FSItemListBuilder(int ntype, unsigned int flags)
{
	// set the unique group ID
	m_nID = m_ncount++;

	m_ntype = ntype;

	m_flags = flags;

	m_refs = 0;
}

FSItemListBuilder::~FSItemListBuilder()
{
	assert(m_refs == 0);
}

void FSItemListBuilder::SetID(int nid)
{ 
	m_nID = nid; 
	m_ncount = (nid >= m_ncount? nid+1: m_ncount);
}

bool FSItemListBuilder::IsValid() const
{ 
	return (m_Item.empty() == false); 
}

bool FSItemListBuilder::Supports(unsigned int itemFlag) const
{
	return (m_flags & itemFlag);
}

void FSItemListBuilder::Save(OArchive& ar)
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

void FSItemListBuilder::Load(IArchive &ar)
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
			throw ReadError("unknown CID in FSItemListBuilder::Load");
		}
		ar.CloseChunk();
	}
	assert((int) m_Item.size() == N);
}

void FSItemListBuilder::add(const std::vector<int>& nodeList)
{
	m_Item.insert(m_Item.end(), nodeList.begin(), nodeList.end());
}

void FSItemListBuilder::remove(int n)
{
	if (m_Item.empty()) return;
	Iterator pi = m_Item.begin();
	for (int i=0; i<n; ++i) pi++;
	m_Item.erase(pi);
}

void FSItemListBuilder::Merge(std::vector<int>& o)
{
	m_Item.insert(m_Item.end(), o.begin(), o.end());

	if (m_Item.empty() == false)
	{
		// sort the items
		std::sort(m_Item.begin(), m_Item.end());

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

void FSItemListBuilder::Subtract(std::vector<int>& o)
{
	std::sort(m_Item.begin(), m_Item.end());

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
		else if (*it < *it2) ++it;
		else ++it2;
	}
}

bool FSItemListBuilder::HasItem(int n) const
{
	for (int m : m_Item)
	{
		if (m == n) return true;
	}
	return false;
}

int FSItemListBuilder::FindItem(int n) const
{
	for (int i = 0; i < m_Item.size(); ++i)
	{
		if (m_Item[i] == n) return i;
	}
	return -1;
}

int FSItemListBuilder::GetReferenceCount() const { return m_refs; }
void FSItemListBuilder::IncRef() { m_refs++; }
void FSItemListBuilder::DecRef() { m_refs--; assert(m_refs >= 0); }
