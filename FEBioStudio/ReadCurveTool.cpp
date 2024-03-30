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
#include "ReadCurveTool.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <GeomLib/GMeshObject.h>
#include <MeshLib/FECurveMesh.h>
#include <GeomLib/GCurveMeshObject.h>
#include "ModelDocument.h"
#include "MainWindow.h"

CReadCurveTool::CReadCurveTool(CMainWindow* wnd) : CBasicTool(wnd, "Read Curve", HAS_APPLY_BUTTON)
{
	addResourceProperty(&m_file, "File");
	addBoolProperty(&m_bcheck, "Close curve");

	SetApplyButtonText("Load");
}

bool CReadCurveTool::OnApply()
{
	QString fileName = m_file;
	if (fileName.isEmpty())
	{
		SetErrorString("No filename specified");
		return false;
	}
	else
	{
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();

		FILE* fp = fopen(szfile, "rt");
		if (fp == 0)
		{
			SetErrorString("Invalid filename specified");
			return false;
		}

		vector<vec3d> pt;
		double x, y, z;
		while (feof(fp) == 0)
		{
			int n = fscanf(fp, "%lg %lg %lg", &x, &y, &z);
			if (n == 3)
			{
				pt.push_back(vec3d(x,y,z));
			}
		}
		fclose(fp);

		if (pt.empty())
		{
			SetErrorString("File did not contain any points");
			return false;
		}
		else
		{
			static int n = 1;
			char sz[256] = {0};
			snprintf(sz, sizeof sz, "CurveObject%d", n++);

			FECurveMesh* pm = new FECurveMesh;
			int nodes = (int) pt.size();
			pm->Create(nodes, m_bcheck);
			for (int i=0; i<nodes; ++i) pm->Node(i).r = pt[i];

			GCurveMeshObject* po = new GCurveMeshObject(pm);
			po->SetName(sz);

			CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
			doc->AddObject(po);
		}
	}

	return true;
}
