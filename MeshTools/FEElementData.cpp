#include "stdafx.h"
#include "FEElementData.h"
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
FEElementData::FEElementData() : FEMeshData(FEMeshData::PART_DATA)
{
}

//-----------------------------------------------------------------------------
void FEElementData::Create(FEMesh* pm, double v)
{
	SetMesh(pm);
	m_data.assign(pm->Elements(), v);
	m_tag.assign(pm->Elements(), 1);
}

//-----------------------------------------------------------------------------
FEElementData::FEElementData(const FEElementData& d) : FEMeshData(FEMeshData::PART_DATA)
{
	SetMesh(d.GetMesh());
	SetName(d.GetName());
	m_data = d.m_data;
	m_tag = d.m_tag;
}

//-----------------------------------------------------------------------------
FEElementData& FEElementData::operator = (const FEElementData& d)
{
	SetMesh(d.GetMesh());
	m_data = d.m_data;
	m_tag = d.m_tag;
	SetName(d.GetName());
	return (*this);
}

//-----------------------------------------------------------------------------
void FEElementData::FillRandomBox(double fmin, double fmax)
{
	int N = (int)m_data.size();
	for (int i = 0; i<N; ++i)
	{
		double f = (double)rand() / (double)RAND_MAX;
		double v = fmin + (fmax - fmin)*f;
		m_data[i] = v;
	}
}

//-----------------------------------------------------------------------------
void FEElementData::ClearTags(int n)
{
	int N = (int)m_tag.size();
	for (int i = 0; i<N; ++i) m_tag[i] = n;
}

//-----------------------------------------------------------------------------
void FEElementData::SetTag(int nelem, int ntag)
{
	m_tag[nelem] = ntag;
}

//-----------------------------------------------------------------------------
void FEElementData::Save(OArchive& ar)
{
	int NE = GetMesh()->Elements();
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], NE);
	ar.WriteChunk(CID_MESH_DATA_TAGS, &m_tag[0], NE);
}

//-----------------------------------------------------------------------------
void FEElementData::Load(IArchive& ar)
{
	const int NE = GetMesh()->Elements();
	while (IO_OK == ar.OpenChunk())
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
			ar.read(&m_data[0], NE);
		}
		else if (nid == CID_MESH_DATA_TAGS)
		{
			const int NE = GetMesh()->Elements();
			ar.read(&m_tag[0], NE);
		}

		ar.CloseChunk();
	}
}
