#include "stdafx.h"
#include "FENodeData.h"
#include <MeshLib/FEMesh.h>

FENodeData::FENodeData() : FEMeshData(FEMeshData::NODE_DATA)
{
}

FENodeData::FENodeData(const FENodeData& d) : FEMeshData(FEMeshData::NODE_DATA)
{
	SetName(d.GetName());
	SetMesh(d.GetMesh());
	m_data = d.m_data;
}

FENodeData& FENodeData::operator=(const FENodeData& d)
{
	SetName(d.GetName());
	SetMesh(d.GetMesh());
	m_data = d.m_data;
	return *this;
}

void FENodeData::Create(FEMesh* pm, double v)
{
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
