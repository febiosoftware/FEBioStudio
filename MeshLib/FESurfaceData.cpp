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

#include "FESurfaceData.h"
#include <MeshLib/FEMesh.h>
#include <GeomLib/GObject.h>

FESurfaceData::FESurfaceData(FSMesh* mesh) : FEMeshData(FEMeshData::SURFACE_DATA)
{
	SetMesh(mesh);
}

FESurfaceData::FESurfaceData(FSMesh* mesh, FEMeshData::DATA_TYPE dataType, FEMeshData::DATA_FORMAT dataFormat) : FEMeshData(FEMeshData::SURFACE_DATA)
{
	SetMesh(mesh);
	m_dataType = dataType;
	m_dataFmt = dataFormat;
	m_dataSize = 0;
	m_maxNodesPerFacet = 0;
}

FESurfaceData::~FESurfaceData()
{
	
}

FESurfaceData::FESurfaceData(const FESurfaceData& d) : FEMeshData(FEMeshData::SURFACE_DATA) {}
void FESurfaceData::operator = (const FESurfaceData& d) {}

void FESurfaceData::Create(FSMesh* mesh, FSSurface* surface, FEMeshData::DATA_TYPE dataType, FEMeshData::DATA_FORMAT dataFormat)
{
	SetMesh(mesh);
	SetItemList(surface);
	m_dataType = dataType;
	m_dataFmt = dataFormat;

	AllocateData();
}

void FESurfaceData::SetItemList(FEItemListBuilder* surf, int n)
{
	FSHasOneItemList::SetItemList(surf);
	AllocateData();
}

void FESurfaceData::AllocateData()
{
	m_dataSize = 0;
	switch (m_dataType)
	{
	case FEMeshData::DATA_SCALAR: m_dataSize = 1; break;
	case FEMeshData::DATA_VEC3D : m_dataSize = 3; break;
	case FEMeshData::DATA_MAT3D : m_dataSize = 9; break;
	default:
		assert(false);
		return;
	}

	FSSurface* surface = GetSurface();

	m_data.clear();
	if (surface)
	{
		int bufSize = 0;
		int faces = surface->size();
		switch (m_dataFmt)
		{
		case FEMeshData::DATA_NODE:
		{
			FSNodeList* pnl = surface->BuildNodeList();
			bufSize = m_dataSize * pnl->Size();
			delete pnl;
		}
		break;
		case FEMeshData::DATA_ITEM:
			bufSize = m_dataSize * faces;
			break;
		case FEMeshData::DATA_MULT:
		{
			m_maxNodesPerFacet = 0;
			for (int i = 0; i < faces; ++i)
			{
				FSFace* pf = surface->GetFace(i);
				int nf = pf->Nodes();
				if (nf > m_maxNodesPerFacet) m_maxNodesPerFacet = nf;
			}
			bufSize = m_maxNodesPerFacet * m_dataSize;
		}
		break;
		}

		m_data.assign(bufSize, 0.0);
	}
}

void FESurfaceData::Save(OArchive& ar)
{
	const string& dataName = GetName();
	const char* szname = dataName.c_str();
	ar.WriteChunk(CID_MESH_DATA_NAME, szname);
	ar.WriteChunk(CID_MESH_DATA_TYPE, (int) m_dataType);
	ar.WriteChunk(CID_MESH_DATA_FORMAT, (int) m_dataFmt);

	// Surface must be saved first so that the number of facets in the surface can be
	// queried before the data is read during the load operation.
	FEItemListBuilder* pi = GetItemList();
	if (pi) ar.WriteChunk(CID_MESH_DATA_ITEMLIST_ID, pi->GetID());
	if (m_data.empty() == false) ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], (int) m_data.size());
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
		else if (nid == CID_MESH_DATA_FORMAT)
		{
			int dFmt;
			ar.read(dFmt);
			m_dataFmt = (FEMeshData::DATA_FORMAT)dFmt;
		}
		else if (nid == CID_MESH_DATA_ITEMLIST_ID)
		{
			int listId = -1;
			ar.read(listId);
			if (po)
			{
				FSSurface* pg = dynamic_cast<FSSurface*>(po->FindFEGroup(listId)); assert(pg);
				SetItemList(pg);
			}
		}
		else if(nid == CID_MESH_DATA_SURFACE)
		{
			// older files (pre 2.1) stored the surface on the mesh data
			FSSurface* surface = new FSSurface(po);
			surface->Load(ar);
			po->AddFESurface(surface);
			SetItemList(surface);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			FSSurface* surf = GetSurface();
			if (surf)
			{
				int n = surf->size();
				m_data.resize(n);
				ar.read(&m_data[0], n);
			}
		}

		ar.CloseChunk();
	}
}
