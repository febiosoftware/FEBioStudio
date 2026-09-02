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

#include "GOCCObject.h"
#include <MeshTools/NetGenOCCMesher.h>
#include <GLLib/GLMesh.h>
#include <MeshLib/FSMesh.h>
#include "GPrimitive.h"
#include "occ_prim.h"

#ifdef HAS_OCC
#include <gp_Pnt.hxx>
#include <gp_Quaternion.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepTools.hxx>
#include <BOPAlgo_MakerVolume.hxx>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <BRepOffsetAPI_MakeFilling.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepLib.hxx>
#include <Precision.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

#endif // HAS_OCC

#ifdef HAS_OCC
class OCC_Data
{
public:
	OCC_Data(){}

public:
	TopoDS_Shape	m_shape;
};

#else
class OCC_Data {
public:
	OCC_Data(){}
};
#endif // HAS_OCC

GOCCObject::GOCCObject(int type) : GObject(type)
{
	m_occ = new OCC_Data;
	SetFEMesher(new NetGenOCCMesher(*this));
}

FEMesher* GOCCObject::CreateDefaultMesher()
{
	return new NetGenOCCMesher(*this);
}

void GOCCObject::SetShape(TopoDS_Shape& shape, bool bupdate)
{
#ifdef HAS_OCC
	m_occ->m_shape = shape;
	if (bupdate)
	{
		BuildGObject();
		SetRenderMesh(nullptr);
	}
#endif
}

TopoDS_Shape& GOCCObject::GetShape()
{
#ifdef HAS_OCC
	return m_occ->m_shape;
#else
	TopoDS_Shape* dummy = nullptr;
	return *dummy;
#endif
}

FSMeshBase* GOCCObject::GetEditableMesh() { return GetFEMesh(); }
FSLineMesh* GOCCObject::GetEditableLineMesh() { return GetFEMesh(); }

void GOCCObject::BuildGObject()
{
	ClearAll();
#ifdef HAS_OCC

	TopoDS_Shape& shape = m_occ->m_shape;

	// build nodes
	TopTools_IndexedMapOfShape vertexMap;
	TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
	for (int i = 1; i <= vertexMap.Extent(); ++i)
	{
		const TopoDS_Vertex& vertex = TopoDS::Vertex(vertexMap(i));
		gp_Pnt p = BRep_Tool::Pnt(vertex);
		AddNode(vec3d(p.X(), p.Y(), p.Z()));
	}

	// build edges
	TopTools_IndexedMapOfShape edgeMap;
	TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);

	for (int i = 1; i <= edgeMap.Extent(); ++i)
	{
		const TopoDS_Edge& occEdge = TopoDS::Edge(edgeMap(i));

		// Get the two vertices of the edge
		TopoDS_Vertex V1, V2;
		TopExp::Vertices(occEdge, V1, V2);

		// Get the vertex indices in the unique vertex map (note that these are 1-based)
		int n0 = vertexMap.FindIndex(V1) - 1;
		int n1 = vertexMap.FindIndex(V2) - 1;

		GEdge* edge = new GEdge(this);
		edge->SetID(GEdge::CreateUniqueID());
		edge->m_node[0] = n0;
		edge->m_node[1] = n1;
		AddEdge(edge);
	}

	// build faces
	TopTools_IndexedMapOfShape faceMap;
	TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
	for (int i = 1; i <= faceMap.Extent(); ++i)
	{
		GFace* face = new GFace(this);
		AddFace(face);
	}

	// add a part
	if ((m_occ->m_shape.ShapeType() == TopAbs_SOLID) || (m_occ->m_shape.ShapeType() == TopAbs_COMPOUND))
	{
		for (TopExp_Explorer ex(shape, TopAbs_SOLID); ex.More(); ex.Next())
		{
			const TopoDS_Solid& solid = TopoDS::Solid(ex.Current());
			GPart* pg = AddSolidPart();

			// get all the faces of this solid
			TopTools_IndexedMapOfShape solidFaceMap;
			TopExp::MapShapes(solid, TopAbs_FACE, solidFaceMap);

			for (int i = 1; i <= solidFaceMap.Extent(); ++i)
			{
				const TopoDS_Face& face = TopoDS::Face(solidFaceMap(i));
				
				int nf = faceMap.FindIndex(face) - 1; // get the global face index (1-based)
				if ((nf >= 0) && (nf < (int)m_Face.size()))
				{
					GFace* pf = Face(nf);
					int m = 0;
					if (pf->m_nPID[0] >= 0) m++;
					if (pf->m_nPID[1] >= 0) m++;
					pf->m_nPID[m] = pg->GetLocalID();
				}
			}
		}

		for (TopExp_Explorer ex(shape, TopAbs_SHELL); ex.More(); ex.Next())
		{
			const TopoDS_Shell& shell = TopoDS::Shell(ex.Current());
			GPart* pg = AddShellPart();

			// get all the faces of this shell
			TopTools_IndexedMapOfShape shellFaceMap;
			TopExp::MapShapes(shell, TopAbs_FACE, shellFaceMap);

			for (int i = 1; i <= shellFaceMap.Extent(); ++i)
			{
				const TopoDS_Face& face = TopoDS::Face(shellFaceMap(i));

				int nf = faceMap.FindIndex(face) - 1; // get the global face index (1-based)
				if ((nf >= 0) && (nf < (int)m_Face.size()))
				{
					GFace* pf = Face(nf);
					int m = 0;
					if (pf->m_nPID[0] >= 0) m++;
					if (pf->m_nPID[1] >= 0) m++;
					pf->m_nPID[m] = pg->GetLocalID();
				}
			}
		}
	}
	else
	{
		AddShellPart();
		for (int i = 0; i < Faces(); ++i) Face(i)->m_nPID[0] = 0;
	}
