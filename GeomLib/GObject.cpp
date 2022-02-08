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

#include "GObject.h"
#include <MeshTools/FEGroup.h>
#include <MeshLib/FECurveMesh.h>
#include <FSCore/FSObjectList.h>
#include <MeshLib/FEMesh.h>
#include <MeshTools/FEMesher.h>
#include <MeshTools/GLMesh.h>
#include <MeshTools/GLMesher.h>
#include <sstream>

using namespace std;

GObject* GObject::m_activeObject = nullptr;

void GObject::SetActiveObject(GObject* po) { m_activeObject = po; }
GObject* GObject::GetActiveObject() { return m_activeObject; }
bool GObject::IsActiveObject() const { return (m_activeObject == this); }

class GObject::Imp
{
public:
	Imp()
	{
		m_pmesh   = nullptr;
		m_pMesher = nullptr;
		m_pGMesh  = nullptr;

		m_col = GLColor(200, 200, 200);

		m_bValid = true;
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
	bool	m_bValid;

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
bool GObject::IsValid() const
{
	return imp->m_bValid;
}

//-----------------------------------------------------------------------------
void GObject::SetValidFlag(bool b)
{
	imp->m_bValid = b;
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
	GLMesher mesher(this);

	// delete the old mesh
	delete imp->m_pGMesh;
	imp->m_pGMesh = mesher.CreateMesh(); assert(imp->m_pGMesh);
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

					if (p.Parameters() > 0)
					{
						ar.BeginChunk(CID_OBJ_PART_PARAMS);
						{
							p.ParamContainer::Save(ar);
						}
						ar.EndChunk();
					}
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
					ar.WriteChunk(CID_OBJ_FACE_TYPE, f.m_ntype);
					ar.WriteChunk(CID_OBJ_FACE_NAME, f.GetName());
					ar.WriteChunk(CID_OBJ_FACE_PID0, f.m_nPID[0]);
					ar.WriteChunk(CID_OBJ_FACE_PID1, f.m_nPID[1]);
					ar.WriteChunk(CID_OBJ_FACE_PID2, f.m_nPID[2]);
					ar.WriteChunk(CID_OBJ_FACE_NODES, (int)f.m_node.size());
					ar.WriteChunk(CID_OBJ_FACE_NODELIST, f.m_node);
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

	// save the mesher object
	if (GetFEMesher())
	{
		ar.BeginChunk(CID_OBJ_FEMESHER);
		{
			int ntype = 0;
//			if (dynamic_cast<FETetGenMesher*>(GetFEMesher())) ntype = 1;
			ar.BeginChunk(ntype);
			{
				GetFEMesher()->Save(ar);
			}
			ar.EndChunk();
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
					case CID_OBJ_PART_PARAMS:
						{
							p->ParamContainer::Load(ar);
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
					case CID_OBJ_FACE_TYPE: ar.read(f->m_ntype); break;
					case CID_OBJ_FACE_NODES: {int n = 0; ar.read(n); f->m_node.assign(n, -1); } break;
					case CID_OBJ_FACE_NODELIST: ar.read(f->m_node); break;
					case CID_OBJ_FACE_NAME:
					{
						char szname[256] = { 0 };
						ar.read(szname);
						f->SetName(szname);
					}
					break;
					case CID_OBJ_FACE_PID0: ar.read(f->m_nPID[0]); break;
					case CID_OBJ_FACE_PID1: ar.read(f->m_nPID[1]); break;
					case CID_OBJ_FACE_PID2: ar.read(f->m_nPID[2]); break;
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
		case CID_OBJ_FEMESHER:
		{
			if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
			else
			{
				int ntype = ar.GetChunkID();
				//				assert(m_pMesher == 0);
				switch (ntype)
				{
				case 0: break;	// use default mesher
//				case 1: SetFEMesher(new FETetGenMesher(this)); break;
				default:
					throw ReadError("error parsing CID_OBJ_FEMESHER");
				}
				GetFEMesher()->Load(ar);
			}
			ar.CloseChunk();
			if (ar.OpenChunk() != IArchive::IO_END) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
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
