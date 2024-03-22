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
#include "FoamGeneratorTool.h"
#include "ModelDocument.h"
#include <GeomLib/GMeshObject.h>
#include <QMessageBox>

CFoamGeneratorTool::CFoamGeneratorTool(CMainWindow* wnd) : CBasicTool(wnd, "Foam generator", HAS_APPLY_BUTTON)
{
	addIntProperty(&m_foam.m_nx, "nx");
	addIntProperty(&m_foam.m_ny, "ny");
	addIntProperty(&m_foam.m_nz, "nz");
	addIntProperty(&m_foam.m_nseed, "seeds");
	addIntProperty(&m_foam.m_nsmooth, "smooth iters.");
	addDoubleProperty(&m_foam.m_ref, "ref");
}

bool CFoamGeneratorTool::OnApply()
{
	static int n = 1;

	// get the document
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return false;

	FSModel* ps = doc->GetFSModel();

	// create the foam mesh
	FSMesh* pm = m_foam.Create();
	if (pm)
	{
		char szname[256] = { 0 };

		// get the active object
		GMeshObject* pa = dynamic_cast<GMeshObject*>(doc->GetActiveObject());

		if (pa) strcpy(szname, pa->GetName().c_str());
		else
		{
			snprintf(szname, sizeof szname, "Foam%02d", n++);
		}

		// if this is the last selected object, delete it so we can replace it
		if (pa && (pa == m_pfo)) doc->DeleteObject(pa);

		// Create a new GMeshObject
		GMeshObject* po = new GMeshObject(pm);
		po->SetName(szname);
		m_pfo = po;

		// add the new object to the model
		doc->AddObject(po);
	}
	else 
	{
		SetErrorString("Mesh generation has failed.");
		return false;
	}

	return true;
}
