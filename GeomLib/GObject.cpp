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
#include <GeomLib/FSGroup.h>
#include <MeshLib/FECurveMesh.h>
#include <FSCore/FSObjectList.h>
#include <MeshLib/FEMesh.h>
#include <MeshTools/FEMesher.h>
#include <MeshLib/GMesh.h>
#include <MeshTools/GLMesher.h>
#include <MeshTools/FETetGenMesher.h>
#include <FSCore/ClassDescriptor.h>
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
		m_glFaceMesh = nullptr;
		m_objManip = nullptr;

		m_col = GLColor(200, 200, 200);

		m_bValid = true;

		m_saveFlag = ObjectSaveFlags::ALL_FLAGS;
	}

	~Imp()
	{
		delete m_pmesh;	m_pmesh = nullptr;
		delete m_pMesher; m_pMesher = nullptr;
		delete m_pGMesh; m_pGMesh = nullptr;
		delete m_glFaceMesh; m_glFaceMesh = nullptr;
		delete m_objManip; m_objManip = nullptr;
	}

public:
	int	m_ntype;	//!< object type identifier
	GLColor	m_col;	//!< color of object
	bool	m_bValid;
	unsigned int m_saveFlag;

	FSMesh*		m_pmesh;	//!< the mesh that this object manages
	FEMesher*	m_pMesher;	//!< the mesher builds the actual mesh
	GMesh*		m_pGMesh;	//!< the mesh for rendering geometry
	GMesh*		m_glFaceMesh;	//!< mesh for rendering FE mesh

	GObjectManipulator* m_objManip;

	FSObjectList<FSElemSet>		m_pFEElemSet;
	FSObjectList<FSSurface>		m_pFESurface;
	FSObjectList<FSEdgeSet>		m_pFEEdgeSet;
	FSObjectList<FSNodeSet>		m_pFENodeSet;
	FSObjectList<FSPartSet>		m_pFEPartSet;
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

bool GObject::CanDelete() const
{
	// see if any of the nodes are required
	for (int i = 0; i < Nodes(); ++i)
	{
		if (Node(i)->IsRequired()) return false;
	}
	return CanDeleteMesh();
}

bool GObject::CanDeleteMesh() const
{
	const FSMesh* pm = GetFEMesh();
	if (pm == nullptr) return true;

	// Check if there are any mesh dependencies.
	// Note that part-sets aren't checked since they don't reference the mesh directly.
	if (FENodeSets() > 0) return false;
	if (FESurfaces() > 0) return false;
	if (FEEdgeSets() > 0) return false;
	if (FEElemSets() > 0) return false;

	return true;
}

//-----------------------------------------------------------------------------
// return type of Object
int GObject::GetType() const { return imp->m_ntype; }

//-----------------------------------------------------------------------------
// get/set object color
GLColor GObject::GetColor() const { return imp->m_col; }

//-----------------------------------------------------------------------------
void GObject::SetColor(const GLColor& c) 
{ 
	imp->m_col = c; 
}

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
FSMesh* GObject::GetFEMesh() { return imp->m_pmesh; }

//-----------------------------------------------------------------------------
const FSMesh* GObject::GetFEMesh() const { return imp->m_pmesh; }

//-----------------------------------------------------------------------------
// delete the mesh
void GObject::DeleteFEMesh() { delete imp->m_pmesh; imp->m_pmesh = 0; }

//-----------------------------------------------------------------------------
void GObject::SetFEMesher(FEMesher *pmesher)
{
	imp->m_pMesher = pmesher;
}

//-----------------------------------------------------------------------------
void GObject::SetFEMesh(FSMesh* pm)
{
	imp->m_pmesh = pm; if (pm) pm->SetGObject(this);

	// rebuild the line mesh
	delete imp->m_glFaceMesh; imp->m_glFaceMesh = nullptr;
	if (pm)
	{
		BuildFERenderMesh();
	}
}