#endif
}

void GOCCObject::BuildGMesh()
{
#ifdef HAS_OCC
	TopoDS_Shape& shape = m_occ->m_shape;

	// Generate a mesh
	BRepMesh_IncrementalMesh aMesh(shape, 0.1, true, 0.25);

	// count the nodes and triangles
	Standard_Integer aNbNodes = 0;
	Standard_Integer aNbTriangles = 0;
	Standard_Integer aNbEdges = 0;

	TopTools_IndexedMapOfShape edgeMap;
	TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);

	// calculate total number of the nodes and triangles
	TopTools_IndexedMapOfShape faceMap;
	TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
	for (int i = 1; i <= faceMap.Extent(); ++i)
	{
		const TopoDS_Face& face = TopoDS::Face(faceMap(i));
		TopLoc_Location aLoc;
		Handle(Poly_Triangulation) aTriangulation = BRep_Tool::Triangulation(face, aLoc);
		if (!aTriangulation.IsNull())
		{
			aNbNodes += aTriangulation->NbNodes();
			aNbTriangles += aTriangulation->NbTriangles();
		}

		// count edges
		for (TopExp_Explorer anExpSF(face, TopAbs_EDGE); anExpSF.More(); anExpSF.Next())
		{
			const TopoDS_Edge& aEdge = TopoDS::Edge(anExpSF.Current());

			//try to find PolygonOnTriangulation
			Handle(Poly_PolygonOnTriangulation) aPT;
			BRep_Tool::PolygonOnTriangulation(aEdge, aPT, aTriangulation, aLoc);

			if (!aPT.IsNull()) {
				Standard_Integer nbNodes = aPT->NbNodes();
				aNbEdges += (nbNodes - 1);
			}
		}
	}

	// create a new GLMesh object
	GLMesh* gmesh = new GLMesh;
	gmesh->Create(aNbNodes, aNbTriangles, aNbEdges);

	Standard_Integer aNodeOffset = 0;
	Standard_Integer aTriangleOffet = 0;
	int edges = 0;
	for (int i = 1; i <= faceMap.Extent(); ++i)
	{
		const TopoDS_Face& face = TopoDS::Face(faceMap(i));

		TopLoc_Location aLoc;
		Handle(Poly_Triangulation) aTriangulation = BRep_Tool::Triangulation(face, aLoc);
		if (aTriangulation.IsNull()) continue;

		// copy nodes
		gp_Trsf aTrsf = aLoc.Transformation();
		for (Standard_Integer aNodeIter = 1; aNodeIter <= aTriangulation->NbNodes(); ++aNodeIter)
		{
			GLMesh::NODE& node = gmesh->Node(aNodeIter + aNodeOffset - 1);
			gp_Pnt aPnt = aTriangulation->Node(aNodeIter);
			aPnt.Transform(aTrsf);
			node.r = vec3f(aPnt.X(), aPnt.Y(), aPnt.Z());
		}

		// copy triangles
		const TopAbs_Orientation anOrientation = face.Orientation();
		for (Standard_Integer aTriIter = 1; aTriIter <= aTriangulation->NbTriangles(); ++aTriIter)
		{
			Poly_Triangle aTri = aTriangulation->Triangle(aTriIter);

			Standard_Integer anId[3];
			aTri.Get(anId[0], anId[1], anId[2]);
			if (anOrientation == TopAbs_REVERSED)
			{
				// Swap 1, 2.
				Standard_Integer aTmpIdx = anId[1];
				anId[1] = anId[2];
				anId[2] = aTmpIdx;
			}

			GLMesh::FACE& face = gmesh->Face(aTriIter + aTriangleOffet - 1);
			face.n[0] = anId[0] + aNodeOffset-1;
			face.n[1] = anId[1] + aNodeOffset-1;
			face.n[2] = anId[2] + aNodeOffset-1;
			face.pid = i-1;
			face.sid = i-1;
		}

		for (TopExp_Explorer edgeExp(face, TopAbs_EDGE); edgeExp.More(); edgeExp.Next())
		{
			const TopoDS_Edge& aEdge = TopoDS::Edge(edgeExp.Current());

			//try to find PolygonOnTriangulation
			Handle(Poly_PolygonOnTriangulation) aPT;
			aPT = BRep_Tool::PolygonOnTriangulation(aEdge, aTriangulation, aLoc);

			if (aPT.IsNull() == false)
			{
				int globalIdx = edgeMap.FindIndex(aEdge) - 1; assert(globalIdx >= 0);

				Standard_Integer nbNodes = aPT->NbNodes();
				const TColStd_Array1OfInteger& nodeList = aPT->Nodes();
				for (Standard_Integer j = 1; j < nbNodes; j++, edges++) {
					int inode0 = nodeList.Value(j);
					int inode1 = nodeList.Value(j + 1);

					GLMesh::EDGE& edge = gmesh->Edge(edges);
					edge.n[0] = inode0 - 1 + aNodeOffset;
					edge.n[1] = inode1 - 1 + aNodeOffset;
					edge.pid = globalIdx;
				}
			}
		}

		aNodeOffset += aTriangulation->NbNodes();
		aTriangleOffet += aTriangulation->NbTriangles();
	}

	// update the GLMesh
	gmesh->Update();
	SetRenderMesh(gmesh);

