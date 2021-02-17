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

#include "GObject.h"
#include <MeshLib/triangulate.h>
#include "geom.h"
#include <MeshTools/FEGroup.h>
#include <MeshLib/FECurveMesh.h>
#include <FSCore/FSObjectList.h>
#include <MeshLib/FEMesh.h>
#include <MeshTools/FEMesher.h>
#include <MeshTools/GLMesh.h>
#include <sstream>

using std::stringstream;

class GObject::Imp
{
public:
	Imp()
	{
		m_pmesh   = nullptr;
		m_pMesher = nullptr;
		m_pGMesh  = nullptr;

		m_col = GLColor(200, 200, 200);
	}

	~Imp()
	{
		delete m_pmesh;	m_pmesh = nullptr;
		delete m_pMesher; m_pMesher = nullptr;
		delete m_pGMesh; m_pGMesh = nullptr;
	}

public:
	int	m_ntype;	//!< object type identifier
	GLColor	m_col;	//!< color of object

	FEMesh*		m_pmesh;	//!< the mesh that this object manages
	FEMesher*	m_pMesher;	//!< the mesher builds the actual mesh
	GLMesh*		m_pGMesh;	//!< the mesh for rendering

	FSObjectList<FEPart>		m_pFEPart;
	FSObjectList<FESurface>		m_pFESurf;
	FSObjectList<FEEdgeSet>		m_pFEESet;
	FSObjectList<FENodeSet>		m_pFENSet;
};

//=============================================================================
// GObject
//=============================================================================
// GObject constructor.
GObject::GObject(int ntype): imp(new GObject::Imp)
{
	m_gid = CreateUniqueID();
	imp->m_ntype = ntype;

	// set the state as visible
	m_state = GEO_VISIBLE;
}

//-----------------------------------------------------------------------------
GObject::~GObject(void)
{
	delete imp;
}

//-----------------------------------------------------------------------------
void GObject::Copy(GObject* po)
{
	if (imp->m_pmesh) DeleteFEMesh();
	GBaseObject::Copy(po);
}

//-----------------------------------------------------------------------------
// return type of Object
int GObject::GetType() const { return imp->m_ntype; }

//-----------------------------------------------------------------------------
// get/set object color
GLColor GObject::GetColor() const { return imp->m_col; }

//-----------------------------------------------------------------------------
void GObject::SetColor(const GLColor& c) { imp->m_col = c; }

//-----------------------------------------------------------------------------
// retrieve the mesher
FEMesher* GObject::GetFEMesher() { return imp->m_pMesher; }

//-----------------------------------------------------------------------------
// create a default mesher
FEMesher* GObject::CreateDefaultMesher()
{
	return nullptr;
}

//-----------------------------------------------------------------------------
// retrieve the FE mesh
FEMesh* GObject::GetFEMesh() { return imp->m_pmesh; }

//-----------------------------------------------------------------------------
const FEMesh* GObject::GetFEMesh() const { return imp->m_pmesh; }

//-----------------------------------------------------------------------------
// delete the mesh
void GObject::DeleteFEMesh() { delete imp->m_pmesh; imp->m_pmesh = 0; }

//-----------------------------------------------------------------------------
void GObject::SetFEMesher(FEMesher *pmesher)
{
	imp->m_pMesher = pmesher;
}

//-----------------------------------------------------------------------------
void GObject::SetFEMesh(FEMesh* pm)
{
	imp->m_pmesh = pm; if (pm) pm->SetGObject(this);
}

//-----------------------------------------------------------------------------
// set the render mesh
void GObject::SetRenderMesh(GLMesh* mesh)
{
	delete imp->m_pGMesh;
	imp->m_pGMesh = mesh;
}

//-----------------------------------------------------------------------------
void GObject::ClearFEParts()
{
	imp->m_pFEPart.Clear();
}

//-----------------------------------------------------------------------------
void GObject::ClearFESurfaces()
{
	imp->m_pFESurf.Clear();
}

//-----------------------------------------------------------------------------
void GObject::ClearFEEdgeSets()
{
	imp->m_pFEESet.Clear();
}

//-----------------------------------------------------------------------------
void GObject::ClearFENodeSets()
{
	imp->m_pFENSet.Clear();
}

//-----------------------------------------------------------------------------
// Clear group data
void GObject::ClearFEGroups()
{
	ClearFEParts();
	ClearFESurfaces();
	ClearFENodeSets();
}

//-----------------------------------------------------------------------------
// Remove groups that are empty.

template <class T> void clearVector(FSObjectList<T>& v)
{
	if (v.IsEmpty()) return;

	for (size_t i=0; i<v.Size(); )
	{
		T* o = v[i];
		if (o->size() == 0)
		{
			v.Remove(o);
		}
		else i++;
	}
}

void GObject::RemoveEmptyFEGroups()
{
	clearVector(imp->m_pFEPart);
	clearVector(imp->m_pFESurf);
	clearVector(imp->m_pFEESet);
	clearVector(imp->m_pFENSet);
}

//-----------------------------------------------------------------------------
// Find a surface from its name.
// Returns NULL if the surface cannot be found
FESurface* GObject::FindFESurface(const string& name)
{
	// loop over all surfaces
	for (size_t i = 0; i<imp->m_pFESurf.Size(); ++i)
	{
		FESurface* psi = imp->m_pFESurf[i];
		if (psi->GetName() == name) return psi;
	}

	// sorry, no luck
	return 0;
}

//-----------------------------------------------------------------------------
// Find a node set from its name.
// Returns NULL if the node set cannot be found
FENodeSet* GObject::FindFENodeSet(const string& name)
{
	// loop over all surfaces
	for (size_t i = 0; i<imp->m_pFENSet.Size(); ++i)
	{
		FENodeSet* psi = imp->m_pFENSet[i];
		if (psi->GetName() == name) return psi;
	}

	// sorry, no luck
	return 0;
}

//-----------------------------------------------------------------------------
// Find a group based on its global ID
FEGroup* GObject::FindFEGroup(int nid)
{
	for (int i = 0; i<FEParts(); ++i)
	if (imp->m_pFEPart[i]->GetID() == nid) return imp->m_pFEPart[i];

	for (int i = 0; i<FESurfaces(); ++i)
	if (imp->m_pFESurf[i]->GetID() == nid) return imp->m_pFESurf[i];

	for (int i = 0; i<FEEdgeSets(); ++i)
	if (imp->m_pFEESet[i]->GetID() == nid) return imp->m_pFEESet[i];

	for (int i = 0; i<FENodeSets(); ++i)
	if (imp->m_pFENSet[i]->GetID() == nid) return imp->m_pFENSet[i];

	return 0;
}

//-----------------------------------------------------------------------------
int GObject::FEParts() const { return (int) imp->m_pFEPart.Size(); }

//-----------------------------------------------------------------------------
int GObject::FESurfaces() const { return (int)imp->m_pFESurf.Size(); }

//-----------------------------------------------------------------------------
int GObject::FEEdgeSets() const { return (int)imp->m_pFEESet.Size(); }

//-----------------------------------------------------------------------------
int GObject::FENodeSets() const { return (int)imp->m_pFENSet.Size(); }

//-----------------------------------------------------------------------------
void GObject::AddFEPart(FEPart*    pg) { imp->m_pFEPart.Add(pg); }

//-----------------------------------------------------------------------------
void GObject::AddFESurface(FESurface* pg) { imp->m_pFESurf.Add(pg); }

//-----------------------------------------------------------------------------
void GObject::AddFEEdgeSet(FEEdgeSet* pg) { imp->m_pFEESet.Add(pg); }

//-----------------------------------------------------------------------------
void GObject::AddFENodeSet(FENodeSet* pg) { imp->m_pFENSet.Add(pg); }

//-----------------------------------------------------------------------------
FEPart* GObject::GetFEPart(int n) { return (n >= 0 && n<(int) imp->m_pFEPart.Size() ? imp->m_pFEPart[n] : 0); }

//-----------------------------------------------------------------------------
FESurface* GObject::GetFESurface(int n) { return (n >= 0 && n<(int)imp->m_pFESurf.Size() ? imp->m_pFESurf[n] : 0); }

//-----------------------------------------------------------------------------
FEEdgeSet* GObject::GetFEEdgeSet(int n) { return (n >= 0 && n<(int)imp->m_pFEESet.Size() ? imp->m_pFEESet[n] : 0); }