void GObject::BuildFERenderMesh()
{
	delete imp->m_glFaceMesh; imp->m_glFaceMesh = nullptr;
	FSMesh* pm = GetFEMesh();
	if (pm == nullptr) return;

	imp->m_glFaceMesh = new GMesh;
	GMesh& gm = *imp->m_glFaceMesh;
	gm.Create(pm->Nodes(), 0, 0);
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		gm.Node(i).r = to_vec3f(pm->Node(i).r);
	}

	int NF = pm->Faces();
	for (int i = 0; i < NF; i++)
	{
		const FSFace& face = pm->Face(i);
		if (face.IsVisible())
		{
			int eid = face.m_elem[0].eid;
			if ((eid >= 0) && (!pm->Element(eid).IsVisible()))
			{
				eid = face.m_elem[1].eid;
			}

			gm.AddFace(face.n, face.Nodes(), face.m_gid, face.m_sid, face.IsExterior(), i, eid);

			int ne = face.Edges();
			for (int j = 0; j < ne; ++j)
			{
				int j1 = (j + 1) % ne;
				if ((face.m_nbr[j] < 0) || (face.n[j] < face.n[j1]))
				{
					int m[2] = { face.n[j], face.n[j1] };
					gm.AddEdge(m, 2);
				}
			}
		}
	}

	// add the exposed surface from hidden elements
	int maxSurfID = Faces(); // we assign this ID to the exposed surface
	FSFace face;
	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.IsVisible())
		{
			int nf = el.Faces();
			for (int j = 0; j < nf; ++j)
			{
				if (el.m_nbr[j] >= 0)
				{
					FSElement& elj = pm->Element(el.m_nbr[j]);
					if (!elj.IsVisible() && (el.m_gid == elj.m_gid))
					{
						el.GetFace(j, face);
						gm.AddFace(face.n, face.Nodes(), maxSurfID, -1, false, -1, i);

						int n[FSEdge::MAX_NODES];
						int ne = face.Edges();
						for (int k = 0; k < ne; ++k)
						{
							int m = face.GetEdgeNodes(k, n);
							if (m == 2)
							{
								if (n[0] < n[1]) gm.AddEdge(n, 2, -1);
							}
						}
					}
				}
			}
		}
	}

	// NOTE: since we only add the visible faces, note that the partitions created in this mesh
	// may not correspond to the surfaces of the geometry object
	gm.Update();
}

void GObject::UpdateFERenderMesh()
{
	FSMesh* pm = GetFEMesh();
	if (pm == nullptr) return;
	if (imp->m_glFaceMesh == nullptr) return;

	GMesh& gm = *imp->m_glFaceMesh;
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		gm.Node(i).r = to_vec3f(pm->Node(i).r);
	}
	for (int i = 0; i < gm.Faces(); ++i)
	{
		GMesh::FACE& face = gm.Face(i);
		face.vr[0] = gm.Node(face.n[0]).r;
		face.vr[1] = gm.Node(face.n[1]).r;
		face.vr[2] = gm.Node(face.n[2]).r;
	}
	for (int i = 0; i < gm.Edges(); ++i)
	{
		GMesh::EDGE& edge = gm.Edge(i);
		edge.vr[0] = gm.Node(edge.n[0]).r;
		edge.vr[1] = gm.Node(edge.n[1]).r;
	}

	gm.UpdateBoundingBox();
	gm.UpdateNormals();
}

//-----------------------------------------------------------------------------
// set the render mesh
void GObject::SetRenderMesh(GMesh* mesh)
{
	delete imp->m_pGMesh;
	imp->m_pGMesh = mesh;
}

//-----------------------------------------------------------------------------
void GObject::ClearFEPartSets()
{
	imp->m_pFEPartSet.Clear();
}

//-----------------------------------------------------------------------------
void GObject::ClearFEElementSets()
{
	imp->m_pFEElemSet.Clear();
}

//-----------------------------------------------------------------------------
void GObject::ClearFESurfaces()
{
	imp->m_pFESurface.Clear();
}

//-----------------------------------------------------------------------------
void GObject::ClearFEEdgeSets()
{
	imp->m_pFEEdgeSet.Clear();
}

//-----------------------------------------------------------------------------
void GObject::ClearFENodeSets()
{
	imp->m_pFENodeSet.Clear();
}

//-----------------------------------------------------------------------------
// Clear group data
void GObject::ClearFEGroups()
{
	ClearFEPartSets();
	ClearFEElementSets();
	ClearFEEdgeSets();
	ClearFESurfaces();
	ClearFENodeSets();
}

//-----------------------------------------------------------------------------
// Remove groups that are empty.

template <class T> void clearVector(FSObjectList<T>& v, std::function<bool(T*)> f)
{
	if (v.IsEmpty()) return;

	for (size_t i=0; i<v.Size(); )
	{
		T* o = v[i];
		if (f(o))
		{
			v.Remove(o);
		}
		else i++;
	}
}

