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
#include "BREPImport.h"
#include <GeomLib/GOCCObject.h>
#include <GeomLib/GModel.h>
#ifdef HAS_OCC
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <STEPControl_Reader.hxx>
#include <IGESControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#endif

BREPImport::BREPImport(FSProject& prj) : FSFileImport(prj)
{
}

BREPImport::~BREPImport()
{
}

bool BREPImport::Load(const char* szfile)
{
#ifdef HAS_OCC
	FSProject& prj = m_prj;
	SetFileName(szfile);
	TopoDS_Shape aShape;
	BRep_Builder aBuilder;
	Standard_Boolean result = BRepTools::Read(aShape, szfile, aBuilder);
	if (result)
	{
		GOCCObject* occ = new GOCCObject;
		occ->SetShape(aShape);

		char szname[256];
		FileTitle(szname);
		occ->SetName(szname);

		GModel& mdl = prj.GetFSModel().GetModel();
		mdl.AddObject(occ);
		return true;
	}
	else return false;
#else
	return false;
#endif
}

//=============================================================================
IGESImport::IGESImport(FSProject& prj) : FSFileImport(prj)
{
}

IGESImport::~IGESImport()
{
}

bool IGESImport::Load( const char* szfile)
{
#ifdef HAS_OCC
	FSProject& prj = m_prj;

	SetFileName(szfile);

	IGESControl_Reader Reader;
	int status = Reader.ReadFile(szfile);

	if (status == IFSelect_RetDone)
	{
		Reader.TransferRoots();
		TopoDS_Shape aShape = Reader.OneShape();

		if (aShape.IsNull()) return error("Shape is NULL.");

		GOCCObject* occ = new GOCCObject;
		occ->SetShape(aShape);

		char szname[256];
		FileTitle(szname);
		occ->SetName(szname);

		GModel& mdl = prj.GetFSModel().GetModel();
		mdl.AddObject(occ);
	}
	else return false;

	return true; 
#else
	return false;
#endif
}
