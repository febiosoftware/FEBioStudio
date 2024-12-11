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
#include "PRVObjectImport.h"
#include <FSCore/Serializable.h>
#include "PRVObjectFormat.h"
#include <FEMLib/FSProject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <FEMLib/GDiscreteObject.h>

//-----------------------------------------------------------------------------
PRVObjectImport::PRVObjectImport(FSProject& prj) : FSFileImport(prj)
{
}

//-----------------------------------------------------------------------------
bool PRVObjectImport::Load(const char* szfile)
{
	if (Open(szfile, "rb") == false) return false;

	// open archive         P V O
	if (m_ar.Open(m_fp, 0x0050564F, szfile) == false) return false;

	bool ret = LoadObjects(m_ar, m_prj);

	Close();

	return ret;
}

void PRVObjectImport::Close()
{
	m_ar.Close();

	FileReader::Close();
}

bool PRVObjectImport::LoadObjects(IArchive& ar, FSProject& prj)
{
	GModel& model = prj.GetFSModel().GetModel();
	m_objList.clear();
	IArchive::IOResult nret = IArchive::IO_OK;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVO_VERSION)
		{
			unsigned int version = 0;
			nret = ar.read(version);
			if (nret != IArchive::IO_OK) return errf("Error reading version number");

			if (version != PVO_VERSION_NUMBER) return false;

			ar.SetVersion(version);
		}
		else if (nid == PVO_OBJECT)
		{
			GObject* po = LoadObject(ar, prj);
			if (po == 0) return false;

			model.AddObject(po);
			po = 0;
		}
		else if (nid == PVO_DISCRETE_OBJECT)
		{
			GDiscreteObject* po = LoadDiscreteObject(ar, prj);
			if (po == 0) return false;
			
			model.AddDiscreteObject(po);
			po = 0;			
		}
		ar.CloseChunk();
	}

	return true;
}

GObject* PRVObjectImport::LoadObject(IArchive& ar, FSProject& prj)
{
	GObject* po = 0;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVO_OBJECT_TYPE)
		{
			int ntype = -1;
			if (ar.read(ntype) != IArchive::IO_OK) return 0;
			po = BuildObject(ntype);
			if (po == 0) return 0;
		}
		else if (nid == PVO_OBJECT_DATA)
		{
			if (po == 0) return 0;
			po->Load(ar);

			// The problem is that the ID of the object items may already be in use.
			// Therefore we need to reinded all the items.
			// The old IDs will be stored in the tags in case we need to figure out dependencies
			ReindexObject(po);

			// add it to the list
			m_objList.push_back(po);
		}

		ar.CloseChunk();
	}

	return po;
}

GDiscreteObject* PRVObjectImport::LoadDiscreteObject(IArchive& ar, FSProject& prj)
{
	GDiscreteObject* po = 0;
	FSModel* fem = &prj.GetFSModel();
	GModel* gm = &prj.GetFSModel().GetModel();
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVO_OBJECT_TYPE)
		{
			int ntype = -1;
			if (ar.read(ntype) != IArchive::IO_OK) return 0;

			switch (ntype)
			{
			case FE_LINEAR_SPRING_SET   : po = new GLinearSpringSet(gm); break;
			case FE_NONLINEAR_SPRING_SET: po = new GNonlinearSpringSet(gm); break;
			case FE_DISCRETE_SPRING_SET : po = new GDiscreteSpringSet(gm); break;
			}
			if (po == 0) return 0;
		}
		else if (nid == PVO_OBJECT_DATA)
		{
			if (po == 0) return 0;
			po->Load(ar);

			// This discrete object will still reference the old IDs of the objects in the pvo file.
			// Therefore we need to reindex them
			ReindexDiscreteObject(po);
		}

		ar.CloseChunk();
	}

	// convert old discrete objects to new 
	if (po)
	{
		int ntype = po->GetType();
		if (ntype == FE_LINEAR_SPRING_SET)
		{
			GDiscreteElementSet* ds = dynamic_cast<GDiscreteElementSet*>(po); assert(ds);
			GDiscreteSpringSet* pnew = new GDiscreteSpringSet(gm);
			pnew->SetName(po->GetName());
			pnew->CopyDiscreteElementSet(ds);
			FSLinearSpringMaterial* mat = new FSLinearSpringMaterial(fem);
			mat->SetSpringConstant(po->GetFloatValue(GLinearSpringSet::MP_E));
			pnew->SetMaterial(mat);
			delete po;
			po = pnew;
		}
		else if (ntype == FE_NONLINEAR_SPRING_SET)
		{
			GDiscreteElementSet* ds = dynamic_cast<GDiscreteElementSet*>(po); assert(ds);
			GDiscreteSpringSet* pnew = new GDiscreteSpringSet(gm);
			pnew->SetName(po->GetName());
			pnew->CopyDiscreteElementSet(ds);
			FSNonLinearSpringMaterial* mat = new FSNonLinearSpringMaterial(fem);
			// TODO: map F parameter
			pnew->SetMaterial(mat);
			delete po;
			po = pnew;
		}
	}

	return po;
}

void PRVObjectImport::ReindexObject(GObject* po)
{
	for (int i=0; i<po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GPart::CreateUniqueID());
	}

	for (int i = 0; i<po->Faces(); ++i)
	{
		GFace* pg = po->Face(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GFace::CreateUniqueID());
	}

	for (int i = 0; i<po->Edges(); ++i)
	{
		GEdge* pg = po->Edge(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GEdge::CreateUniqueID());
	}

	for (int i = 0; i<po->Nodes(); ++i)
	{
		GNode* pg = po->Node(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GNode::CreateUniqueID());
	}
}

void PRVObjectImport::ReindexDiscreteObject(GDiscreteObject* po)
{
	// This only works for discrete element sets
	GDiscreteElementSet* ps = dynamic_cast<GDiscreteElementSet*>(po);
	assert(ps);

	int NE = ps->size();
	for (int i=0; i<NE; ++i)
	{
		GDiscreteElement& de = ps->element(i);
		int n[2] = {-1, -1};
		for (int j=0; j<2; ++j)
		{
			int nj = de.Node(j);
			for (int k=0; k<m_objList.size(); ++k)
			{
				GObject* pk = m_objList[k];

				GNode* node = pk->FindNodeFromTag(nj); assert(node);
				if (node)
				{
					n[j] = node->GetID();
				}
			}
		}
		de.SetNodes(n[0], n[1]);
	}
}