void GObject::RemoveEmptyFEGroups()
{
	clearVector<FSPartSet>(imp->m_pFEPartSet, [](FSPartSet* pg) { return (pg->size() == 0); });
	clearVector<FSElemSet>(imp->m_pFEElemSet, [](FSElemSet* pg) { return (pg->size() == 0); });
	clearVector<FSSurface>(imp->m_pFESurface, [](FSSurface* pg) { return (pg->size() == 0); });
	clearVector<FSEdgeSet>(imp->m_pFEEdgeSet, [](FSEdgeSet* pg) { return (pg->size() == 0); });
	clearVector<FSNodeSet>(imp->m_pFENodeSet, [](FSNodeSet* pg) { return (pg->size() == 0); });
}

void GObject::RemoveUnusedFEGroups()
{
	clearVector<FSPartSet>(imp->m_pFEPartSet, [](FSPartSet* pg) { return (pg->GetReferenceCount() == 0); });
	clearVector<FSElemSet>(imp->m_pFEElemSet, [](FSElemSet* pg) { return (pg->GetReferenceCount() == 0); });
	clearVector<FSSurface>(imp->m_pFESurface, [](FSSurface* pg) { return (pg->GetReferenceCount() == 0); });
	clearVector<FSEdgeSet>(imp->m_pFEEdgeSet, [](FSEdgeSet* pg) { return (pg->GetReferenceCount() == 0); });
	clearVector<FSNodeSet>(imp->m_pFENodeSet, [](FSNodeSet* pg) { return (pg->GetReferenceCount() == 0); });
}

//-----------------------------------------------------------------------------
// Find a surface from its name.
// Returns NULL if the surface cannot be found
FSSurface* GObject::FindFESurface(const string& name)
{
	// loop over all surfaces
	for (size_t i = 0; i<imp->m_pFESurface.Size(); ++i)
	{
		FSSurface* psi = imp->m_pFESurface[i];
		if (psi->GetName() == name) return psi;
	}

	// sorry, no luck
	return nullptr;
}

// Find a surface from its name.
// Returns NULL if the surface cannot be found
FSEdgeSet* GObject::FindFEEdgeSet(const string& name)
{
	for (size_t i = 0; i < imp->m_pFESurface.Size(); ++i)
	{
		FSEdgeSet* psi = imp->m_pFEEdgeSet[i];
		if (psi->GetName() == name) return psi;
	}

	// sorry, no luck
	return nullptr;
}

//-----------------------------------------------------------------------------
// Find a node set from its name.
// Returns NULL if the node set cannot be found
FSNodeSet* GObject::FindFENodeSet(const string& name)
{
	// loop over all surfaces
	for (size_t i = 0; i<imp->m_pFENodeSet.Size(); ++i)
	{
		FSNodeSet* psi = imp->m_pFENodeSet[i];
		if (psi->GetName() == name) return psi;
	}

	// sorry, no luck
	return 0;
}

//-----------------------------------------------------------------------------
// Find a group based on its global ID
FSGroup* GObject::FindFEGroup(int nid)
{
	for (int i = 0; i < FEPartSets(); ++i)
	if (imp->m_pFEPartSet[i]->GetID() == nid) return imp->m_pFEPartSet[i];

	for (int i = 0; i<FEElemSets(); ++i)
	if (imp->m_pFEElemSet[i]->GetID() == nid) return imp->m_pFEElemSet[i];

	for (int i = 0; i<FESurfaces(); ++i)
	if (imp->m_pFESurface[i]->GetID() == nid) return imp->m_pFESurface[i];

	for (int i = 0; i<FEEdgeSets(); ++i)
	if (imp->m_pFEEdgeSet[i]->GetID() == nid) return imp->m_pFEEdgeSet[i];

	for (int i = 0; i<FENodeSets(); ++i)
	if (imp->m_pFENodeSet[i]->GetID() == nid) return imp->m_pFENodeSet[i];

	return 0;
}

//-----------------------------------------------------------------------------
int GObject::FEPartSets() const { return (int)imp->m_pFEPartSet.Size(); }

//-----------------------------------------------------------------------------
int GObject::FEElemSets() const { return (int) imp->m_pFEElemSet.Size(); }

//-----------------------------------------------------------------------------
int GObject::FESurfaces() const { return (int)imp->m_pFESurface.Size(); }

//-----------------------------------------------------------------------------
int GObject::FEEdgeSets() const { return (int)imp->m_pFEEdgeSet.Size(); }

//-----------------------------------------------------------------------------
int GObject::FENodeSets() const { return (int)imp->m_pFENodeSet.Size(); }

