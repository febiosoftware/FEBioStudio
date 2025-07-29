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
#include "FSElementData.h"
#include <MeshLib/FSMesh.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GGroup.h>

//-----------------------------------------------------------------------------
FSElementData::FSElementData(FSMesh* mesh) : FSMeshData(ELEM_DATA)
{
	m_scale = 1.0;
	SetMesh(mesh);
}

//-----------------------------------------------------------------------------
FSElementData::FSElementData(FSMesh* mesh, DATA_TYPE dataType, DATA_FORMAT dataFormat) : FSMeshData(ELEM_DATA)
{
	SetMesh(mesh);
	SetDataFormat(dataFormat);
	SetDataType(dataType);
	m_maxNodesPerElem = 0;
}

//-----------------------------------------------------------------------------
void FSElementData::Create(FSMesh* pm, FSElemSet* part, DATA_TYPE dataType, DATA_FORMAT dataFormat)
{
	SetMesh(pm);
	SetDataFormat(dataFormat);
	SetDataType(dataType);
	SetItemList(part);
}

//-----------------------------------------------------------------------------
void FSElementData::AllocateData()
{
	FSElemSet* elemSet = GetElementSet();

	int itemSize = ItemSize();
	assert(itemSize > 0);

	m_data.clear();
	if (elemSet)
	{
		int bufSize = 0;
		int elems = elemSet->size();
		switch (GetDataFormat())
		{
		case DATA_NODE:
		{
			FSNodeList* pnl = elemSet->BuildNodeList();
			bufSize = itemSize * pnl->Size();
			delete pnl;
		}
		break;
		case DATA_ITEM:
			bufSize = itemSize * elems;
			break;
		case DATA_MULT:
		{
			m_maxNodesPerElem = 0;
			for (int i = 0; i < elems; ++i)
			{
				FSElement_* pe = elemSet->GetElement(i);
				int ne = pe->Nodes();
				if (ne > m_maxNodesPerElem) m_maxNodesPerElem = ne;
			}
			bufSize = m_maxNodesPerElem * itemSize;
		}
		break;
		}

		m_data.assign(bufSize, 0.0);
	}
}

//-----------------------------------------------------------------------------
FSElementData::FSElementData(const FSElementData& d) : FSMeshData(ELEM_DATA) {}
void FSElementData::operator = (const FSElementData& d) {}

//-----------------------------------------------------------------------------
FSElemSet* FSElementData::GetElementSet() { return dynamic_cast<FSElemSet*>(GetItemList()); }

//-----------------------------------------------------------------------------
void FSElementData::SetItemList(FSItemListBuilder* item, int n)
{
	FSHasOneItemList::SetItemList(item);
	AllocateData();
}

//-----------------------------------------------------------------------------
void FSElementData::FillRandomBox(double fmin, double fmax)
{
	assert(GetDataType() == DATA_SCALAR);
	int N = (int)m_data.size();
	for (int i = 0; i<N; ++i)
	{
		double f = (double)rand() / (double)RAND_MAX;
		double v = fmin + (fmax - fmin)*f;
		m_data[i] = v;
	}
}

//-----------------------------------------------------------------------------
void FSElementData::SetScaleFactor(double s)
{
	m_scale = s;
}

//-----------------------------------------------------------------------------
double FSElementData::GetScaleFactor() const
{
	return m_scale;
}

//-----------------------------------------------------------------------------
void FSElementData::Save(OArchive& ar)
{
	int dataType = (int) GetDataType();
	int dataFmt  = (int) GetDataFormat();
	const std::string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME  , szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE  , dataType);
	ar.WriteChunk(CID_MESH_DATA_FORMAT, dataFmt);
	ar.WriteChunk(CID_MESH_DATA_SCALE , m_scale);

	// Parts must be saved first so that the number of elements in the part can be
	// queried before the data is read during the load operation.
	FSItemListBuilder* pi = GetItemList();
	if (pi) ar.WriteChunk(CID_MESH_DATA_ITEMLIST_ID, pi->GetID());

	if (m_data.empty() == false) ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], (int) m_data.size());
}

//-----------------------------------------------------------------------------
void FSElementData::Load(IArchive& ar)
{
	FSMesh* pm = GetMesh();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_MESH_DATA_NAME)
		{
			char szname[256];
			ar.read(szname);
			SetName(szname);
		}
		else if (nid == CID_MESH_DATA_TYPE)
		{
			int dType;
			ar.read(dType);
			SetDataType((DATA_TYPE) dType);
		}
		else if (nid == CID_MESH_DATA_FORMAT)
		{
			int dFmt;
			ar.read(dFmt);
			SetDataFormat((DATA_FORMAT)dFmt);
		}
		else if (nid == CID_MESH_DATA_SCALE)
		{
			ar.read(m_scale);
		}
		else if (nid == CID_MESH_DATA_ITEMLIST_ID)
		{
			int listId = -1;
			ar.read(listId);
			if (pm)
			{
				FSElemSet* pg = dynamic_cast<FSElemSet*>(pm->FindFEGroup(listId)); assert(pg);
				SetItemList(pg);
			}
		}
		else if (nid == CID_MESH_DATA_PART)
		{
			// older files (pre 2.1) used to store their own element sets. Now, the parent GObject stores
			// all element sets and we only need a pointer. 
			FSElemSet* part = new FSElemSet(pm);
			part->Load(ar);
			if (part->GetName().empty()) part->SetName(GetName());
			pm->AddFEElemSet(part);
			SetItemList(part);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			ar.read(&m_data[0], m_data.size());
		}

		ar.CloseChunk();
	}
}