//-----------------------------------------------------------------------------
FENodeSet* GObject::GetFENodeSet(int n) { return (n >= 0 && n<(int)imp->m_pFENSet.Size() ? imp->m_pFENSet[n] : 0); }

//-----------------------------------------------------------------------------
// Remove a named part from the mesh
int GObject::RemoveFEPart(FEPart* pg)
{
	return imp->m_pFEPart.Remove(pg);
}

//-----------------------------------------------------------------------------
// Remove a named surface from the mesh
int GObject::RemoveFESurface(FESurface* pg)
{
	return imp->m_pFESurf.Remove(pg);
}

//-----------------------------------------------------------------------------
int GObject::RemoveFEEdgeSet(FEEdgeSet* pg)
{
	return imp->m_pFEESet.Remove(pg);
}

//-----------------------------------------------------------------------------
// Remove a named nodeset from the mesh
int GObject::RemoveFENodeSet(FENodeSet* pg)
{
	return imp->m_pFENSet.Remove(pg);
}

//-----------------------------------------------------------------------------
void GObject::InsertFEPart(int n, FEPart* pg) { imp->m_pFEPart.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::InsertFESurface(int n, FESurface* pg) { imp->m_pFESurf.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::InsertFEEdgeSet(int n, FEEdgeSet* pg) { imp->m_pFEESet.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::InsertFENodeSet(int n, FENodeSet* pg) { imp->m_pFENSet.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::CollapseTransform()
{
	// collapse the node positions
	for (int i = 0; i<(int)m_Node.size(); ++i)
	{
		Node(i)->LocalPosition() = Node(i)->Position();
	}

	Transform& transform = GetTransform();

	// collapse the mesh' nodes
	if (imp->m_pmesh)
	{
		FEMesh& m = *imp->m_pmesh;
		for (int i = 0; i<m.Nodes(); ++i)
		{
			FENode& node = m.Node(i);
			node.r = transform.LocalToGlobal(node.r);
		}
	}

	// reset the transform info
	GetTransform().Reset();

	if (imp->m_pmesh) imp->m_pmesh->UpdateMesh();
	Update();
}

//-----------------------------------------------------------------------------
// Assign a material to all parts of this obbject.
void GObject::AssignMaterial(int matid)
{
	for (int i=0; i<Parts(); ++i)
	{
		GPart& g = *m_Part[i];
		g.SetMaterialID(matid);
	}
}

//-----------------------------------------------------------------------------
// Assign a material to a part. The nid parameter is the global ID of the part.
// The pm parameter is a pointer to the material.
// Note that we also assign the material ID to the elements of the mesh, if there
// is a mesh.
void GObject::AssignMaterial(int partid, int matid)
{
	GPart* pg = FindPart(partid);
	assert(pg);
	int gid = -1;
	if (pg) 
	{
		pg->SetMaterialID(matid);
		gid = pg->GetLocalID();
	}
}

//-----------------------------------------------------------------------------
//! This function makes sure that the GNodes correspond to the mesh' nodes
//! When the mesh is transformed independently from the gobject te nodes
//! might not coincide any more and this function should be called. 
//! \todo I think only the GMeshObject requires this functionality so
//!		  maybe I should move that function there.
void GObject::UpdateGNodes()
{
	FELineMesh* pm = GetEditableLineMesh();
	if (pm == 0) return;
	for (int i=0; i<pm->Nodes(); ++i)
	{
		FENode& n = pm->Node(i);
		if (n.m_gid >= 0) m_Node[n.m_gid]->LocalPosition() = n.r;
	}

	BuildGMesh();
}

//-----------------------------------------------------------------------------
// Replace the current mesh. Note that we don't delete the current mesh since
// it is assumed that another class will take care of that.
void GObject::ReplaceFEMesh(FEMesh* pm, bool bup, bool bdel)
{
	if (bdel) delete imp->m_pmesh;
	SetFEMesh(pm);
	Update(bup);
}

//-----------------------------------------------------------------------------
// Replace the current mesh. Note that we don't delete the current mesh since
// it is assumed that another class will take care of that.
void GObject::ReplaceSurfaceMesh(FESurfaceMesh* pm)
{
	assert(false);
}

//-----------------------------------------------------------------------------
bool GObject::Update(bool b)
{
	BuildGMesh();
	return FSObject::Update(b);
}

//-----------------------------------------------------------------------------
// \todo This function deletes the old mesh. However, the tetgen remesher
//		uses the old mesh to create the new mesh and when the user undoes the last
//		mesh we need to be able to restore that mesh. Therefore, we should not delete
//		the old mesh.
FEMesh* GObject::BuildMesh()
{
	if (imp->m_pMesher)
	{
		// keep a pointer to the old mesh since some mesher use the old
		// mesh to create a new mesh
		FEMesh* pold = imp->m_pmesh;
		SetFEMesh(imp->m_pMesher->BuildMesh());

		// now it is safe to delete the old mesh
		if (pold) delete pold;

		Update();
		return imp->m_pmesh;
	}
	else return 0;
}

//-----------------------------------------------------------------------------
FENode* GObject::GetFENode(int gid)
{
	FEMesh* pm = GetFEMesh();
	if (pm == 0) return 0;

	for (int i=0; i<pm->Nodes(); ++i)
	{
		if (pm->Node(i).m_gid == gid) return pm->NodePtr(i);
	}
	return 0;
}

//-----------------------------------------------------------------------------
BOX GObject::GetLocalBox() const
{
	if (imp->m_pGMesh) return imp->m_pGMesh->GetBoundingBox();

	BOX b;
	int N = Nodes();
	if (N > 0)
	{
		b = BOX(m_Node[0]->LocalPosition(), m_Node[0]->LocalPosition());
		for (int i = 1; i<N; ++i) b += m_Node[i]->LocalPosition();
	}
	return b;
}

//-----------------------------------------------------------------------------
// get the global bounding box
BOX GObject::GetGlobalBox() const
{
	BOX box = GetLocalBox();
	vec3d r0 = vec3d(box.x0, box.y0, box.z0);
	vec3d r1 = vec3d(box.x1, box.y1, box.z1);
	r0 = GetTransform().LocalToGlobal(r0);
	r1 = GetTransform().LocalToGlobal(r1);
	return BOX(r0.x, r0.y, r0.z, r1.x, r1.y, r1.z);
}

//-----------------------------------------------------------------------------
bool GObject::IsFaceVisible(const GFace* pf) const
{
	// see if the object is visible
	if (IsVisible() == false) return false;

	// see if the parts are visible
	int pid0 = pf->m_nPID[0]; assert(pid0 >= 0);
	int pid1 = pf->m_nPID[1]; 

	const GPart* p0 = Part(pid0);
	const GPart* p1 = (pid1 >= 0 ? Part(pid1) : 0);

	if (!p0->IsVisible() && ((p1 == 0) || !p1->IsVisible())) return false;

	// see if the face is visible
	return pf->IsVisible();
}

//-----------------------------------------------------------------------------
// get the render mesh
GLMesh*	GObject::GetRenderMesh()
{ 
	return imp->m_pGMesh;
}

//-----------------------------------------------------------------------------
void GObject::BuildGMesh()
{
	// delete the old mesh
	delete imp->m_pGMesh;
	imp->m_pGMesh = nullptr;

	// create a new mesh
	GLMesh* gmesh = new GLMesh;

	// the render mesh is created based of the faces (if there are any)
	int NF = Faces();
	if (NF > 0)
	{
		// repeat for all faces
		for (int j = 0; j<NF; ++j)
		{
			GFace& f = *Face(j);

			switch (f.m_ntype)
			{
			case FACE_POLYGON      : BuildFacePolygon     (gmesh, f); break;
			case FACE_EXTRUDE      : BuildFaceExtrude     (gmesh, f); break;
			case FACE_QUAD         : BuildFaceQuad        (gmesh, f); break;
			case FACE_REVOLVE      : BuildFaceRevolve     (gmesh, f); break;
			case FACE_REVOLVE_WEDGE: BuildFaceRevolveWedge(gmesh, f); break;
			default:
				assert(false);
			}
		}
	}
	else
	{
		// if there are no faces, we build a line mesh of the edge curves
		int NC = Edges();
		if (NC > 0)
		{
			for (int i=0; i<NC; ++i)
			{
				GEdge& e = *Edge(i);
				switch (e.Type())
				{
				case EDGE_MESH: BuildEdgeMesh(gmesh, e); break;
				default:
					assert(false);
				}
			}
		}
		else
		{
			// just add the nodes
			int NN = Nodes();
			for (int i=0; i<NN; ++i)
			{
				GNode* ni = Node(i);
				gmesh->AddNode(ni->LocalPosition(), ni->GetID());
			}
			gmesh->Update();
		}
	}

	// assign new mesh
	imp->m_pGMesh = gmesh;
}

//-----------------------------------------------------------------------------
void GObject::BuildEdgeMesh(GLMesh* glmsh, GEdge& e)
{
	// This assumes there is a mesh
	FECurveMesh* mesh = GetFECurveMesh(e.GetLocalID());
	assert(mesh);
	if (mesh == 0) return;

	// tag all nodes
	mesh->TagAllNodes(-1);

	// get the local edge id
	int edgeID = e.GetLocalID();
	for (int i=0; i<mesh->Edges(); ++i)
	{
		FEEdge& edge = mesh->Edge(i);
		mesh->Node(edge.n[0]).m_ntag = 0;
		mesh->Node(edge.n[1]).m_ntag = 0;
	}

	// get all the nodes
	int NN = 0;
	for (int i = 0; i<mesh->Nodes(); ++i)
		if (mesh->Node(i).m_ntag != -1) mesh->Node(i).m_ntag = NN++;

	GLMesh glMesh;
	glMesh.Create(NN, 0);
	for (int i = 0; i<mesh->Nodes(); ++i)
	{
		FENode& node = mesh->Node(i);
		if (node.m_ntag != -1)
		{
			GLMesh::NODE& gnode = glMesh.Node(node.m_ntag);
			gnode.r = node.pos();
		} 
	}

	// add all the edges
	int n[2];
	for (int i = 0; i<mesh->Edges(); ++i)
	{
		FEEdge& e = mesh->Edge(i);
		n[0] = mesh->Node(e.n[0]).m_ntag;
		n[1] = mesh->Node(e.n[1]).m_ntag;
		glMesh.AddEdge(n, 2, edgeID);
	}

	// clean up
	delete mesh;

	// update mesh internal data structures
	glMesh.Update();

	// attach it to the render mesh
	glmsh->Attach(glMesh);
}

//-----------------------------------------------------------------------------
void GObject::BuildFacePolygon(GLMesh* glmesh, GFace& f)
{
	const int M = 50;

	// find the face normal
	vec3d fn(0,0,0);
	vec3d rc = Node(f.m_node[0])->LocalPosition();
	for (int i=1; i<f.Nodes()-1; ++i)
	{
		vec3d r1 = Node(f.m_node[i])->LocalPosition() - rc;
		vec3d r2 = Node(f.m_node[i + 1])->LocalPosition() - rc;
		fn += r1^r2;
	}
	fn.Normalize();

	// find the rotation to bring it back to the x-y plane
	quatd q(fn, vec3d(0,0,1)), qi = q.Inverse();

	// create triangulation object
	GTriangulate c;
	c.Clear();

	// create all nodes
	int ne = f.Edges();
	for (int i=0; i<ne; ++i)
	{
		GEdge& e = *Edge(f.m_edge[i].nid);
		int ew = f.m_edge[i].nwn;
		int en0 = (ew == 1? e.m_node[0] : e.m_node[1]);
		int en1 = (ew == 1? e.m_node[1] : e.m_node[0]);
		int n0 = Node(en0)->GetLocalID();
		switch (e.m_ntype)
		{
		case EDGE_LINE:
			{
				vec3d r = Node(en0)->LocalPosition() - rc;
				q.RotateVector(r);
				c.AddNode(r, n0);
			}
			break;
		case EDGE_3P_CIRC_ARC:
			{
				vec3d r0 = Node(e.m_cnode)->LocalPosition() - rc;
				vec3d r1 = Node(e.m_node[0])->LocalPosition() - rc;
				vec3d r2 = Node(e.m_node[1])->LocalPosition() - rc;
				q.RotateVector(r0);
				q.RotateVector(r1);
				q.RotateVector(r2);

				vec2d a0(r0.x, r0.y);
				vec2d a1(r1.x, r1.y);
				vec2d a2(r2.x, r2.y);

				GM_CIRCLE_ARC ca(a0, a1, a2);

				if (ew == 1) c.AddNode(r1, n0); else c.AddNode(r2, n0);

				int j0, j1, ji;
				if (f.m_edge[i].nwn == 1) { j0 = 1; j1 = M; ji = 1; } else { j0 = M-1; j1 = 0; ji = -1; }
				for (int j=j0; j != j1; j += ji)
				{
					double l = (double) j / (double) M;
					c.AddNode(ca.Point(l), -1);
				}
			}
			break;
		case EDGE_3P_ARC:
			{
				vec3d r0 = Node(e.m_cnode)->LocalPosition() - rc;
				vec3d r1 = Node(e.m_node[0])->LocalPosition() - rc;
				vec3d r2 = Node(e.m_node[1])->LocalPosition() - rc;
				q.RotateVector(r0);
				q.RotateVector(r1);
				q.RotateVector(r2);

				vec2d a0(r0.x, r0.y);
				vec2d a1(r1.x, r1.y);
				vec2d a2(r2.x, r2.y);

				GM_ARC ca(a0, a1, a2);

				if (ew == 1) c.AddNode(r1, n0); else c.AddNode(r2, n0);

				int j0, j1, ji;
				if (f.m_edge[i].nwn == 1) { j0 = 1; j1 = M; ji = 1; } else { j0 = M-1; j1 = 0; ji = -1; }
				for (int j=j0; j != j1; j += ji)
				{
					double l = (double) j / (double) M;
					c.AddNode(ca.Point(l), -1);
				}
			}
			break;
		default:
			assert(false);
		}
	}

	// create all edges
	int NN = c.Nodes();
	int m = 0;
	for (int i=0; i<ne; ++i)
	{
		GEdge& e = *Edge(f.m_edge[i].nid);
		int eid = e.GetLocalID();
		switch (e.m_ntype)
		{
		case EDGE_LINE:
			{
				int n0 = m++;
				int n1 = (n0+1)%NN;
				c.AddEdge(n0, n1, eid);
			}
			break;
		case EDGE_3P_CIRC_ARC:
			for (int j=0; j<M; ++j)
			{
				int n0 = m++;
				int n1 = (n0+1)%NN;
				c.AddEdge(n0, n1, eid);
			}
			break;
		case EDGE_3P_ARC:
			for (int j=0; j<M; ++j)
			{
				int n0 = m++;
				int n1 = (n0+1)%NN;
				c.AddEdge(n0, n1, eid);
			}
			break;
		default:
			assert(false);
		}
	}

	// triangulate the face
	GLMesh* pm = triangulate(c);

	// Position the face at the correct position
	for (int i=0; i<pm->Nodes(); ++i) 
	{
		vec3d r = pm->Node(i).r;
		qi.RotateVector(r);
		r += rc;
		pm->Node(i).r = r;
	}

	// set the proper face IDs
	for (int i=0; i<pm->Faces(); ++i) pm->Face(i).pid = f.GetLocalID();

	// attach this mesh to our mesh
	glmesh->Attach(*pm);

	// don't forget to delete this mesh
	delete pm;
}

//-----------------------------------------------------------------------------
// Here, we assume that the face is built from extruding an edge.
// It must be true that the face has four nodes and four edges.
// Edges 0 and 2 must be of the same type and edges 1 and 3 must be
// straight lines.
void GObject::BuildFaceExtrude(GLMesh* glmesh, GFace& f)
{
	const int M = 50;

	// get number of nodes and edges
	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN==4);
	assert(NE==4);

	// get the edges
	GEdge& e0 = *Edge(f.m_edge[0].nid);
	GEdge& e1 = *Edge(f.m_edge[1].nid);
	GEdge& e2 = *Edge(f.m_edge[2].nid);
	GEdge& e3 = *Edge(f.m_edge[3].nid);
	assert(e0.m_ntype == e2.m_ntype);
	assert(e1.m_ntype == EDGE_LINE);
	assert(e3.m_ntype == EDGE_LINE);

	// this is the mesh we'll be building
	GLMesh m;

	// build the face
	switch (e0.m_ntype)
	{
	case EDGE_LINE: // TODO: create a finer mesh
		{
			m.Create(4, 2, 4);
			for (int i=0; i<4; ++i)
			{
				GMesh::NODE& ni = m.Node(i);
				ni.r = Node(f.m_node[i])->LocalPosition();
				ni.pid = Node(f.m_node[i])->GetLocalID();
			}

			for (int i=0; i<4; ++i)
			{
				GMesh::EDGE& ei = m.Edge(i);
				ei.n[0] = i;
				ei.n[1] = (i+1)%4;
				ei.pid = Edge(f.m_edge[i].nid)->GetLocalID();
			}

			GMesh::FACE& f0 = m.Face(0);
			GMesh::FACE& f1 = m.Face(1);

			f0.n[0] = 0; 
			f0.n[1] = 1;
			f0.n[2] = 2;
			f0.pid = f.GetLocalID();

			f1.n[0] = 2; 
			f1.n[1] = 3;
			f1.n[2] = 0;
			f1.pid = f.GetLocalID();
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			// get the nodes of the bottom edge
			vec3d r0 = Node(e0.m_cnode)->LocalPosition();
			int n0 = 0, n1 = 1;
			if (f.m_edge[0].nwn == -1) { n0 = 1; n1 = 0; }
			vec3d r1 = Node(e0.m_node[n0])->LocalPosition();
			vec3d r2 = Node(e0.m_node[n1])->LocalPosition();

			// get the extrusion direction
			vec3d t;
			if (f.m_edge[1].nwn == 1) t = Node(e1.m_node[1])->LocalPosition() - Node(e1.m_node[0])->LocalPosition();
			else  t = Node(e1.m_node[0])->LocalPosition() - Node(e1.m_node[1])->LocalPosition();

			// project the points on a plane
			vec2d a0(r0.x, r0.y);
			vec2d a1(r1.x, r1.y);
			vec2d a2(r2.x, r2.y);

			// create an arc object
			GM_CIRCLE_ARC ca(a0, a1, a2, f.m_edge[0].nwn);

			// allocate the mesh
			m.Create(2*(M+1), 2*M, 2*M+2);

			// create nodes
			for (int i=0; i<=M; ++i)
			{
				GMesh::NODE& ni = m.Node(i);
				GMesh::NODE& nj = m.Node(i+(M+1));

				ni.r = ca.Point(i/(double) M);
				nj.r = ni.r + t;
				ni.pid = -1;
				nj.pid = -1;
			}

			// mark the corner nodes
			m.Node(    0).pid = Node(e0.m_node[n0])->GetLocalID();
			m.Node(    M).pid = Node(e0.m_node[n1])->GetLocalID();

			int m0 = 0, m1 = 1;
			if (f.m_edge[2].nwn == -1) { m0 = 1; m1 = 0; }

			m.Node(  M+1).pid = Node(e2.m_node[m1])->GetLocalID();
			m.Node(2*M+1).pid = Node(e2.m_node[m0])->GetLocalID();

			// create the faces
			for (int i=0; i<M; ++i)
			{
				GMesh::FACE& f0 = m.Face(2*i);
				GMesh::FACE& f1 = m.Face(2*i+1);

				f0.n[0] = i;
				f0.n[1] = i+1;
				f0.n[2] = (M+1) + i+1;
				f0.pid = f.GetLocalID();

				f1.n[0] = (M+1) + i+1;
				f1.n[1] = (M+1) + i;
				f1.n[2] = i;
				f1.pid = f.GetLocalID();
			}

			// create the edges
			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e0 = m.Edge(2*i);
				GMesh::EDGE& e1 = m.Edge(2*i+1);

				e0.n[0] = i;
				e0.n[1] = i+1;
				e0.pid = Edge(f.m_edge[0].nid)->GetLocalID();

				e1.n[0] = (M+1) + i;
				e1.n[1] = (M+1) + i+1;
				e1.pid = Edge(f.m_edge[2].nid)->GetLocalID();
			}

			GMesh::EDGE& e0 = m.Edge(2*M);
			GMesh::EDGE& e1 = m.Edge(2*M+1);

			e0.n[0] = 0;
			e0.n[1] = M+1;
			e0.pid = Edge(f.m_edge[3].nid)->GetLocalID();

			e1.n[0] = M;
			e1.n[1] = 2*M+1;
			e1.pid = Edge(f.m_edge[1].nid)->GetLocalID();
		}
		break;
	case EDGE_3P_ARC:
		{
			// get the nodes of the bottom edge
			vec3d r0 = Node(e0.m_cnode)->LocalPosition();
			int n0 = 0, n1 = 1;
			if (f.m_edge[0].nwn == -1) { n0 = 1; n1 = 0; }
			vec3d r1 = Node(e0.m_node[n0])->LocalPosition();
			vec3d r2 = Node(e0.m_node[n1])->LocalPosition();

			// get the extrusion direction
			vec3d t;
			if (f.m_edge[1].nwn == 1) t = Node(e1.m_node[1])->LocalPosition() - Node(e1.m_node[0])->LocalPosition();
			else  t = Node(e1.m_node[0])->LocalPosition() - Node(e1.m_node[1])->LocalPosition();

			// project the points on a plane
			vec2d a0(r0.x, r0.y);
			vec2d a1(r1.x, r1.y);
			vec2d a2(r2.x, r2.y);

			// create an arc object
			GM_ARC ca(a0, a1, a2, f.m_edge[0].nwn);

			// allocate the mesh
			m.Create(2*(M+1), 2*M, 2*M+2);

			// create nodes
			for (int i=0; i<=M; ++i)
			{
				GMesh::NODE& ni = m.Node(i);
				GMesh::NODE& nj = m.Node(i+(M+1));

				ni.r = ca.Point(i/(double) M);
				nj.r = ni.r + t;
				ni.pid = -1;
				nj.pid = -1;
			}

			// mark the corner nodes
			m.Node(    0).pid = Node(e0.m_node[n0])->GetLocalID();
			m.Node(    M).pid = Node(e0.m_node[n1])->GetLocalID();

			int m0 = 0, m1 = 1;
			if (f.m_edge[2].nwn == -1) { m0 = 1; m1 = 0; }

			m.Node(  M+1).pid = Node(e2.m_node[m1])->GetLocalID();
			m.Node(2*M+1).pid = Node(e2.m_node[m0])->GetLocalID();

			// create the faces
			for (int i=0; i<M; ++i)
			{
				GMesh::FACE& f0 = m.Face(2*i);
				GMesh::FACE& f1 = m.Face(2*i+1);

				f0.n[0] = i;
				f0.n[1] = i+1;
				f0.n[2] = (M+1) + i+1;
				f0.pid = f.GetLocalID();

				f1.n[0] = (M+1) + i+1;
				f1.n[1] = (M+1) + i;
				f1.n[2] = i;
				f1.pid = f.GetLocalID();
			}

			// create the edges
			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e0 = m.Edge(2*i);
				GMesh::EDGE& e1 = m.Edge(2*i+1);

				e0.n[0] = i;
				e0.n[1] = i+1;
				e0.pid = Edge(f.m_edge[0].nid)->GetLocalID();

				e1.n[0] = (M+1) + i;
				e1.n[1] = (M+1) + i+1;
				e1.pid = Edge(f.m_edge[2].nid)->GetLocalID();
			}

			GMesh::EDGE& e0 = m.Edge(2*M);
			GMesh::EDGE& e1 = m.Edge(2*M+1);

			e0.n[0] = 0;
			e0.n[1] = M+1;
			e0.pid = Edge(f.m_edge[3].nid)->GetLocalID();

			e1.n[0] = M;
			e1.n[1] = 2*M+1;
			e1.pid = Edge(f.m_edge[1].nid)->GetLocalID();
		}
		break;
	case EDGE_ZARC:
		{
			// get the nodes of the bottom edge
			vec3d r0 = vec3d(0,0,0);
			int n0 = 0, n1 = 1;
			if (f.m_edge[0].nwn == -1) { n0 = 1; n1 = 0; }
			vec3d r1 = Node(e0.m_node[n0])->LocalPosition();
			vec3d r2 = Node(e0.m_node[n1])->LocalPosition();

			// get the extrusion direction
			vec3d t;
			if (f.m_edge[1].nwn == 1) t = Node(e1.m_node[1])->LocalPosition() - Node(e1.m_node[0])->LocalPosition();
			else  t = Node(e1.m_node[0])->LocalPosition() - Node(e1.m_node[1])->LocalPosition();

			// project the points on a plane
			vec2d a0(r0.x, r0.y);
			vec2d a1(r1.x, r1.y);
			vec2d a2(r2.x, r2.y);

			// create an arc object
			GM_CIRCLE_ARC ca(a0, a1, a2, f.m_edge[0].nwn);

			// allocate the mesh
			m.Create(2*(M+1), 2*M, 2*M+2);

			// create nodes
			for (int i=0; i<=M; ++i)
			{
				GMesh::NODE& ni = m.Node(i);
				GMesh::NODE& nj = m.Node(i+(M+1));

				ni.r = ca.Point(i/(double) M);
				nj.r = ni.r + t;
				ni.pid = -1;
				nj.pid = -1;
			}

			// mark the corner nodes
			m.Node(    0).pid = Node(e0.m_node[n0])->GetLocalID();
			m.Node(    M).pid = Node(e0.m_node[n1])->GetLocalID();

			int m0 = 0, m1 = 1;
			if (f.m_edge[2].nwn == -1) { m0 = 1; m1 = 0; }

			m.Node(  M+1).pid = Node(e2.m_node[m1])->GetLocalID();
			m.Node(2*M+1).pid = Node(e2.m_node[m0])->GetLocalID();

			// create the faces
			for (int i=0; i<M; ++i)
			{
				GMesh::FACE& f0 = m.Face(2*i);
				GMesh::FACE& f1 = m.Face(2*i+1);

				f0.n[0] = i;
				f0.n[1] = i+1;
				f0.n[2] = (M+1) + i+1;
				f0.pid = f.GetLocalID();

				f1.n[0] = (M+1) + i+1;
				f1.n[1] = (M+1) + i;
				f1.n[2] = i;
				f1.pid = f.GetLocalID();
			}

			// create the edges
			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e0 = m.Edge(2*i);
				GMesh::EDGE& e1 = m.Edge(2*i+1);

				e0.n[0] = i;
				e0.n[1] = i+1;
				e0.pid = Edge(f.m_edge[0].nid)->GetLocalID();

				e1.n[0] = (M+1) + i;
				e1.n[1] = (M+1) + i+1;
				e1.pid = Edge(f.m_edge[2].nid)->GetLocalID();
			}

			GMesh::EDGE& e0 = m.Edge(2*M);
			GMesh::EDGE& e1 = m.Edge(2*M+1);

			e0.n[0] = 0;
			e0.n[1] = M+1;
			e0.pid = Edge(f.m_edge[3].nid)->GetLocalID();

			e1.n[0] = M;
			e1.n[1] = 2*M+1;
			e1.pid = Edge(f.m_edge[1].nid)->GetLocalID();
		}
		break;
	default:
		assert(false);
	}

	m.Update();
	m.UpdateNormals();
	glmesh->Attach(m);
}

