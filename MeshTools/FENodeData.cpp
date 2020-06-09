#include "stdafx.h"
#include "FENodeData.h"
#include <MeshLib/FEMesh.h>
#include <GeomLib/GObject.h>

FENodeData::FENodeData(GObject* po) : FEMeshData(FEMeshData::NODE_DATA)
{
	m_po = po;
	m_nodeSet = nullptr;
}

FENodeData::FENodeData(const FENodeData& d) : FEMeshData(FEMeshData::NODE_DATA)
{
}

FENodeData& FENodeData::operator=(const FENodeData& d)
{
	return *this;
}

void FENodeData::Create(double v)
{
	delete m_nodeSet;
	m_nodeSet = new FENodeSet(m_po);
	m_nodeSet->CreateFromMesh();
	FEMesh* pm = m_po->GetFEMesh();
	SetMesh(pm);
	m_data.assign(pm->Nodes(), v);
}

void FENodeData::Save(OArchive& ar)
{
	int NN = GetMesh()->Nodes();
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], NN);
}

void FENodeData::Load(IArchive& ar)
{
	const int NN = GetMesh()->Nodes();
	Create();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_MESH_DATA_NAME)
		{
			char szname[256];
			ar.read(szname);
			SetName(szname);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			ar.read(&m_data[0], NN);
		}

		ar.CloseChunk();
	}
}

// get the item list
FEItemListBuilder* FENodeData::GetItemList()
{
	return m_nodeSet;
}