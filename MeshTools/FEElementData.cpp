/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "FEElementData.h"
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
FEElementData::FEElementData(FEMesh* mesh) : FEMeshData(FEMeshData::ELEMENT_DATA)
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
FEElementData::FEElementData(const FEElementData& d) : FEMeshData(FEMeshData::ELEMENT_DATA)
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

	int NE = m_part->size();
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

//=============================================================================
FEPartData::FEPartData(FEMesh* mesh) : FEMeshData(FEMeshData::PART_DATA)
{
	SetMesh(mesh);
}

FEPartData::FEPartData(const FEPartData& d) : FEMeshData(FEMeshData::PART_DATA)
{

}

FEPartData& FEPartData::operator = (const FEPartData& d)
{
	return *this;
}

// create a data field
bool FEPartData::Create(const vector<int>& partList, FEMeshData::DATA_TYPE dataType)
{
	FEMesh* mesh = GetMesh();
	assert(mesh);
	m_data.clear();
	m_part = partList;

	int NE = mesh->Elements();
	int nsize = 0;
	for (int i = 0; i < partList.size(); ++i)
	{
		int pid = partList[i];
		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = mesh->Element(i);
			if (el.m_gid == pid) nsize++;
		}
	}
	m_data.resize(nsize);

	return true;
}

FEElemList* FEPartData::BuildElemList()
{
	FEMesh* mesh = GetMesh();
	assert(mesh);

	FEElemList* elemList = new FEElemList;
	int NE = mesh->Elements();
	for (int i = 0; i < m_part.size(); ++i)
	{
		int pid = m_part[i];
		for (int i = 0; i < NE; ++i)
		{
			FEElement& el = mesh->Element(i);
			if (el.m_gid == pid)
			{
				elemList->Add(mesh, &el, i);
			}
		}
	}
	return elemList;
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

	// Parts must be saved first so that the number of elements in the part can be
	// queried before the data is read during the load operation.
	ar.WriteChunk(CID_MESH_DATA_PART, m_part);

	ar.WriteChunk(CID_MESH_DATA_VALUES, m_data);
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
		else if (nid == CID_MESH_DATA_PART)
		{
			ar.read(m_part);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			ar.read(m_data);
		}

		ar.CloseChunk();
	}
}