//-----------------------------------------------------------------------------
void GObject::AddFEPartSet(FSPartSet* pg) { imp->m_pFEPartSet.Add(pg); }

//-----------------------------------------------------------------------------
void GObject::AddFEElemSet(FSElemSet* pg) { imp->m_pFEElemSet.Add(pg); }

//-----------------------------------------------------------------------------
void GObject::AddFESurface(FSSurface* pg) { imp->m_pFESurface.Add(pg); }

//-----------------------------------------------------------------------------
void GObject::AddFEEdgeSet(FSEdgeSet* pg) { imp->m_pFEEdgeSet.Add(pg); }

//-----------------------------------------------------------------------------
void GObject::AddFENodeSet(FSNodeSet* pg) { imp->m_pFENodeSet.Add(pg); }

//-----------------------------------------------------------------------------
FSPartSet* GObject::GetFEPartSet(int n) { return (n >= 0 && n < (int)imp->m_pFEPartSet.Size() ? imp->m_pFEPartSet[n] : 0); }

//-----------------------------------------------------------------------------
FSElemSet* GObject::GetFEElemSet(int n) { return (n >= 0 && n<(int) imp->m_pFEElemSet.Size() ? imp->m_pFEElemSet[n] : 0); }

//-----------------------------------------------------------------------------
FSSurface* GObject::GetFESurface(int n) { return (n >= 0 && n<(int)imp->m_pFESurface.Size() ? imp->m_pFESurface[n] : 0); }

//-----------------------------------------------------------------------------
FSEdgeSet* GObject::GetFEEdgeSet(int n) { return (n >= 0 && n<(int)imp->m_pFEEdgeSet.Size() ? imp->m_pFEEdgeSet[n] : 0); }

//-----------------------------------------------------------------------------
FSNodeSet* GObject::GetFENodeSet(int n) { return (n >= 0 && n<(int)imp->m_pFENodeSet.Size() ? imp->m_pFENodeSet[n] : 0); }

//-----------------------------------------------------------------------------
int GObject::RemoveFEPartSet(FSPartSet* pg)
{
	return imp->m_pFEPartSet.Remove(pg);
}

//-----------------------------------------------------------------------------
int GObject::RemoveFEElemSet(FSElemSet* pg)
{
	return imp->m_pFEElemSet.Remove(pg);
}

//-----------------------------------------------------------------------------
// Remove a named surface from the mesh
int GObject::RemoveFESurface(FSSurface* pg)
{
	return imp->m_pFESurface.Remove(pg);
}

//-----------------------------------------------------------------------------
int GObject::RemoveFEEdgeSet(FSEdgeSet* pg)
{
	return imp->m_pFEEdgeSet.Remove(pg);
}

//-----------------------------------------------------------------------------
// Remove a named nodeset from the mesh
int GObject::RemoveFENodeSet(FSNodeSet* pg)
{
	return imp->m_pFENodeSet.Remove(pg);
}

