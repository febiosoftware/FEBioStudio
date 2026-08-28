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
#include "occ_prim.h"
#include "GPrimitive.h"

// OCC includes
#ifdef HAS_OCC
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepOffsetAPI_MakeFilling.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Circ.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Mat.hxx>
#include <gp_XYZ.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom2d_Line.hxx>
#include <BRepLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopExp_Explorer.hxx>
#include <Precision.hxx>
#endif

#ifdef HAS_OCC
static bool SewFacesToSolid(const std::vector<TopoDS_Face>& faces, double tol, TopoDS_Solid& solid)
{
	BRepBuilderAPI_Sewing sewing(tol);

	for (const TopoDS_Face& face : faces)
	{
		sewing.Add(face);
	}

	sewing.Perform();

	TopoDS_Shape sewedShape = sewing.SewedShape();
	if (sewedShape.IsNull()) return false;

	TopoDS_Shell shell;

	if (sewedShape.ShapeType() == TopAbs_SHELL)
	{
		shell = TopoDS::Shell(sewedShape);
	}
	else
	{
		TopExp_Explorer shellExplorer(sewedShape, TopAbs_SHELL);
		if (!shellExplorer.More()) return false;
		shell = TopoDS::Shell(shellExplorer.Current());
	}

	if (shell.IsNull()) return false;

	solid = BRepBuilderAPI_MakeSolid(shell);
	if (solid.IsNull()) return false;

	BRepLib::SameParameter(solid, tol, Standard_True);

	BRepCheck_Analyzer check(solid);
	return check.IsValid();
}

static bool SewFacesToShell(const std::vector<TopoDS_Face>& faces, double tol, TopoDS_Shell& shell)
{
	BRepBuilderAPI_Sewing sewing(tol);

	for (const TopoDS_Face& face : faces)
	{
		sewing.Add(face);
	}

	sewing.Perform();

	TopoDS_Shape sewedShape = sewing.SewedShape();
	if (sewedShape.IsNull()) return false;

	if (sewedShape.ShapeType() == TopAbs_SHELL)
	{
		shell = TopoDS::Shell(sewedShape);
	}
	else
	{
		TopExp_Explorer shellExplorer(sewedShape, TopAbs_SHELL);
		if (!shellExplorer.More()) return false;
		shell = TopoDS::Shell(shellExplorer.Current());
	}

	return !shell.IsNull();
}

static bool MakeOpenShellFromFaces(const std::vector<TopoDS_Face>& faces, double tol, TopoDS_Shell& shell)
{
	if (faces.empty()) return false;

	if (SewFacesToShell(faces, tol, shell))
	{
		BRepLib::SameParameter(shell, tol, Standard_True);
		return true;
	}

	BRep_Builder builder;
	builder.MakeShell(shell);
	for (const TopoDS_Face& face : faces)
	{
		if (face.IsNull()) return false;
		builder.Add(shell, face);
	}

	if (shell.IsNull()) return false;

	BRepLib::SameParameter(shell, tol, Standard_True);
	return true;
}

static GOCCObject* CreateOCCObjectFromShellFaces(GObject* po, const std::vector<TopoDS_Face>& faces, double tol)
{
	TopoDS_Shell shell;
	if (!MakeOpenShellFromFaces(faces, tol, shell)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = shell;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
}

static gp_Pnt CirclePoint(double radius, double z, double u)
{
	return gp_Pnt(radius * cos(u), radius * sin(u), z);
}

static gp_Pnt EllipsoidPoint(double radiusX, double radiusY, double radiusZ, double latitude, double longitude)
{
	const double clat = cos(latitude);
	return gp_Pnt(radiusX * clat * cos(longitude), radiusY * clat * sin(longitude), radiusZ * sin(latitude));
}

static TopoDS_Shape ScaleShape(const TopoDS_Shape& shape, double scaleX, double scaleY, double scaleZ)
{
	gp_GTrsf scale(gp_Mat(scaleX, 0.0, 0.0, 0.0, scaleY, 0.0, 0.0, 0.0, scaleZ), gp_XYZ(0.0, 0.0, 0.0));
	BRepBuilderAPI_GTransform transform(shape, scale, Standard_True);
	return transform.Shape();
}

static TopoDS_Face MakeEllipsoidFace(double radiusX, double radiusY, double radiusZ, double longitude0, double longitude1, double latitude0, double latitude1)
{
	Handle(Geom_SphericalSurface) sphereSurface =
		new Geom_SphericalSurface(gp_Ax3(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0)), 1.0);

	TopoDS_Face sphereFace =
		BRepBuilderAPI_MakeFace(sphereSurface, longitude0, longitude1, latitude0, latitude1, Precision::Confusion());

	if (sphereFace.IsNull()) return TopoDS_Face();

	TopoDS_Shape ellipsoidFace = ScaleShape(sphereFace, radiusX, radiusY, radiusZ);
	if (ellipsoidFace.IsNull()) return TopoDS_Face();

	return TopoDS::Face(ellipsoidFace);
}