//-----------------------------------------------------------------------------
// Build a revolved surface
// The revolved surface has four edges, the two side ones of type EDGE_YARC
void GObject::BuildFaceRevolve(GLMesh* glmesh, GFace& f)
{
#ifdef _DEBUG
	const int M = 5;
#else
	const int M = 50;
#endif

	// get number of nodes and edges
	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN==4);
	assert(NE==4);

	// get the edges
	GEdge& e0 = *Edge(f.m_edge[0].nid);
	GEdge& e1 = *Edge(f.m_edge[1].nid);
	GEdge& e2 = *Edge(f.m_edge[2].nid);
	GEdge& e3 = *Edge(f.m_edge[3].nid);
	assert(e0.m_ntype == e2.m_ntype);
	assert((e1.m_ntype == EDGE_YARC)||(e1.m_ntype == EDGE_ZARC));
	assert((e3.m_ntype == EDGE_YARC)||(e3.m_ntype == EDGE_ZARC));

	// this is the mesh we'll be building
	GLMesh m;

	// build the mesh
	switch (e0.m_ntype)
	{
	case EDGE_LINE:
		{
			m.Create(2*(M+1), 2*M, 2*M + 2);
			vec3d r0 = Node(f.m_node[0])->LocalPosition();
			vec3d r1 = Node(f.m_node[1])->LocalPosition();
			vec3d r2 = Node(f.m_node[2])->LocalPosition();
			vec3d r3 = Node(f.m_node[3])->LocalPosition();
			double y0 = r0.y;
			double y1 = r1.y;

			vec2d a0(r0.x, r0.z), a1(r1.x, r1.z);
			vec2d b0(r3.x, r3.z), b1(r2.x, r2.z);

			// create an arc object
			GM_CIRCLE_ARC c0(vec2d(0,0), a0, b0);
			GM_CIRCLE_ARC c1(vec2d(0,0), a1, b1);
			
			for (int i=0; i<=M; ++i)
			{
				double t = (double) i / (double) M;

				GMesh::NODE& n0 = m.Node(2*i);
				GMesh::NODE& n1 = m.Node(2*i+1);
				vec2d p0 = c0.Point(t);
				vec2d p1 = c1.Point(t);
				n0.r = vec3d(p0.x, y0, p0.y);
				n1.r = vec3d(p1.x, y1, p1.y);
				n0.pid = -1;
				n1.pid = -1;
			}

			m.Node(0).pid = Node(f.m_node[0])->GetLocalID();
			m.Node(1).pid = Node(f.m_node[1])->GetLocalID();
			m.Node(2*(M+1)-1).pid = Node(f.m_node[2])->GetLocalID();
			m.Node(2*(M+1)-2).pid = Node(f.m_node[3])->GetLocalID();

			// add edges
			GMesh::EDGE& e0 = m.Edge(0);
			GMesh::EDGE& e1 = m.Edge(1);
			e0.n[0] = 0; e0.n[1] = 1; e0.pid = Edge(f.m_edge[0].nid)->GetLocalID();
			e1.n[0] = 2*(M+1)-2; e1.n[1] = 2*(M+1)-1; e1.pid = Edge(f.m_edge[2].nid)->GetLocalID();

			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e0 = m.Edge(2 + 2*i);
				GMesh::EDGE& e1 = m.Edge(2 + 2*i + 1);
				e0.n[0] = 2*i  ; e0.n[1] = 2*(i+1)  ; e0.pid = Edge(f.m_edge[3].nid)->GetLocalID();
				e1.n[0] = 2*i+1; e1.n[1] = 2*(i+1)+1; e1.pid = Edge(f.m_edge[1].nid)->GetLocalID();
			}

			for (int i=0; i<M; ++i)
			{
				GMesh::FACE& f0 = m.Face(2*i);
				GMesh::FACE& f1 = m.Face(2*i+1);

				f0.n[0] = 2*i; 
				f0.n[1] = 2*i+1;
				f0.n[2] = 2*(i+1);
				f0.pid = f.GetLocalID();

				f1.n[0] = 2*(i+1); 
				f1.n[1] = 2*i+1;
				f1.n[2] = 2*(i+1)+1;
				f1.pid = f.GetLocalID();
			}
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = Node(f.m_node[0])->LocalPosition();
			vec3d r1 = Node(f.m_node[1])->LocalPosition();
			vec3d r2 = Node(f.m_node[2])->LocalPosition();
			vec3d r3 = Node(f.m_node[3])->LocalPosition();
			m.Create((M+1)*(M+1), 2*M*M, 4*M);

			for (int i=0; i<=M; ++i)
			{
				double s = (double) i / (double) M;

				// get the points on the edges
				vec3d p0 = e0.Point(s);
				vec3d p3 = e2.Point(s);

				switch (e1.m_ntype)
				{
				case EDGE_YARC:
					{
						double y = p0.y;
						// set-up an arc
						vec2d a(p0.x, p0.z), b(p3.x, p3.z);
						GM_CIRCLE_ARC c(vec2d(0,0), a, b);

						for (int j=0; j<=M; ++j)
						{
							double t = (double) j / (double) M;

							vec2d p = c.Point(t);
							vec3d r = vec3d(p.x, y, p.y);

							GMesh::NODE& nd = m.Node(i*(M+1)+j);
							nd.r = r;
							nd.pid = -1;
						}
					}
					break;
				case EDGE_ZARC:
					{
						double z = p0.z;
						// set-up an arc
						vec2d a(p0.x, p0.y), b(p3.x, p3.y);
						GM_CIRCLE_ARC c(vec2d(0,0), a, b);

						for (int j=0; j<=M; ++j)
						{
							double t = (double) j / (double) M;

							vec2d p = c.Point(t);
							vec3d r = vec3d(p.x, p.y, z);

							GMesh::NODE& nd = m.Node(i*(M+1)+j);
							nd.r = r;
							nd.pid = -1;
						}
					}
					break;
				default:
					assert(false);
				}
			}

			m.Node(0            ).pid = Node(f.m_node[0])->GetLocalID();
			m.Node(M*(M+1)      ).pid = Node(f.m_node[1])->GetLocalID();
			m.Node((M+1)*(M+1)-1).pid = Node(f.m_node[2])->GetLocalID();
			m.Node(M            ).pid = Node(f.m_node[3])->GetLocalID();

			// add faces
			for (int i=0; i<M; ++i)
			{
				for (int j=0; j<M; ++j)
				{
					GMesh::FACE& f0 = m.Face(i*(2*M) + 2*j);
					f0.n[0] = i*(M+1) + j;
					f0.n[1] = i*(M+1) + j+1;
					f0.n[2] = (i+1)*(M+1) + j;
					f0.pid = f.GetLocalID();

					GMesh::FACE& f1 = m.Face(i*(2*M) + 2*j + 1);
					f1.n[0] = (i+1)*(M+1) + j;
					f1.n[1] = i*(M+1) + j+1;
					f1.n[2] = (i+1)*(M+1) + j+1;
					f1.pid = f.GetLocalID();
				}
			}

			// add edges
			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e = m.Edge(i);
				e.n[0] = i*(M+1); e.n[1] = (i+1)*(M+1);
				e.pid = e0.GetLocalID();
			}

			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e = m.Edge(M + i);
				e.n[0] = M*(M+1) + i; e.n[1] = M*(M+1) + i + 1;
				e.pid = e1.GetLocalID();
			}

			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e = m.Edge(2*M + i);
				e.n[0] = M + i*(M+1); e.n[1] = M + (i+1)*(M+1);
				e.pid = e2.GetLocalID();
			}

			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e = m.Edge(3*M + i);
				e.n[0] = i; e.n[1] = i+1;
				e.pid = e3.GetLocalID();
			}
		}
		break;
	default:
		assert(false);
	}

	m.Update();
	m.UpdateNormals();
	glmesh->Attach(m);
}