#endif // HAS_OCC
}

void GOCCObject::Save(OArchive& ar)
{
#ifdef HAS_OCC
	ar.BeginChunk(0);
	{
		GObject::Save(ar);
	}
	ar.EndChunk();

	std::stringstream ss;
	BRepTools::Write(m_occ->m_shape, ss);
	ar.WriteChunk(1, ss.str());
#endif
}

void GOCCObject::Load(IArchive& ar)
{
#ifdef HAS_OCC
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case 0: GObject::Load(ar); break;
		case 1:
		{
			std::string s;
			ar.read(s);
			std::stringstream ss(s);
			BRep_Builder aBuilder;
			BRepTools::Read(m_occ->m_shape, ss, aBuilder);
			SetRenderMesh(nullptr);
		}
		break;
		}
		ar.CloseChunk();
	}
#endif
}

//===================================================================
GOCCBottle::GOCCBottle() : GOCCObject(GOCC_BOTTLE)
{
#ifdef HAS_OCC
	AddDoubleParam(1.0, "width");
	AddDoubleParam(1.5, "height");
	AddDoubleParam(0.5, "thickness");
#endif
}

bool GOCCBottle::Update(bool b)
{
#ifdef HAS_OCC

	double h = GetFloatValue(HEIGHT);
	double w = GetFloatValue(WIDTH);
	double t = GetFloatValue(THICKNESS);
	
	if (w <= 0.0) return false;
	if (h <= 0.0) return false;
	if (t <= 0.0) return false;
	
	// make the OCC object
	MakeBottle(h,w,t);

	// build the GObject structure
	BuildGObject();

	return GObject::Update();

#else
	return false;
#endif // HAS_OCC
}

