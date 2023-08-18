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
#include "FEElementData.h"
#include <MeshLib/FEMesh.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GGroup.h>

//-----------------------------------------------------------------------------
FEElementData::FEElementData(FSMesh* mesh) : FEMeshData(FEMeshData::ELEMENT_DATA)
{
	m_scale = 1.0;
	SetMesh(mesh);
}

//-----------------------------------------------------------------------------
FEElementData::FEElementData(FSMesh* mesh, FEMeshData::DATA_TYPE dataType, FEMeshData::DATA_FORMAT dataFormat) : FEMeshData(FEMeshData::ELEMENT_DATA)
{
	SetMesh(mesh);
	m_dataType = dataType;
	m_dataFmt = dataFormat;
	m_dataSize = 0;
	m_maxNodesPerElem = 0;
}

//-----------------------------------------------------------------------------
void FEElementData::Create(FSMesh* pm, FSElemSet* part, FEMeshData::DATA_TYPE dataType, FEMeshData::DATA_FORMAT dataFormat)
{
	SetMesh(pm);
	m_dataType = dataType;
	m_dataFmt = dataFormat;
	m_dataSize = ItemSize();
	SetItemList(part);
}

//-----------------------------------------------------------------------------
void FEElementData::AllocateData()
{
	m_dataSize = 0;
	switch (m_dataType)
	{
	case FEMeshData::DATA_SCALAR: m_dataSize = 1; break;
	case FEMeshData::DATA_VEC3D: m_dataSize = 3; break;
	case FEMeshData::DATA_MAT3D: m_dataSize = 9; break;
	default:
		assert(false);
		return;
	}

	FSElemSet* elemSet = GetElementSet();

	m_data.clear();
	if (elemSet)
	{
		int bufSize = 0;
		int elems = elemSet->size();
		switch (m_dataFmt)
		{
		case FEMeshData::DATA_NODE:
		{
			FSNodeList* pnl = elemSet->BuildNodeList();
			bufSize = m_dataSize * pnl->Size();
			delete pnl;
		}
		break;
		case FEMeshData::DATA_ITEM:
			bufSize = m_dataSize * elems;
			break;
		case FEMeshData::DATA_MULT:
		{
			m_maxNodesPerElem = 0;
			for (int i = 0; i < elems; ++i)
			{
				FEElement_* pe = elemSet->GetElement(i);
				int ne = pe->Nodes();
				if (ne > m_maxNodesPerElem) m_maxNodesPerElem = ne;
			}
			bufSize = m_maxNodesPerElem * m_dataSize;
		}
		break;
		}

		m_data.assign(bufSize, 0.0);
	}
}

//-----------------------------------------------------------------------------
FEElementData::FEElementData(const FEElementData& d) : FEMeshData(FEMeshData::ELEMENT_DATA) {}
void FEElementData::operator = (const FEElementData& d) {}

//-----------------------------------------------------------------------------
FSElemSet* FEElementData::GetElementSet() { return dynamic_cast<FSElemSet*>(GetItemList()); }

//-----------------------------------------------------------------------------
void FEElementData::SetItemList(FEItemListBuilder* item, int n)
{
	FSHasOneItemList::SetItemList(item);
	AllocateData();
}

//-----------------------------------------------------------------------------
void FEElementData::FillRandomBox(double fmin, double fmax)
{
	assert(m_dataType == DATA_SCALAR);
	int N = (int)m_data.size();
	for (int i = 0; i<N; ++i)
	{
		double f = (double)rand() / (double)RAND_MAX;
		double v = fmin + (fmax - fmin)*f;
		m_data[i] = v;
	}
}

//-----------------------------------------------------------------------------
void FEElementData::SetScaleFactor(double s)
{
	m_scale = s;
}

//-----------------------------------------------------------------------------
double FEElementData::GetScaleFactor() const
{
	return m_scale;
}

//-----------------------------------------------------------------------------
int FEElementData::ItemSize() const
{
	switch (m_dataType)
	{
	case DATA_SCALAR: return 1; break;
	case DATA_VEC3D : return 3; break;
	case DATA_MAT3D : return 9; break;
	default:
		assert(false);
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FEElementData::get(int n, double* d)
{
	assert((m_dataSize > 0) && (m_dataSize == ItemSize()));
	for (int i = 0; i < m_dataSize; ++i)
		d[i] = m_data[m_dataSize * n + i];
}

//-----------------------------------------------------------------------------
void FEElementData::set(int n, const mat3d& v)
{
	assert(m_dataType == DATA_MAT3D);
	assert(m_dataSize == 9);
	int m = 0;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j, ++m)
			m_data[9 * n + m] = v(i, j);
}

//-----------------------------------------------------------------------------
void FEElementData::Save(OArchive& ar)
{
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE, (int)m_dataType);
	ar.WriteChunk(CID_MESH_DATA_FORMAT, (int)m_dataFmt);
	ar.WriteChunk(CID_MESH_DATA_SCALE, m_scale);

	// Parts must be saved first so that the number of elements in the part can be
	// queried before the data is read during the load operation.
	FEItemListBuilder* pi = GetItemList();
	if (pi) ar.WriteChunk(CID_MESH_DATA_ITEMLIST_ID, pi->GetID());

	if (m_data.empty() == false) ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], (int) m_data.size());
}