//-----------------------------------------------------------------------------
// Build a revolved wedge surface
// The revolved surface has four edges, the two side ones of type EDGE_YARC
void GObject::BuildFaceRevolveWedge(GLMesh* glmesh, GFace& f)
{
	const int M = 50;

	// get number of nodes and edges
	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN==3);
	assert(NE==3);

	// get the edges
	GEdge& e0 = *Edge(f.m_edge[0].nid);
	GEdge& e1 = *Edge(f.m_edge[1].nid);
	GEdge& e2 = *Edge(f.m_edge[2].nid);
	assert(e0.m_ntype == e2.m_ntype);
	assert((e1.m_ntype == EDGE_YARC)||(e1.m_ntype == EDGE_ZARC));

	// this is the mesh we'll be building
	GLMesh m;

	// build the mesh
	switch (e0.m_ntype)
	{
	case EDGE_LINE:
		{
			m.Create(M+2, M, M+2);
			vec3d r0 = Node(f.m_node[0])->LocalPosition();
			vec3d r1 = Node(f.m_node[1])->LocalPosition();
			vec3d r2 = Node(f.m_node[2])->LocalPosition();

			if (e1.m_ntype == EDGE_YARC)
			{
				double y0 = r0.y;
				double y1 = r1.y;

				vec2d a(r1.x, r1.z), b(r2.x, r2.z);

				// create an arc object
				GM_CIRCLE_ARC c0(vec2d(0,0), a, b, f.m_edge[1].nwn);

				// position the center point
				GMesh::NODE& n0 = m.Node(0);
				n0.r = r0;
				n0.pid = -1;
			
				// create the arc points
				for (int i=0; i<=M; ++i)
				{
					double t = (double) i / (double) M;

					GMesh::NODE& n0 = m.Node(i+1);
					vec2d p0 = c0.Point(t);
					n0.r = vec3d(p0.x, y1, p0.y);
					n0.pid = -1;
				}
			}
			else if (e1.m_ntype == EDGE_ZARC)
			{
				double z0 = r0.z;
				double z1 = r1.z;

				vec2d a(r1.x, r1.y), b(r2.x, r2.y);

				// create an arc object
				GM_CIRCLE_ARC c0(vec2d(0,0), a, b, f.m_edge[1].nwn);

				// position the center point
				GMesh::NODE& n0 = m.Node(0);
				n0.r = r0;
				n0.pid = -1;
			
				// create the arc points
				for (int i=0; i<=M; ++i)
				{
					double t = (double) i / (double) M;

					GMesh::NODE& n0 = m.Node(i+1);
					vec2d p0 = c0.Point(t);
					n0.r = vec3d(p0.x, p0.y, z1);
					n0.pid = -1;
				}
			}

			m.Node(  0).pid = Node(f.m_node[0])->GetLocalID();
			m.Node(  1).pid = Node(f.m_node[1])->GetLocalID();
			m.Node(M+1).pid = Node(f.m_node[2])->GetLocalID();

			// add edges
			GMesh::EDGE& e0 = m.Edge(0);
			GMesh::EDGE& e1 = m.Edge(1);
			e0.n[0] = 0; e0.n[1] =   1; e0.pid = Edge(f.m_edge[0].nid)->GetLocalID();
			e1.n[0] = 0; e1.n[1] = M+1; e1.pid = Edge(f.m_edge[2].nid)->GetLocalID();

			for (int i=0; i<M; ++i)
			{
				GMesh::EDGE& e = m.Edge(2 + i);
				e.n[0] = i+1; e.n[1] = i+2; e.pid = Edge(f.m_edge[1].nid)->GetLocalID();
			}

			for (int i=0; i<M; ++i)
			{
				GMesh::FACE& f0 = m.Face(i);

				f0.n[0] = 0; 
				f0.n[1] = i+1;
				f0.n[2] = i+2;
				f0.pid = f.GetLocalID();
			}
		}
		break;
	default:
		assert(false);
	}

	m.Update();
	m.UpdateNormals();
	glmesh->Attach(m);
}