void GOCCBottle::MakeBottle(double h, double w, double t)
{
#ifdef HAS_OCC
	gp_Pnt aPnt1(-w / 2., 0, 0);
	gp_Pnt aPnt2(-w / 2., -t / 4., 0);
	gp_Pnt aPnt3(0, -t / 2., 0);
	gp_Pnt aPnt4(w / 2., -t / 4., 0);
	gp_Pnt aPnt5(w / 2., 0, 0);

	Handle(Geom_TrimmedCurve) aArcOfCircle = GC_MakeArcOfCircle(aPnt2, aPnt3, aPnt4);
	Handle(Geom_TrimmedCurve) aSegment1 = GC_MakeSegment(aPnt1, aPnt2);
	Handle(Geom_TrimmedCurve) aSegment2 = GC_MakeSegment(aPnt4, aPnt5);

	TopoDS_Edge aEdge1 = BRepBuilderAPI_MakeEdge(aSegment1);
	TopoDS_Edge aEdge2 = BRepBuilderAPI_MakeEdge(aArcOfCircle);
	TopoDS_Edge aEdge3 = BRepBuilderAPI_MakeEdge(aSegment2);


	TopoDS_Wire aWire = BRepBuilderAPI_MakeWire(aEdge1, aEdge2, aEdge3);

	const gp_Ax1& xAxis = gp::OX();

	gp_Trsf aTrsf;
	aTrsf.SetMirror(xAxis);

	BRepBuilderAPI_Transform aBRepTrsf(aWire, aTrsf);

	TopoDS_Shape aMirrorShape = aBRepTrsf.Shape();

	TopoDS_Wire aMirroredWire = TopoDS::Wire(aMirrorShape);

	BRepBuilderAPI_MakeWire mkWire;
	mkWire.Add(aWire);
	mkWire.Add(aMirroredWire);
	TopoDS_Wire myWireProfile = mkWire.Wire();

	TopoDS_Face myFaceProfile = BRepBuilderAPI_MakeFace(myWireProfile);

	gp_Vec aPrismVec(0, 0, h);

	TopoDS_Shape myBody = BRepPrimAPI_MakePrism(myFaceProfile, aPrismVec);

	BRepFilletAPI_MakeFillet mkFillet(myBody);

	for (TopExp_Explorer anExpSF(myBody, TopAbs_EDGE); anExpSF.More(); anExpSF.Next())
	{
		TopoDS_Edge anEdge = TopoDS::Edge(anExpSF.Current());
		mkFillet.Add(t / 12.0, anEdge);
	}

	myBody = mkFillet.Shape();

	gp_Pnt neckLocation(0, 0, h);
	gp_Dir neckAxis = gp::DZ();
	gp_Ax2 neckAx2(neckLocation, neckAxis);

	Standard_Real myNeckRadius = t / 4.;
	Standard_Real myNeckHeight = h / 10.;
	BRepPrimAPI_MakeCylinder MKCylinder(neckAx2, myNeckRadius, myNeckHeight);
	TopoDS_Shape myNeck = MKCylinder.Shape();

	myBody = BRepAlgoAPI_Fuse(myBody, myNeck);

	SetShape(myBody, false);

#endif
}

//===================================================================
GOCCBox::GOCCBox() : GOCCObject(GOCC_BOX)
{
}

