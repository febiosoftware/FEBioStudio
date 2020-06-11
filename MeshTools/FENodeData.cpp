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