//-----------------------------------------------------------------------------
void GObject::BuildFaceQuad(GLMesh* glmesh, GFace &f)
{
	const int M = 10;

	int NN = f.Nodes();
	int NE = f.Edges();
	assert(NN == 4);
	assert(NE == 4);

	vec3d y[4];
	y[0] = Node(f.m_node[0])->LocalPosition();
	y[1] = Node(f.m_node[1])->LocalPosition();
	y[2] = Node(f.m_node[2])->LocalPosition();
	y[3] = Node(f.m_node[3])->LocalPosition();

	GLMesh m;
	m.Create((M+1)*(M+1), 2*M*M, 4*M);

	// build nodes
	for (int j=0; j<=M; ++j)
		for (int i=0; i<=M; ++i)
		{
			double r = -1.0 + (double) i * 2.0 / (double) M;
			double s = -1.0 + (double) j * 2.0 / (double) M;

			double H0 = 0.25*(1 - r)*(1 - s);
			double H1 = 0.25*(1 + r)*(1 - s);
			double H2 = 0.25*(1 + r)*(1 + s);
			double H3 = 0.25*(1 - r)*(1 + s);

			GLMesh::NODE& n = m.Node(j*(M+1)+i);
			n.r = y[0]*H0 + y[1]*H1 + y[2]*H2 + y[3]*H3;
			n.pid = -1;
		}

	m.Node(            0).pid = Node(f.m_node[0])->GetLocalID();
	m.Node(            M).pid = Node(f.m_node[1])->GetLocalID();
	m.Node(      M*(M+1)).pid = Node(f.m_node[3])->GetLocalID();
	m.Node((M+1)*(M+1)-1).pid = Node(f.m_node[2])->GetLocalID();

	// build the faces
	for (int j=0; j<M; ++j)
		for (int i=0; i<M; ++i)
		{
			GMesh::FACE& f0 = m.Face(j*(2*M) + 2*i);
			GMesh::FACE& f1 = m.Face(j*(2*M) + 2*i + 1);

			f0.n[0] = i*(M+1) + j;
			f0.n[1] = (i+1)*(M+1) + j;
			f0.n[2] = (i+1)*(M+1) + j+1;
			f0.pid = f.GetLocalID();

			f1.n[0] = (i+1)*(M+1) + j+1;
			f1.n[1] = i*(M+1) + j+1;
			f1.n[2] = i*(M+1) + j;
			f1.pid = f.GetLocalID();
		}

	// build the edges
	for (int i=0; i<M; ++i)
	{
		GMesh::EDGE& e = m.Edge(i);
		e.n[0] = i;
		e.n[1] = i+1;
		e.pid = Edge(f.m_edge[0].nid)->GetLocalID();
	}

	for (int i=0; i<M; ++i)
	{
		GMesh::EDGE& e = m.Edge(M + i);
		e.n[0] = (i+1)*(M+1) - 1;
		e.n[1] = (i+2)*(M+1) - 1;
		e.pid = Edge(f.m_edge[1].nid)->GetLocalID();
	}

	for (int i=0; i<M; ++i)
	{
		GMesh::EDGE& e = m.Edge(2*M + i);
		e.n[0] = (M+1)*(M+1) - 1 - i;
		e.n[1] = (M+1)*(M+1) - 1 - i - 1;
		e.pid = Edge(f.m_edge[2].nid)->GetLocalID();
	}

	for (int i=0; i<M; ++i)
	{
		GMesh::EDGE& e = m.Edge(3*M + i);
		e.n[0] = (M - i)*(M+1);
		e.n[1] = (M - i - 1)*(M+1);
		e.pid = Edge(f.m_edge[3].nid)->GetLocalID();
	}

	m.Update();
	m.UpdateNormals();
	glmesh->Attach(m);
}

