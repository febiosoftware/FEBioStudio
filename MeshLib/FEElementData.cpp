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
#include "FEElementData.h"
#include <MeshLib/FEMesh.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GGroup.h>

//-----------------------------------------------------------------------------
FEElementData::FEElementData(FSMesh* mesh) : FEMeshData(FEMeshData::ELEMENT_DATA)
{
	m_scale = 1.0;
	m_stride = 0;
	m_part = nullptr;
	SetMesh(mesh);
}

//-----------------------------------------------------------------------------
void FEElementData::Create(FSMesh* pm, FSElemSet* part, FEMeshData::DATA_TYPE dataType)
{
	SetMesh(pm);
	m_part = part;
	m_dataType = dataType;
	m_stride = ItemSize();
	m_data.assign(part->size()*m_stride, 0.0);
}

//-----------------------------------------------------------------------------
FEElementData::FEElementData(const FEElementData& d) : FEMeshData(FEMeshData::ELEMENT_DATA)
{
	SetMesh(d.GetMesh());
	SetName(d.GetName());
	m_data = d.m_data;
	m_part = d.m_part;
	m_dataType = d.m_dataType;
	m_stride = d.m_stride;
}

//-----------------------------------------------------------------------------
FEElementData& FEElementData::operator = (const FEElementData& d)
{
	SetName(d.GetName());
	SetMesh(d.GetMesh());
	m_data = d.m_data;
	m_part = d.m_part;
	m_dataType = d.m_dataType;
	m_stride = d.m_stride;
	return (*this);
}

//-----------------------------------------------------------------------------
void FEElementData::FillRandomBox(double fmin, double fmax)
{
	assert(m_dataType == DATA_SCALAR);
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
int FEElementData::ItemSize() const
{
	switch (m_dataType)
	{
	case DATA_SCALAR: return 1; break;
	case DATA_VEC3D : return 3; break;
	case DATA_MAT3D : return 9; break;
	default:
		assert(false);
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FEElementData::get(int n, double* d)
{
	assert((m_stride > 0) && (m_stride == ItemSize()));
	for (int i = 0; i < m_stride; ++i)
		d[i] = m_data[m_stride * n + i];
}

//-----------------------------------------------------------------------------
void FEElementData::set(int n, const mat3d& v)
{
	assert(m_dataType == DATA_MAT3D);
	assert(m_stride == 9);
	int m = 0;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j, ++m)
			m_data[9 * n + m] = v(i, j);
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
	assert(m_data.size() == NE * m_stride);
	ar.WriteChunk(CID_MESH_DATA_VALUES, &m_data[0], NE*m_stride);
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
			m_part = new FSElemSet(po);
			m_part->Load(ar);
		}
		else if (nid == CID_MESH_DATA_VALUES)
		{
			int NE = m_part->size();
			m_stride = ItemSize();

			int dataSize = NE * m_stride;
			m_data.resize(dataSize);
			ar.read(&m_data[0], dataSize);
		}

		ar.CloseChunk();
	}
}

//=============================================================================
FEPartData::FEPartData(FSMesh* mesh) : FEMeshData(FEMeshData::PART_DATA)
{
	SetMesh(mesh);
	m_maxElemItems = 1;
}

FEPartData::FEPartData(const FEPartData& d) : FEMeshData(FEMeshData::PART_DATA)
{

}

FEPartData& FEPartData::operator = (const FEPartData& d)
{
	return *this;
}

// create a data field
bool FEPartData::Create(const vector<int>& partList, FEMeshData::DATA_TYPE dataType, FEMeshData::DATA_FORMAT dataFmt)
{
	FSMesh* mesh = GetMesh();
	assert(mesh);
	m_data.clear();
	m_part = partList;

	int NE = mesh->Elements();
	int maxNodes = 0;
	int nsize = 0;
	for (int i = 0; i < partList.size(); ++i)
	{
		int pid = partList[i];
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = mesh->Element(i);
			if (el.m_gid == pid)
			{
				int nn = el.Nodes();
				if (nn > maxNodes) maxNodes = nn;
				nsize++;
			}
		}
	}

	m_dataFmt = dataFmt;
	if (dataFmt == DATA_ITEM)
	{
		m_maxElemItems = 1;
	}
	else if (dataFmt == DATA_MULT)
	{
		m_maxElemItems = maxNodes;
		nsize *= maxNodes;
	}

	m_data.resize(nsize);

	return true;
}

FEElemList* FEPartData::BuildElemList()
{
	FSMesh* mesh = GetMesh();
	assert(mesh);

	FEElemList* elemList = new FEElemList;
	int NE = mesh->Elements();
	for (int i = 0; i < m_part.size(); ++i)
	{
		int pid = m_part[i];
		for (int j = 0; j < NE; ++j)
		{
			FSElement& el = mesh->Element(j);
			if (el.m_gid == pid)
			{
				elemList->Add(mesh, &el, j);
			}
		}
	}
	return elemList;
}

GPartList* FEPartData::GetPartList(GModel* fem)
{
	GObject* po = GetMesh()->GetGObject();
	GPartList* partList = new GPartList(fem);
	for (int i = 0; i < m_part.size(); ++i)
	{
		GPart* pg = po->Part(m_part[i]);
		partList->add(pg->GetID());
	}
	return partList;
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
	ar.WriteChunk(CID_MESH_DATA_FORMAT, (int)m_dataFmt);
	ar.WriteChunk(CID_MESH_DATA_DPI, (int)m_maxElemItems);

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
		else if (nid == CID_MESH_DATA_FORMAT)
		{
			int fType;
			ar.read(fType);
			m_dataFmt = (FEMeshData::DATA_FORMAT) fType;
		}
		else if (nid == CID_MESH_DATA_DPI)
		{
			ar.read(m_maxElemItems);
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
