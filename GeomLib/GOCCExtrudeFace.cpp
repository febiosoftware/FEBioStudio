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
#include "GOCCExtrudeFace.h"

#ifdef HAS_OCC
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepLib.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <vector>

static gp_Dir FaceNormal(const TopoDS_Face& face)
{
	BRepAdaptor_Surface surface(face);
	gp_Dir normal = surface.Plane().Axis().Direction();
	if (face.Orientation() == TopAbs_REVERSED) normal.Reverse();
	return normal;
}
#endif

GOCCExtrudeFace::GOCCExtrudeFace() : GOCCModifier("Extrude Face")
{
	AddDoubleParam(0.1, "distance", "Distance");
}

GOCCObject* GOCCExtrudeFace::Apply(GOCCObject* po)
{
#ifdef HAS_OCC
	Reset();
	if (po == nullptr) return nullptr;

	std::vector<int> faceIds;
	for (int i = 0; i < po->Faces(); ++i)
	{
		GFace* face = po->Face(i);
		if (face && face->IsSelected()) faceIds.push_back(i);
	}
	if (faceIds.empty()) return nullptr;

	double distance = GetFloatValue(0);
	if (distance <= 0.0) return nullptr;

	TopoDS_Shape resultShape = po->GetShape();
	if (resultShape.IsNull()) return nullptr;

	TopTools_IndexedMapOfShape faceMap;
	TopExp::MapShapes(resultShape, TopAbs_FACE, faceMap);
	if (faceMap.Extent() == 0) return nullptr;

	for (int faceId : faceIds)
	{
		if ((faceId < 0) || (faceId >= faceMap.Extent())) return nullptr;

		const TopoDS_Face& face = TopoDS::Face(faceMap(faceId + 1));

		BRepAdaptor_Surface surface(face);
		if (surface.GetType() != GeomAbs_Plane) return nullptr;

		gp_Vec extrudeVector(FaceNormal(face));
		extrudeVector *= distance;

		BRepPrimAPI_MakePrism prism(face, extrudeVector, Standard_True, Standard_True);
		prism.Build();
		if (!prism.IsDone()) return nullptr;

		TopoDS_Shape prismShape = prism.Shape();
		if (prismShape.IsNull()) return nullptr;

		BRepAlgoAPI_Fuse fuse(resultShape, prismShape);
		fuse.Build();
		if (!fuse.IsDone()) return nullptr;

		resultShape = fuse.Shape();
		if (resultShape.IsNull()) return nullptr;
	}

	BRepLib::SameParameter(resultShape, Precision::Confusion(), Standard_True);

	BRepCheck_Analyzer check(resultShape);
	if (!check.IsValid()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(resultShape);
	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}
