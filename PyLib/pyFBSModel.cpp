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

#include "PyFBSUI.h"

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <GeomLib/GPrimitive.h>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/Commands.h>
#include <FEBioLink/FEBioClass.h>
#include <MeshTools/GModel.h>
#include <FEBio/FEBioExport4.h>

CModelDocument* GetActiveDocument()
{
	CMainWindow* wnd = FBS::getMainWindow();
	CModelDocument* doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
	return doc;
}

FSModel* GetActiveModel()
{
	CModelDocument* doc = GetActiveDocument();
	return doc->GetFSModel();
}

GModel* GetActiveGModel()
{
	CModelDocument* doc = GetActiveDocument();
	return doc->GetGModel();
}

// This function will add an object to the currently active model
void AddObjectToModel(GObject* po)
{
	GModel* gm = GetActiveGModel();
	gm->AddObject(po);
}

// Create a box primitve
GObject* CreateBox(std::string& name, vec3d pos, double width, double height, double depth)
{
	// create the box
	GBox* gbox = new GBox();
	gbox->SetName(name);
	gbox->SetFloatValue(GBox::WIDTH, width);
	gbox->SetFloatValue(GBox::HEIGHT, height);
	gbox->SetFloatValue(GBox::DEPTH, depth);
	gbox->Update();
	gbox->GetTransform().SetPosition(pos);

	// add it to the model
	AddObjectToModel(gbox);

	return gbox;
}

GMaterial* AddMaterial(std::string& name, std::string& type)
{
	FSModel* fem = GetActiveModel();
	FSMaterial* pm = FEBio::CreateMaterial(type, fem);
	if (pm)
	{
		GMaterial* gm = new GMaterial;
		gm->SetName(name);
		gm->SetMaterialProperties(pm);
		fem->AddMaterial(gm);
		return gm;
	}
	return nullptr;
}

FSStep* AddStep(std::string& name)
{
	FSModel* fem = GetActiveModel();
	FSStep* step = FEBio::CreateStep("solid", fem);
	if (step) fem->AddStep(step);
	return step;
}

bool ExportFEB(std::string& fileName)
{
	CModelDocument* doc = GetActiveDocument();
	FEBioExport4 feb(doc->GetProject());
	return feb.Write(fileName.c_str());
}

// Initializes the fbs.mdl module
void init_FBSModel(pybind11::module& m)
{
	pybind11::module mdl = m.def_submodule("mdl", "Module used to interact with an FEBio model.");

	mdl.def("CreateBox", CreateBox);
	mdl.def("AddMaterial", AddMaterial);
	mdl.def("AddStep", AddStep);
	mdl.def("ExportFEB", ExportFEB);

	pybind11::class_<FSStep, std::unique_ptr<FSStep, pybind11::nodelete>>(mdl, "FSStep");

	pybind11::class_<GMaterial, std::unique_ptr<GMaterial, pybind11::nodelete>>(mdl, "GMaterial")
		.def("set", [](GMaterial* gm, std::string& paramName, float value) {
			FSMaterial* pm = gm->GetMaterialProperties();
			if (pm) {
				Param* p = pm->GetParam(paramName.c_str());
				if (p) p->SetFloatValue(value);
			}
		});

	pybind11::class_<GObject, std::unique_ptr<GObject, pybind11::nodelete>>(mdl, "GObject")
		.def("AssignMaterial", [](GObject* po, GMaterial* pm) {
			for (int i = 0; i < po->Parts(); ++i) po->Part(i)->SetMaterialID(pm->GetID());
		})
		.def("BuildMesh", [](GObject* po) {
			po->BuildMesh();
			});

}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
