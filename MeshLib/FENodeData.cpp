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

FENodeData::FENodeData(GObject* po) : FEMeshData(NODE_DATA)
{
	m_po = po;
	if (po) SetMesh(po->GetFEMesh());
}

FENodeData::FENodeData(GObject* po, DATA_TYPE dataType) : FEMeshData(NODE_DATA)
{
	m_po = po;
	SetDataType(dataType);
	if (po) SetMesh(po->GetFEMesh());
}

FENodeData::FENodeData(const FENodeData& d) : FEMeshData(NODE_DATA)
{
	assert(false);
}

FENodeData& FENodeData::operator=(const FENodeData& d)
{
	assert(false);
	return *this;
}

void FENodeData::Create(FSNodeSet* nset, double v, DATA_TYPE dataType)
{
	FSHasOneItemList::SetItemList(nset);
	SetDataType(dataType);

	if (nset == nullptr)
	{
		SetMesh(nullptr);
		m_data.clear();
	}
	else
	{
		assert(m_po->GetFEMesh() == nset->GetMesh());
		SetMesh(nset->GetMesh());

		int nodes = nset->size();

		int bufsize = nodes * ItemSize();
		m_data.assign(bufsize, v);
	}
}

void FENodeData::SetItemList(FEItemListBuilder* pl, int n)
{
	Create(dynamic_cast<FSNodeSet*>(pl), 0.0, GetDataType());
}

void FENodeData::Save(OArchive& ar)
{
	int dataType = (int)GetDataType();
	const std::string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE, dataType);
	FEItemListBuilder* pi = GetItemList();
	if (pi) ar.WriteChunk(CID_MESH_DATA_ITEMLIST_ID, pi->GetID());
	if (m_data.empty()==false) ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], (int)m_data.size());
}

void FENodeData::Load(IArchive& ar)
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
			int dataType = 0;
			ar.read(dataType);
			SetDataType((DATA_TYPE) dataType);
		}
		else if (nid == CID_MESH_DATA_ITEMLIST_ID)
		{
			int listId = -1;
			ar.read(listId);
			if (pm)
			{
				FSNodeSet* pg = dynamic_cast<FSNodeSet*>(pm->FindFEGroup(listId)); assert(pg);
				SetItemList(pg);
			}
		}
		else if (nid == CID_MESH_DATA_NODESET)
		{
			// older files (pre 2.1) stored their own node sets. 
			// now these are stored on the parent object
			FSNodeSet* nodeSet = new FSNodeSet(pm);
			nodeSet->Load(ar);

			// add it to the parent object
			if (nodeSet->GetName().empty()) nodeSet->SetName(GetName());
			SetItemList(nodeSet);
			pm->AddFENodeSet(nodeSet);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			// Older files defined node data on the entire mesh. The node sets were 
			// not saved.
			FSNodeSet* nodeSet = dynamic_cast<FSNodeSet*>(GetItemList());
			if (nodeSet == nullptr)
			{
				nodeSet = new FSNodeSet(pm);
				nodeSet->CreateFromMesh();
				nodeSet->SetName(GetName());
				SetItemList(nodeSet);
				pm->AddFENodeSet(nodeSet);
			}

			ar.read(&m_data[0], m_data.size());
		}

		ar.CloseChunk();
	}
}
