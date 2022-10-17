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

#include "stdafx.h"
#include "FESurfaceData.h"
#include <MeshLib/FEMesh.h>

FESurfaceData::FESurfaceData(FSMesh* mesh) : FEMeshData(FEMeshData::SURFACE_DATA), m_surface(nullptr)
{
	SetMesh(mesh);
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

void FESurfaceData::Create(FSMesh* mesh, FSSurface* surface, FEMeshData::DATA_TYPE dataType)
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

std::vector<double>* FESurfaceData::getData()
{
	return &m_data;
}

void FESurfaceData::Save(OArchive& ar)
{
	int NF = m_surface->size();
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE, (int) m_dataType);

	// Surface must be saved first so that the number of facets in the surface can be
	// queried before the data is read during the load operation.
	ar.BeginChunk(CID_MESH_DATA_SURFACE);
	{
		m_surface->Save(ar);
	}
	ar.EndChunk();

	ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], NF);
}

void FESurfaceData::Load(IArchive& ar)
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
		else if(nid == CID_MESH_DATA_TYPE)
		{
			int dType;
			ar.read(dType);
			m_dataType = (FEMeshData::DATA_TYPE) dType;
		}
		else if(nid == CID_MESH_DATA_SURFACE)
		{
			m_surface = new FSSurface(po);
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