// get the mesh of an edge curve
FECurveMesh* GObject::GetFECurveMesh(int edgeId)
{
	FEMesh* mesh = GetFEMesh();
	if (mesh == 0) return 0;

	mesh->TagAllNodes(-1);
	int NC = mesh->Edges();
	int ne = 0;
	for (int i=0; i<NC; ++i)
	{
		FEEdge& e = mesh->Edge(i);
		if (e.m_gid == edgeId)
		{
			mesh->Node(e.n[0]).m_ntag = 0;
			mesh->Node(e.n[1]).m_ntag = 0;
			ne++;
		}
	}

	FECurveMesh* curve = new FECurveMesh;

	int NN = mesh->Nodes();
	int nn = 0;
	for (int i=0; i<NN; ++i)
	{
		FENode& node = mesh->Node(i);
		if (node.m_ntag != -1) 
		{
			node.m_ntag = nn++;
			vec3d r = GetTransform().LocalToGlobal(node.r);
			curve->AddNode(r);
		}
	}

	for (int i=0; i<NC; ++i)
	{
		FEEdge& sedge = mesh->Edge(i);
		if (sedge.m_gid == edgeId)
		{
			int n0 = mesh->Node(sedge.n[0]).m_ntag;
			int n1 = mesh->Node(sedge.n[1]).m_ntag;
			curve->AddEdge(n0, n1);
		}
	}

	curve->BuildMesh();

	return curve;
}

