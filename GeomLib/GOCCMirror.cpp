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
#include "GOCCMirror.h"

#ifdef HAS_OCC
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepLib.hxx>
#include <Precision.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

static gp_Ax2 MirrorPlane(int plane)
{
	switch (plane)
	{
	case 1: return gp_Ax2(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0)); // YZ
	case 2: return gp_Ax2(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 1.0, 0.0)); // XZ
	default:
		return gp_Ax2(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0)); // XY
	}
}
#endif

GOCCMirror::GOCCMirror() : GOCCModifier("Mirror")
{
	AddChoiceParam(0, "plane", "Plane")->SetEnumNames("XY\0YZ\0XZ\0");
}

GOCCObject* GOCCMirror::Apply(GOCCObject* po)
{
#ifdef HAS_OCC
	Reset();
	if (po == nullptr) return nullptr;

	TopoDS_Shape shape = po->GetShape();
	if (shape.IsNull()) return nullptr;

	gp_Trsf mirrorTransform;
	mirrorTransform.SetMirror(MirrorPlane(GetIntValue(0)));

	BRepBuilderAPI_Transform transform(shape, mirrorTransform, Standard_True);
	transform.Build();
	if (!transform.IsDone()) return nullptr;

	TopoDS_Shape mirroredShape = transform.Shape();
	if (mirroredShape.IsNull()) return nullptr;

	BRepLib::SameParameter(mirroredShape, Precision::Confusion(), Standard_True);

	BRepCheck_Analyzer check(mirroredShape);
	if (!check.IsValid()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(mirroredShape);
	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}
