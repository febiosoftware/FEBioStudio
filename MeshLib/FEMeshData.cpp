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
#include "FEMeshData.h"
#include "FEItemListBuilder.h"


FEMeshData::FEMeshData(FEMeshData::DATA_CLASS dataClass)
	: m_dataClass(dataClass), m_pMesh(0), m_dataType(DATA_TYPE::DATA_SCALAR)
{
	m_dataFmt = DATA_ITEM;
	m_itemSize = 0;

	switch (m_dataClass)
	{
	case NODE_DATA   : SetMeshItemType(MESH_ITEM_FLAGS::FE_NODE_FLAG); break;
	case SURFACE_DATA: SetMeshItemType(MESH_ITEM_FLAGS::FE_FACE_FLAG); break;
	case ELEMENT_DATA: SetMeshItemType(MESH_ITEM_FLAGS::FE_ELEM_FLAG); break;
	case PART_DATA   : SetMeshItemType(MESH_ITEM_FLAGS::FE_PART_FLAG); break;
	default:
		assert(false);
	}
}

FEMeshData::~FEMeshData()
{
}

FEMeshData::DATA_CLASS FEMeshData::GetDataClass() const
{
	return m_dataClass;
}

DATA_TYPE FEMeshData::GetDataType() const
{
	return m_dataType;
}

DATA_FORMAT FEMeshData::GetDataFormat() const
{
	return m_dataFmt;
}

int FEMeshData::DataSize() const
{
	return (int)m_data.size();
}

// nr of data items
int FEMeshData::DataItems() const
{
	assert(m_itemSize > 0);
	if (m_itemSize == 0) return 0;
	return (int)m_data.size() / m_itemSize;
}

int FEMeshData::ItemSize() const
{
	return m_itemSize;
}

void FEMeshData::SetDataFormat(DATA_FORMAT dataFormat)
{
	m_dataFmt = dataFormat;
}

void FEMeshData::SetDataType(DATA_TYPE dataType)
{
	assert(m_itemSize == 0);
	m_dataType = dataType;
	switch (dataType)
	{
	case DATA_SCALAR: m_itemSize = 1; break;
	case DATA_VEC3 : m_itemSize = 3; break;
	case DATA_MAT3 : m_itemSize = 9; break;
	}
}

FSMesh* FEMeshData::GetMesh() const
{
	return m_pMesh;
}

void FEMeshData::SetMesh(FSMesh* mesh)
{
	m_pMesh = mesh;
}
