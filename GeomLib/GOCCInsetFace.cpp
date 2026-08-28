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
#include "GOCCInsetFace.h"

#ifdef HAS_OCC
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepLib.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Tool.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <algorithm>
#include <cmath>
#include <vector>

static gp_Dir FaceNormal(const TopoDS_Face& face)
{
	BRepAdaptor_Surface surface(face);
	gp_Dir normal = surface.Plane().Axis().Direction();
	if (face.Orientation() == TopAbs_REVERSED) normal.Reverse();
	return normal;
}

static bool GetOuterWirePoints(const TopoDS_Face& face, std::vector<gp_Pnt>& points)
{
	points.clear();

	TopoDS_Wire wire = BRepTools::OuterWire(face);
	if (wire.IsNull()) return false;

	for (BRepTools_WireExplorer wireExplorer(wire, face); wireExplorer.More(); wireExplorer.Next())
	{
		TopoDS_Vertex vertex = wireExplorer.CurrentVertex();
		if (vertex.IsNull()) return false;

		points.push_back(BRep_Tool::Pnt(vertex));
	}

	return (points.size() >= 3);
}

static TopoDS_Face MakePlanarFace(const std::vector<gp_Pnt>& points, const gp_Dir& normal)
{
	if (points.size() < 3) return TopoDS_Face();

	BRepBuilderAPI_MakeWire wireBuilder;
	for (size_t i = 0; i < points.size(); ++i)
	{
		const gp_Pnt& p0 = points[i];
		const gp_Pnt& p1 = points[(i + 1) % points.size()];

		TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p0, p1);
		if (edge.IsNull()) return TopoDS_Face();

		wireBuilder.Add(edge);
	}
	if (!wireBuilder.IsDone()) return TopoDS_Face();

	BRepBuilderAPI_MakeFace faceBuilder(wireBuilder.Wire(), Standard_True);
	if (!faceBuilder.IsDone()) return TopoDS_Face();

	TopoDS_Face face = faceBuilder.Face();
	if (face.IsNull()) return TopoDS_Face();

	gp_Vec e0(points[0], points[1]);
	gp_Vec e1(points[1], points[2]);
	gp_Vec faceNormal = e0.Crossed(e1);
	if ((faceNormal.SquareMagnitude() > Precision::SquareConfusion()) && (faceNormal.Dot(gp_Vec(normal)) < 0.0))
	{
		face.Reverse();
	}

	return face;
}

struct InsetPoint2D
{
	double x;
	double y;
};

static double Cross2D(const InsetPoint2D& a, const InsetPoint2D& b)
{
	return a.x * b.y - a.y * b.x;
}

static double PolygonArea2D(const std::vector<InsetPoint2D>& points)
{
	double area = 0.0;
	for (size_t i = 0; i < points.size(); ++i)
	{
		size_t j = (i + 1) % points.size();
		area += Cross2D(points[i], points[j]);
	}
	return 0.5 * area;
}

static bool IntersectLines2D(
	const InsetPoint2D& p0,
	const InsetPoint2D& d0,
	const InsetPoint2D& p1,
	const InsetPoint2D& d1,
	InsetPoint2D& intersection)
{
	double det = Cross2D(d0, d1);
	if (std::abs(det) <= Precision::Confusion()) return false;

	InsetPoint2D dp = { p1.x - p0.x, p1.y - p0.y };
	double t = Cross2D(dp, d1) / det;

	intersection.x = p0.x + t * d0.x;
	intersection.y = p0.y + t * d0.y;

	return true;
}