//-----------------------------------------------------------------------------
void FEElementData::Load(IArchive& ar)
{
	GObject* po = GetMesh()->GetGObject();
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
			m_dataType = (FEMeshData::DATA_TYPE) dType;
		}
		else if (nid == CID_MESH_DATA_FORMAT)
		{
			int dFmt;
			ar.read(dFmt);
			m_dataFmt = (FEMeshData::DATA_FORMAT)dFmt;
		}
		else if (nid == CID_MESH_DATA_SCALE)
		{
			ar.read(m_scale);
		}
		else if (nid == CID_MESH_DATA_ITEMLIST_ID)
		{
			int listId = -1;
			ar.read(listId);
			if (po)
			{
				FSElemSet* pg = dynamic_cast<FSElemSet*>(po->FindFEGroup(listId)); assert(pg);
				SetItemList(pg);
			}
		}
		else if (nid == CID_MESH_DATA_PART)
		{
			// older files (pre 2.1) used to store their own element sets. Now, the parent GObject stores
			// all element sets and we only need a pointer. 
			FSElemSet* part = new FSElemSet(po);
			part->Load(ar);
			if (part->GetName().empty()) part->SetName(GetName());
			po->AddFEElemSet(part);
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
FEPartData::FEPartData(FSMesh* mesh) : FEMeshData(FEMeshData::PART_DATA)
{
	SetMesh(mesh);
	m_maxElemItems = 1;
}

FEPartData::FEPartData(FSMesh* mesh, FEMeshData::DATA_TYPE dataType, FEMeshData::DATA_FORMAT dataFmt) : FEMeshData(FEMeshData::PART_DATA)
{
	SetMesh(mesh);
	m_dataType = dataType;
	m_dataFmt = dataFmt;
	m_dataSize = 0;
	m_maxElemItems = 0;
}

FEPartData::FEPartData(const FEPartData& d) : FEMeshData(FEMeshData::PART_DATA)
{

}

FEPartData& FEPartData::operator = (const FEPartData& d)
{
	return *this;
}

// create a data field
bool FEPartData::Create(FSPartSet* partList, FEMeshData::DATA_TYPE dataType, FEMeshData::DATA_FORMAT dataFmt)
{
	FSMesh* mesh = GetMesh();
	assert(mesh);
	m_data.clear();

	m_dataType = dataType;
	m_dataFmt = dataFmt;

	SetItemList(partList);

	return true;
}

void FEPartData::AllocateData()
{
	FSMesh* mesh = GetMesh();
	int NE = mesh->Elements();
	int maxNodes = 0;
	int nsize = 0;
	FSPartSet* partList = GetPartSet();
	m_lut.assign(NE, -1);
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

	m_dataSize = 0;
	switch (m_dataType)
	{
	case DATA_SCALAR: m_dataSize = 1; break;
	case DATA_VEC3D : m_dataSize = 3; break;
	case DATA_MAT3D : m_dataSize = 9; break;
	default:
		assert(false);
	}

	if (m_dataFmt == DATA_ITEM)
	{
		m_maxElemItems = 1;
	}
	else if (m_dataFmt == DATA_NODE)
	{
		m_maxElemItems = 1;
	}
	else if (m_dataFmt == DATA_MULT)
	{
		m_maxElemItems = maxNodes;
		nsize *= maxNodes;
	}

	m_data.resize(nsize*m_dataSize);
}

FEElemList* FEPartData::BuildElemList()
{
	FSMesh* mesh = GetMesh();
	assert(mesh);

	FSPartSet* partList = GetPartSet();
	if (partList == nullptr) return nullptr;

	FEElemList* elemList = new FEElemList;
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

void FEPartData::SetItemList(FEItemListBuilder* item, int n)
{
	FSHasOneItemList::SetItemList(item);
	AllocateData();
}

int FEPartData::GetElementIndex(int nelem) { return m_lut[nelem]; }

FSPartSet* FEPartData::GetPartSet()
{
	return dynamic_cast<FSPartSet*>(GetItemList());
}

// size of data field
int FEPartData::Size() const
{ 
	return (int)m_data.size(); 
}

void FEPartData::Save(OArchive& ar)
{
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE, (int)m_dataType);
	ar.WriteChunk(CID_MESH_DATA_FORMAT, (int)m_dataFmt);
	ar.WriteChunk(CID_MESH_DATA_DPI, (int)m_maxElemItems);

	// Parts must be saved first so that the number of elements in the part can be
	// queried before the data is read during the load operation.
	FSPartSet* partSet = GetPartSet();
	if (partSet) ar.WriteChunk(CID_MESH_DATA_ITEMLIST_ID, partSet->GetID());

	if (m_data.empty() == false) ar.WriteChunk(CID_MESH_DATA_VALUES, m_data);
}

void FEPartData::Load(IArchive& ar)
{
	GObject* po = GetMesh()->GetGObject();
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
			m_dataType = (FEMeshData::DATA_TYPE) dType;
		}
		else if (nid == CID_MESH_DATA_FORMAT)
		{
			int fType;
			ar.read(fType);
			m_dataFmt = (FEMeshData::DATA_FORMAT) fType;
		}
		else if (nid == CID_MESH_DATA_DPI)
		{
			ar.read(m_maxElemItems);
		}
		else if (nid == CID_MESH_DATA_ITEMLIST_ID)
		{
			int listId = -1;
			ar.read(listId);
			if (po)
			{
				FSPartSet* pg = dynamic_cast<FSPartSet*>(po->FindFEGroup(listId)); assert(pg);
				SetItemList(pg);
			}
		}
		else if (nid == CID_MESH_DATA_PART)
		{
			std::vector<int> part;
			ar.read(part);
			if (part.empty())
			{
				FSPartSet* ps = new FSPartSet(po);
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