static TopoDS_Edge MakeEllipsoidLatitudeEdge(double radiusX, double radiusY, double radiusZ, double latitude, double longitude0, double longitude1)
{
	gp_Ax2 axis(gp_Pnt(0.0, 0.0, sin(latitude)), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	gp_Circ circle(axis, cos(latitude));

	TopoDS_Edge sphereEdge = BRepBuilderAPI_MakeEdge(circle, longitude0, longitude1);
	if (sphereEdge.IsNull()) return TopoDS_Edge();

	TopoDS_Shape ellipsoidEdge = ScaleShape(sphereEdge, radiusX, radiusY, radiusZ);
	if (ellipsoidEdge.IsNull()) return TopoDS_Edge();

	return TopoDS::Edge(ellipsoidEdge);
}

static TopoDS_Face MakeTruncatedEllipsoidCapFace(
	double outerX, double outerY, double outerZ,
	double innerX, double innerY, double innerZ,
	double latitude, double longitude0, double longitude1)
{
	TopoDS_Edge outerArc = MakeEllipsoidLatitudeEdge(outerX, outerY, outerZ, latitude, longitude0, longitude1);
	TopoDS_Edge radialIn = BRepBuilderAPI_MakeEdge(
		EllipsoidPoint(outerX, outerY, outerZ, latitude, longitude1),
		EllipsoidPoint(innerX, innerY, innerZ, latitude, longitude1));
	TopoDS_Edge innerArc = MakeEllipsoidLatitudeEdge(innerX, innerY, innerZ, latitude, longitude0, longitude1);
	TopoDS_Edge radialOut = BRepBuilderAPI_MakeEdge(
		EllipsoidPoint(innerX, innerY, innerZ, latitude, longitude0),
		EllipsoidPoint(outerX, outerY, outerZ, latitude, longitude0));

	if (outerArc.IsNull() || radialIn.IsNull() || innerArc.IsNull() || radialOut.IsNull()) return TopoDS_Face();

	BRepBuilderAPI_MakeWire wireBuilder;
	wireBuilder.Add(outerArc);
	wireBuilder.Add(radialIn);
	wireBuilder.Add(TopoDS::Edge(innerArc.Reversed()));
	wireBuilder.Add(radialOut);
	if (!wireBuilder.IsDone()) return TopoDS_Face();

	BRepOffsetAPI_MakeFilling filling;
	filling.Add(outerArc, GeomAbs_C0);
	filling.Add(radialIn, GeomAbs_C0);
	filling.Add(TopoDS::Edge(innerArc.Reversed()), GeomAbs_C0);
	filling.Add(radialOut, GeomAbs_C0);
	filling.Build();
	if (!filling.IsDone()) return TopoDS_Face();

	return TopoDS::Face(filling.Shape());
}

static TopoDS_Face MakeTubeCapFace(double innerRadius, double outerRadius, double z, double u0, double u1, bool topFace)
{
	gp_Ax2 axis(gp_Pnt(0.0, 0.0, z), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	gp_Circ outerCircle(axis, outerRadius);
	gp_Circ innerCircle(axis, innerRadius);

	TopoDS_Edge outerArc = BRepBuilderAPI_MakeEdge(outerCircle, u0, u1);
	TopoDS_Edge radialIn = BRepBuilderAPI_MakeEdge(CirclePoint(outerRadius, z, u1), CirclePoint(innerRadius, z, u1));
	TopoDS_Edge innerArc = BRepBuilderAPI_MakeEdge(innerCircle, u0, u1);
	TopoDS_Edge radialOut = BRepBuilderAPI_MakeEdge(CirclePoint(innerRadius, z, u0), CirclePoint(outerRadius, z, u0));

	BRepBuilderAPI_MakeWire wireBuilder;
	wireBuilder.Add(outerArc);
	wireBuilder.Add(radialIn);
	wireBuilder.Add(TopoDS::Edge(innerArc.Reversed()));
	wireBuilder.Add(radialOut);
	if (!wireBuilder.IsDone()) return TopoDS_Face();

	TopoDS_Face face = BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
	if (!topFace && !face.IsNull()) face.Reverse();

	return face;
}

static TopoDS_Face MakeDiskFace(double radius, double z, bool topFace)
{
	gp_Ax2 axis(gp_Pnt(0.0, 0.0, z), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	gp_Circ circle(axis, radius);

	BRepBuilderAPI_MakeWire wireBuilder;
	for (int i = 0; i < 4; ++i)
	{
		double u0 = i * 0.5 * M_PI;
		double u1 = (i + 1) * 0.5 * M_PI;
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(circle, u0, u1));
	}

	if (!wireBuilder.IsDone()) return TopoDS_Face();

	TopoDS_Face face = BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
	if (!topFace && !face.IsNull()) face.Reverse();

	return face;
}

static TopoDS_Face MakeSectorCapFace(double radius, double z, double u0, double u1, bool topFace)
{
	gp_Ax2 axis(gp_Pnt(0.0, 0.0, z), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	gp_Circ circle(axis, radius);

	TopoDS_Edge radialOut = BRepBuilderAPI_MakeEdge(CirclePoint(0.0, z, u0), CirclePoint(radius, z, u0));
	TopoDS_Edge arc = BRepBuilderAPI_MakeEdge(circle, u0, u1);
	TopoDS_Edge radialIn = BRepBuilderAPI_MakeEdge(CirclePoint(radius, z, u1), CirclePoint(0.0, z, u1));

	BRepBuilderAPI_MakeWire wireBuilder;
	wireBuilder.Add(radialOut);
	wireBuilder.Add(arc);
	wireBuilder.Add(radialIn);
	if (!wireBuilder.IsDone()) return TopoDS_Face();

	TopoDS_Face face = BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
	if (!topFace && !face.IsNull()) face.Reverse();

	return face;
}

static TopoDS_Face MakeRadialFace(double radius, double height, double u, bool startFace)
{
	gp_Pnt centerBottom(0.0, 0.0, 0.0);
	gp_Pnt centerTop(0.0, 0.0, height);
	gp_Pnt outerBottom = CirclePoint(radius, 0.0, u);
	gp_Pnt outerTop = CirclePoint(radius, height, u);

	BRepBuilderAPI_MakeWire wireBuilder;
	if (startFace)
	{
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(centerBottom, outerBottom));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerBottom, outerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerTop, centerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(centerTop, centerBottom));
	}
	else
	{
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(centerBottom, centerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(centerTop, outerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerTop, outerBottom));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerBottom, centerBottom));
	}

	if (!wireBuilder.IsDone()) return TopoDS_Face();

	return BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
}

static TopoDS_Face MakeAnnularRadialFace(double innerRadius, double outerRadius, double height, double u, bool startFace)
{
	gp_Pnt outerBottom = CirclePoint(outerRadius, 0.0, u);
	gp_Pnt innerBottom = CirclePoint(innerRadius, 0.0, u);
	gp_Pnt outerTop = CirclePoint(outerRadius, height, u);
	gp_Pnt innerTop = CirclePoint(innerRadius, height, u);

	BRepBuilderAPI_MakeWire wireBuilder;
	if (startFace)
	{
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerBottom, outerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerTop, innerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(innerTop, innerBottom));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(innerBottom, outerBottom));
	}
	else
	{
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerBottom, innerBottom));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(innerBottom, innerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(innerTop, outerTop));
		wireBuilder.Add(BRepBuilderAPI_MakeEdge(outerTop, outerBottom));
	}

	if (!wireBuilder.IsDone()) return TopoDS_Face();

	return BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
}

static TopoDS_Face MakeQuadFace(const gp_Pnt& p0, const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3)
{
	BRepBuilderAPI_MakeWire wireBuilder;
	wireBuilder.Add(BRepBuilderAPI_MakeEdge(p0, p1));
	wireBuilder.Add(BRepBuilderAPI_MakeEdge(p1, p2));
	wireBuilder.Add(BRepBuilderAPI_MakeEdge(p2, p3));
	wireBuilder.Add(BRepBuilderAPI_MakeEdge(p3, p0));
	if (!wireBuilder.IsDone()) return TopoDS_Face();

	return BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
}

