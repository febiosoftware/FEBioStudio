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
#include "ExtrudeMapTool.h"
#include <MeshTools/FEExtrudeFaces.h>
#include "ModelDocument.h"
#include "MainWindow.h"
#include <GeomLib/GMeshObject.h>
#include <QFile>
#include <MeshLib/FENodeData.h>
#include "DlgImportData.h"

CExtrudeMapTool::CExtrudeMapTool(CMainWindow* wnd) : CBasicTool(wnd, "Extrude map", HAS_APPLY_BUTTON)
{
	m_D = 1;
	m_nsegs = 1;
	m_useLocalNormal = true;
	m_meshBias = 1;
	m_symmetricBias = false;
	addResourceProperty(&m_filename, "map file:");
	addDoubleProperty(&m_D, "distance");
	addIntProperty(&m_nsegs, "segments");
	addBoolProperty(&m_useLocalNormal, "use local normal");
	addDoubleProperty(&m_meshBias, "mesh bias");
	addBoolProperty(&m_symmetricBias, "symmetric mesh bias");
}

bool CExtrudeMapTool::OnApply()
{
	GMeshObject* po = dynamic_cast<GMeshObject*>(GetActiveObject());
	if (po == nullptr)
	{
		SetErrorString("You need to select an Editable Mesh object.");
		return false;
	}

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr)
	{
		SetErrorString("The selected object has no mesh.");
		return false;
	}

	int n = pm->CountSelectedFaces();
	if (n == 0)
	{
		SetErrorString("No faces are selected. Cannot apply tool.");
		return false;
	}

	vector<int> node;
	vector<double> data[3];

	QFile mapFile(m_filename);
	if (!mapFile.open(QIODeviceBase::Text | QIODeviceBase::ReadOnly))
	{
		SetErrorString("mapFile.errorString()");
		return false;
	}

	QTextStream in(&mapFile);
	QString txt = in.readAll();
	mapFile.close();

	CDlgImportData dlg(txt, DataType::DOUBLE);
	if (!dlg.exec()) return true;

	QList<QStringList> values = dlg.GetValues();
	for (QStringList& fields : values)
	{
		int n = fields.count();
		if ((n != 2) && (n != 4))
		{
			SetErrorString("Invalid data in map file.");
			return false;
		}
		node.push_back(fields[0].toInt());
		for (int i = 0; i < n - 1; ++i)
		{
			double di = fields.at(i + 1).toDouble();
			data[i].push_back(di);
		}
	}

	int fields = 1;
	if ((data[0].size() == data[1].size()) &&
		(data[0].size() == data[2].size())) fields = 3;

	// create a nodeset
	FSNodeSet nodeSet(po);
	for (int i = 0; i < node.size(); ++i)
	{
		int nid = node[i];
		int m = pm->NodeIndexFromID(nid);
		nodeSet.add(m);
	}

	// create a data map
	FENodeData map(po);
	if (fields == 1)
	{
		map.Create(&nodeSet, 0, DATA_SCALAR);
		for (int i = 0; i < node.size(); ++i)
		{
			map.set(i, data[0][i]);
		}
	}
	else
	{
		map.Create(&nodeSet, 0, DATA_VEC3);
		for (int i = 0; i < node.size(); ++i)
		{
			map.set(i, vec3d(data[0][i], data[1][i], data[2][i]));
		}
	}

	FEExtrudeFaces mod;
	mod.SetExtrusionDistance(m_D);
	mod.SetSegments(m_nsegs);
	mod.SetUseNormalLocal(m_useLocalNormal);
	mod.SetMeshBiasFactor(m_meshBias);
	mod.SetSymmetricBias(m_symmetricBias);
	mod.SetNodalMap(&map);

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	bool b = doc->ApplyFEModifier(mod, GetActiveObject());
	if (!b) SetErrorString(QString::fromStdString(mod.GetErrorString()));
	updateUi();

	return b;
}
