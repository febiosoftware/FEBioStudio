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
#include "rhiDocument.h"
#include <MeshIO/STLimport.h>
#include <MeshIO/PLYImport.h>
#include <FEMLib/FSProject.h>
#include <FEMLib/FSModel.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <QFileInfo>

rhiDocument::rhiDocument(CMainWindow* wnd) : CDocument(wnd)
{

}

bool rhiDocument::ImportFile(const QString& fileName)
{
	QFileInfo fi(fileName);

	QString ext = fi.suffix();

	// read STL from file.
	std::string sfile = fileName.toStdString();
	FSProject dummy;

	if (ext == "stl")
	{
		STLimport stl(dummy);
		if (stl.Load(sfile.c_str()) == false) return false;
	}
	else if (ext == "ply")
	{
		PLYImport ply(dummy);
		if (ply.Load(sfile.c_str()) == false) return false;
	}

	GModel& gm = dummy.GetFSModel().GetModel();
	if (gm.Objects() > 0)
	{
		GObject * po = gm.Object(0);
		GLMesh* pm = po->GetRenderMesh();

		GLMesh* copy = new GLMesh(*pm);
		m_obj.reset(copy);

		SetDocTitle(fi.baseName().toStdString());

		return true;
	}

	return false;
}
