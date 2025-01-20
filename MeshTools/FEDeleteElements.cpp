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
#include "FEModifier.h"
#include <MeshLib/FSElementData.h>

FEDeleteElements::FEDeleteElements() : FEModifier("delete elements")
{

}

FSMesh* FEDeleteElements::Apply(FSMesh* pm)
{
	const int TAG = 1;
	int NE0 = pm->Elements();
	int NN0 = pm->Nodes();
	int nsel = 0;
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (!el.IsSelected()) {
			el.m_ntag = TAG; nsel++;
		}
		else el.m_ntag = 0;
	}
	if (nsel == 0)
	{
		SetError("No elements selected.");
		return nullptr;
	}

	pm->TagAllNodes(TAG);
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag != TAG)
		{
			int nn = el.Nodes();
			for (int j = 0; j < nn; ++j) pm->Node(el.m_node[j]).m_ntag = 0;
		}
	}
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == TAG)
		{
			int nn = el.Nodes();
			for (int j = 0; j < nn; ++j) pm->Node(el.m_node[j]).m_ntag = TAG;
		}
	}
	for (int i = 0; i < NN0; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsRequired()) node.m_ntag = TAG;
	}

	// count elements
	int NE1 = 0;
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == TAG) el.m_ntag = NE1++;
		else el.m_ntag = -1;
	}

	// allocate nodes
	int NN1 = 0;
	for (int i = 0; i < NN0; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag == TAG) node.m_ntag = NN1++;
		else node.m_ntag = -1;
	}

	// create new mesh
	FSMesh* newMesh = new FSMesh;
	newMesh->Create(NN1, NE1);

	for (int i = 0; i < NN0; ++i)
	{
		FSNode& ns = pm->Node(i);
		if (ns.m_ntag >= 0)
		{
			FSNode& nd = newMesh->Node(ns.m_ntag);
			nd = ns;
		}
	}

	for (int i = 0; i < NE0; ++i)
	{
		FSElement& es = pm->Element(i);
		if (es.m_ntag >= 0)
		{
			FSElement& ed = newMesh->Element(es.m_ntag);
			ed.SetType(es.Type());
			ed.m_nid = es.m_nid;
			ed.m_gid = es.m_gid;
			int nn = es.Nodes();
			for (int j = 0; j < nn; ++j)
			{
				ed.m_node[j] = pm->Node(es.m_node[j]).m_ntag;
				assert(ed.m_node[j] >= 0);
			}
		}
	}

	newMesh->RebuildMesh();

	int N = std::max(pm->CountNodePartitions(), newMesh->CountNodePartitions());
	for (int i = 0; i < NN1; ++i)
	{
		FSNode& nd = newMesh->Node(i);
		if (nd.m_gid >= 0) nd.m_gid += N;
	}
	for (int i = 0; i < NN0; ++i)
	{
		FSNode& ns = pm->Node(i);
		if ((ns.m_gid >= 0) && (ns.m_ntag >= 0))
		{
			FSNode& nd = newMesh->Node(ns.m_ntag);
			nd.m_gid = ns.m_gid;
		}
	}
	newMesh->UpdateNodePartitions();

	for (int i = 0; i < NE0; ++i)
	{
		FSElement& es = pm->Element(i);
		if (es.m_ntag >= 0)
		{
			FSElement& ed = newMesh->Element(es.m_ntag);
			ed.m_ntag = i;
		}
	}

	// copy the named selections
	for (int i = 0; i < pm->FENodeSets(); ++i)
	{
		FSNodeSet* ps = pm->GetFENodeSet(i);
		FSNodeSet* pd = new FSNodeSet(newMesh);
		pd->SetName(ps->GetName());
		std::vector<int> src = ps->CopyItems();
		std::vector<int> nodeList;
		for (int j = 0; j < src.size(); ++j)
		{
			FSNode& node = pm->Node(src[j]);
			if (node.m_ntag >= 0) nodeList.push_back(node.m_ntag);
		}
		pd->add(nodeList);
		
		newMesh->AddFENodeSet(pd);
	}

	for (int i = 0; i < pm->FEElemSets(); ++i)
	{
		FSElemSet* ps = pm->GetFEElemSet(i);
		FSElemSet* pd = new FSElemSet(newMesh);
		pd->SetName(ps->GetName());
		std::vector<int> src = ps->CopyItems();
		std::vector<int> elemList;
		for (int j = 0; j < src.size(); ++j)
		{
			FSElement& el = pm->Element(src[j]);
			if (el.m_ntag >= 0) elemList.push_back(el.m_ntag);
		}
		pd->add(elemList);
		newMesh->AddFEElemSet(pd);
	}

	for (int i = 0; i < pm->FEPartSets(); ++i)
	{
		FSPartSet* ps = pm->GetFEPartSet(i);
		FSPartSet* pd = new FSPartSet(newMesh);
		pd->add(ps->CopyItems());
		pd->SetName(ps->GetName());
		newMesh->AddFEPartSet(pd);
	}

	for (int i = 0; i < pm->FESurfaces(); ++i)
	{
		FSSurface* ps = pm->GetFESurface(i);
		FSSurface* pd = new FSSurface(newMesh);
		pd->SetName(ps->GetName());
		newMesh->AddFESurface(pd);

		std::vector<int> src = ps->CopyItems();
		for (int j = 0; j < src.size(); ++j)
		{
			FSFace& face = pm->Face(src[j]);
			FSFace tmp;
			tmp.SetType((FEFaceType)face.Type());
			for (int n = 0; n < tmp.Nodes(); ++n)
			{
				tmp.n[n] = pm->Node(face.n[n]).m_ntag;
			}
			int n = newMesh->FindFaceIndex(tmp);
			if (n >= 0) pd->add(n);
		}
	}

	// copy data
	for (int i = 0; i < pm->MeshDataFields(); ++i)
	{
		FSPartData* ps = dynamic_cast<FSPartData*>(pm->GetMeshDataField(i));
		if (ps)
		{
			FSItemListBuilder* ls = ps->GetItemList();
			FSPartSet* pg = newMesh->FindFEPartSet(ls->GetName()); assert(pg);

			if (pg)
			{
				FSPartData* pd = new FSPartData(newMesh);
				pd->Create(pg, ps->GetDataType(), ps->GetDataFormat());
				pd->SetName(ps->GetName());
				newMesh->AddMeshDataField(pd);

				if (ps->GetDataFormat() == DATA_FORMAT::DATA_ITEM)
				{
					std::vector<int> tag(NE0, -1);

					FSElemList* srcList = ps->BuildElemList();
					FSElemList::Iterator it = srcList->First();
					for (int j = 0; j < srcList->Size(); ++j, ++it)
					{
						tag[it->m_lid] = j;
					}

					FSElemList* elemList = pd->BuildElemList();
					it = elemList->First();
					for (int j = 0; j < elemList->Size(); ++j, ++it)
					{
						int m = tag[it->m_pi->m_ntag]; assert(m >= 0);
						if (m >= 0)
						{
							switch (ps->GetDataType())
							{
							case DATA_TYPE::DATA_SCALAR: pd->set(j, ps->getScalar(m)); break;
							case DATA_TYPE::DATA_VEC3  : pd->set(j, ps->getVec3d(m)); break;
							case DATA_TYPE::DATA_MAT3  : pd->set(j, ps->getMat3d(m)); break;
							default:
								assert(false);
							}
						}
					}
				}
			}
		}
	}

	return newMesh;
}
