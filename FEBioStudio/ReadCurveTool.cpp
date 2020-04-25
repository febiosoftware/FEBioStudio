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
#include "Document.h"
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
			sprintf(sz, "CurveObject%d", n++);

			FECurveMesh* pm = new FECurveMesh;
			int nodes = (int) pt.size();
			pm->Create(nodes, m_bcheck);
			for (int i=0; i<nodes; ++i) pm->Node(i).r = pt[i];

			GCurveMeshObject* po = new GCurveMeshObject(pm);
			po->SetName(sz);

			CDocument* doc = GetDocument();
			doc->AddObject(po);
		}
	}

	return true;
}
