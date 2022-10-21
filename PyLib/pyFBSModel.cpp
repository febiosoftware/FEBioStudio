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

// This function will add an object to the currently active model
void AddObjectToModel(GObject* po)
{
	auto wnd = FBS::getMainWindow();
	auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
	doc->DoCommand(new CCmdAddAndSelectObject(doc->GetGModel(), po), po->GetName());
	wnd->UpdateModel(po);
}

// Create a box primitve
GBox* CreateBox(std::string& name, vec3d pos, double width, double height, double depth)
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

// Initializes the fbs.mdl module
void init_FBSModel(pybind11::module& m)
{
	pybind11::module mdl = m.def_submodule("mdl", "Module used to interact with an FEBio model.");

	mdl.def("CreateBox", CreateBox);
}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