//-----------------------------------------------------------------------------
void GObject::Show()
{
	// show the object
	ShowItem();

	// show all the parts
	for (int i=0; i<Parts(); ++i) Part(i)->ShowItem();

	// Update visibility of child items
	UpdateItemVisibility();
}

//-----------------------------------------------------------------------------
void GObject::Hide()
{
	// hide the object
	HideItem();

	// hide all the parts
	for (int i=0; i<Parts(); ++i) Part(i)->HideItem();

	// Update visibility of child items
	UpdateItemVisibility();
}

//-----------------------------------------------------------------------------
void GObject::ShowPart(int n, bool bshow)
{
	// Show the item
	GPart* pg = Part(n); assert(pg);
	if (bshow) pg->ShowItem(); else pg->HideItem();

	// Update visibility of child items
	UpdateItemVisibility();
}

//-----------------------------------------------------------------------------
void GObject::ShowPart(GPart& part, bool bshow)
{
	assert(part.Object() == this);
	if (bshow) part.ShowItem(); else part.HideItem();

	// Update visibility of child items
	UpdateItemVisibility();
}

//-----------------------------------------------------------------------------
void GObject::ShowAllParts()
{
	for (int i=0; i<Parts(); ++i)
	{
		GPart& p = *Part(i);
		p.ShowItem();
	}

	// Update visibility of child items
	UpdateItemVisibility();
}

//-----------------------------------------------------------------------------
void GObject::UpdateItemVisibility()
{
	// update surface visibility
	for (int i=0; i<Faces(); ++i)
	{
		GFace* face = Face(i);

		GPart* p0 = Part(face->m_nPID[0]);
		GPart* p1 = Part(face->m_nPID[1]);

		if (!p0->IsVisible() && ((p1==0) || (!p1->IsVisible()))) face->HideItem();
		else face->ShowItem();
	}

	// update edge visibility
	for (int i=0; i<Edges(); ++i) Edge(i)->m_ntag = 0;
	for (int i=0; i<Faces(); ++i)
	{
		GFace* face = Face(i);
		if (face->IsVisible())
		{
			for (int j=0; j<face->m_edge.size(); ++j)
			{
				Edge(face->m_edge[j].nid)->m_ntag = 1;
			}
		}
	}
	for (int i=0; i<Edges(); ++i)
	{
		GEdge* edge = Edge(i);
		if (edge->m_ntag == 0) edge->HideItem();
		else edge->ShowItem();
	}

	// update node visibility
	for (int i=0; i<Nodes(); ++i) Node(i)->m_ntag = 0;
	for (int i=0; i<Edges(); ++i)
	{
		GEdge* edge = Edge(i);
		if (edge->IsVisible())
		{
			Node(edge->m_node[0])->m_ntag = 1;
			Node(edge->m_node[1])->m_ntag = 1;
		}
	}
	for (int i=0; i<Nodes(); ++i)
	{
		GNode* node = Node(i);
		if (node->m_ntag == 0) node->HideItem();
		else node->ShowItem();
	}

	// update visibility of mesh items
	FEMesh* mesh = GetFEMesh();
	if (mesh)
	{
		int NE = mesh->Elements();
		for (int i=0; i<NE; ++i)
		{
			FEElement& el = mesh->Element(i);
			GPart* pg = Part(el.m_gid);
			assert(pg);
			if (pg->IsVisible()) { el.Show(); el.Unhide(); }
			else el.Hide();
		}

		mesh->UpdateItemVisibility();
	}
}

//-----------------------------------------------------------------------------
// is called whenever the selection has changed (default does nothing)
void GObject::UpdateSelection()
{

}

