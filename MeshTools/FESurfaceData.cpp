#include "stdafx.h"
#include "FESurfaceData.h"
#include <MeshLib/FEMesh.h>

FESurfaceData::FESurfaceData() : FEMeshData(FEMeshData::SURFACE_DATA), m_surface(nullptr)
{
}

FESurfaceData::~FESurfaceData()
{
	delete m_surface;
}

FESurfaceData::FESurfaceData(const FESurfaceData& d) : FEMeshData(FEMeshData::SURFACE_DATA)
{
	SetName(d.GetName());
	SetMesh(d.GetMesh());
	m_data = d.m_data;
	m_surface = d.m_surface;
}

void FESurfaceData::operator = (const FESurfaceData& d)
{
	SetName(d.GetName());
	SetMesh(d.GetMesh());
	m_data = d.m_data;
	m_surface = d.m_surface;
}

void FESurfaceData::Create(FEMesh* mesh, FESurface* surface, FEMeshData::DATA_TYPE dataType)
{
	SetMesh(mesh);
	m_surface = surface;
	m_dataType = dataType;
	m_data.assign(surface->size(), 0.0);
}

double& FESurfaceData::operator [] (int index)
{
	return m_data[index];
}

vector<double>* FESurfaceData::getData()
{
	return &m_data;
}

void FESurfaceData::Save(OArchive& ar)
{
	int NF = m_surface->size();
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TAGS, (int) m_dataType);

	// Surface must be saved first so that the number of facets in the surface can be
	// queried before the data is read during the load operation.
	ar.BeginChunk(CID_MESH_SURFACE_DATA_SURFACE);
	{
		m_surface->Save(ar);
	}
	ar.EndChunk();

	ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], NF);
}

void FESurfaceData::Load(IArchive& ar)
{
	while (IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_MESH_DATA_NAME)
		{
			char szname[256];
			ar.read(szname);
			SetName(szname);
		}
		else if(nid == CID_MESH_DATA_TAGS)
		{
			int dType;
			ar.read(dType);
			m_dataType = (FEMeshData::DATA_TYPE) dType;
		}
		else if(nid == CID_MESH_SURFACE_DATA_SURFACE)
		{
			m_surface = new FESurface(nullptr);
			m_surface->Load(ar);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			int n = m_surface->size();
			m_data.resize(n);
			ar.read(&m_data[0], n);
		}

		ar.CloseChunk();
	}
}
