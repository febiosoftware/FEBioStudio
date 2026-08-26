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
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <gp_Ax2.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Circ.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom2d_Line.hxx>
#include <BRepLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopExp_Explorer.hxx>
#include <Precision.hxx>
#endif

#ifdef HAS_OCC
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

	// Sew faces into a shell.
	BRepBuilderAPI_Sewing sewing(tol);

	for (const TopoDS_Face& face : faces)
	{
		sewing.Add(face);
	}

	sewing.Perform();

	TopoDS_Shape sewedShape = sewing.SewedShape();
	if (sewedShape.IsNull()) return nullptr;

	TopoDS_Shell shell;

	if (sewedShape.ShapeType() == TopAbs_SHELL)
	{
		shell = TopoDS::Shell(sewedShape);
	}
	else
	{
		TopExp_Explorer shellExplorer(sewedShape, TopAbs_SHELL);
		if (!shellExplorer.More()) return nullptr;
		shell = TopoDS::Shell(shellExplorer.Current());
	}

	if (shell.IsNull()) return nullptr;

	TopoDS_Solid solid = BRepBuilderAPI_MakeSolid(shell);
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

#endif

GOCCObject* CreateOCCObjectFromPrimitive(GPrimitive* po)
{
#ifdef HAS_OCC
	if (dynamic_cast<GBox*>(po)) return CreateOCCObjectFromBox(dynamic_cast<GBox*>(po));
	if (dynamic_cast<GCylinder*>(po)) return CreateOCCObjectFromCylinder(dynamic_cast<GCylinder*>(po));
	return nullptr;
#else
	return nullptr;
#endif
}
