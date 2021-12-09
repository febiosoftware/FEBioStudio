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
#include "PRVObjectExport.h"
#include <FSCore/Archive.h>
#include "PRVObjectFormat.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>

PRVObjectExport::PRVObjectExport(FEProject& prj) : FEFileExport(prj)
{
	m_selectedObjectsOnly = true;
	m_exportDiscrete = true;
}

bool PRVObjectExport::Write(const char* szfile)
{
	OArchive ar;
	if (ar.Create(szfile, 0x0050564F) == false) return false;

	bool ret = true;
	try
	{
		ret = SaveObjects(ar, m_prj);
	}
	catch (...)
	{
		ret = false;
	}

	ar.Close();

	return ret;
}

bool PRVObjectExport::SaveObjects(OArchive& ar, FEProject& prj)
{
	// save version info
	unsigned int version = PVO_VERSION_NUMBER;
	ar.WriteChunk(PVO_VERSION, version);

	// write objects
	GModel& model = prj.GetFSModel().GetModel();
	int objects = model.Objects();
	for (int i = 0; i<objects; ++i)
	{
		GObject* po = model.Object(i);
		po->m_ntag = 0;
		if ((m_selectedObjectsOnly == false) || (po->IsSelected()))
		{
			po->m_ntag = 1;
			int ntype = po->GetType();
			ar.BeginChunk(PVO_OBJECT);
			{
				ar.WriteChunk(PVO_OBJECT_TYPE, ntype);
				ar.BeginChunk(PVO_OBJECT_DATA);
				{
					po->Save(ar);
				}
				ar.EndChunk();
			}
			ar.EndChunk();
		}
	}

	// write discrete objects
	if (m_exportDiscrete)
	{
		// tag the nodes that will be exported
		int n = 0;
		GNodeIterator it(model);
		while (it.isValid())
		{
			GBaseObject* po = it->Object();
			if (po->m_ntag == 1) it->m_ntag = n++; else it->m_ntag = -1;
			++it;
		}

		// we only export discrete sets that are connected to objects that are being exported
		int disco = model.DiscreteObjects();
		for (int i=0; i<disco; ++i)
		{
			GDiscreteElementSet* set = dynamic_cast<GDiscreteElementSet*>(model.DiscreteObject(i));
			if (set)
			{
				// see if all elements are connected to objects that are being exported
				bool bexport = true;
				int NE = set->size();
				for (int j=0; j<NE; ++j)
				{
					GDiscreteElement& de = set->element(j);
					GNode* n0 = model.FindNode(de.Node(0)); assert(n0);
					GNode* n1 = model.FindNode(de.Node(1));

					if ((n0->m_ntag == -1) || (n1->m_ntag == -1))
					{
						bexport = false;
						break;
					}
				}

				if (bexport)
				{
					int ntype = set->GetType();
					ar.BeginChunk(PVO_DISCRETE_OBJECT);
					{
						ar.WriteChunk(PVO_OBJECT_TYPE, ntype);
						ar.BeginChunk(PVO_OBJECT_DATA);
						{
							set->Save(ar);
						}
						ar.EndChunk();
					}
					ar.EndChunk();
				}				
			}
		}
	}

	return true;
}