static TopoDS_Face MakeCylinderInBoxCapFace(double halfWidth, double halfHeight, double radius, double z, int sector, bool topFace)
{
	const double halfPi = 0.5 * M_PI;
	const double u0 = -0.75 * M_PI + sector * halfPi;
	const double u1 = u0 + halfPi;

	gp_Pnt box[4] = {
		gp_Pnt(-halfWidth, -halfHeight, z),
		gp_Pnt( halfWidth, -halfHeight, z),
		gp_Pnt( halfWidth,  halfHeight, z),
		gp_Pnt(-halfWidth,  halfHeight, z)
	};

	gp_Ax2 axis(gp_Pnt(0.0, 0.0, z), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	gp_Circ circle(axis, radius);

	const int next = (sector + 1) % 4;

	TopoDS_Edge outerEdge = BRepBuilderAPI_MakeEdge(box[sector], box[next]);
	TopoDS_Edge radialIn = BRepBuilderAPI_MakeEdge(box[next], CirclePoint(radius, z, u1));
	TopoDS_Edge innerArc = BRepBuilderAPI_MakeEdge(circle, u0, u1);
	TopoDS_Edge radialOut = BRepBuilderAPI_MakeEdge(CirclePoint(radius, z, u0), box[sector]);

	if (outerEdge.IsNull() || radialIn.IsNull() || innerArc.IsNull() || radialOut.IsNull()) return TopoDS_Face();

	BRepBuilderAPI_MakeWire wireBuilder;
	wireBuilder.Add(outerEdge);
	wireBuilder.Add(radialIn);
	wireBuilder.Add(TopoDS::Edge(innerArc.Reversed()));
	wireBuilder.Add(radialOut);
	if (!wireBuilder.IsDone()) return TopoDS_Face();

	TopoDS_Face face = BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
	if (!topFace && !face.IsNull()) face.Reverse();

	return face;
}

static TopoDS_Face MakeQuartDogBoneCapFace(double clampWidth, double clampHeight, double radius, double length, double wing, double depth, bool topFace)
{
	const double z = (topFace ? depth : 0.0);
	const double cx = clampWidth - wing;
	const double cy = clampHeight + radius;

	gp_Pnt p0(0.0, 0.0, z);
	gp_Pnt p1(clampWidth, 0.0, z);
	gp_Pnt p2(clampWidth, clampHeight, z);
	gp_Pnt p3(clampWidth - wing, clampHeight, z);
	gp_Pnt p4(clampWidth - wing - radius, clampHeight + radius, z);
	gp_Pnt p5(clampWidth - wing - radius, clampHeight + radius + length, z);
	gp_Pnt p6(0.0, clampHeight + radius + length, z);

	gp_Ax2 axis(gp_Pnt(cx, cy, z), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	gp_Circ circle(axis, radius);

	TopoDS_Edge e0 = BRepBuilderAPI_MakeEdge(p0, p1);
	TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
	TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
	TopoDS_Edge arc = BRepBuilderAPI_MakeEdge(circle, -M_PI, -0.5 * M_PI);
	TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p5);
	TopoDS_Edge e5 = BRepBuilderAPI_MakeEdge(p5, p6);
	TopoDS_Edge e6 = BRepBuilderAPI_MakeEdge(p6, p0);

	if (e0.IsNull() || e1.IsNull() || e2.IsNull() || arc.IsNull() || e4.IsNull() || e5.IsNull() || e6.IsNull()) return TopoDS_Face();

	BRepBuilderAPI_MakeWire wireBuilder;
	wireBuilder.Add(e0);
	wireBuilder.Add(e1);
	wireBuilder.Add(e2);
	wireBuilder.Add(TopoDS::Edge(arc.Reversed()));
	wireBuilder.Add(e4);
	wireBuilder.Add(e5);
	wireBuilder.Add(e6);
	if (!wireBuilder.IsDone()) return TopoDS_Face();

	TopoDS_Face face = BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
	if (!topFace && !face.IsNull()) face.Reverse();

	return face;
}

GOCCObject* CreateOCCObjectFromBox(GBox* po)
{
	BRep_Builder B;
	// build the vertices
	std::vector<TopoDS_Vertex> vertices;
	for (int i = 0; i < po->Nodes(); ++i)
	{
		GNode* node = po->Node(i);
		vec3d r = node->LocalPosition();
		TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(gp_Pnt(r.x, r.y, r.z));
		vertices.push_back(v);
	}
	// build edges
	std::vector<TopoDS_Edge> edges;
	for (int i = 0; i < po->Edges(); ++i)
	{
		GEdge* edge = po->Edge(i);
		assert(edge->Type() == EDGE_LINE);
		int n1 = edge->m_node[0];
		int n2 = edge->m_node[1];
		TopoDS_Edge occEdge = BRepBuilderAPI_MakeEdge(vertices[n1], vertices[n2]);
		if (occEdge.IsNull())
		{
			return nullptr;
		}
		edges.push_back(occEdge);
	}
	// build faces
	std::vector<TopoDS_Face> faces;
	for (int i = 0; i < po->Faces(); ++i)
	{
		GFace* face = po->Face(i);
		BRepBuilderAPI_MakeWire makeWire;
		for (int j = 0; j < face->Edges(); ++j)
		{
			int edgeID = face->m_edge[j].nid;
			int nwn = face->m_edge[j].nwn;
			if ((edgeID >= 0) && (edgeID < (int)edges.size()))
			{
				if (nwn > 0)
					makeWire.Add(edges[edgeID]);
				else
					makeWire.Add(TopoDS::Edge(edges[edgeID].Reversed()));
			}
		}
		if (!makeWire.IsDone())
			return nullptr;

		TopoDS_Face occFace = BRepBuilderAPI_MakeFace(makeWire.Wire(), Standard_True);
		if (occFace.IsNull())
		{
			return nullptr;
		}
		faces.push_back(occFace);
	}

	TopoDS_Shell shell;
	B.MakeShell(shell);
	for (const TopoDS_Face& face : faces)
	{
		B.Add(shell, face);
	}

	TopoDS_Shape topo = BRepBuilderAPI_MakeSolid(shell);
	if (topo.IsNull())
	{
		return nullptr;
	}

	// create a new OCC object
	GOCCObject* occ = new GOCCObject;
	occ->SetShape(topo);

	// copy some other stuff
	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
}

GOCCObject* CreateOCCObjectFromPatch(GPatch* po)
{
#ifdef HAS_OCC
	const double width = po->GetFloatValue(GPatch::W);
	const double height = po->GetFloatValue(GPatch::H);

	if (width <= 0.0 || height <= 0.0) return nullptr;

	const double w = 0.5 * width;
	const double h = 0.5 * height;
	const double tol = Precision::Confusion();

	std::vector<TopoDS_Face> faces;
	TopoDS_Face face = MakeQuadFace(
		gp_Pnt(-w, -h, 0.0),
		gp_Pnt( w, -h, 0.0),
		gp_Pnt( w,  h, 0.0),
		gp_Pnt(-w,  h, 0.0));
	if (face.IsNull()) return nullptr;

	faces.push_back(face);
	return CreateOCCObjectFromShellFaces(po, faces, tol);
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromDisc(GDisc* po)
{
#ifdef HAS_OCC
	const double radius = po->Radius();
	if (radius <= 0.0) return nullptr;

	const double halfPi = 0.5 * M_PI;
	const double tol = Precision::Confusion();

	std::vector<TopoDS_Face> faces;
	for (int i = 0; i < 4; ++i)
	{
		const double u0 = i * halfPi;
		const double u1 = (i + 1) * halfPi;

		TopoDS_Face face = MakeSectorCapFace(radius, 0.0, u0, u1, true);
		if (face.IsNull()) return nullptr;

		faces.push_back(face);
	}

	return CreateOCCObjectFromShellFaces(po, faces, tol);
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromRing(GRing* po)
{
#ifdef HAS_OCC
	const double innerRadius = po->GetFloatValue(GRing::RIN);
	const double outerRadius = po->GetFloatValue(GRing::ROUT);

	if (innerRadius <= 0.0 || outerRadius <= innerRadius) return nullptr;

	const double halfPi = 0.5 * M_PI;
	const double tol = Precision::Confusion();

	std::vector<TopoDS_Face> faces;
	for (int i = 0; i < 4; ++i)
	{
		const double u0 = i * halfPi;
		const double u1 = (i + 1) * halfPi;

		TopoDS_Face face = MakeTubeCapFace(innerRadius, outerRadius, 0.0, u0, u1, true);
		if (face.IsNull()) return nullptr;

		faces.push_back(face);
	}

	return CreateOCCObjectFromShellFaces(po, faces, tol);
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromThinTube(GThinTube* po)
{
#ifdef HAS_OCC
	const double radius = po->Radius();
	const double height = po->Height();

	if (radius <= 0.0 || height <= 0.0) return nullptr;

	const double halfPi = 0.5 * M_PI;
	const double tol = Precision::Confusion();

	Handle(Geom_CylindricalSurface) cylSurface =
		new Geom_CylindricalSurface(
			gp_Ax3(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0)),
			radius);

	std::vector<TopoDS_Face> faces;
	for (int i = 0; i < 4; ++i)
	{
		const double u0 = i * halfPi;
		const double u1 = (i + 1) * halfPi;

		TopoDS_Face face =
			BRepBuilderAPI_MakeFace(cylSurface, u0, u1, 0.0, height, tol);
		if (face.IsNull()) return nullptr;

		faces.push_back(face);
	}

	return CreateOCCObjectFromShellFaces(po, faces, tol);
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromBoxInBox(GBoxInBox* po)
{
#ifdef HAS_OCC
	const double outerWidth = po->OuterWidth();
	const double outerHeight = po->OuterHeight();
	const double outerDepth = po->OuterDepth();
	const double innerWidth = po->InnerWidth();
	const double innerHeight = po->InnerHeight();
	const double innerDepth = po->InnerDepth();

	if (outerWidth <= 0.0 || outerHeight <= 0.0 || outerDepth <= 0.0) return nullptr;
	if (innerWidth <= 0.0 || innerHeight <= 0.0 || innerDepth <= 0.0) return nullptr;
	if (innerWidth >= outerWidth || innerHeight >= outerHeight || innerDepth >= outerDepth) return nullptr;

	const double ow = 0.5 * outerWidth;
	const double oh = 0.5 * outerHeight;
	const double iw = 0.5 * innerWidth;
	const double ih = 0.5 * innerHeight;
	const double z0 = 0.5 * (outerDepth - innerDepth);
	const double z1 = 0.5 * (outerDepth + innerDepth);
	const double tol = Precision::Confusion();

	std::vector<TopoDS_Face> outerFaces;
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-ow, -oh, 0.0),
		gp_Pnt( ow, -oh, 0.0),
		gp_Pnt( ow, -oh, outerDepth),
		gp_Pnt(-ow, -oh, outerDepth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(ow, -oh, 0.0),
		gp_Pnt(ow,  oh, 0.0),
		gp_Pnt(ow,  oh, outerDepth),
		gp_Pnt(ow, -oh, outerDepth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt( ow, oh, 0.0),
		gp_Pnt(-ow, oh, 0.0),
		gp_Pnt(-ow, oh, outerDepth),
		gp_Pnt( ow, oh, outerDepth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-ow,  oh, 0.0),
		gp_Pnt(-ow, -oh, 0.0),
		gp_Pnt(-ow, -oh, outerDepth),
		gp_Pnt(-ow,  oh, outerDepth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-ow, -oh, 0.0),
		gp_Pnt(-ow,  oh, 0.0),
		gp_Pnt( ow,  oh, 0.0),
		gp_Pnt( ow, -oh, 0.0)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-ow, -oh, outerDepth),
		gp_Pnt( ow, -oh, outerDepth),
		gp_Pnt( ow,  oh, outerDepth),
		gp_Pnt(-ow,  oh, outerDepth)));

	for (const TopoDS_Face& face : outerFaces)
	{
		if (face.IsNull()) return nullptr;
	}

	gp_Pnt p[8] = {
		gp_Pnt(-iw, -ih, z0),
		gp_Pnt( iw, -ih, z0),
		gp_Pnt( iw,  ih, z0),
		gp_Pnt(-iw,  ih, z0),
		gp_Pnt(-iw, -ih, z1),
		gp_Pnt( iw, -ih, z1),
		gp_Pnt( iw,  ih, z1),
		gp_Pnt(-iw,  ih, z1)
	};

	std::vector<TopoDS_Face> innerFaces;
	innerFaces.push_back(MakeQuadFace(p[0], p[4], p[5], p[1]));
	innerFaces.push_back(MakeQuadFace(p[1], p[5], p[6], p[2]));
	innerFaces.push_back(MakeQuadFace(p[2], p[6], p[7], p[3]));
	innerFaces.push_back(MakeQuadFace(p[3], p[7], p[4], p[0]));
	innerFaces.push_back(MakeQuadFace(p[0], p[1], p[2], p[3]));
	innerFaces.push_back(MakeQuadFace(p[7], p[6], p[5], p[4]));

	for (const TopoDS_Face& face : innerFaces)
	{
		if (face.IsNull()) return nullptr;
	}

	TopoDS_Shell outerShell;
	if (!SewFacesToShell(outerFaces, tol, outerShell)) return nullptr;

	TopoDS_Shell innerShell;
	if (!SewFacesToShell(innerFaces, tol, innerShell)) return nullptr;

	BRep_Builder builder;
	TopoDS_Solid solid;
	builder.MakeSolid(solid);
	builder.Add(solid, outerShell);
	builder.Add(solid, innerShell);

	if (solid.IsNull()) return nullptr;

	BRepLib::SameParameter(solid, tol, Standard_True);

	BRepCheck_Analyzer check(solid);
	if (!check.IsValid()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromCylinderInBox(GCylinderInBox* po)
{
#ifdef HAS_OCC
	const double width = po->GetFloatValue(GCylinderInBox::WIDTH);
	const double height = po->GetFloatValue(GCylinderInBox::HEIGHT);
	const double depth = po->GetFloatValue(GCylinderInBox::DEPTH);
	const double radius = po->GetFloatValue(GCylinderInBox::RADIUS);

	if (width <= 0.0 || height <= 0.0 || depth <= 0.0 || radius <= 0.0) return nullptr;
	if (2.0 * radius >= (width < height ? width : height)) return nullptr;

	const double halfWidth = 0.5 * width;
	const double halfHeight = 0.5 * height;
	const double halfPi = 0.5 * M_PI;
	const double tol = Precision::Confusion();

	Handle(Geom_CylindricalSurface) cylSurface =
		new Geom_CylindricalSurface(
			gp_Ax3(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0)),
			radius);

	std::vector<TopoDS_Face> faces;

	// Outer box faces, ordered to give outward-facing normals.
	faces.push_back(MakeQuadFace(
		gp_Pnt(-halfWidth, -halfHeight, 0.0),
		gp_Pnt( halfWidth, -halfHeight, 0.0),
		gp_Pnt( halfWidth, -halfHeight, depth),
		gp_Pnt(-halfWidth, -halfHeight, depth)));
	faces.push_back(MakeQuadFace(
		gp_Pnt(halfWidth, -halfHeight, 0.0),
		gp_Pnt(halfWidth,  halfHeight, 0.0),
		gp_Pnt(halfWidth,  halfHeight, depth),
		gp_Pnt(halfWidth, -halfHeight, depth)));
	faces.push_back(MakeQuadFace(
		gp_Pnt( halfWidth, halfHeight, 0.0),
		gp_Pnt(-halfWidth, halfHeight, 0.0),
		gp_Pnt(-halfWidth, halfHeight, depth),
		gp_Pnt( halfWidth, halfHeight, depth)));
	faces.push_back(MakeQuadFace(
		gp_Pnt(-halfWidth,  halfHeight, 0.0),
		gp_Pnt(-halfWidth, -halfHeight, 0.0),
		gp_Pnt(-halfWidth, -halfHeight, depth),
		gp_Pnt(-halfWidth,  halfHeight, depth)));

	for (const TopoDS_Face& face : faces)
	{
		if (face.IsNull()) return nullptr;
	}

	// Inner cylindrical cavity, split into four 90-degree patches.
	for (int i = 0; i < 4; ++i)
	{
		const double u0 = -0.75 * M_PI + i * halfPi;
		const double u1 = u0 + halfPi;

		TopoDS_Face sideFace =
			BRepBuilderAPI_MakeFace(cylSurface, u0, u1, 0.0, depth, tol);

		if (sideFace.IsNull()) return nullptr;

		sideFace.Reverse();
		faces.push_back(sideFace);
	}

	// Bottom and top planar patches around the circular opening.
	for (int i = 0; i < 4; ++i)
	{
		TopoDS_Face bottomFace = MakeCylinderInBoxCapFace(halfWidth, halfHeight, radius, 0.0, i, false);
		if (bottomFace.IsNull()) return nullptr;
		faces.push_back(bottomFace);
	}
	for (int i = 0; i < 4; ++i)
	{
		TopoDS_Face topFace = MakeCylinderInBoxCapFace(halfWidth, halfHeight, radius, depth, i, true);
		if (topFace.IsNull()) return nullptr;
		faces.push_back(topFace);
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromSphereInBox(GSphereInBox* po)
{
#ifdef HAS_OCC
	const double width = po->GetFloatValue(GSphereInBox::WIDTH);
	const double height = po->GetFloatValue(GSphereInBox::HEIGHT);
	const double depth = po->GetFloatValue(GSphereInBox::DEPTH);
	const double radius = po->GetFloatValue(GSphereInBox::RADIUS);

	if (width <= 0.0 || height <= 0.0 || depth <= 0.0 || radius <= 0.0) return nullptr;
	if (2.0 * radius >= (width < height ? (width < depth ? width : depth) : (height < depth ? height : depth))) return nullptr;

	const double halfWidth = 0.5 * width;
	const double halfHeight = 0.5 * height;
	const double halfDepth = 0.5 * depth;
	const double halfPi = 0.5 * M_PI;
	const double tol = Precision::Confusion();

	std::vector<TopoDS_Face> outerFaces;
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-halfWidth, -halfHeight, 0.0),
		gp_Pnt( halfWidth, -halfHeight, 0.0),
		gp_Pnt( halfWidth, -halfHeight, depth),
		gp_Pnt(-halfWidth, -halfHeight, depth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(halfWidth, -halfHeight, 0.0),
		gp_Pnt(halfWidth,  halfHeight, 0.0),
		gp_Pnt(halfWidth,  halfHeight, depth),
		gp_Pnt(halfWidth, -halfHeight, depth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt( halfWidth, halfHeight, 0.0),
		gp_Pnt(-halfWidth, halfHeight, 0.0),
		gp_Pnt(-halfWidth, halfHeight, depth),
		gp_Pnt( halfWidth, halfHeight, depth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-halfWidth,  halfHeight, 0.0),
		gp_Pnt(-halfWidth, -halfHeight, 0.0),
		gp_Pnt(-halfWidth, -halfHeight, depth),
		gp_Pnt(-halfWidth,  halfHeight, depth)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-halfWidth, -halfHeight, 0.0),
		gp_Pnt(-halfWidth,  halfHeight, 0.0),
		gp_Pnt( halfWidth,  halfHeight, 0.0),
		gp_Pnt( halfWidth, -halfHeight, 0.0)));
	outerFaces.push_back(MakeQuadFace(
		gp_Pnt(-halfWidth, -halfHeight, depth),
		gp_Pnt( halfWidth, -halfHeight, depth),
		gp_Pnt( halfWidth,  halfHeight, depth),
		gp_Pnt(-halfWidth,  halfHeight, depth)));

	for (const TopoDS_Face& face : outerFaces)
	{
		if (face.IsNull()) return nullptr;
	}

	std::vector<TopoDS_Face> innerFaces;
	gp_Ax3 sphereAxis(gp_Pnt(0.0, 0.0, halfDepth), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	Handle(Geom_SphericalSurface) sphereSurface =
		new Geom_SphericalSurface(sphereAxis, radius);

	for (int band = 0; band < 2; ++band)
	{
		const double v0 = (band == 0 ? -halfPi : 0.0);
		const double v1 = (band == 0 ? 0.0 : halfPi);

		for (int i = 0; i < 4; ++i)
		{
			const double u0 = i * halfPi;
			const double u1 = (i + 1) * halfPi;

			TopoDS_Face face =
				BRepBuilderAPI_MakeFace(sphereSurface, u0, u1, v0, v1, tol);
			if (face.IsNull()) return nullptr;

			face.Reverse();
			innerFaces.push_back(face);
		}
	}

	for (const TopoDS_Face& face : innerFaces)
	{
		if (face.IsNull()) return nullptr;
	}

	TopoDS_Shell outerShell;
	if (!SewFacesToShell(outerFaces, tol, outerShell)) return nullptr;

	TopoDS_Shell innerShell;
	if (!SewFacesToShell(innerFaces, tol, innerShell)) return nullptr;

	BRep_Builder builder;
	TopoDS_Solid solid;
	builder.MakeSolid(solid);
	builder.Add(solid, outerShell);
	builder.Add(solid, innerShell);

	if (solid.IsNull()) return nullptr;

	BRepLib::SameParameter(solid, tol, Standard_True);

	BRepCheck_Analyzer check(solid);
	if (!check.IsValid()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromHollowSphere(GHollowSphere* po)
{
#ifdef HAS_OCC
	const double innerRadius = po->GetFloatValue(GHollowSphere::RIN);
	const double outerRadius = po->GetFloatValue(GHollowSphere::ROUT);

	if (innerRadius <= 0.0 || outerRadius <= innerRadius) return nullptr;

	const double halfPi = 0.5 * M_PI;
	const double tol = Precision::Confusion();

	gp_Ax3 sphereAxis(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	Handle(Geom_SphericalSurface) outerSurface =
		new Geom_SphericalSurface(sphereAxis, outerRadius);
	Handle(Geom_SphericalSurface) innerSurface =
		new Geom_SphericalSurface(sphereAxis, innerRadius);

	std::vector<TopoDS_Face> outerFaces;
	std::vector<TopoDS_Face> innerFaces;

	for (int band = 0; band < 2; ++band)
	{
		const double v0 = (band == 0 ? -halfPi : 0.0);
		const double v1 = (band == 0 ? 0.0 : halfPi);

		for (int i = 0; i < 4; ++i)
		{
			const double u0 = i * halfPi;
			const double u1 = (i + 1) * halfPi;

			TopoDS_Face outerFace =
				BRepBuilderAPI_MakeFace(outerSurface, u0, u1, v0, v1, tol);
			if (outerFace.IsNull()) return nullptr;
			outerFaces.push_back(outerFace);

			TopoDS_Face innerFace =
				BRepBuilderAPI_MakeFace(innerSurface, u0, u1, v0, v1, tol);
			if (innerFace.IsNull()) return nullptr;
			innerFace.Reverse();
			innerFaces.push_back(innerFace);
		}
	}

	TopoDS_Shell outerShell;
	if (!SewFacesToShell(outerFaces, tol, outerShell)) return nullptr;

	TopoDS_Shell innerShell;
	if (!SewFacesToShell(innerFaces, tol, innerShell)) return nullptr;

	BRep_Builder builder;
	TopoDS_Solid solid;
	builder.MakeSolid(solid);
	builder.Add(solid, outerShell);
	builder.Add(solid, innerShell);

	if (solid.IsNull()) return nullptr;

	BRepLib::SameParameter(solid, tol, Standard_True);

	BRepCheck_Analyzer check(solid);
	if (!check.IsValid()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromQuartDogBone(GQuartDogBone* po)
{
#ifdef HAS_OCC
	const double clampWidth = po->GetFloatValue(GQuartDogBone::CWIDTH);
	const double clampHeight = po->GetFloatValue(GQuartDogBone::CHEIGHT);
	const double radius = po->GetFloatValue(GQuartDogBone::RADIUS);
	const double length = po->GetFloatValue(GQuartDogBone::LENGTH);
	const double depth = po->GetFloatValue(GQuartDogBone::DEPTH);
	const double wing = po->GetFloatValue(GQuartDogBone::WING);

	if (clampWidth <= 0.0 || clampHeight <= 0.0 || radius <= 0.0 || length <= 0.0 || depth <= 0.0) return nullptr;
	if (wing < 0.0 || wing + radius >= clampWidth) return nullptr;

	const double cx = clampWidth - wing;
	const double cy = clampHeight + radius;
	const double tol = Precision::Confusion();

	gp_Pnt b[7] = {
		gp_Pnt(0.0, 0.0, 0.0),
		gp_Pnt(clampWidth, 0.0, 0.0),
		gp_Pnt(clampWidth, clampHeight, 0.0),
		gp_Pnt(clampWidth - wing, clampHeight, 0.0),
		gp_Pnt(clampWidth - wing - radius, clampHeight + radius, 0.0),
		gp_Pnt(clampWidth - wing - radius, clampHeight + radius + length, 0.0),
		gp_Pnt(0.0, clampHeight + radius + length, 0.0)
	};
	gp_Pnt t[7] = {
		gp_Pnt(0.0, 0.0, depth),
		gp_Pnt(clampWidth, 0.0, depth),
		gp_Pnt(clampWidth, clampHeight, depth),
		gp_Pnt(clampWidth - wing, clampHeight, depth),
		gp_Pnt(clampWidth - wing - radius, clampHeight + radius, depth),
		gp_Pnt(clampWidth - wing - radius, clampHeight + radius + length, depth),
		gp_Pnt(0.0, clampHeight + radius + length, depth)
	};

	std::vector<TopoDS_Face> faces;

	TopoDS_Face bottomFace = MakeQuartDogBoneCapFace(clampWidth, clampHeight, radius, length, wing, depth, false);
	if (bottomFace.IsNull()) return nullptr;
	faces.push_back(bottomFace);

	for (int i = 0; i < 3; ++i)
	{
		TopoDS_Face face = MakeQuadFace(b[i], b[i + 1], t[i + 1], t[i]);
		if (face.IsNull()) return nullptr;
		faces.push_back(face);
	}

	Handle(Geom_CylindricalSurface) shoulderSurface =
		new Geom_CylindricalSurface(
			gp_Ax3(gp_Pnt(cx, cy, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0)),
			radius);

	TopoDS_Face shoulderFace =
		BRepBuilderAPI_MakeFace(shoulderSurface, -M_PI, -0.5 * M_PI, 0.0, depth, tol);
	if (shoulderFace.IsNull()) return nullptr;
	shoulderFace.Reverse();
	faces.push_back(shoulderFace);

	for (int i = 4; i < 7; ++i)
	{
		const int next = (i + 1) % 7;
		TopoDS_Face face = MakeQuadFace(b[i], b[next], t[next], t[i]);
		if (face.IsNull()) return nullptr;
		faces.push_back(face);
	}

	TopoDS_Face topFace = MakeQuartDogBoneCapFace(clampWidth, clampHeight, radius, length, wing, depth, true);
	if (topFace.IsNull()) return nullptr;
	faces.push_back(topFace);

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromTruncatedEllipsoid(GTruncatedEllipsoid* po)
{
#ifdef HAS_OCC
	const double ra = po->GetFloatValue(GTruncatedEllipsoid::RA);
	const double rb = po->GetFloatValue(GTruncatedEllipsoid::RB);
	const double rc = po->GetFloatValue(GTruncatedEllipsoid::RC);
	const double wt = po->GetFloatValue(GTruncatedEllipsoid::WT);
	const double vend = po->GetFloatValue(GTruncatedEllipsoid::VEND) * M_PI / 180.0;
	const double tol = Precision::Confusion();

	if (ra <= wt || rb <= wt || rc <= wt || wt <= 0.0) return nullptr;
	if (vend <= -0.5 * M_PI + tol || vend >= 0.5 * M_PI - tol) return nullptr;

	const double outerX = ra + wt;
	const double outerY = rb + wt;
	const double outerZ = rc + wt;
	const double innerX = ra - wt;
	const double innerY = rb - wt;
	const double innerZ = rc - wt;

	const double halfPi = 0.5 * M_PI;
	const double latitude0 = -halfPi;

	std::vector<TopoDS_Face> faces;

	for (int i = 0; i < 4; ++i)
	{
		double longitude0 = i * halfPi;
		double longitude1 = (i + 1) * halfPi;

		TopoDS_Face outerFace = MakeEllipsoidFace(outerX, outerY, outerZ, longitude0, longitude1, latitude0, vend);
		if (outerFace.IsNull()) return nullptr;

		faces.push_back(outerFace);
	}

	for (int i = 0; i < 4; ++i)
	{
		double longitude0 = i * halfPi;
		double longitude1 = (i + 1) * halfPi;

		TopoDS_Face innerFace = MakeEllipsoidFace(innerX, innerY, innerZ, longitude0, longitude1, latitude0, vend);
		if (innerFace.IsNull()) return nullptr;

		innerFace.Reverse();
		faces.push_back(innerFace);
	}

	for (int i = 0; i < 4; ++i)
	{
		double longitude0 = i * halfPi;
		double longitude1 = (i + 1) * halfPi;

		TopoDS_Face capFace = MakeTruncatedEllipsoidCapFace(
			outerX, outerY, outerZ, innerX, innerY, innerZ, vend, longitude0, longitude1);
		if (capFace.IsNull()) return nullptr;

		faces.push_back(capFace);
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromHexagon(GHexagon* po)
{
#ifdef HAS_OCC
	std::vector<TopoDS_Vertex> vertices;
	for (int i = 0; i < po->Nodes(); ++i)
	{
		GNode* node = po->Node(i);
		vec3d r = node->LocalPosition();
		TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(gp_Pnt(r.x, r.y, r.z));
		vertices.push_back(v);
	}

	std::vector<TopoDS_Edge> edges;
	for (int i = 0; i < po->Edges(); ++i)
	{
		GEdge* edge = po->Edge(i);
		if (edge->Type() != EDGE_LINE) return nullptr;

		int n1 = edge->m_node[0];
		int n2 = edge->m_node[1];
		TopoDS_Edge occEdge = BRepBuilderAPI_MakeEdge(vertices[n1], vertices[n2]);
		if (occEdge.IsNull()) return nullptr;

		edges.push_back(occEdge);
	}

	std::vector<TopoDS_Face> faces;
	for (int i = 0; i < po->Faces(); ++i)
	{
		GFace* face = po->Face(i);

		BRepBuilderAPI_MakeWire wireBuilder;
		for (int j = 0; j < face->Edges(); ++j)
		{
			int edgeId = face->m_edge[j].nid;
			int winding = face->m_edge[j].nwn;
			if ((edgeId < 0) || (edgeId >= (int)edges.size())) return nullptr;

			if (winding > 0)
				wireBuilder.Add(edges[edgeId]);
			else
				wireBuilder.Add(TopoDS::Edge(edges[edgeId].Reversed()));
		}

		if (!wireBuilder.IsDone()) return nullptr;

		TopoDS_Face occFace = BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);
		if (occFace.IsNull()) return nullptr;

		faces.push_back(occFace);
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, Precision::Confusion(), solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromTorus(GTorus* po)
{
#ifdef HAS_OCC
	const double majorRadius = po->GetFloatValue(GTorus::RIN);
	const double minorRadius = po->GetFloatValue(GTorus::ROUT);

	if (majorRadius <= 0.0 || minorRadius <= 0.0) return nullptr;
	if (majorRadius <= minorRadius) return nullptr;

	const double pi = M_PI;
	const double halfPi = 0.5 * pi;
	const double tol = Precision::Confusion();

	gp_Ax3 torusAxis(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	Handle(Geom_ToroidalSurface) torusSurface =
		new Geom_ToroidalSurface(torusAxis, majorRadius, minorRadius);

	std::vector<TopoDS_Face> faces;

	for (int circum = 0; circum < 4; ++circum)
	{
		double v0 = circum * halfPi;
		double v1 = (circum + 1) * halfPi;

		for (int section = 0; section < 4; ++section)
		{
			double u0 = section * halfPi;
			double u1 = (section + 1) * halfPi;

			TopoDS_Face face =
				BRepBuilderAPI_MakeFace(torusSurface, u0, u1, v0, v1, tol);
			if (face.IsNull()) return nullptr;

			faces.push_back(face);
		}
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromSolidArc(GSolidArc* po)
{
#ifdef HAS_OCC
	const double innerRadius = po->GetFloatValue(GSolidArc::RIN);
	const double outerRadius = po->GetFloatValue(GSolidArc::ROUT);
	const double height = po->GetFloatValue(GSolidArc::HEIGHT);
	const double angle = po->GetFloatValue(GSolidArc::ARC) * M_PI / 180.0;
	const double tol = Precision::Confusion();

	if (innerRadius <= 0.0 || outerRadius <= innerRadius || height <= 0.0) return nullptr;
	if (angle <= tol || angle >= 2.0 * M_PI - tol) return nullptr;

	gp_Ax2 cylAxis(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	Handle(Geom_CylindricalSurface) outerSurface =
		new Geom_CylindricalSurface(cylAxis, outerRadius);
	Handle(Geom_CylindricalSurface) innerSurface =
		new Geom_CylindricalSurface(cylAxis, innerRadius);

	std::vector<TopoDS_Face> faces;

	TopoDS_Face outerFace = BRepBuilderAPI_MakeFace(outerSurface, 0.0, angle, 0.0, height, tol);
	if (outerFace.IsNull()) return nullptr;
	faces.push_back(outerFace);

	TopoDS_Face endFace = MakeAnnularRadialFace(innerRadius, outerRadius, height, angle, false);
	if (endFace.IsNull()) return nullptr;
	faces.push_back(endFace);

	TopoDS_Face innerFace = BRepBuilderAPI_MakeFace(innerSurface, 0.0, angle, 0.0, height, tol);
	if (innerFace.IsNull()) return nullptr;
	innerFace.Reverse();
	faces.push_back(innerFace);

	TopoDS_Face startFace = MakeAnnularRadialFace(innerRadius, outerRadius, height, 0.0, true);
	if (startFace.IsNull()) return nullptr;
	faces.push_back(startFace);

	TopoDS_Face bottomFace = MakeTubeCapFace(innerRadius, outerRadius, 0.0, 0.0, angle, false);
	if (bottomFace.IsNull()) return nullptr;
	faces.push_back(bottomFace);

	TopoDS_Face topFace = MakeTubeCapFace(innerRadius, outerRadius, height, 0.0, angle, true);
	if (topFace.IsNull()) return nullptr;
	faces.push_back(topFace);

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromSlice(GSlice* po)
{
#ifdef HAS_OCC
	const double radius = po->GetFloatValue(GSlice::RADIUS);
	const double height = po->GetFloatValue(GSlice::HEIGHT);
	const double angle = po->GetFloatValue(GSlice::ANGLE) * M_PI / 180.0;
	const double tol = Precision::Confusion();

	if (radius <= 0.0 || height <= 0.0) return nullptr;
	if (angle <= tol || angle >= 2.0 * M_PI - tol) return nullptr;

	gp_Ax2 cylAxis(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	Handle(Geom_CylindricalSurface) cylSurface =
		new Geom_CylindricalSurface(cylAxis, radius);

	std::vector<TopoDS_Face> faces;

	TopoDS_Face startFace = MakeRadialFace(radius, height, 0.0, true);
	if (startFace.IsNull()) return nullptr;
	faces.push_back(startFace);

	TopoDS_Face arcFace = BRepBuilderAPI_MakeFace(cylSurface, 0.0, angle, 0.0, height, tol);
	if (arcFace.IsNull()) return nullptr;
	faces.push_back(arcFace);

	TopoDS_Face endFace = MakeRadialFace(radius, height, angle, false);
	if (endFace.IsNull()) return nullptr;
	faces.push_back(endFace);

	TopoDS_Face bottomFace = MakeSectorCapFace(radius, 0.0, 0.0, angle, false);
	if (bottomFace.IsNull()) return nullptr;
	faces.push_back(bottomFace);

	TopoDS_Face topFace = MakeSectorCapFace(radius, height, 0.0, angle, true);
	if (topFace.IsNull()) return nullptr;
	faces.push_back(topFace);

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromCone(GCone* po)
{
#ifdef HAS_OCC
	const double bottomRadius = po->BottomRadius();
	const double topRadius = po->TopRadius();
	const double height = po->Height();

	if (bottomRadius < 0.0 || topRadius < 0.0 || height <= 0.0) return nullptr;
	if (bottomRadius <= Precision::Confusion() && topRadius <= Precision::Confusion()) return nullptr;

	const double pi = M_PI;
	const double halfPi = 0.5 * pi;
	const double tol = Precision::Confusion();

	gp_Dir zDir(0.0, 0.0, 1.0);
	gp_Dir xDir(1.0, 0.0, 0.0);
	gp_Pnt baseCenter(0.0, 0.0, 0.0);
	gp_Ax2 cylAxis(baseCenter, zDir, xDir);
	gp_Ax3 coneAxis(baseCenter, zDir, xDir);

	std::vector<TopoDS_Face> faces;

	if (bottomRadius > tol)
	{
		TopoDS_Face bottomFace = MakeDiskFace(bottomRadius, 0.0, false);
		if (bottomFace.IsNull()) return nullptr;

		faces.push_back(bottomFace);
	}

	if (fabs(topRadius - bottomRadius) <= tol)
	{
		Handle(Geom_CylindricalSurface) cylSurface =
			new Geom_CylindricalSurface(cylAxis, bottomRadius);

		for (int i = 0; i < 4; ++i)
		{
			double u0 = i * halfPi;
			double u1 = (i + 1) * halfPi;

			TopoDS_Face face =
				BRepBuilderAPI_MakeFace(cylSurface, u0, u1, 0.0, height, tol);
			if (face.IsNull()) return nullptr;

			faces.push_back(face);
		}
	}
	else
	{
		double semiAngle = atan2(topRadius - bottomRadius, height);
		double v1 = height / cos(semiAngle);

		Handle(Geom_ConicalSurface) coneSurface =
			new Geom_ConicalSurface(coneAxis, semiAngle, bottomRadius);

		for (int i = 0; i < 4; ++i)
		{
			double u0 = i * halfPi;
			double u1 = (i + 1) * halfPi;

			TopoDS_Face face =
				BRepBuilderAPI_MakeFace(coneSurface, u0, u1, 0.0, v1, tol);
			if (face.IsNull()) return nullptr;

			faces.push_back(face);
		}
	}

	if (topRadius > tol)
	{
		TopoDS_Face topFace = MakeDiskFace(topRadius, height, true);
		if (topFace.IsNull()) return nullptr;

		faces.push_back(topFace);
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromTube(GTube* po)
{
#ifdef HAS_OCC
	const double innerRadius = po->InnerRadius();
	const double outerRadius = po->OuterRadius();
	const double height = po->Height();

	if (innerRadius <= 0.0 || outerRadius <= innerRadius || height <= 0.0) return nullptr;

	const double pi = M_PI;
	const double halfPi = 0.5 * pi;
	const double tol = Precision::Confusion();

	gp_Dir zDir(0.0, 0.0, 1.0);
	gp_Dir xDir(1.0, 0.0, 0.0);
	gp_Pnt baseCenter(0.0, 0.0, 0.0);

	gp_Ax2 cylAxis(baseCenter, zDir, xDir);
	Handle(Geom_CylindricalSurface) outerSurface =
		new Geom_CylindricalSurface(cylAxis, outerRadius);
	Handle(Geom_CylindricalSurface) innerSurface =
		new Geom_CylindricalSurface(cylAxis, innerRadius);

	std::vector<TopoDS_Face> faces;

	// Bottom annular cap, split into four 90-degree sectors.
	for (int i = 0; i < 4; ++i)
	{
		double u0 = i * halfPi;
		double u1 = (i + 1) * halfPi;

		TopoDS_Face face = MakeTubeCapFace(innerRadius, outerRadius, 0.0, u0, u1, false);
		if (face.IsNull()) return nullptr;

		faces.push_back(face);
	}

	// Outer cylindrical wall, split into four 90-degree patches.
	for (int i = 0; i < 4; ++i)
	{
		double u0 = i * halfPi;
		double u1 = (i + 1) * halfPi;

		TopoDS_Face face = BRepBuilderAPI_MakeFace(outerSurface, u0, u1, 0.0, height, tol);
		if (face.IsNull()) return nullptr;

		faces.push_back(face);
	}

	// Inner cylindrical wall, split into four 90-degree patches. Reverse so normals face into the bore.
	for (int i = 0; i < 4; ++i)
	{
		double u0 = i * halfPi;
		double u1 = (i + 1) * halfPi;

		TopoDS_Face face = BRepBuilderAPI_MakeFace(innerSurface, u0, u1, 0.0, height, tol);
		if (face.IsNull()) return nullptr;

		face.Reverse();
		faces.push_back(face);
	}

	// Top annular cap, split into four 90-degree sectors.
	for (int i = 0; i < 4; ++i)
	{
		double u0 = i * halfPi;
		double u1 = (i + 1) * halfPi;

		TopoDS_Face face = MakeTubeCapFace(innerRadius, outerRadius, height, u0, u1, true);
		if (face.IsNull()) return nullptr;

		faces.push_back(face);
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromCylinder(GCylinder* po)
{
#ifdef HAS_OCC
	const double radius = po->Radius();
	const double height = po->Height();

	if (radius <= 0.0 || height <= 0.0) return nullptr;

	const double pi = M_PI;
	const double halfPi = 0.5 * pi;
	const double tol = Precision::Confusion();

	gp_Dir zDir(0.0, 0.0, 1.0);
	gp_Dir xDir(1.0, 0.0, 0.0);

	gp_Pnt baseCenter(0.0, 0.0, 0.0);
	gp_Pnt topCenter(0.0, 0.0, height);

	gp_Ax2 cylAxis(baseCenter, zDir, xDir);
	gp_Ax2 bottomAxis(baseCenter, zDir, xDir);
	gp_Ax2 topAxis(topCenter, zDir, xDir);

	Handle(Geom_CylindricalSurface) cylSurface =
		new Geom_CylindricalSurface(cylAxis, radius);

	std::vector<TopoDS_Face> faces;

	// Bottom cap, split into four circular edges.
	{
		gp_Circ bottomCircle(bottomAxis, radius);
		BRepBuilderAPI_MakeWire wireBuilder;

		for (int i = 0; i < 4; ++i)
		{
			double u0 = i * halfPi;
			double u1 = (i + 1) * halfPi;
			wireBuilder.Add(BRepBuilderAPI_MakeEdge(bottomCircle, u0, u1));
		}

		if (!wireBuilder.IsDone()) return nullptr;

		TopoDS_Face bottomFace =
			BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);

		if (bottomFace.IsNull()) return nullptr;

		bottomFace.Reverse();
		faces.push_back(bottomFace);
	}

	// Four cylindrical side faces, each a 90-degree UV patch.
	for (int i = 0; i < 4; ++i)
	{
		double u0 = i * halfPi;
		double u1 = (i + 1) * halfPi;

		TopoDS_Face sideFace =
			BRepBuilderAPI_MakeFace(cylSurface, u0, u1, 0.0, height, tol);

		if (sideFace.IsNull()) return nullptr;

		faces.push_back(sideFace);
	}

	// Top cap, split into four circular edges.
	{
		gp_Circ topCircle(topAxis, radius);
		BRepBuilderAPI_MakeWire wireBuilder;

		for (int i = 0; i < 4; ++i)
		{
			double u0 = i * halfPi;
			double u1 = (i + 1) * halfPi;
			wireBuilder.Add(BRepBuilderAPI_MakeEdge(topCircle, u0, u1));
		}

		if (!wireBuilder.IsDone()) return nullptr;

		TopoDS_Face topFace =
			BRepBuilderAPI_MakeFace(wireBuilder.Wire(), Standard_True);

		if (topFace.IsNull()) return nullptr;

		faces.push_back(topFace);
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* CreateOCCObjectFromSphere(GSphere* po)
{
#ifdef HAS_OCC
	const double radius = po->Radius();
	if (radius <= 0.0) return nullptr;

	const double pi = M_PI;
	const double halfPi = 0.5 * pi;
	const double tol = Precision::Confusion();

	gp_Ax3 sphereAxis(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(0.0, 0.0, 1.0), gp_Dir(1.0, 0.0, 0.0));
	Handle(Geom_SphericalSurface) sphereSurface =
		new Geom_SphericalSurface(sphereAxis, radius);

	std::vector<TopoDS_Face> faces;

	// Four lower and four upper spherical quadrants, matching the GSphere face topology.
	for (int band = 0; band < 2; ++band)
	{
		double v0 = (band == 0 ? -halfPi : 0.0);
		double v1 = (band == 0 ? 0.0 : halfPi);

		for (int i = 0; i < 4; ++i)
		{
			double u0 = i * halfPi;
			double u1 = (i + 1) * halfPi;

			TopoDS_Face face =
				BRepBuilderAPI_MakeFace(sphereSurface, u0, u1, v0, v1, tol);

			if (face.IsNull()) return nullptr;

			faces.push_back(face);
		}
	}

	TopoDS_Solid solid;
	if (!SewFacesToSolid(faces, tol, solid)) return nullptr;

	GOCCObject* occ = new GOCCObject;
	TopoDS_Shape shape = solid;
	occ->SetShape(shape);

	occ->SetName(po->GetName());
	occ->GetTransform() = po->GetTransform();

	return occ;
#else
	return nullptr;
#endif
}

#endif

GOCCObject* CreateOCCObjectFromPrimitive(GPrimitive* po)
{
#ifdef HAS_OCC
	if (dynamic_cast<GBox*>(po)) return CreateOCCObjectFromBox(dynamic_cast<GBox*>(po));
	if (dynamic_cast<GPatch*>(po)) return CreateOCCObjectFromPatch(dynamic_cast<GPatch*>(po));
	if (dynamic_cast<GDisc*>(po)) return CreateOCCObjectFromDisc(dynamic_cast<GDisc*>(po));
	if (dynamic_cast<GRing*>(po)) return CreateOCCObjectFromRing(dynamic_cast<GRing*>(po));
	if (dynamic_cast<GThinTube*>(po)) return CreateOCCObjectFromThinTube(dynamic_cast<GThinTube*>(po));
	if (dynamic_cast<GBoxInBox*>(po)) return CreateOCCObjectFromBoxInBox(dynamic_cast<GBoxInBox*>(po));
	if (dynamic_cast<GCylinderInBox*>(po)) return CreateOCCObjectFromCylinderInBox(dynamic_cast<GCylinderInBox*>(po));
	if (dynamic_cast<GSphereInBox*>(po)) return CreateOCCObjectFromSphereInBox(dynamic_cast<GSphereInBox*>(po));
	if (dynamic_cast<GHollowSphere*>(po)) return CreateOCCObjectFromHollowSphere(dynamic_cast<GHollowSphere*>(po));
	if (dynamic_cast<GTruncatedEllipsoid*>(po)) return CreateOCCObjectFromTruncatedEllipsoid(dynamic_cast<GTruncatedEllipsoid*>(po));
	if (dynamic_cast<GHexagon*>(po)) return CreateOCCObjectFromHexagon(dynamic_cast<GHexagon*>(po));
	if (dynamic_cast<GQuartDogBone*>(po)) return CreateOCCObjectFromQuartDogBone(dynamic_cast<GQuartDogBone*>(po));
	if (dynamic_cast<GTorus*>(po)) return CreateOCCObjectFromTorus(dynamic_cast<GTorus*>(po));
	if (dynamic_cast<GSolidArc*>(po)) return CreateOCCObjectFromSolidArc(dynamic_cast<GSolidArc*>(po));
	if (dynamic_cast<GSlice*>(po)) return CreateOCCObjectFromSlice(dynamic_cast<GSlice*>(po));
	if (dynamic_cast<GCone*>(po)) return CreateOCCObjectFromCone(dynamic_cast<GCone*>(po));
	if (dynamic_cast<GTube*>(po)) return CreateOCCObjectFromTube(dynamic_cast<GTube*>(po));
	if (dynamic_cast<GCylinder*>(po)) return CreateOCCObjectFromCylinder(dynamic_cast<GCylinder*>(po));
	if (dynamic_cast<GSphere*>(po)) return CreateOCCObjectFromSphere(dynamic_cast<GSphere*>(po));
	return nullptr;
#else
	return nullptr;
#endif
}