static bool BuildInsetFaces(const TopoDS_Face& face, double distance, std::vector<TopoDS_Face>& insetFaces)
{
	if (distance <= 0.0) return false;

	BRepAdaptor_Surface surface(face);
	if (surface.GetType() != GeomAbs_Plane) return false;

	std::vector<gp_Pnt> outerPoints;
	if (!GetOuterWirePoints(face, outerPoints)) return false;

	gp_Dir normal = FaceNormal(face);

	gp_Pnt origin = outerPoints[0];
	gp_Vec xVec;
	for (size_t i = 0; i < outerPoints.size(); ++i)
	{
		xVec = gp_Vec(outerPoints[i], outerPoints[(i + 1) % outerPoints.size()]);
		if (xVec.SquareMagnitude() > Precision::SquareConfusion()) break;
	}
	if (xVec.SquareMagnitude() <= Precision::SquareConfusion()) return false;

	gp_Dir xDir(xVec);
	gp_Vec yVec = gp_Vec(normal).Crossed(gp_Vec(xDir));
	if (yVec.SquareMagnitude() <= Precision::SquareConfusion()) return false;
	gp_Dir yDir(yVec);

	std::vector<InsetPoint2D> outer2D;
	outer2D.reserve(outerPoints.size());
	for (const gp_Pnt& point : outerPoints)
	{
		gp_Vec r(origin, point);
		outer2D.push_back({ r.Dot(gp_Vec(xDir)), r.Dot(gp_Vec(yDir)) });
	}

	if (PolygonArea2D(outer2D) < 0.0)
	{
		std::reverse(outerPoints.begin(), outerPoints.end());
		std::reverse(outer2D.begin(), outer2D.end());
	}

	if (PolygonArea2D(outer2D) <= Precision::Confusion()) return false;

	std::vector<InsetPoint2D> linePoints(outer2D.size());
	std::vector<InsetPoint2D> lineDirs(outer2D.size());
	for (size_t i = 0; i < outer2D.size(); ++i)
	{
		size_t j = (i + 1) % outer2D.size();
		InsetPoint2D edge = { outer2D[j].x - outer2D[i].x, outer2D[j].y - outer2D[i].y };
		double length = std::sqrt(edge.x * edge.x + edge.y * edge.y);
		if (length <= Precision::Confusion()) return false;

		lineDirs[i] = { edge.x / length, edge.y / length };

		InsetPoint2D inwardNormal = { -lineDirs[i].y, lineDirs[i].x };
		linePoints[i] = {
			outer2D[i].x + distance * inwardNormal.x,
			outer2D[i].y + distance * inwardNormal.y
		};
	}

	std::vector<InsetPoint2D> inner2D(outer2D.size());
	for (size_t i = 0; i < outer2D.size(); ++i)
	{
		size_t previous = (i + outer2D.size() - 1) % outer2D.size();
		if (!IntersectLines2D(linePoints[previous], lineDirs[previous], linePoints[i], lineDirs[i], inner2D[i]))
		{
			return false;
		}
	}

	if (PolygonArea2D(inner2D) <= Precision::Confusion()) return false;

	std::vector<gp_Pnt> innerPoints;
	innerPoints.reserve(inner2D.size());
	for (const InsetPoint2D& point : inner2D)
	{
		gp_XYZ r = origin.XYZ();
		r += xDir.XYZ() * point.x;
		r += yDir.XYZ() * point.y;
		innerPoints.push_back(gp_Pnt(r));
	}

	TopoDS_Face centerFace = MakePlanarFace(innerPoints, normal);
	if (centerFace.IsNull()) return false;
	insetFaces.push_back(centerFace);

	for (size_t i = 0; i < outerPoints.size(); ++i)
	{
		size_t j = (i + 1) % outerPoints.size();

		std::vector<gp_Pnt> borderPoints;
		borderPoints.push_back(outerPoints[i]);
		borderPoints.push_back(outerPoints[j]);
		borderPoints.push_back(innerPoints[j]);
		borderPoints.push_back(innerPoints[i]);

		TopoDS_Face borderFace = MakePlanarFace(borderPoints, normal);
		if (borderFace.IsNull()) return false;

		insetFaces.push_back(borderFace);
	}

	return true;
}

static TopoDS_Shape SewFacesToShape(const std::vector<TopoDS_Face>& faces, bool makeSolid)
{
	if (faces.empty()) return TopoDS_Shape();

	const double tol = Precision::Confusion();
	BRepBuilderAPI_Sewing sewing(tol);
	for (const TopoDS_Face& face : faces)
	{
		sewing.Add(face);
	}
	sewing.Perform();

	TopoDS_Shape sewedShape = sewing.SewedShape();
	if (sewedShape.IsNull()) return TopoDS_Shape();

	if (!makeSolid) return sewedShape;

	TopoDS_Shell shell;
	if (sewedShape.ShapeType() == TopAbs_SHELL)
	{
		shell = TopoDS::Shell(sewedShape);
	}
	else
	{
		TopExp_Explorer shellExplorer(sewedShape, TopAbs_SHELL);
		if (!shellExplorer.More()) return TopoDS_Shape();
		shell = TopoDS::Shell(shellExplorer.Current());
	}

	if (shell.IsNull()) return TopoDS_Shape();

	TopoDS_Shape solid = BRepBuilderAPI_MakeSolid(shell);
	return solid;
}
#endif

GOCCInsetFace::GOCCInsetFace() : GOCCModifier("Inset Face")
{
	AddDoubleParam(0.1, "distance", "Distance");
}

GOCCObject* GOCCInsetFace::Apply(GOCCObject* po)
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

	TopoDS_Shape shape = po->GetShape();
	if (shape.IsNull()) return nullptr;

	TopTools_IndexedMapOfShape faceMap;
	TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
	if (faceMap.Extent() == 0) return nullptr;

	std::vector<bool> selectedFaces(faceMap.Extent(), false);
	for (int faceId : faceIds)
	{
		if ((faceId < 0) || (faceId >= faceMap.Extent())) return nullptr;
		selectedFaces[faceId] = true;
	}

	std::vector<TopoDS_Face> faces;
	for (int i = 1; i <= faceMap.Extent(); ++i)
	{
		const TopoDS_Face& face = TopoDS::Face(faceMap(i));
		if (selectedFaces[i - 1])
		{
			std::vector<TopoDS_Face> insetFaces;
			if (!BuildInsetFaces(face, distance, insetFaces)) return nullptr;

			for (const TopoDS_Face& insetFace : insetFaces)
			{
				faces.push_back(insetFace);
			}
		}
		else
		{
			faces.push_back(face);
		}
	}

	bool makeSolid = false;
	for (TopExp_Explorer solidExplorer(shape, TopAbs_SOLID); solidExplorer.More(); solidExplorer.Next())
	{
		makeSolid = true;
		break;
	}

	TopoDS_Shape insetShape = SewFacesToShape(faces, makeSolid);
	if (insetShape.IsNull()) return nullptr;

	BRepLib::SameParameter(insetShape, Precision::Confusion(), Standard_True);

	BRepCheck_Analyzer check(insetShape);
	if (!check.IsValid()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(insetShape);
	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}
