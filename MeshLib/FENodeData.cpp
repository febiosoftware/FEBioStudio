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
#include "FENodeData.h"
#include <MeshLib/FEMesh.h>
#include <GeomLib/GObject.h>

FENodeData::FENodeData(GObject* po) : FEMeshData(FEMeshData::NODE_DATA)
{
	m_po = po;
	m_dataSize = 0;
	if (po) SetMesh(po->GetFEMesh());
}

FENodeData::FENodeData(GObject* po, FEMeshData::DATA_TYPE dataType) : FEMeshData(FEMeshData::NODE_DATA)
{
	m_po = po;
	m_dataSize = 0;
	m_dataType = dataType;
	if (po) SetMesh(po->GetFEMesh());
}

FENodeData::FENodeData(const FENodeData& d) : FEMeshData(FEMeshData::NODE_DATA)
{
	assert(false);
}

FENodeData& FENodeData::operator=(const FENodeData& d)
{
	assert(false);
	return *this;
}

void FENodeData::Create(FSNodeSet* nset, double v, FEMeshData::DATA_TYPE dataType)
{
	FSHasOneItemList::SetItemList(nset);
	m_dataType = dataType;
	assert(m_po->GetFEMesh() == nset->GetMesh());
	SetMesh(nset->GetMesh());

	int nodes = nset->size();
	m_dataSize = 0;
	switch (dataType)
	{
	case FEMeshData::DATA_SCALAR: m_dataSize = 1; break;
	case FEMeshData::DATA_VEC3D : m_dataSize = 3; break;
	case FEMeshData::DATA_MAT3D : m_dataSize = 9; break;
	default:
		assert(false);
	}

	int bufsize = nodes * m_dataSize;
	m_data.assign(bufsize, v);
}

void FENodeData::SetItemList(FEItemListBuilder* pl, int n)
{
	Create(dynamic_cast<FSNodeSet*>(pl), 0.0, m_dataType);
}

void FENodeData::Save(OArchive& ar)
{
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE, (int)m_dataType);
	FEItemListBuilder* pi = GetItemList();
	if (pi) ar.WriteChunk(CID_MESH_DATA_ITEMLIST_ID, pi->GetID());
	ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], (int)m_data.size());
}

void FENodeData::Load(IArchive& ar)
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
			int dataType = 0;
			ar.read(dataType);
			m_dataType = (FEMeshData::DATA_TYPE) dataType;

			switch (m_dataType)
			{
			case FEMeshData::DATA_SCALAR: m_dataSize = 1; break;
			case FEMeshData::DATA_VEC3D : m_dataSize = 3; break;
			case FEMeshData::DATA_MAT3D : m_dataSize = 9; break;
			default:
				assert(false);
			}
		}
		else if (nid == CID_MESH_DATA_ITEMLIST_ID)
		{
			int listId = -1;
			ar.read(listId);
			if (po)
			{
				FSNodeSet* pg = dynamic_cast<FSNodeSet*>(po->FindFEGroup(listId)); assert(pg);
				SetItemList(pg);
			}
		}
		else if (nid == CID_MESH_DATA_NODESET)
		{
			// older files (pre 2.1) stored their own node sets. 
			// now these are stored on the parent object
			FSNodeSet* nodeSet = new FSNodeSet(po);
			nodeSet->Load(ar);

			// add it to the parent object
			if (nodeSet->GetName().empty()) nodeSet->SetName(GetName());
			SetItemList(nodeSet);
			po->AddFENodeSet(nodeSet);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			// Older files defined node data on the entire mesh. The node sets were 
			// not saved.
			FSNodeSet* nodeSet = dynamic_cast<FSNodeSet*>(GetItemList());
			if (nodeSet == nullptr)
			{
				nodeSet = new FSNodeSet(m_po);
				nodeSet->CreateFromMesh();
				nodeSet->SetName(GetName());
				m_po->AddFENodeSet(nodeSet);
			}
			else m_data.assign(nodeSet->size() * m_dataSize, 0);

			int NN = nodeSet->size();
			ar.read(&m_data[0], NN*m_dataSize);
		}

		ar.CloseChunk();
	}
}