//=============================================================================
FSPartData::FSPartData(FSMesh* mesh) : FSMeshData(PART_DATA)
{
	SetMesh(mesh);
	m_maxElemItems = 1;
}

FSPartData::FSPartData(FSMesh* mesh, DATA_TYPE dataType, DATA_FORMAT dataFmt) : FSMeshData(PART_DATA)
{
	SetMesh(mesh);
	SetDataFormat(dataFmt);
	SetDataType(dataType);
	m_maxElemItems = 0;
}

FSPartData::FSPartData(const FSPartData& d) : FSMeshData(PART_DATA)
{

}

FSPartData& FSPartData::operator = (const FSPartData& d)
{
	return *this;
}

// create a data field
bool FSPartData::Create(FSPartSet* partList, DATA_TYPE dataType, DATA_FORMAT dataFmt)
{
	FSMesh* mesh = GetMesh();
	assert(mesh);
	m_data.clear();
	SetDataFormat(dataFmt);
	SetDataType(dataType);
	SetItemList(partList);

	return true;
}

void FSPartData::AllocateData()
{
	FSMesh* mesh = GetMesh();
	int NE = mesh->Elements();
	int maxNodes = 0;
	int nsize = 0;
	FSPartSet* partList = GetPartSet();
	m_lut.assign(NE, -1);
	if (partList)
	{
		for (int i = 0; i < partList->size(); ++i)
		{
			int pid = (*partList)[i];
			for (int i = 0; i < NE; ++i)
			{
				FSElement& el = mesh->Element(i);
				if (el.m_gid == pid)
				{
					int nn = el.Nodes();
					if (nn > maxNodes) maxNodes = nn;

					m_lut[i] = nsize++;
				}
			}
		}
	}

	int itemSize = ItemSize();

	switch (GetDataFormat())
	{
	case DATA_ITEM: m_maxElemItems = 1; break;
	case DATA_NODE: m_maxElemItems = 1; break;
	case DATA_MULT:
	{
		m_maxElemItems = maxNodes;
		nsize *= maxNodes;
	}
	break;
	default:
		assert(false);
	}

	m_data.resize(nsize* itemSize);
}

FSElemList* FSPartData::BuildElemList()
{
	FSMesh* mesh = GetMesh();
	assert(mesh);

	FSPartSet* partList = GetPartSet();
	if (partList == nullptr) return nullptr;

	FSElemList* elemList = new FSElemList;
	int NE = mesh->Elements();
	for (int i = 0; i < partList->size(); ++i)
	{
		int pid = (*partList)[i];
		for (int j = 0; j < NE; ++j)
		{
			FSElement& el = mesh->Element(j);
			if (el.m_gid == pid)
			{
				elemList->Add(mesh, &el, j);
			}
		}
	}
	return elemList;
}

void FSPartData::SetItemList(FSItemListBuilder* item, int n)
{
	FSHasOneItemList::SetItemList(item);
	AllocateData();
}

bool FSPartData::AddPart(int localPartID)
{
	FSPartSet* ps = GetPartSet();
	if (ps == nullptr) return false;
	if (ps->HasItem(localPartID)) return false;
	ps->add(localPartID);
	AllocateData();
	return true;
}

int FSPartData::GetElementIndex(int nelem) { return m_lut[nelem]; }

FSPartSet* FSPartData::GetPartSet()
{
	return dynamic_cast<FSPartSet*>(GetItemList());
}

void FSPartData::Save(OArchive& ar)
{
	int dataType = (int) GetDataType();
	int dataFmt  = (int) GetDataFormat();
	const std::string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME  , szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE  , dataType);
	ar.WriteChunk(CID_MESH_DATA_FORMAT, dataFmt);
	ar.WriteChunk(CID_MESH_DATA_DPI   , (int)m_maxElemItems);

	// Parts must be saved first so that the number of elements in the part can be
	// queried before the data is read during the load operation.
	FSPartSet* partSet = GetPartSet();
	if (partSet) ar.WriteChunk(CID_MESH_DATA_ITEMLIST_ID, partSet->GetID());

	if (m_data.empty() == false) ar.WriteChunk(CID_MESH_DATA_VALUES, m_data);
}

void FSPartData::Load(IArchive& ar)
{
	FSMesh* pm = GetMesh();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_MESH_DATA_NAME)
		{
			char szname[256];
			ar.read(szname);
			SetName(szname);
		}
		else if (nid == CID_MESH_DATA_TYPE)
		{
			int dType;
			ar.read(dType);
			SetDataType((DATA_TYPE)dType);
		}
		else if (nid == CID_MESH_DATA_FORMAT)
		{
			int fType;
			ar.read(fType);
			SetDataFormat((DATA_FORMAT) fType);
		}
		else if (nid == CID_MESH_DATA_DPI)
		{
			ar.read(m_maxElemItems);
		}
		else if (nid == CID_MESH_DATA_ITEMLIST_ID)
		{
			int listId = -1;
			ar.read(listId);
			if (pm)
			{
				FSPartSet* pg = dynamic_cast<FSPartSet*>(pm->FindFEGroup(listId)); assert(pg);
				SetItemList(pg);
			}
		}
		else if (nid == CID_MESH_DATA_PART)
		{
			std::vector<int> part;
			ar.read(part);
			if (part.empty())
			{
				FSPartSet* ps = new FSPartSet(pm);
				ps->SetName(GetName());
				for (int n : part) ps->add(n);
				SetItemList(ps);
			}
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			ar.read(m_data);
		}

		ar.CloseChunk();
	}
}