bool GOCCBox::Update(bool b)
{
#ifdef HAS_OCC
	// make the OCC object
	MakeBox();

	// build the GObject structure
	BuildGObject();

	// build the viz mesh
	SetRenderMesh(nullptr);

	return true;
#else
	return false;
#endif // HAS_OCC
}

void GOCCBox::MakeBox()
{
#ifdef HAS_OCC
	TopoDS_Solid box1 = BRepPrimAPI_MakeBox(gp_Pnt(0, 0, 0), 1, 1, 1);
	SetShape(box1, false);
#endif
}

#ifdef HAS_OCC
static TopoDS_Shape TransformedShape(GOCCObject* po)
{
	Transform T = po->GetTransform();

	vec3d trans = T.GetPosition();
	gp_Vec t(trans.x, trans.y, trans.z);

	quatd q = T.GetRotation();
	gp_Quaternion Q(q.x, q.y, q.z, q.w);

	gp_Trsf trsf;
	trsf.SetTransformation(Q, t);

	BRepBuilderAPI_Transform brepTransform(po->GetShape(), trsf);
	return brepTransform.Shape();
}

static TopoDS_Shell MakeShellFromFaces(const TopoDS_Shape& shape)
{
	BRep_Builder builder;
	TopoDS_Shell shell;
	builder.MakeShell(shell);

	int faces = 0;
	for (TopExp_Explorer it(shape, TopAbs_FACE); it.More(); it.Next())
	{
		builder.Add(shell, TopoDS::Face(it.Current()));
		++faces;
	}

	return (faces > 0 ? shell : TopoDS_Shell());
}
#endif

#ifdef HAS_OCC
bool AllSolids(std::vector<GOCCObject*> occlist)
{
	for (GOCCObject* po : occlist)
	{
		if (po == nullptr) return false;
		if (po->GetShape().ShapeType() != TopAbs_SOLID) return false;
	}
	return true;
}
#endif

#ifdef HAS_OCC
bool AllShells(std::vector<GOCCObject*> occlist)
{
	for (GOCCObject* po : occlist)
	{
		if (po == nullptr) return false;
		if (po->GetShape().ShapeType() != TopAbs_SHELL) return false;
	}
	return true;
}
#endif

#ifdef HAS_OCC
TopoDS_Shape ApplySolidUnion(std::vector<GOCCObject*> occlist)
{
	// empty compound, as nothing has been added yet
	BOPAlgo_MakerVolume aBuilder;
	for (GOCCObject* po : occlist)
	{
		TopoDS_Shape transformedSolid = TransformedShape(po);
		aBuilder.AddArgument(transformedSolid);
	}

	aBuilder.SetIntersect(true);
	aBuilder.SetAvoidInternalShapes(false);
	aBuilder.Perform();
	if (aBuilder.HasErrors())
	{
		return TopoDS_Shape();
	}

	TopoDS_Shape solid = aBuilder.Shape();
	return solid;
}
#endif

#ifdef HAS_OCC
TopoDS_Shape ApplyShellUnion(std::vector<GOCCObject*> occlist)
{
	if (occlist.empty()) return TopoDS_Shape();
	if (occlist[0] == nullptr) return TopoDS_Shape();

	TopoDS_Shape result = TransformedShape(occlist[0]);
	if (result.IsNull()) return TopoDS_Shape();

	for (size_t i = 1; i < occlist.size(); ++i)
	{
		if (occlist[i] == nullptr) return TopoDS_Shape();

		TopoDS_Shape tool = TransformedShape(occlist[i]);
		if (tool.IsNull()) return TopoDS_Shape();

		BRepAlgoAPI_Fuse fuse(result, tool);
		fuse.Build();

		if (!fuse.IsDone() || fuse.HasErrors()) return TopoDS_Shape();

		result = fuse.Shape();
		if (result.IsNull()) return TopoDS_Shape();
	}

	ShapeUpgrade_UnifySameDomain unify(result, Standard_True, Standard_True, Standard_True);
	unify.Build();
	result = unify.Shape();
	if (result.IsNull()) return TopoDS_Shape();

	TopoDS_Shell shell = MakeShellFromFaces(result);
	if (shell.IsNull()) return TopoDS_Shape();

	return shell;
}
#endif