//-----------------------------------------------------------------------------
GNode* GObject::FindNodeFromTag(int ntag)
{
	int N = Nodes();
	for (int i=0; i<N; ++i)
	{
		GNode* ni = Node(i);
		if (ni->m_ntag == ntag) return ni;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Save data to file
void GObject::Save(OArchive &ar)
{
	// save the name
	ar.WriteChunk(CID_OBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// save the transform stuff
	ar.BeginChunk(CID_OBJ_HEADER);
	{
		int nid = GetID();
		ar.WriteChunk(CID_OBJ_ID, nid);
		ar.WriteChunk(CID_OBJ_POS, GetTransform().GetPosition());
		ar.WriteChunk(CID_OBJ_ROT, GetTransform().GetRotation());
		ar.WriteChunk(CID_OBJ_SCALE, GetTransform().GetScale());
		ar.WriteChunk(CID_OBJ_COLOR, GetColor());

		int nparts = Parts();
		int nfaces = Faces();
		int nedges = Edges();
		int nnodes = Nodes();

		ar.WriteChunk(CID_OBJ_PARTS, nparts);
		ar.WriteChunk(CID_OBJ_FACES, nfaces);
		ar.WriteChunk(CID_OBJ_EDGES, nedges);
		ar.WriteChunk(CID_OBJ_NODES, nnodes);
	}
	ar.EndChunk();

	// save the parameters
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_OBJ_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	// save the parts
	if (Parts() > 0)
	{
		ar.BeginChunk(CID_OBJ_PART_SECTION);
		{
			for (int i = 0; i<Parts(); ++i)
			{
				ar.BeginChunk(CID_OBJ_PART);
				{
					GPart& p = *Part(i);
					int nid = p.GetID();
					int mid = p.GetMaterialID();
					ar.WriteChunk(CID_OBJ_PART_ID, nid);
					ar.WriteChunk(CID_OBJ_PART_MAT, mid);
					ar.WriteChunk(CID_OBJ_PART_NAME, p.GetName());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the surfaces
	if (Faces() > 0)
	{
		ar.BeginChunk(CID_OBJ_FACE_SECTION);
		{
			for (int i = 0; i<Faces(); ++i)
			{
				ar.BeginChunk(CID_OBJ_FACE);
				{
					GFace& f = *Face(i);
					int nid = f.GetID();
					ar.WriteChunk(CID_OBJ_FACE_ID, nid);
					ar.WriteChunk(CID_OBJ_FACE_NAME, f.GetName());
					ar.WriteChunk(CID_OBJ_FACE_PID0, f.m_nPID[0]);
					ar.WriteChunk(CID_OBJ_FACE_PID1, f.m_nPID[1]);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the edges
	if (Edges() > 0)
	{
		ar.BeginChunk(CID_OBJ_EDGE_SECTION);
		{
			for (int i = 0; i<Edges(); ++i)
			{
				ar.BeginChunk(CID_OBJ_EDGE);
				{
					GEdge& e = *Edge(i);
					int nid = e.GetID();
					ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
					ar.WriteChunk(CID_OBJ_EDGE_NAME, e.GetName());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the nodes
	// note that it is possible that an object doesn't have any nodes
	// for instance, a shell disc
	if (Nodes()>0)
	{
		ar.BeginChunk(CID_OBJ_NODE_SECTION);
		{
			for (int i = 0; i<Nodes(); ++i)
			{
				ar.BeginChunk(CID_OBJ_NODE);
				{
					GNode& v = *Node(i);
					int nid = v.GetID();
					ar.WriteChunk(CID_OBJ_NODE_ID, nid);
					ar.WriteChunk(CID_OBJ_NODE_POS, v.LocalPosition());
					ar.WriteChunk(CID_OBJ_NODE_NAME, v.GetName());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the mesh
	if (imp->m_pmesh)
	{
		ar.BeginChunk(CID_MESH);
		{
			imp->m_pmesh->Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
// Load data from file
void GObject::Load(IArchive& ar)
{
	TRACE("GObject::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
			// object name
		case CID_OBJ_NAME:
		{
			string name;
			ar.read(name);
			SetName(name);
		}
		break;
		// object info
		case CID_FEOBJ_INFO:
		{
			string info;
			ar.read(info);
			SetInfo(info);
		}
		break;
		// header
		case CID_OBJ_HEADER:
		{
			vec3d pos, scl;
			quatd rot;
			GLColor col;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				int oid;
				switch (nid)
				{
				case CID_OBJ_ID: ar.read(oid); SetID(oid); break;
				case CID_OBJ_POS: ar.read(pos); break;
				case CID_OBJ_ROT: ar.read(rot); break;
				case CID_OBJ_SCALE: ar.read(scl); break;
				case CID_OBJ_COLOR: ar.read(col); break;
				case CID_OBJ_PARTS: ar.read(nparts); break;
				case CID_OBJ_FACES: ar.read(nfaces); break;
				case CID_OBJ_EDGES: ar.read(nedges); break;
				case CID_OBJ_NODES: ar.read(nnodes); break;
				}
				ar.CloseChunk();
			}

			SetColor(col);

			Transform& transform = GetTransform();
			transform.SetPosition(pos);
			transform.SetRotation(rot);
			transform.SetScale(scl);
		}
		break;
		// object parameters
		case CID_OBJ_PARAMS:
			ParamContainer::Load(ar);
			break;
			// object parts
		case CID_OBJ_PART_SECTION:
		{
			assert(nparts > 0);
			m_Part.reserve(nparts);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_SECTION (GMeshObject::Load)");

				GPart* p = new GPart(this);
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid, mid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_PART_ID: ar.read(nid); p->SetID(nid); break;
					case CID_OBJ_PART_MAT: ar.read(mid); p->SetMaterialID(mid); break;
					case CID_OBJ_PART_NAME:
					{
						char szname[256] = { 0 };
						ar.read(szname);
						p->SetName(szname);
					}
					break;
					}
					ar.CloseChunk();
				}
				ar.CloseChunk();

				p->SetLocalID(n++);

				m_Part.push_back(p);
			}
			assert((int)m_Part.size() == nparts);
		}
		break;
		// object surfaces
		case CID_OBJ_FACE_SECTION:
		{
			assert(nfaces > 0);
			m_Face.reserve(nfaces);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_SECTION (GMeshObject::Load)");

				GFace* f = new GFace(this);
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_FACE_ID: ar.read(nid); f->SetID(nid); break;
					case CID_OBJ_FACE_NAME:
					{
						char szname[256] = { 0 };
						ar.read(szname);
						f->SetName(szname);
					}
					break;
					case CID_OBJ_FACE_PID0: ar.read(f->m_nPID[0]); break;
					case CID_OBJ_FACE_PID1: ar.read(f->m_nPID[1]); break;
					}
					ar.CloseChunk();
				}
				ar.CloseChunk();

				f->SetLocalID(n++);

				m_Face.push_back(f);
			}
			assert((int)m_Face.size() == nfaces);
		}
		break;
		// object edges
		case CID_OBJ_EDGE_SECTION:
		{
			m_Edge.clear();
			if (nedges > 0) m_Edge.reserve(nedges);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_SECTION (GMeshObject::Load)");

				GEdge* e = new GEdge(this);
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_EDGE_ID: ar.read(nid); e->SetID(nid); break;
					case CID_OBJ_EDGE_NAME:
					{
						char szname[256] = { 0 };
						ar.read(szname);
						e->SetName(szname);
					}
					break;
					}
					ar.CloseChunk();
				}
				ar.CloseChunk();

				e->SetLocalID(n++);

				m_Edge.push_back(e);
			}
			assert((int)m_Edge.size() == nedges);
		}
		break;
		// object nodes
		case CID_OBJ_NODE_SECTION:
		{
			m_Node.clear();
			if (nnodes > 0)
			{
				m_Node.reserve(nnodes);
				int m = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_SECTION (GMeshObject::Load)");

					GNode* n = new GNode(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_NODE_ID: ar.read(nid); n->SetID(nid); break;
						case CID_OBJ_NODE_POS: ar.read(n->LocalPosition()); break;
						case CID_OBJ_NODE_NAME:
						{
							char szname[256] = { 0 };
							ar.read(szname);
							n->SetName(szname);
						}
						break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					n->SetLocalID(m++);

					m_Node.push_back(n);
				}
				assert((int)m_Node.size() == nnodes);
			}
		}
		break;
		// the mesh object
		case CID_MESH:
			if (imp->m_pmesh) delete imp->m_pmesh;
			SetFEMesh(new FEMesh);
			imp->m_pmesh->Load(ar);
			break;
		}
		ar.CloseChunk();
	}

	Update(false);
}
//-----------------------------------------------------------------------------
void GObject::Reindex()
{
	SetID(CreateUniqueID());
	for (int i = 0; i < Parts(); ++i)
	{
		GPart* pg = Part(i); pg->SetID(GPart::CreateUniqueID());
		stringstream ss;
		ss << "Part" << pg->GetID();
		pg->SetName(ss.str());
	}
	for (int i = 0; i < Faces(); ++i)
	{
		GFace* pg = Face(i); pg->SetID(GFace::CreateUniqueID());
		stringstream ss;
		ss << "Surface" << pg->GetID();
		pg->SetName(ss.str());
	}
	for (int i = 0; i < Edges(); ++i)
	{
		GEdge* pg = Edge(i); pg->SetID(GEdge::CreateUniqueID());
		stringstream ss;
		ss << "Curve" << pg->GetID();
		pg->SetName(ss.str());
	}
	for (int i = 0; i < Nodes(); ++i)
	{
		GNode* pg = Node(i); pg->SetID(GNode::CreateUniqueID());
		stringstream ss;
		ss << "Node" << pg->GetID();
		pg->SetName(ss.str());
	}
}
