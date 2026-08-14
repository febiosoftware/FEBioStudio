/*This file is part of the FEBio Studio source code and is licensed under the MIT license
 listed below.

 See Copyright-FEBio-Studio.txt for details.

 Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#include "STEPExport.h"
#include <GeomLib/GOCCObject.h>

#ifdef HAS_OCC
#include <STEPControl_Writer.hxx>
#include <TopoDS_Solid.hxx>
#endif

STEPExport::STEPExport(FSModel& fem) : m_fem(fem)
{
}

STEPExport::~STEPExport(void)
{
}

bool STEPExport::Write(const char* szfile)
{
#ifdef HAS_OCC
	// Get the model
	FSModel& fem = m_fem;
	GModel& model = fem.GetModel();
	
	// Get the first OCC geometry that is selected.
	GOCCObject* po = nullptr;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* obj = model.Object(i);
		if (obj->GetType() == GOCCOBJECT && obj->IsSelected())
		{
			po = dynamic_cast<GOCCObject*>(obj);
			break;
		}
	}
	if (po == nullptr)
	{
		// No OCC geometry selected, so we cannot export.
		return false;
	}

	// Get the shape
	TopoDS_Shape shape = po->GetShape();

	// write the shape to a STEP file
	STEPControl_Writer writer;
	writer.Transfer(shape, STEPControl_AsIs);
	IFSelect_ReturnStatus ret = writer.Write(szfile);

	return (ret == IFSelect_RetDone);
#else
	return false;
#endif
}
