#include "stdafx.h"
#include "FEElementData.h"
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
FEElementData::FEElementData(FEMesh* mesh) : FEMeshData(FEMeshData::PART_DATA)
{
	m_scale = 1.0;
	m_part = nullptr;
	SetMesh(mesh);
}

//-----------------------------------------------------------------------------
void FEElementData::Create(FEMesh* pm, FEPart* part, FEMeshData::DATA_TYPE dataType)
{
	SetMesh(pm);
	m_part = part;
	m_data.assign(part->size(), 0.0);
}

//-----------------------------------------------------------------------------
FEElementData::FEElementData(const FEElementData& d) : FEMeshData(FEMeshData::PART_DATA)
{
	SetMesh(d.GetMesh());
	SetName(d.GetName());
	m_data = d.m_data;
	m_part = d.m_part;
}

//-----------------------------------------------------------------------------
FEElementData& FEElementData::operator = (const FEElementData& d)
{
	SetName(d.GetName());
	SetMesh(d.GetMesh());
	m_data = d.m_data;
	m_part = d.m_part;
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
void FEElementData::Save(OArchive& ar)
{
	int NE = GetMesh()->Elements();
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE, (int)m_dataType);
	ar.WriteChunk(CID_MESH_DATA_SCALE, m_scale);

	// Parts must be saved first so that the number of elements in the part can be
	// queried before the data is read during the load operation.
	ar.BeginChunk(CID_MESH_DATA_PART);
	{
		m_part->Save(ar);
	}
	ar.EndChunk();

	ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], NE);

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
		else if (nid == CID_MESH_DATA_SCALE)
		{
			ar.read(m_scale);
		}
		else if (nid == CID_MESH_DATA_PART)
		{
			m_part = new FEPart(po);
			m_part->Load(ar);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			int NE = m_part->size();
			m_data.resize(NE);
			ar.read(&m_data[0], NE);
		}

		ar.CloseChunk();
	}
}