GOCCObject* ApplyBooleanUnion(std::vector<GOCCObject*> occlist)
{
#ifdef HAS_OCC
	TopoDS_Shape shape;
	if (AllSolids(occlist))
	{
		shape = ApplySolidUnion(occlist);
	}
	else if (AllShells(occlist))
	{
		shape = ApplyShellUnion(occlist);
	}
	else return nullptr;
	if (shape.IsNull()) return nullptr;

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(shape);
	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* ApplyBooleanSubtract(std::vector<GOCCObject*> occlist)
{
#ifdef HAS_OCC
	if (occlist.size() < 2) return nullptr;
	if (occlist[0] == nullptr) return nullptr;

	TopoDS_Shape result = TransformedShape(occlist[0]);
	if (result.IsNull()) return nullptr;

	for (size_t i = 1; i < occlist.size(); ++i)
	{
		if (occlist[i] == nullptr) return nullptr;

		TopoDS_Shape tool = TransformedShape(occlist[i]);
		if (tool.IsNull()) return nullptr;

		BRepAlgoAPI_Cut cut(result, tool);
		if (!cut.IsDone() || cut.HasErrors()) return nullptr;

		result = cut.Shape();
		if (result.IsNull()) return nullptr;
	}

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(result);
	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* ApplyBooleanIntersect(std::vector<GOCCObject*> occlist)
{
#ifdef HAS_OCC
	if (occlist.size() < 2) return nullptr;
	if (occlist[0] == nullptr) return nullptr;

	TopoDS_Shape result = TransformedShape(occlist[0]);
	if (result.IsNull()) return nullptr;

	for (size_t i = 1; i < occlist.size(); ++i)
	{
		if (occlist[i] == nullptr) return nullptr;

		TopoDS_Shape tool = TransformedShape(occlist[i]);
		if (tool.IsNull()) return nullptr;

		BRepAlgoAPI_Common common(result, tool);
		if (!common.IsDone() || common.HasErrors()) return nullptr;

		result = common.Shape();
		if (result.IsNull()) return nullptr;
	}

	GOCCObject* occ = new GOCCObject;
	occ->SetShape(result);
	return occ;
#else
	return nullptr;
#endif
}

GOCCObject* ConvertToOCCObject(GObject* po)
{
#ifdef HAS_OCC

	if (dynamic_cast<GPrimitive*>(po)) return CreateOCCObjectFromPrimitive(dynamic_cast<GPrimitive*>(po));

	BRep_Builder B;

	// build the vertices
	vector<TopoDS_Vertex> vertices;
	for (int i = 0; i < po->Nodes(); ++i)
	{
		GNode* node = po->Node(i);
		vec3d r = node->LocalPosition();

		TopoDS_Vertex v = BRepBuilderAPI_MakeVertex(gp_Pnt(r.x, r.y, r.z));

		vertices.push_back(v);
	}

	// build edges
	vector<TopoDS_Edge> edges;
	for (int i = 0; i < po->Edges(); ++i)
	{
		GEdge* edge = po->Edge(i);

		if (edge->Type() == EDGE_LINE)
		{
			int n1 = edge->m_node[0];
			int n2 = edge->m_node[1];
			TopoDS_Edge occEdge = BRepBuilderAPI_MakeEdge(vertices[n1], vertices[n2]);
			if (occEdge.IsNull())
			{
				return nullptr;
			}
			edges.push_back(occEdge);
		}
		else if (edge->Type() == EDGE_3P_CIRC_ARC)
		{
			int n1 = edge->m_node[0];
			int n2 = edge->m_node[1];
			int n3 = edge->m_cnode[0]; // center node
			gp_Pnt p1 = BRep_Tool::Pnt(vertices[n1]);
			gp_Pnt p2 = BRep_Tool::Pnt(vertices[n2]);
			gp_Pnt center = BRep_Tool::Pnt(vertices[n3]);

			// Define plane of the circle (normal required)
			gp_Vec ex(1, 0, 0);
			gp_Vec v1(center, p1);
			gp_Vec v2(center, p2);
			gp_Vec normal = v1.Crossed(v2);

			gp_Ax2 axis(center, normal);
			gp_Circ circ(axis, center.Distance(p1));

			// compute angles
			double angle1 = ex.Angle(v1);

			if (ex.Crossed(v1).Dot(normal) < 0)
				angle1 = 2 * M_PI - angle1;

			double angle2 = angle1 + v1.Angle(v2);

			TopoDS_Edge occEdge = BRepBuilderAPI_MakeEdge(circ, angle1, angle2);
			if (occEdge.IsNull())
			{
				return nullptr;
			}
			edges.push_back(occEdge);
		}
		else
		{
			return nullptr; // unsupported edge type
		}
	}

	// build faces
	vector<TopoDS_Face> faces;
	for (int i = 0; i < po->Faces(); ++i)
	{
		GFace* face = po->Face(i);

		if ((face->m_ntype == FACE_POLYGON) || (face->m_ntype == FACE_QUAD))
		{
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
		else
		{
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
			{
				BRepBuilderAPI_WireError err = makeWire.Error();
				return nullptr;
			}

			BRepOffsetAPI_MakeFilling filling;

			for (TopExp_Explorer exp(makeWire.Wire(), TopAbs_EDGE); exp.More(); exp.Next())
			{
				filling.Add(TopoDS::Edge(exp.Current()), GeomAbs_C0);
			}

			filling.Build();
			if (!filling.IsDone())
			{
				return nullptr;
			}

			TopoDS_Face topoFace = TopoDS::Face(filling.Shape());
			if (topoFace.IsNull())
			{
				return nullptr;
			}

			faces.push_back(topoFace);
		}
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
#else
	return nullptr;
#endif
}

bool MinMaxEdgeLength(GOCCObject* po, double& min, double& max)
{
#ifdef HAS_OCC
	double mnedg = 0.0;
	double mxedg = 0.0;
	TopoDS_Shape& occ = po->GetShape();
	TopExp_Explorer anExp(occ, TopAbs_EDGE);
	for (; anExp.More(); anExp.Next()) {
		const TopoDS_Edge& anEdge = TopoDS::Edge(anExp.Current());
		GProp_GProps props;
		BRepGProp::LinearProperties(anEdge, props);
		double length = props.Mass();
		if (mnedg == 0) mnedg = length;
		else {
			mnedg = fmin(mnedg, length);
			mxedg = fmax(mxedg, length);
		}
	}
	return true;
#else
	return false;
#endif
}

bool GOCCObject::DeletePart(GPart* pg)
{
#ifdef HAS_OCC
	if (pg == nullptr) return false;

	BRep_Builder builder;
	TopoDS_Compound result;
	builder.MakeCompound(result);

	int i = 0;
	bool found = false;
	int count = 0;
	for (TopExp_Explorer ex(GetShape(), TopAbs_SOLID); ex.More(); ex.Next(), ++i)
	{
		if (i != pg->GetLocalID())
		{
			TopoDS_Solid solid = TopoDS::Solid(ex.Current());
			builder.Add(result, solid);
			count++;
		}
		else
		{
			found = true;
		}
	}

	if (!found || count == 0) return false;

	// check for errors
	if (result.IsNull())
	{
		return false;
	}

	SetShape(result, true);

	return true;
#else
	return false;
#endif
}

GObject* GOCCObject::Clone()
{
#ifdef HAS_OCC
	BRepBuilderAPI_Copy copy(GetShape());
	TopoDS_Shape shapeCopy = copy.Shape();

	GOCCObject* po = new GOCCObject;
	po->SetShape(shapeCopy);
	po->SetName(GetName());
	po->GetTransform() = GetTransform();
	return po;
#else
	return nullptr;
#endif
}
