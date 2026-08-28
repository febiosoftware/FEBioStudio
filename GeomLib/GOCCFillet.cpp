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
#include "GOCCFillet.h"

#ifdef HAS_OCC
#include <BRepCheck_Analyzer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepLib.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#endif

GOCCFillet::GOCCFillet() : GOCCModifier("Fillet")
{
	AddDoubleParam(0.0, "radius", "Radius");
}

GOCCObject* GOCCFillet::Apply(GOCCObject* po)
{
#ifdef HAS_OCC
	Reset();
	if (po == nullptr) return nullptr;

	std::vector<int> edgeIds;
	for (int i = 0; i < po->Edges(); ++i)
	{
		GEdge* edge = po->Edge(i);
		if (edge && edge->IsSelected()) edgeIds.push_back(i);
	}
	if (edgeIds.empty()) return nullptr;

	double radius = GetFloatValue(0);
	if (radius <= 0.0) return nullptr;

	TopoDS_Shape shape = po->GetShape();
	if (shape.IsNull()) return nullptr;

	TopTools_IndexedMapOfShape edgeMap;
	TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
	if (edgeMap.Extent() == 0) return nullptr;

	BRepFilletAPI_MakeFillet fillet(shape);
	for (int edgeId : edgeIds)
	{
		if ((edgeId < 0) || (edgeId >= edgeMap.Extent())) return nullptr;

		const TopoDS_Edge& edge = TopoDS::Edge(edgeMap(edgeId + 1));
		fillet.Add(radius, edge);
	}

	fillet.Build();
	if (!fillet.IsDone()) return nullptr;

	TopoDS_Shape filletedShape = fillet.Shape();
	if (filletedShape.IsNull()) return nullptr;

	BRepLib::SameParameter(filletedShape, Precision::Confusion(), Standard_True);

	BRepCheck_Analyzer check(filletedShape);
	if (!check.IsValid()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(filletedShape);
	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}