//-----------------------------------------------------------------------------
void GObject::InsertFEPartSet(int n, FSPartSet* pg) { imp->m_pFEPartSet.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::InsertFEElemSet(int n, FSElemSet* pg) { imp->m_pFEElemSet.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::InsertFESurface(int n, FSSurface* pg) { imp->m_pFESurface.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::InsertFEEdgeSet(int n, FSEdgeSet* pg) { imp->m_pFEEdgeSet.Insert(n, pg); }

//-----------------------------------------------------------------------------
void GObject::InsertFENodeSet(int n, FSNodeSet* pg) { imp->m_pFENodeSet.Insert(n, pg); }

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
		FSMesh& m = *imp->m_pmesh;
		for (int i = 0; i<m.Nodes(); ++i)
		{
			FSNode& node = m.Node(i);
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
	UpdateFEElementMatIDs();
}

//-----------------------------------------------------------------------------
// Assign a material to a part. The partid parameter is the global ID of the part,
// matId is the global material ID
void GObject::AssignMaterial(int partid, int matid)
{
	GPart* pg = FindPart(partid); assert(pg);
	if (pg) AssignMaterial(pg, matid);
}

void GObject::AssignMaterial(GPart* part, int matid)
{
	assert(part && (part->Object() == this));
	part->SetMaterialID(matid);
	UpdateFEElementMatIDs(part->GetLocalID());
}

//-----------------------------------------------------------------------------
//! This function makes sure that the GNodes correspond to the mesh' nodes
//! When the mesh is transformed independently from the gobject te nodes
//! might not coincide any more and this function should be called. 
//! \todo I think only the GMeshObject requires this functionality so
//!		  maybe I should move that function there.
void GObject::UpdateGNodes()
{
	FSLineMesh* pm = GetEditableLineMesh();
	if (pm == 0) return;
	for (int i=0; i<pm->Nodes(); ++i)
	{
		FSNode& n = pm->Node(i);
		if (n.m_gid >= 0) m_Node[n.m_gid]->LocalPosition() = n.r;
	}

	BuildGMesh();
}

//-----------------------------------------------------------------------------
// Replace the current mesh. Note that we don't delete the current mesh since
// it is assumed that another class will take care of that.
void GObject::ReplaceFEMesh(FSMesh* pm, bool bup, bool bdel)
{
	if (bdel) delete imp->m_pmesh;
	SetFEMesh(pm);
	Update(bup);
}

//-----------------------------------------------------------------------------
// Replace the current mesh. Note that we don't delete the current mesh since
// it is assumed that another class will take care of that.
void GObject::ReplaceSurfaceMesh(FSSurfaceMesh* pm)
{
	assert(false);
}

//-----------------------------------------------------------------------------
bool GObject::Update(bool b)
{
	for (int i = 0; i < Parts(); ++i) Part(i)->Update(b);

	// assign part materials to element matIDs. 
	UpdateFEElementMatIDs();

	BuildGMesh();
	BuildFERenderMesh();
	return GBaseObject::Update(b);
}

void GObject::UpdateFEElementMatIDs()
{
	FSMesh* pm = GetFEMesh();
	if (pm == nullptr) return;

	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		GPart* pg = Part(el.m_gid); assert(pg);
		if (pg) el.m_MatID = pg->GetMaterialID();
	}
}

void GObject::UpdateFEElementMatIDs(int partIndex)
{
	FSMesh* pm = GetFEMesh();
	if (pm == nullptr) return;
	if ((partIndex < 0) || (partIndex >= Parts())) return;

	GPart* pg = Part(partIndex);
	int matId = pg->GetMaterialID();
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_gid == partIndex) el.m_MatID = matId;
	}
}

//-----------------------------------------------------------------------------
// \todo This function deletes the old mesh. However, the tetgen remesher
//		uses the old mesh to create the new mesh and when the user undoes the last
//		mesh we need to be able to restore that mesh. Therefore, we should not delete
//		the old mesh.
FSMesh* GObject::BuildMesh()
{
	if (imp->m_pMesher)
	{
		// keep a pointer to the old mesh since some mesher use the old
		// mesh to create a new mesh
		FSMesh* pold = imp->m_pmesh;
		SetFEMesh(imp->m_pMesher->BuildMesh());

		// now it is safe to delete the old mesh
		if (pold) delete pold;

		Update();
		return imp->m_pmesh;
	}
	else return 0;
}

//-----------------------------------------------------------------------------
FSNode* GObject::GetFENode(int gid)
{
	FSMesh* pm = GetFEMesh();
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
	return LocalToGlobalBox(box, GetTransform());
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
GMesh*	GObject::GetRenderMesh()
{ 
	return imp->m_pGMesh;
}

GMesh* GObject::GetFERenderMesh()
{
	return imp->m_glFaceMesh;
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
	FSMesh* mesh = GetFEMesh();
	if (mesh == 0) return 0;

	mesh->TagAllNodes(-1);
	int NC = mesh->Edges();
	int ne = 0;
	for (int i=0; i<NC; ++i)
	{
		FSEdge& e = mesh->Edge(i);
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
		FSNode& node = mesh->Node(i);
		if (node.m_ntag != -1) 
		{
			node.m_ntag = nn++;
			vec3d r = GetTransform().LocalToGlobal(node.r);
			curve->AddNode(r);
		}
	}

	for (int i=0; i<NC; ++i)
	{
		FSEdge& sedge = mesh->Edge(i);
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
	FSMesh* mesh = GetFEMesh();
	if (mesh)
	{
		int NE = mesh->Elements();
		for (int i=0; i<NE; ++i)
		{
			FSElement& el = mesh->Element(i);
			GPart* pg = Part(el.m_gid);
			assert(pg);
			if (pg->IsVisible()) { el.Show(); el.Unhide(); }
			else el.Hide();
		}

		mesh->UpdateItemVisibility();
	}

	// rebuild render meshes
	BuildFERenderMesh();
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
void GObject::SetSaveFlags(unsigned int flags)
{
	imp->m_saveFlag = flags;
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
		ar.BeginChunk(CID_OBJ_PART_LIST);
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
					ar.WriteChunk(CID_OBJ_PART_MESHWEIGHT, p.GetMeshWeight());
					ar.WriteChunk(CID_OBJ_PART_NAME, p.GetName());

					if (!p.m_node.empty()) ar.WriteChunk(CID_OBJ_PART_NODELIST, p.m_node);
					if (!p.m_edge.empty()) ar.WriteChunk(CID_OBJ_PART_EDGELIST, p.m_edge);
					if (!p.m_face.empty()) ar.WriteChunk(CID_OBJ_PART_FACELIST, p.m_face);

					if (p.Parameters() > 0)
					{
						ar.BeginChunk(CID_OBJ_PART_PARAMS);
						{
							p.ParamContainer::Save(ar);
						}
						ar.EndChunk();
					}

					GPartSection* section = p.GetSection();
					if (section)
					{
						GSolidSection* solid = dynamic_cast<GSolidSection*>(section);
						if (solid)
						{
							ar.BeginChunk(CID_OBJ_PART_SOLIDSECTION);
							{
								solid->Save(ar);
							}
							ar.EndChunk();
						}

						GShellSection* shell = dynamic_cast<GShellSection*>(section);
						if (shell)
						{
							ar.BeginChunk(CID_OBJ_PART_SHELLSECTION);
							{
								shell->Save(ar);
							}
							ar.EndChunk();
						}

						GBeamSection* beam = dynamic_cast<GBeamSection*>(section);
						if (beam)
						{
							ar.BeginChunk(CID_OBJ_PART_BEAMSECTION);
							{
								beam->Save(ar);
							}
							ar.EndChunk();
						}
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
		ar.BeginChunk(CID_OBJ_FACE_LIST);
		{
			for (int i = 0; i<Faces(); ++i)
			{
				ar.BeginChunk(CID_OBJ_FACE);
				{
					GFace& f = *Face(i);
					int nid = f.GetID();
					ar.WriteChunk(CID_OBJ_FACE_ID, nid);
					ar.WriteChunk(CID_OBJ_FACE_TYPE, f.m_ntype);
					ar.WriteChunk(CID_OBJ_FACE_MESHWEIGHT, f.GetMeshWeight());
					ar.WriteChunk(CID_OBJ_FACE_NAME, f.GetName());
					ar.WriteChunk(CID_OBJ_FACE_PID0, f.m_nPID[0]);
					ar.WriteChunk(CID_OBJ_FACE_PID1, f.m_nPID[1]);
					ar.WriteChunk(CID_OBJ_FACE_PID2, f.m_nPID[2]);

					if (!f.m_node.empty()) ar.WriteChunk(CID_OBJ_FACE_NODELIST, f.m_node);
					if (!f.m_edge.empty()) ar.WriteChunk(CID_OBJ_FACE_EDGELIST, f.m_edge);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the edges
	if (Edges() > 0)
	{
		ar.BeginChunk(CID_OBJ_EDGE_LIST);
		{
			for (int i = 0; i<Edges(); ++i)
			{
				ar.BeginChunk(CID_OBJ_EDGE);
				{
					GEdge& e = *Edge(i);
					int nid = e.GetID();
					ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
					ar.WriteChunk(CID_OBJ_EDGE_NAME, e.GetName());
					ar.WriteChunk(CID_OBJ_EDGE_TYPE, e.Type());
					ar.WriteChunk(CID_OBJ_EDGE_MESHWEIGHT, e.GetMeshWeight());
					ar.WriteChunk(CID_OBJ_EDGE_ORIENT, e.m_orient);
					ar.WriteChunk(CID_OBJ_EDGE_NODE0, e.m_node[0]);
					ar.WriteChunk(CID_OBJ_EDGE_NODE1, e.m_node[1]);
					ar.WriteChunk(CID_OBJ_EDGE_NODE2, e.m_cnode);
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
		ar.BeginChunk(CID_OBJ_NODE_LIST);
		{
			for (int i = 0; i<Nodes(); ++i)
			{
				ar.BeginChunk(CID_OBJ_NODE);
				{
					GNode& v = *Node(i);
					int nid = v.GetID();
					ar.WriteChunk(CID_OBJ_NODE_ID, nid);
					ar.WriteChunk(CID_OBJ_NODE_TYPE, v.Type());
					ar.WriteChunk(CID_OBJ_NODE_MESHWEIGHT, v.GetMeshWeight());
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
		FEMesher* mesher = GetFEMesher();
		ar.BeginChunk(CID_OBJ_FEMESHER);
		{
			int ntype = mesher->Type();
			ar.BeginChunk(ntype);
			{
				GetFEMesher()->Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}

	// save the mesh
	if (imp->m_pmesh && (imp->m_saveFlag & SAVE_MESH))
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

	// Some objects already have the number of parts, faces, etc. allocated (e.g. primitives)
	// Others will rely on this function to allocate. 
	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;

	// process file
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
		case CID_OBJ_PART_LIST:
		{
			assert(nparts > 0);
			assert(m_Part.empty() || (m_Part.size() == nparts));

			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_LIST (GMeshObject::Load)");

				GPart* p = nullptr;
				if (n < m_Part.size()) p = Part(n);
				else {
					p = new GPart(this);
					m_Part.push_back(p);
				}

				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid, mid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_PART_ID: ar.read(nid); p->SetID(nid); break;
					case CID_OBJ_PART_MAT: ar.read(mid); p->SetMaterialID(mid); break;
					case CID_OBJ_PART_MESHWEIGHT: { double w = 0.0; ar.read(w); p->SetMeshWeight(w); } break;
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
					case CID_OBJ_PART_SOLIDSECTION:
					{
						GSolidSection* solid = new GSolidSection(p);
						p->SetSection(solid);
						solid->Load(ar);
					}
					break;
					case CID_OBJ_PART_SHELLSECTION:
					{
						GShellSection* shell = new GShellSection(p);
						p->SetSection(shell);
						shell->Load(ar);
					}
					break;
					case CID_OBJ_PART_BEAMSECTION:
					{
						GBeamSection* beam = new GBeamSection(p);
						p->SetSection(beam);
						beam->Load(ar);
					}
					break;
					case CID_OBJ_PART_NODELIST: ar.read(p->m_node); break;
					case CID_OBJ_PART_EDGELIST: ar.read(p->m_edge); break;
					case CID_OBJ_PART_FACELIST: ar.read(p->m_face); break;
					}
					ar.CloseChunk();
				}
				ar.CloseChunk();

				p->SetLocalID(n++);
			}
			assert((int)m_Part.size() == nparts);
		}
		break;
		// object surfaces
		case CID_OBJ_FACE_LIST:
		{
			assert(nfaces > 0);
			assert(m_Face.empty() || (m_Face.size() == nfaces));

			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_LIST (GMeshObject::Load)");

				GFace* f = nullptr;
				if (n < m_Face.size()) f = Face(n);
				else {
					f = new GFace(this);
					m_Face.push_back(f);
				}

				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_FACE_ID: ar.read(nid); f->SetID(nid); break;
					case CID_OBJ_FACE_TYPE: ar.read(f->m_ntype); break;
					case CID_OBJ_FACE_MESHWEIGHT: { double w = 0.0; ar.read(w); f->SetMeshWeight(w); } break;
					case CID_OBJ_FACE_NODELIST: ar.read(f->m_node); break;
					case CID_OBJ_FACE_EDGELIST: ar.read(f->m_edge); break;
					case CID_OBJ_FACE_NAME:
					{
						char szname[256] = { 0 };
						ar.read(szname);
						f->SetName(szname);
					}
					break;
					case CID_OBJ_FACE_PID0    : ar.read(f->m_nPID[0]); break;
					case CID_OBJ_FACE_PID1    : ar.read(f->m_nPID[1]); break;
					}
					ar.CloseChunk();
				}
				ar.CloseChunk();

				f->SetLocalID(n++);
			}
			assert((int)m_Face.size() == nfaces);
		}
		break;
		// object edges
		case CID_OBJ_EDGE_LIST:
		{
			assert(nedges > 0);
			assert(m_Edge.empty() || (m_Edge.size() == nedges));

			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_LIST (GMeshObject::Load)");

				GEdge* e = nullptr;
				if (n < m_Edge.size()) e = Edge(n);
				else {
					e = new GEdge(this);
					m_Edge.push_back(e);
				}

				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_EDGE_ID: ar.read(nid); e->SetID(nid); break;
					case CID_OBJ_EDGE_TYPE: ar.read(e->m_ntype); break;
					case CID_OBJ_EDGE_MESHWEIGHT: { double w = 0.0; ar.read(w); e->SetMeshWeight(w); } break;
					case CID_OBJ_EDGE_NAME:
					{
						char szname[256] = { 0 };
						ar.read(szname);
						e->SetName(szname);
					}
					break;
					case CID_OBJ_EDGE_ORIENT: ar.read(e->m_orient); break;
					case CID_OBJ_EDGE_NODE0 : ar.read(e->m_node[0]); break;
					case CID_OBJ_EDGE_NODE1 : ar.read(e->m_node[1]); break;
					case CID_OBJ_EDGE_NODE2 : ar.read(e->m_cnode); break;
					}
					ar.CloseChunk();
				}
				ar.CloseChunk();

				e->SetLocalID(n++);
			}
			assert((int)m_Edge.size() == nedges);
		}
		break;
		// object nodes
		case CID_OBJ_NODE_LIST:
		{
			assert(nnodes > 0);
			assert(m_Node.empty() || (m_Node.size() == nnodes));

			if (nnodes > 0)
			{
				int m = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_LIST (GMeshObject::Load)");

					GNode* n = nullptr;
					if (m < m_Node.size()) n = Node(m);
					else {
						n = new GNode(this);
						m_Node.push_back(n);
					}

					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_NODE_ID: ar.read(nid); n->SetID(nid); break;
						case CID_OBJ_NODE_TYPE: { int ntype;  ar.read(ntype); n->SetType(ntype); } break;
						case CID_OBJ_NODE_MESHWEIGHT: { double w = 0.0; ar.read(w); n->SetMeshWeight(w); } break;
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
				}
				assert((int)m_Node.size() == nnodes);
			}
		}
		break;
		// mesher object (new way)
		case CID_OBJ_FEMESHER:
		{
			if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error parsing CID_OBJ_FEMESHER (GObject::Load)");
			else
			{
				int ntype = ar.GetChunkID();
				switch (ntype)
				{
				case 0: break;	// use default mesher
				case 1:
				{
					FEMesher* mesher = new FETetGenMesher(this);
					SetFEMesher(mesher);
				}
				break;
				default:
				{
					FEMesher* mesher = FSCore::CreateClassFromID<FEMesher>(CLASS_MESHER, ntype);
					assert(mesher);
					if (mesher == nullptr) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
					SetFEMesher(mesher);
				}
				break;
				}

				if (GetFEMesher())
					GetFEMesher()->Load(ar);
			}
			ar.CloseChunk();
			if (ar.OpenChunk() != IArchive::IO_END) throw ReadError("error parsing CID_OBJ_FEMESHER (GObject::Load)");
		}
		break;
		// the mesh object
		case CID_MESH:
			if (imp->m_pmesh) delete imp->m_pmesh;
			SetFEMesh(new FSMesh);
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

void GObject::ShowElements(vector<int>& elemList, bool show)
{
	FSMesh* mesh = GetFEMesh();
	if (mesh)
	{
		mesh->ShowElements(elemList, show);
		BuildFERenderMesh();
	}
}

GObjectManipulator* GObject::GetManipulator()
{
	return imp->m_objManip;
}

void GObject::SetManipulator(GObjectManipulator* om)
{
	imp->m_objManip = om;
}

bool IsSameFace(int n[4], int m[4])
{
	if ((n[0] == m[0]) && (n[1] == m[1]) && (n[2] == m[2]) && (n[3] == m[3])) return true;
	if ((n[0] == m[1]) && (n[1] == m[2]) && (n[2] == m[3]) && (n[3] == m[0])) return true;
	if ((n[0] == m[2]) && (n[1] == m[3]) && (n[2] == m[0]) && (n[3] == m[1])) return true;
	if ((n[0] == m[3]) && (n[1] == m[0]) && (n[2] == m[1]) && (n[3] == m[2])) return true;

	if ((n[0] == m[0]) && (n[1] == m[3]) && (n[2] == m[2]) && (n[3] == m[1])) return true;
	if ((n[0] == m[3]) && (n[1] == m[2]) && (n[2] == m[1]) && (n[3] == m[0])) return true;
	if ((n[0] == m[2]) && (n[1] == m[1]) && (n[2] == m[0]) && (n[3] == m[3])) return true;
	if ((n[0] == m[1]) && (n[1] == m[0]) && (n[2] == m[3]) && (n[3] == m[2])) return true;

	return false;
}

GObjectManipulator::GObjectManipulator(GObject* po) : m_po(po)
{
	assert(po);
}

GObjectManipulator::~GObjectManipulator()
{
	m_po = nullptr;
}

GObject* GObjectManipulator::GetObject()
{
	return m_po;
}
