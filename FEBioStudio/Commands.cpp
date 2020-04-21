#include "stdafx.h"
#include "Commands.h"
#include "Document.h"
#include "GLView.h"
#include <GeomLib/GObject.h>
#include <MeshTools/FEMesher.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMultiBox.h>
#include <MeshTools/GModel.h>
#include <GeomLib/MeshLayer.h>

//////////////////////////////////////////////////////////////////////
// CCmdAddObject
//////////////////////////////////////////////////////////////////////

CCmdAddObject::CCmdAddObject(GObject* po) : CCommand("Add object") 
{ 
	m_pobj = po; 
	m_bdel = true; 
}

CCmdAddObject::~CCmdAddObject() 
{ 
	if (m_bdel) delete m_pobj; 
}

void CCmdAddObject::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// add the mesh to the model
	m.AddObject(m_pobj);

	m_bdel = false;
}

void CCmdAddObject::UnExecute()
{
	GModel* gm = m_pDoc->GetGModel();

	// remove the mesh from the model
	gm->RemoveObject(m_pobj, true);

	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddDiscreteObject
//////////////////////////////////////////////////////////////////////

void CCmdAddDiscreteObject::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// add the mesh to the model
	m.AddDiscreteObject(m_pobj);

	m_bdel = false;
}

void CCmdAddDiscreteObject::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// remove the mesh from the model
	m.RemoveDiscreteObject(m_pobj);

	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddInterface
//////////////////////////////////////////////////////////////////////

void CCmdAddInterface::Execute()
{
	// add the mesh to the step
	m_ps->AddInterface(m_pint);
	m_bdel = false;
}

void CCmdAddInterface::UnExecute()
{
	// remove the mesh from the step
	m_ps->RemoveInterface(m_pint);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddRigidConnector
//////////////////////////////////////////////////////////////////////

void CCmdAddRigidConnector::Execute()
{
	// add the connector to the step
	m_ps->AddRigidConnector(m_pint);
	m_bdel = false;
}

void CCmdAddRigidConnector::UnExecute()
{
	// remove the connector from the step
	m_ps->RemoveRigidConnector(m_pint);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddPart
//////////////////////////////////////////////////////////////////////

void CCmdAddPart::Execute()
{
	// add the group to the mesh
	m_po->AddFEPart(m_pg);
	m_bdel = false;
}

void CCmdAddPart::UnExecute()
{
	// remove the mesh from the model
	m_po->RemoveFEPart(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddSurface
//////////////////////////////////////////////////////////////////////

void CCmdAddSurface::Execute()
{
	// add the group to the mesh
	m_po->AddFESurface(m_pg);
	m_bdel = false;
}

void CCmdAddSurface::UnExecute()
{
	// remove the mesh from the model
	m_po->RemoveFESurface(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddFEEdgeSet
//////////////////////////////////////////////////////////////////////

void CCmdAddFEEdgeSet::Execute()
{
	// add the group to the mesh
	m_po->AddFEEdgeSet(m_pg);
	m_bdel = false;
}

void CCmdAddFEEdgeSet::UnExecute()
{
	// remove the mesh from the model
	m_po->RemoveFEEdgeSet(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddNodeSet
//////////////////////////////////////////////////////////////////////

void CCmdAddNodeSet::Execute()
{
	// add the group to the mesh
	m_po->AddFENodeSet(m_pg);
	m_bdel = false;
}

void CCmdAddNodeSet::UnExecute()
{
	// remove the mesh from the model
	m_po->RemoveFENodeSet(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddGPartGroup
//////////////////////////////////////////////////////////////////////

void CCmdAddGPartGroup::Execute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.AddPartList(m_pg);
	m_bdel = false;
}

void CCmdAddGPartGroup::UnExecute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.RemovePartList(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddGFaceGroup
//////////////////////////////////////////////////////////////////////

void CCmdAddGFaceGroup::Execute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.AddFaceList(m_pg);
	m_bdel = false;
}

void CCmdAddGFaceGroup::UnExecute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.RemoveFaceList(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddGEdgeGroup
//////////////////////////////////////////////////////////////////////

void CCmdAddGEdgeGroup::Execute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.AddEdgeList(m_pg);
	m_bdel = false;
}

void CCmdAddGEdgeGroup::UnExecute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.RemoveEdgeList(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddGNodeGroup
//////////////////////////////////////////////////////////////////////

void CCmdAddGNodeGroup::Execute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.AddNodeList(m_pg);
	m_bdel = false;
}

void CCmdAddGNodeGroup::UnExecute()
{
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m.RemoveNodeList(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddBC
//////////////////////////////////////////////////////////////////////

void CCmdAddBC::Execute()
{
	// add the group to the mesh
	m_ps->AddBC(m_pbc);
	m_bdel = false;
}

void CCmdAddBC::UnExecute()
{
	// remove the mesh from the model
	m_ps->RemoveBC(m_pbc);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddIC
//////////////////////////////////////////////////////////////////////

void CCmdAddIC::Execute()
{
	m_ps->AddIC(m_pic);
	m_bdel = false;
}

void CCmdAddIC::UnExecute()
{
	m_ps->RemoveIC(m_pic);
	m_bdel = true;
}


//////////////////////////////////////////////////////////////////////
// CCmdAddLoad
//////////////////////////////////////////////////////////////////////

void CCmdAddLoad::Execute()
{
	// add the group to the mesh
	m_ps->AddLoad(m_pfc);
	m_bdel = false;
}

void CCmdAddLoad::UnExecute()
{
	// remove the mesh from the model
	m_ps->RemoveLoad(m_pfc);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddRC
//////////////////////////////////////////////////////////////////////

void CCmdAddRC::Execute()
{
	m_ps->AddRC(m_prc);
	m_bdel = false;
}

void CCmdAddRC::UnExecute()
{
	m_ps->RemoveRC(m_prc);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdDeleteDiscreteObject
//////////////////////////////////////////////////////////////////////

void CCmdDeleteDiscreteObject::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// remove the mesh to the model
	m_npos = m.RemoveDiscreteObject(m_pobj);

	m_pDoc->SetItemMode(ITEM_MESH);

	m_bdel = true;
}

void CCmdDeleteDiscreteObject::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// remove the mesh from the model
	m.InsertDiscreteObject(m_pobj, m_npos);

	m_bdel = false;
}

//////////////////////////////////////////////////////////////////////
// CCmdTranslateSelection
//////////////////////////////////////////////////////////////////////

CCmdTranslateSelection::CCmdTranslateSelection(vec3d dr) : CCommand("Translate")
{
	m_dr = dr;
}

void CCmdTranslateSelection::Execute()
{
	m_pDoc->GetCurrentSelection()->Translate(m_dr);
}

void CCmdTranslateSelection::UnExecute()
{
	m_pDoc->GetCurrentSelection()->Translate(-m_dr);
}

//////////////////////////////////////////////////////////////////////
// CCmdRotateSelection
//////////////////////////////////////////////////////////////////////

CCmdRotateSelection::CCmdRotateSelection(quatd q, vec3d rc) : CCommand("Rotate")
{
	m_q = q;
	m_rc = rc;
}

void CCmdRotateSelection::Execute()
{
	m_pDoc->GetCurrentSelection()->Rotate(m_q, m_rc);
}

void CCmdRotateSelection::UnExecute()
{
	m_pDoc->GetCurrentSelection()->Rotate(m_q.Inverse(), m_rc);
}

//////////////////////////////////////////////////////////////////////
// CCmdScaleSelection
//////////////////////////////////////////////////////////////////////

CCmdScaleSelection::CCmdScaleSelection(double s, vec3d dr, vec3d rc) : CCommand("Scale")
{
	m_s = s;
	m_dr = dr;
	m_rc = rc;
}

void CCmdScaleSelection::Execute()
{
	m_pDoc->GetCurrentSelection()->Scale(m_s, m_dr, m_rc);
}

void CCmdScaleSelection::UnExecute()
{
	m_pDoc->GetCurrentSelection()->Scale(1 / m_s, m_dr, m_rc);
}

//=============================================================================
CCmdToggleObjectVisibility::CCmdToggleObjectVisibility() : CCommand("Toggle visibility")
{
}

void CCmdToggleObjectVisibility::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int N = m.Objects();
	for (int i = 0; i<N; ++i)
	{
		GObject* po = m.Object(i);
		if (po->IsVisible()) po->Hide(); else po->Show();
	}
}

void CCmdToggleObjectVisibility::UnExecute()
{
	Execute();
}

//=============================================================================
CCmdTogglePartVisibility::CCmdTogglePartVisibility() : CCommand("Toggle visibility")
{
}

void CCmdTogglePartVisibility::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int N = m.Objects();
	for (int i = 0; i<N; ++i)
	{
		GObject* po = m.Object(i);
		if (po->IsVisible())
		{
			int NP = po->Parts();
			for (int j = 0; j<NP; ++j)
			{
				GPart* pg = po->Part(j);
				if (pg->IsVisible()) pg->HideItem(); else pg->ShowItem();
			}
			po->UpdateItemVisibility();
		}
	}
}

void CCmdTogglePartVisibility::UnExecute()
{
	Execute();
}


//=============================================================================
CCmdToggleDiscreteVisibility::CCmdToggleDiscreteVisibility() : CCommand("Toggle visibility")
{
}

void CCmdToggleDiscreteVisibility::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int N = m.DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* po = m.DiscreteObject(i);
		if (po->IsVisible()) po->Hide(); else po->Show();
	}
}

void CCmdToggleDiscreteVisibility::UnExecute()
{
	Execute();
}

//=============================================================================
CCmdToggleElementVisibility::CCmdToggleElementVisibility() : CCommand("Toggle visibility")
{
}

void CCmdToggleElementVisibility::Execute()
{
	GObject* po = m_pDoc->GetActiveObject();
	if (po == 0) return;

	FEMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	for (int i = 0; i<pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsVisible()) el.Hide(); else el.Show();
	}
	pm->UpdateItemVisibility();
}

void CCmdToggleElementVisibility::UnExecute()
{
	Execute();
}

//=============================================================================
CCmdToggleFEFaceVisibility::CCmdToggleFEFaceVisibility() : CCommand("Toggle visibility")
{
}

void CCmdToggleFEFaceVisibility::Execute()
{
	GObject* po = m_pDoc->GetActiveObject();
	if (po == 0) return;

	FEMeshBase* pm = po->GetEditableMesh();
	if (pm == 0) return;

	for (int i = 0; i<pm->Faces(); ++i)
	{
		FEFace& face = pm->Face(i);
		if (face.IsVisible()) face.Hide(); else face.Show();
	}
	pm->UpdateItemVisibility();
}

void CCmdToggleFEFaceVisibility::UnExecute()
{
	Execute();
}


//////////////////////////////////////////////////////////////////////
// CCmdSelectObject
//////////////////////////////////////////////////////////////////////

CCmdSelectObject::CCmdSelectObject(GObject* po, bool badd) : CCommand("Select Object")
{
	m_badd = badd;

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// store the meshes selection state

	int M = m.Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m.Object(i)->IsSelected();

	// store the meshes we need to select
	if (po != 0)
	{
		m_N = 1;
		m_ppo = new GObject*[m_N];
		m_ppo[0] = po;
	}
	else
	{
		m_N = 0;
		m_ppo = 0;
	}
}

CCmdSelectObject::CCmdSelectObject(GObject** ppo, int N, bool badd) : CCommand("Select Object")
{
	m_badd = badd;

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// store the meshes selection state
	int M = m.Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m.Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = ppo[i];
}

CCmdSelectObject::CCmdSelectObject(const vector<GObject*>& po, bool badd) : CCommand("Select Object")
{
	m_badd = badd;
	int N = (int)po.size();

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// store the meshes selection state
	int M = m.Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m.Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = po[i];
}


void CCmdSelectObject::Execute()
{
	m_pDoc->SetItemMode(ITEM_MESH);

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (!m_badd)
	{
		for (int i = 0; i<m.Objects(); ++i) m.Object(i)->UnSelect();
	}

	for (int i = 0; i<m_N; ++i) m_ppo[i]->Select();
}

void CCmdSelectObject::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_MESH);

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	for (int i = 0; i<m.Objects(); ++i)
	{
		GObject* po = m.Object(i);
		if (m_ptag[i])
			po->Select();
		else
			po->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectObject
//////////////////////////////////////////////////////////////////////

CCmdUnselectObject::CCmdUnselectObject(GObject* po) : CCommand("Unselect Object")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// store the meshes selection state
	int M = m.Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m.Object(i)->IsSelected();

	// store the meshes we need to unselect
	m_N = 1;
	m_ppo = new GObject*[m_N];
	m_ppo[0] = po;
}

CCmdUnselectObject::CCmdUnselectObject(GObject** ppo, int N) : CCommand("Unselect Object")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// store the meshes selection state
	int M = m.Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m.Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = ppo[i];
}

CCmdUnselectObject::CCmdUnselectObject(const vector<GObject*>& po) : CCommand("Unselect Object")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	// store the meshes selection state
	int N = (int)po.size();
	int M = m.Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m.Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = po[i];
}


void CCmdUnselectObject::Execute()
{
	m_pDoc->SetItemMode(ITEM_MESH);
	for (int i = 0; i<m_N; ++i) m_ppo[i]->UnSelect();
}

void CCmdUnselectObject::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	m_pDoc->SetItemMode(ITEM_MESH);
	for (int i = 0; i<m.Objects(); ++i)
	{
		GObject* po = m.Object(i);
		if (m_ptag[i])
			po->Select();
		else
			po->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectPart
//////////////////////////////////////////////////////////////////////

CCmdSelectPart::CCmdSelectPart(FEModel* ps, int* npart, int n, bool badd) : CCommand("Select Part")
{
	m_ps = ps;

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = npart[i];
	}

	m_bold.resize(m.Parts());
	int N = m.Parts();
	for (int i = 0; i<N; ++i) m_bold[i] = m.Part(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectPart::CCmdSelectPart(FEModel* ps, const vector<int>& part, bool badd) : CCommand("Select Part")
{
	m_ps = ps;
	int n = (int)part.size();

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = part[i];
	}

	m_bold.resize(m.Parts());
	int N = m.Parts();
	for (int i = 0; i<N; ++i) m_bold[i] = m.Part(i)->IsSelected();
	m_badd = badd;
}

void CCmdSelectPart::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	if (!m_badd)
	{
		n = m.Parts();
		for (i = 0; i<n; ++i) m.Part(i)->UnSelect();
	}

	n = (int)m_npart.size();
	for (i = 0; i<n; ++i)
	{
		GPart* pg = m.FindPart(m_npart[i]);
		if (pg) pg->Select();
	}

	m_pDoc->SetSelectionMode(SELECT_PART);
}

void CCmdSelectPart::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GPart* pg = m.Part(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectPart
//////////////////////////////////////////////////////////////////////

CCmdUnSelectPart::CCmdUnSelectPart(FEModel* ps, int* npart, int n) : CCommand("Unselect Part")
{
	m_ps = ps;

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = npart[i];
	}

	int N = m.Parts();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Part(i)->IsSelected();
}

CCmdUnSelectPart::CCmdUnSelectPart(FEModel* ps, const vector<int>& part) : CCommand("Unselect Part")
{
	m_ps = ps;
	int n = (int)part.size();

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = part[i];
	}

	int N = m.Parts();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Part(i)->IsSelected();
}


void CCmdUnSelectPart::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	n = m_npart.size();
	for (i = 0; i<n; ++i)
	{
		GPart* pg = m.FindPart(m_npart[i]);
		if (pg) pg->UnSelect();
	}
	m_pDoc->SetSelectionMode(SELECT_PART);
}

void CCmdUnSelectPart::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GPart* pg = m.Part(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectSurface
//////////////////////////////////////////////////////////////////////

CCmdSelectSurface::CCmdSelectSurface(FEModel* ps, int* nsurf, int n, bool badd) : CCommand("Select Surface")
{
	m_ps = ps;

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = nsurf[i];
	}

	int N = m.Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Surface(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectSurface::CCmdSelectSurface(FEModel* ps, const vector<int>& surf, bool badd) : CCommand("Select Surface")
{
	m_ps = ps;
	int n = (int)surf.size();

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = surf[i];
	}

	int N = m.Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Surface(i)->IsSelected();
	m_badd = badd;
}

void CCmdSelectSurface::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	if (!m_badd)
	{
		n = m.Surfaces();
		for (i = 0; i<n; ++i) m.Surface(i)->UnSelect();
	}

	n = m_nsurf.size();
	for (i = 0; i<n; ++i)
	{
		GFace* ps = m.FindSurface(m_nsurf[i]);
		if (ps) ps->Select();
		else
		{
			//			flx_error("Reference to undefined surface in CCmdSelectSurface::Execute().");
			break;
		}
	}

	m_pDoc->SetSelectionMode(SELECT_FACE);
}

void CCmdSelectSurface::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GFace* pg = m.Surface(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectSurface
//////////////////////////////////////////////////////////////////////

CCmdUnSelectSurface::CCmdUnSelectSurface(FEModel* ps, int* nsurf, int n) : CCommand("Unselect Surface")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	m_ps = ps;
	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = nsurf[i];
	}

	int N = m.Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Surface(i)->IsSelected();
}

CCmdUnSelectSurface::CCmdUnSelectSurface(FEModel* ps, const vector<int>& surf) : CCommand("Unselect Surface")
{
	int n = (int)surf.size();

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	m_ps = ps;
	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = surf[i];
	}

	int N = m.Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Surface(i)->IsSelected();
}


void CCmdUnSelectSurface::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	n = m_nsurf.size();
	for (i = 0; i<n; ++i)
	{
		GFace* ps = m.FindSurface(m_nsurf[i]);
		if (ps) ps->UnSelect();
	}
	m_pDoc->SetSelectionMode(SELECT_FACE);
}

void CCmdUnSelectSurface::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GFace* pg = m.Surface(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectEdge
//////////////////////////////////////////////////////////////////////

CCmdSelectEdge::CCmdSelectEdge(FEModel* ps, int* nedge, int n, bool badd) : CCommand("Select Edge")
{
	m_ps = ps;

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = nedge[i];
	}

	int N = m.Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Edge(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectEdge::CCmdSelectEdge(FEModel* ps, const vector<int>& edge, bool badd) : CCommand("Select Edge")
{
	m_ps = ps;
	int n = (int)edge.size();

	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = edge[i];
	}

	int N = m.Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Edge(i)->IsSelected();
	m_badd = badd;
}


void CCmdSelectEdge::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	if (!m_badd)
	{
		n = m.Edges();
		for (i = 0; i<n; ++i) m.Edge(i)->UnSelect();
	}

	n = m_nedge.size();
	for (i = 0; i<n; ++i)
	{
		GEdge* ps = m.FindEdge(m_nedge[i]);
		if (ps) ps->Select();
	}

	m_pDoc->SetSelectionMode(SELECT_EDGE);
}

void CCmdSelectEdge::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GEdge* pg = m.Edge(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectEdge
//////////////////////////////////////////////////////////////////////

CCmdUnSelectEdge::CCmdUnSelectEdge(FEModel* ps, int* nedge, int n) : CCommand("Unselect Edge")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	m_ps = ps;
	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = nedge[i];
	}

	int N = m.Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Edge(i)->IsSelected();
}

CCmdUnSelectEdge::CCmdUnSelectEdge(FEModel* ps, const vector<int>& edge) : CCommand("Unselect Edge")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = (int)edge.size();
	m_ps = ps;
	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = edge[i];
	}

	int N = m.Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Edge(i)->IsSelected();
}


void CCmdUnSelectEdge::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	n = m_nedge.size();
	for (i = 0; i<n; ++i)
	{
		GEdge* ps = m.FindEdge(m_nedge[i]);
		if (ps) ps->UnSelect();
	}
	m_pDoc->SetSelectionMode(SELECT_EDGE);
}

void CCmdUnSelectEdge::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GEdge* pg = m.Edge(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectNode
//////////////////////////////////////////////////////////////////////

CCmdSelectNode::CCmdSelectNode(FEModel* ps, int* node, int n, bool badd) : CCommand("Select Node")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	m_ps = ps;
	if (n > 0)
	{
		m_node.resize(n);
		for (int i = 0; i<n; ++i) m_node[i] = node[i];
	}

	int N = m.Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Node(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectNode::CCmdSelectNode(FEModel* ps, const vector<int>& node, bool badd) : CCommand("Select Node")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = (int)node.size();
	m_ps = ps;
	m_node = node;

	int N = m.Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Node(i)->IsSelected();
	m_badd = badd;
}


void CCmdSelectNode::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	if (!m_badd)
	{
		n = m.Nodes();
		for (i = 0; i<n; ++i) m.Node(i)->UnSelect();
	}

	n = m_node.size();
	for (i = 0; i<n; ++i)
	{
		GNode* pn = m.FindNode(m_node[i]);
		if (pn) pn->Select();
	}

	m_pDoc->SetSelectionMode(SELECT_NODE);
}

void CCmdSelectNode::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GNode* pn = m.Node(i);
		if (m_bold[i]) pn->Select(); else pn->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectNode
//////////////////////////////////////////////////////////////////////

CCmdUnSelectNode::CCmdUnSelectNode(FEModel* ps, int* node, int n) : CCommand("Unselect Node")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	m_ps = ps;
	if (n > 0)
	{
		m_node.resize(n);
		for (int i = 0; i<n; ++i) m_node[i] = node[i];
	}

	int N = m.Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Node(i)->IsSelected();
}

CCmdUnSelectNode::CCmdUnSelectNode(FEModel* ps, const vector<int>& node) : CCommand("Unselect Node")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = (int)node.size();
	m_ps = ps;
	if (n > 0)
	{
		m_node.resize(n);
		for (int i = 0; i<n; ++i) m_node[i] = node[i];
	}

	int N = m.Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.Node(i)->IsSelected();
}

void CCmdUnSelectNode::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int i, n;
	n = m_node.size();
	for (i = 0; i<n; ++i)
	{
		GNode* pn = m.FindNode(m_node[i]);
		if (pn) pn->UnSelect();
	}
	m_pDoc->SetSelectionMode(SELECT_NODE);
}

void CCmdUnSelectNode::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GNode* pn = m.Node(i);
		if (m_bold[i]) pn->Select(); else pn->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectDiscrete
//////////////////////////////////////////////////////////////////////

CCmdSelectDiscrete::CCmdSelectDiscrete(GModel* ps, int* pobj, int n, bool badd) : CCommand("Select Discrete")
{
	m_pgm = ps;
	GModel& m = *m_pgm;
	if (n > 0)
	{
		m_obj.resize(n);
		for (int i = 0; i<n; ++i) m_obj[i] = m.DiscreteObject(pobj[i]);
	}

	int N = m.DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.DiscreteObject(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectDiscrete::CCmdSelectDiscrete(GModel* ps, const vector<int>& obj, bool badd) : CCommand("Select Discrete")
{
	m_pgm = ps;
	GModel& m = *m_pgm;

	int n = obj.size();
	m_obj.resize(n);
	for (int i = 0; i<n; ++i) m_obj[i] = m.DiscreteObject(obj[i]);

	int N = m.DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.DiscreteObject(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectDiscrete::CCmdSelectDiscrete(GModel* ps, const vector<GDiscreteObject*>& obj, bool badd) : CCommand("Select Discrete")
{
	m_pgm = ps;
	GModel& m = *m_pgm;
	
	m_obj = obj;

	int N = m.DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.DiscreteObject(i)->IsSelected();
	m_badd = badd;
}

void CCmdSelectDiscrete::Execute()
{
	GModel& m = *m_pgm;
	int ND = m.DiscreteObjects();
	int i, n;
	if (!m_badd)
	{
		n = m.DiscreteObjects();
		for (i = 0; i<n; ++i) m.DiscreteObject(i)->UnSelect();
	}

	n = m_obj.size();
	for (i = 0; i<n; ++i)
	{
		GDiscreteObject* pn = m_obj[i];
		if (pn) pn->Select();
	}

	m_pDoc->SetSelectionMode(SELECT_DISCRETE);
}

void CCmdSelectDiscrete::UnExecute()
{
	GModel& m = *m_pgm;
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GDiscreteObject* pn = m.DiscreteObject(i);
		if (m_bold[i]) pn->Select(); else pn->UnSelect();
	}
}


//////////////////////////////////////////////////////////////////////
// CCmdSelectDiscreteElements
//////////////////////////////////////////////////////////////////////

CCmdSelectDiscreteElements::CCmdSelectDiscreteElements(GDiscreteElementSet* set, const vector<int>& elemList, bool badd) : CCommand("Select Discrete")
{
	m_ps = set;
	m_elemList = elemList;
	m_badd = badd;

	int N = set->size();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m_ps->element(i).IsSelected();
}

void CCmdSelectDiscreteElements::Execute()
{
	if (!m_badd)
	{
		int N = m_ps->size();
		for (int i = 0; i<N; ++i) m_ps->element(i).UnSelect();
	}

	int N = m_elemList.size();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteElement& de = m_ps->element(m_elemList[i]);
		de.Select();
	}

	m_pDoc->SetSelectionMode(SELECT_DISCRETE);
}

void CCmdSelectDiscreteElements::UnExecute()
{
	int N = m_bold.size();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteElement& de = m_ps->element(i);
		if (m_bold[i]) de.Select(); else de.UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectNode
//////////////////////////////////////////////////////////////////////

CCmdUnSelectDiscrete::CCmdUnSelectDiscrete(FEModel* ps, int* pobj, int n) : CCommand("Unselect Discrete")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	m_ps = ps;
	if (n > 0)
	{
		m_obj.resize(n);
		for (int i = 0; i<n; ++i) m_obj[i] = pobj[i];
	}

	int N = m.DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.DiscreteObject(i)->IsSelected();
}

CCmdUnSelectDiscrete::CCmdUnSelectDiscrete(FEModel* ps, const vector<int>& obj) : CCommand("Unselect Discrete")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = (int)obj.size();
	m_ps = ps;
	if (n > 0)
	{
		m_obj.resize(n);
		for (int i = 0; i<n; ++i) m_obj[i] = obj[i];
	}

	int N = m.DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m.DiscreteObject(i)->IsSelected();
}

void CCmdUnSelectDiscrete::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	int ND = m.DiscreteObjects();
	int i, n;
	n = m_obj.size();
	for (i = 0; i<n; ++i)
	{
		GDiscreteObject* pn = m.DiscreteObject(m_obj[i]);
		if (pn) pn->UnSelect();
	}
	m_pDoc->SetSelectionMode(SELECT_DISCRETE);
}

void CCmdUnSelectDiscrete::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	int ND = m.DiscreteObjects();
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GDiscreteObject* pn = m.DiscreteObject(i);
		if (m_bold[i]) pn->Select(); else pn->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdInvertSelection
//////////////////////////////////////////////////////////////////////

CCmdInvertSelection::CCmdInvertSelection() : CCommand("Invert selection")
{

}

void CCmdInvertSelection::Execute()
{
	m_pDoc->GetCurrentSelection()->Invert();
}

void CCmdInvertSelection::UnExecute()
{
	m_pDoc->GetCurrentSelection()->Invert();
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectElements
//////////////////////////////////////////////////////////////////////

CCmdSelectElements::CCmdSelectElements(FEMesh* pm, int* pe, int N, bool badd) : CCommand("Select Elements")
{
	int i;

	m_badd = badd;
	m_pel = 0;
	m_N = 0;

	// get the current mesh
	m_pm = pm;
	int M = pm->Elements();

	// store the elements selection state
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = pm->Element(i).IsSelected();

	// store the elements we need to select
	if (N != 0)
	{
		m_N = N;
		m_pel = new int[m_N];
		for (i = 0; i<N; ++i) m_pel[i] = pe[i];
	}
}

CCmdSelectElements::CCmdSelectElements(FEMesh* pm, vector<int>& el, bool badd) : CCommand("Select Elements")
{
	int i;
	int N = (int)el.size();

	m_badd = badd;
	m_pel = 0;
	m_N = 0;

	// get the current mesh
	m_pm = pm;
	int M = pm->Elements();

	// store the elements selection state
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = pm->Element(i).IsSelected();

	// store the elements we need to select
	if (N != 0)
	{
		m_N = N;
		m_pel = new int[m_N];
		for (i = 0; i<N; ++i) m_pel[i] = el[i];
	}
}


void CCmdSelectElements::Execute()
{
	int i;
	m_pDoc->SetItemMode(ITEM_ELEM);
	if (!m_badd)
	{
		for (i = 0; i<m_pm->Elements(); ++i) m_pm->Element(i).Unselect();
	}

	int NE = m_pm->Elements();
	for (i = 0; i<m_N; ++i)
	{
		int n = m_pel[i];
		if ((n >= 0) && (n<NE)) m_pm->Element(n).Select();
	}

	m_pm->UpdateSelection();
}

void CCmdSelectElements::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_ELEM);
	for (int i = 0; i<m_pm->Elements(); ++i)
	{
		FEElement& el = m_pm->Element(i);
		if (m_ptag[i])
			el.Select();
		else
			el.Unselect();
	}

	m_pm->UpdateSelection();
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectElements
//////////////////////////////////////////////////////////////////////

CCmdUnselectElements::CCmdUnselectElements(FEMesh* mesh, int* pe, int N) : CCommand("Unselect")
{
	// get the current mesh
	m_mesh = mesh;
	FEMesh* pm = mesh;
	int M = pm->Elements();

	// store the elements selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Element(i).IsSelected();

	// store the elements we need to select
	m_N = N;
	m_pel = new int[m_N];
	for (int i = 0; i<N; ++i) m_pel[i] = pe[i];
}

CCmdUnselectElements::CCmdUnselectElements(FEMesh* mesh, const vector<int>& elem) : CCommand("Unselect")
{
	int N = (int)elem.size();

	// get the current mesh
	m_mesh = mesh;
	FEMesh* pm = m_mesh;
	int M = pm->Elements();

	// store the elements selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Element(i).IsSelected();

	// store the elements we need to select
	m_N = N;
	m_pel = new int[m_N];
	for (int i = 0; i<N; ++i) m_pel[i] = elem[i];
}


void CCmdUnselectElements::Execute()
{
	m_pDoc->SetItemMode(ITEM_ELEM);
	FEMesh* pm = m_mesh;
	for (int i = 0; i<m_N; ++i) pm->Element(m_pel[i]).Unselect();

	pm->UpdateSelection();
}

void CCmdUnselectElements::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_ELEM);
	FEMesh* pm = m_mesh;
	for (int i = 0; i<pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		if (m_ptag[i])
			el.Select();
		else
			el.Unselect();
	}
	pm->UpdateSelection();
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectFaces
//////////////////////////////////////////////////////////////////////

CCmdSelectFaces::CCmdSelectFaces(FEMeshBase* pm, int* pf, int N, bool badd) : CCommand("Select Faces")
{
	int i;

	m_pm = pm;
	m_badd = badd;
	m_pface = 0;
	m_N = 0;

	// get the current mesh
	int M = pm->Faces();

	// store the faces selection state
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = pm->Face(i).IsSelected();

	// store the faces we need to select
	if (N != 0)
	{
		m_N = N;
		m_pface = new int[m_N];
		for (i = 0; i<N; ++i) m_pface[i] = pf[i];
	}
}

CCmdSelectFaces::CCmdSelectFaces(FEMeshBase* pm, vector<int>& fl, bool badd) : CCommand("Select Faces")
{
	int i;
	int N = (int)fl.size();

	m_pm = pm;
	m_badd = badd;
	m_pface = 0;
	m_N = 0;

	// get the current mesh
	int M = pm->Faces();

	// store the faces selection state
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = pm->Face(i).IsSelected();

	// store the faces we need to select
	if (N != 0)
	{
		m_N = N;
		m_pface = new int[m_N];
		for (i = 0; i<N; ++i) m_pface[i] = fl[i];
	}
}

void CCmdSelectFaces::Execute()
{
	int i;
	m_pDoc->SetItemMode(ITEM_FACE);
	if (!m_badd)
	{
		for (i = 0; i<m_pm->Faces(); ++i) m_pm->Face(i).Unselect();
	}

	int NF = m_pm->Faces();
	for (i = 0; i<m_N; ++i)
	{
		int n = m_pface[i];
		if ((n >= 0) && (n<NF)) m_pm->Face(n).Select();
	}

	m_pm->UpdateSelection();
}

void CCmdSelectFaces::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_FACE);
	for (int i = 0; i<m_pm->Faces(); ++i)
	{
		FEFace& face = m_pm->Face(i);
		if (m_ptag[i])
			face.Select();
		else
			face.Unselect();
	}
	m_pm->UpdateSelection();
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectFaces
//////////////////////////////////////////////////////////////////////

CCmdUnselectFaces::CCmdUnselectFaces(FEMeshBase* pm, int* pf, int N) : CCommand("Unselect")
{
	// store the mesh
	m_pm = pm;

	// store the faces selection state
	int M = pm->Faces();
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Face(i).IsSelected();

	// store the faces we need to select
	m_N = N;
	m_pface = new int[m_N];
	for (int i = 0; i<N; ++i) m_pface[i] = pf[i];
}

CCmdUnselectFaces::CCmdUnselectFaces(FEMeshBase* pm, const vector<int>& face) : CCommand("Unselect")
{
	// store the mesh
	m_pm = pm;

	// store the faces selection state
	int M = pm->Faces();
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Face(i).IsSelected();

	// store the faces we need to select
	m_N = (int)face.size();
	m_pface = new int[m_N];
	for (int i = 0; i<m_N; ++i) m_pface[i] = face[i];
}


void CCmdUnselectFaces::Execute()
{
	m_pDoc->SetItemMode(ITEM_FACE);
	for (int i = 0; i<m_N; ++i) m_pm->Face(m_pface[i]).Unselect();
	m_pm->UpdateSelection();
}

void CCmdUnselectFaces::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_FACE);
	for (int i = 0; i<m_pm->Faces(); ++i)
	{
		FEFace& face = m_pm->Face(i);
		if (m_ptag[i])
			face.Select();
		else
			face.Unselect();
	}
	m_pm->UpdateSelection();
}


//////////////////////////////////////////////////////////////////////
// CCmdSelectFEEdges
//////////////////////////////////////////////////////////////////////

CCmdSelectFEEdges::CCmdSelectFEEdges(FELineMesh* pm, int* pe, int N, bool badd) : CCommand("Select Edges")
{
	m_pm = pm;
	m_badd = badd;
	m_pedge = 0;
	m_N = 0;

	// get the current mesh
	int M = pm->Edges();

	// store the edges selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Edge(i).IsSelected();

	// store the faces we need to select
	if (N != 0)
	{
		m_N = N;
		m_pedge = new int[m_N];
		for (int i = 0; i<N; ++i) m_pedge[i] = pe[i];
	}
}

CCmdSelectFEEdges::CCmdSelectFEEdges(FELineMesh* pm, vector<int>& el, bool badd) : CCommand("Select Edges")
{
	int N = (int)el.size();

	m_pm = pm;
	m_badd = badd;
	m_pedge = 0;
	m_N = 0;

	// get the current mesh
	int M = pm->Edges();

	// store the edge selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Edge(i).IsSelected();

	// store the edges we need to select
	if (N != 0)
	{
		m_N = N;
		m_pedge = new int[m_N];
		for (int i = 0; i<N; ++i) m_pedge[i] = el[i];
	}
}

void CCmdSelectFEEdges::Execute()
{
	m_pDoc->SetItemMode(ITEM_EDGE);
	if (!m_badd)
	{
		for (int i = 0; i<m_pm->Edges(); ++i) m_pm->Edge(i).Unselect();
	}

	int NE = m_pm->Edges();
	for (int i = 0; i<m_N; ++i)
	{
		int n = m_pedge[i];
		if ((n >= 0) && (n<NE)) m_pm->Edge(n).Select();
	}
	m_pm->UpdateSelection();
}

void CCmdSelectFEEdges::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_EDGE);
	for (int i = 0; i<m_pm->Edges(); ++i)
	{
		FEEdge& edge = m_pm->Edge(i);
		if (m_ptag[i])
			edge.Select();
		else
			edge.Unselect();
	}
	m_pm->UpdateSelection();
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectFEEdges
//////////////////////////////////////////////////////////////////////

CCmdUnselectFEEdges::CCmdUnselectFEEdges(FELineMesh* pm, int* pe, int N) : CCommand("Unselect")
{
	m_pm = pm;

	// get the current mesh
	int M = m_pm->Edges();

	// store the edges selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Edge(i).IsSelected();

	// store the edges we need to select
	m_N = N;
	m_pedge = new int[m_N];
	for (int i = 0; i<N; ++i) m_pedge[i] = pe[i];
}

CCmdUnselectFEEdges::CCmdUnselectFEEdges(FELineMesh* pm, const vector<int>& edge) : CCommand("Unselect")
{
	m_pm = pm;

	int N = (int)edge.size();

	// get the current mesh
	int M = pm->Edges();

	// store the edges selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Edge(i).IsSelected();

	// store the edges we need to select
	m_N = N;
	m_pedge = new int[m_N];
	for (int i = 0; i<N; ++i) m_pedge[i] = edge[i];
}


void CCmdUnselectFEEdges::Execute()
{
	m_pDoc->SetItemMode(ITEM_EDGE);
	for (int i = 0; i<m_N; ++i) m_pm->Edge(m_pedge[i]).Unselect();
	m_pm->UpdateSelection();
}

void CCmdUnselectFEEdges::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_EDGE);
	for (int i = 0; i<m_pm->Edges(); ++i)
	{
		FEEdge& edge = m_pm->Edge(i);
		if (m_ptag[i])
			edge.Select();
		else
			edge.Unselect();
	}
	m_pm->UpdateSelection();
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectFENodes
//////////////////////////////////////////////////////////////////////

CCmdSelectFENodes::CCmdSelectFENodes(FELineMesh* pm, int* pn, int N, bool badd) : CCommand("Select Nodes")
{
	int i;

	m_badd = badd;
	m_pn = 0;
	m_N = 0;

	// get the current mesh
	m_pm = pm;
	int M = pm->Nodes();

	// store the nodes selection state
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = pm->Node(i).IsSelected();

	// store the nodes we need to select
	if (N != 0)
	{
		m_N = N;
		m_pn = new int[m_N];
		for (i = 0; i<N; ++i) m_pn[i] = pn[i];
	}
}

CCmdSelectFENodes::CCmdSelectFENodes(FELineMesh* pm, vector<int>& nl, bool badd) : CCommand("Select Nodes")
{
	int i;
	int N = (int)nl.size();

	m_badd = badd;
	m_pn = 0;
	m_N = 0;

	// get the current mesh
	m_pm = pm;
	int M = pm->Nodes();

	// store the nodes selection state
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = pm->Node(i).IsSelected();

	// store the nodes we need to select
	if (N != 0)
	{
		m_N = N;
		m_pn = new int[m_N];
		for (i = 0; i<N; ++i) m_pn[i] = nl[i];
	}
}

void CCmdSelectFENodes::Execute()
{
	int i;
	m_pDoc->SetItemMode(ITEM_NODE);
	if (!m_badd)
	{
		for (i = 0; i<m_pm->Nodes(); ++i) m_pm->Node(i).Unselect();
	}

	int NN = m_pm->Nodes();
	for (i = 0; i<m_N; ++i)
	{
		int n = m_pn[i];
		if ((n >= 0) && (n<NN)) m_pm->Node(n).Select();
	}
	m_pm->UpdateSelection();
}

void CCmdSelectFENodes::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_NODE);
	for (int i = 0; i<m_pm->Nodes(); ++i)
	{
		FENode& node = m_pm->Node(i);
		if (m_ptag[i])
			node.Select();
		else
			node.Unselect();
	}
	m_pm->UpdateSelection();
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectNodes
//////////////////////////////////////////////////////////////////////

CCmdUnselectNodes::CCmdUnselectNodes(FELineMesh* pm, int* pn, int N) : CCommand("Unselect")
{
	// get the current mesh
	m_mesh = pm;
	int M = pm->Nodes();

	// store the nodes selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Node(i).IsSelected();

	// store the nodes we need to select
	m_N = N;
	m_pn = new int[m_N];
	for (int i = 0; i<N; ++i) m_pn[i] = pn[i];
}

CCmdUnselectNodes::CCmdUnselectNodes(FELineMesh* pm, const vector<int>& node) : CCommand("Unselect")
{
	int N = (int)node.size();

	// get the current mesh
	m_mesh = pm;
	int M = pm->Nodes();

	// store the nodes selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Node(i).IsSelected();

	// store the nodes we need to select
	m_N = N;
	m_pn = new int[m_N];
	for (int i = 0; i<N; ++i) m_pn[i] = node[i];
}


void CCmdUnselectNodes::Execute()
{
	m_pDoc->SetItemMode(ITEM_NODE);
	FELineMesh* pm = m_mesh;
	for (int i = 0; i<m_N; ++i) pm->Node(m_pn[i]).Unselect();
	pm->UpdateSelection();
}

void CCmdUnselectNodes::UnExecute()
{
	m_pDoc->SetItemMode(ITEM_NODE);
	FELineMesh* pm = m_mesh;
	for (int i = 0; i<pm->Nodes(); ++i)
	{
		FENode& node = pm->Node(i);
		if (m_ptag[i])
			node.Select();
		else
			node.Unselect();
	}
	pm->UpdateSelection();
}

//////////////////////////////////////////////////////////////////////
// CCmdAssignPartMaterial
//////////////////////////////////////////////////////////////////////

CCmdAssignPartMaterial::CCmdAssignPartMaterial(FEModel* ps, vector<int> npart, int nmat) : CCommand("Assign material")
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int n = npart.size();

	m_ps = ps;
	m_part.resize(n);
	m_old.resize(n);
	for (int i = 0; i<n; ++i)
	{
		GPart* pg = m.FindPart(npart[i]);
		m_part[i] = npart[i];
		m_old[i] = pg->GetMaterialID();
	}
	m_mat = nmat;
}

void CCmdAssignPartMaterial::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int N = m_part.size();
	for (int i = 0; i<N; ++i)
	{
		GPart* pg = m.FindPart(m_part[i]);
		GObject* po = dynamic_cast<GObject*>(pg->Object());
		po->AssignMaterial(m_part[i], m_mat);
	}
}

void CCmdAssignPartMaterial::UnExecute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();

	int N = m_part.size();
	for (int i = 0; i<N; ++i)
	{
		GPart* pg = m.FindPart(m_part[i]);
		GObject* po = dynamic_cast<GObject*>(pg->Object());
		po->AssignMaterial(m_part[i], m_old[i]);
	}
}

//-----------------------------------------------------------------------------
// CCmdAssignObjectMaterial
//-----------------------------------------------------------------------------

CCmdAssignObjectMaterial::CCmdAssignObjectMaterial(GObject* po, int mat) : CCommand("Assign material")
{
	m_po = po;
	m_mat = mat;
	int N = po->Parts();
	m_old.resize(N);
	for (int i = 0; i<N; ++i) m_old[i] = po->Part(i)->GetMaterialID();
}

void CCmdAssignObjectMaterial::Execute()
{
	int N = m_po->Parts();
	for (int i = 0; i<N; ++i) m_po->AssignMaterial(m_po->Part(i)->GetID(), m_mat);
}

void CCmdAssignObjectMaterial::UnExecute()
{
	int N = m_po->Parts();
	for (int i = 0; i<N; ++i) m_po->AssignMaterial(m_po->Part(i)->GetID(), m_old[i]);
}

//-----------------------------------------------------------------------------
// CCmdAssignObjectListMaterial
//-----------------------------------------------------------------------------

CCmdAssignObjectListMaterial::CCmdAssignObjectListMaterial(vector<GObject*> o, int mat) : CCmdGroup("Assign material")
{
	for (int i = 0; i<(int)o.size(); ++i) AddCommand(new CCmdAssignObjectMaterial(o[i], mat));
}

//=============================================================================
// CCmdDeleteFESelection
//-----------------------------------------------------------------------------

CCmdDeleteFESelection::CCmdDeleteFESelection(GMeshObject* po) : CCommand("Delete")
{
	m_pnew = 0;
	m_pobj = po;
	m_pold = po->GetFEMesh();
	m_item = m_state.nitem;
}

void CCmdDeleteFESelection::Execute()
{
	// create a copy of the old mesh
	if (m_pnew == 0)
	{
		m_pnew = new FEMesh(*m_pold);

		if (m_item == ITEM_ELEM) m_pnew->DeleteSelectedElements();
		else if (m_item == ITEM_FACE) m_pnew->DeleteSelectedFaces();
		else if (m_item == ITEM_NODE) m_pnew->DeleteSelectedNodes();

		// make sure you select the object
		m_pobj->Select();
	}

	// set the object's mesh
	m_pobj->ReplaceFEMesh(m_pnew, false);

	// swap meshes
	FEMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
}

//-----------------------------------------------------------------------------
//! Undo the delete FE selection
//! \todo this function does not restore the original GMeshObject
void CCmdDeleteFESelection::UnExecute()
{
	// set the object's mesh
	m_pobj->ReplaceFEMesh(m_pnew);

	// swap meshes
	FEMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
}

//=============================================================================
// CCmdDeleteFESurfaceSelection
//-----------------------------------------------------------------------------

CCmdDeleteFESurfaceSelection::CCmdDeleteFESurfaceSelection(GSurfaceMeshObject* po) : CCommand("Delete")
{
	m_pnew = 0;
	m_pobj = po;
	m_pold = po->GetSurfaceMesh();
	m_item = m_state.nitem;
}

void CCmdDeleteFESurfaceSelection::Execute()
{
	// create a copy of the old mesh
	if (m_pnew == 0)
	{
		m_pnew = new FESurfaceMesh(*m_pold);

		if (m_item == ITEM_FACE) m_pnew->DeleteSelectedFaces();
		else if (m_item == ITEM_EDGE) m_pnew->DeleteSelectedEdges();
		else if (m_item == ITEM_NODE) m_pnew->DeleteSelectedNodes();

		// make sure you select the object
		m_pobj->Select();
	}

	// set the object's mesh
	m_pobj->ReplaceSurfaceMesh(m_pnew);

	// swap meshes
	FESurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
}

//-----------------------------------------------------------------------------
//! Undo the delete FE selection
//! \todo this function does not restore the original GMeshObject
void CCmdDeleteFESurfaceSelection::UnExecute()
{
	// set the object's mesh
	m_pobj->ReplaceSurfaceMesh(m_pnew);

	// swap meshes
	FESurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
}

//////////////////////////////////////////////////////////////////////
// CCmdHideObject
//////////////////////////////////////////////////////////////////////

CCmdHideObject::CCmdHideObject(GObject* po, bool bhide) : CCommand(bhide ? "Hide" : "Unhide")
{
	m_pobj.push_back(po);
	m_bhide = bhide;
}

CCmdHideObject::CCmdHideObject(vector<GObject*> po, bool bhide) : CCommand(bhide ? "Hide" : "Unhide")
{
	m_pobj = po;
	m_bhide = bhide;
}

void CCmdHideObject::Execute()
{
	for (GObject* po : m_pobj)
		if (m_bhide) po->Hide(); else po->Show();
}

void CCmdHideObject::UnExecute()
{
	for (GObject* po : m_pobj)
		if (m_bhide) po->Show(); else po->Hide();
}

//////////////////////////////////////////////////////////////////////
// CCmdHidePart
//////////////////////////////////////////////////////////////////////

CCmdHideParts::CCmdHideParts(std::list<GPart*> partList) : CCommand("Hide")
{
	m_partList = partList;
}

void CCmdHideParts::Execute()
{
	GModel& model = m_pDoc->GetFEModel()->GetModel();
	model.ShowParts(m_partList, false);
}

void CCmdHideParts::UnExecute()
{
	GModel& model = m_pDoc->GetFEModel()->GetModel();
	model.ShowParts(m_partList, true);
}

//////////////////////////////////////////////////////////////////////
// CCmdShowParts
//////////////////////////////////////////////////////////////////////

CCmdShowParts::CCmdShowParts(std::list<GPart*> partList) : CCommand("Show parts")
{
	m_partList = partList;
}

void CCmdShowParts::Execute()
{
	GModel& model = m_pDoc->GetFEModel()->GetModel();
	model.ShowParts(m_partList, true);
}

void CCmdShowParts::UnExecute()
{
	GModel& model = m_pDoc->GetFEModel()->GetModel();
	model.ShowParts(m_partList, false);
}

//////////////////////////////////////////////////////////////////////
// CCmdHideElements
//////////////////////////////////////////////////////////////////////

CCmdHideElements::CCmdHideElements(FEMesh* mesh, const vector<int>& elemList) : CCommand("Hide")
{
	m_mesh = mesh;
	m_elemList = elemList;
}

void CCmdHideElements::Execute()
{
	m_mesh->ShowElements(m_elemList, false);
}

void CCmdHideElements::UnExecute()
{
	m_mesh->ShowElements(m_elemList, true);
}

//////////////////////////////////////////////////////////////////////
// CCmdHideFaces
//////////////////////////////////////////////////////////////////////

CCmdHideFaces::CCmdHideFaces(FESurfaceMesh* mesh, const vector<int>& faceList) : CCommand("Hide")
{
	m_mesh = mesh;
	m_faceList = faceList;
}

void CCmdHideFaces::Execute()
{
	m_mesh->ShowFaces(m_faceList, false);
}

void CCmdHideFaces::UnExecute()
{
	m_mesh->ShowFaces(m_faceList, true);
}

//////////////////////////////////////////////////////////////////////
// CCmdHideSelection
//////////////////////////////////////////////////////////////////////

CCmdHideSelection::CCmdHideSelection() : CCommand("Hide")
{
	FESelection* ps = m_pDoc->GetCurrentSelection();
	assert(ps->Size());
	m_item.resize(ps->Size());
	int m = 0;

	// get the model
	GModel& model = m_pDoc->GetFEModel()->GetModel();
	GObject* po = m_pDoc->GetActiveObject();

	m_nitem = m_state.nitem;

	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_state.nselect == SELECT_OBJECT)
		{
			for (int i = 0; i<model.Objects(); ++i)
				if (model.Object(i)->IsSelected()) m_item[m++] = i;
		}
		else if (m_state.nselect == SELECT_PART)
		{
			GPartSelection* pgs = dynamic_cast<GPartSelection*>(ps);
			assert(pgs);
			GPartSelection::Iterator pg(pgs);
			for (int i = 0; i<pgs->Size(); ++i, ++pg)
			{
				if (pg->IsSelected()) m_item[m++] = pg->GetID();
			}
		}
		break;
	case ITEM_ELEM:
	{
		FEMesh* mesh = (po ? po->GetFEMesh() : 0);
		if (mesh)
		{
			for (int i = 0; i<mesh->Elements(); ++i)
			{
				FEElement& el = mesh->Element(i);
				if (el.IsSelected()) m_item[m++] = i;
			}
		}
	}
	break;
	case ITEM_FACE:
	{
		FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
		if (pm)
		{
			for (int i = 0; i<pm->Faces(); ++i)
			{
				FEFace& face = pm->Face(i);
				if (face.IsSelected()) m_item[m++] = i;
			}
		}
	}
	break;
	}
}

void CCmdHideSelection::Execute()
{
	// get the model
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	GObject* po = m_pDoc->GetActiveObject();
	m_pDoc->SetItemMode(m_nitem);
	int N = m_item.size();
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_state.nselect == SELECT_OBJECT)
			m.ShowObjects(m_item, false);
		else if (m_state.nselect == SELECT_PART)
			m.ShowParts(m_item, false);
		break;
	case ITEM_ELEM:
	{
		FEMesh* pm = po->GetFEMesh();
		if (pm) pm->ShowElements(m_item, false);
	}
	break;
	case ITEM_FACE:
	{
		FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
		if (pm) pm->ShowFaces(m_item, false);
	}
	break;
	}
}

void CCmdHideSelection::UnExecute()
{
	GObject* po = m_pDoc->GetActiveObject();
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m_pDoc->SetItemMode(m_nitem);
	int N = m_item.size();
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_state.nselect == SELECT_OBJECT)
		{
			m.ShowObjects(m_item);
			m.SelectObjects(m_item);
		}
		else if (m_state.nselect == SELECT_PART)
		{
			m.ShowParts(m_item, true, true);
		}
		break;
	case ITEM_ELEM:
	{
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			pm->ShowElements(m_item);
			pm->SelectElements(m_item);
		}
	}
	break;
	case ITEM_FACE:
	{
		FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
		if (pm)
		{
			pm->ShowFaces(m_item);
			pm->SelectFaces(m_item);
		}
	}
	break;
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdHideUnselected
//////////////////////////////////////////////////////////////////////

CCmdHideUnselected::CCmdHideUnselected() : CCommand("Hide")
{
	FESelection* ps = m_pDoc->GetCurrentSelection();
	assert(ps->Size());
	m_item.resize(ps->Size());
	int m = 0;

	GObject* po = m_pDoc->GetActiveObject();
	GModel& model = m_pDoc->GetFEModel()->GetModel();
	m_nitem = m_state.nitem;

	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_state.nselect == SELECT_OBJECT)
		{
			for (int i = 0; i<model.Objects(); ++i)
			{
				po = model.Object(i);
				if (!po->IsSelected()) m_item[m++] = i;
			}
		}
		else if (m_state.nselect == SELECT_PART)
		{
			GPartSelection* pgs = dynamic_cast<GPartSelection*>(ps);
			assert(pgs);
			GPartSelection::Iterator pg(pgs);
			for (int i = 0; i<pgs->Size(); ++i, ++pg)
			{
				if (pg->IsSelected()) m_item[m++] = pg->GetID();
			}
		}
		break;
	case ITEM_ELEM:
	{
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			for (int i = 0; i<pm->Elements(); ++i) if (!pm->Element(i).IsSelected()) m_item[m++] = i;
		}
	}
	break;
	case ITEM_FACE:
	{
		FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
		if (pm)
		{
			for (int i = 0; i<pm->Faces(); ++i) if (!pm->Face(i).IsSelected()) m_item[m++] = i;
		}
	}
	break;
	}
}

void CCmdHideUnselected::Execute()
{
	GObject* po = m_pDoc->GetActiveObject();
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m_pDoc->SetItemMode(m_nitem);
	int N = m_item.size();
	if (N == 0) return;
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_state.nselect == SELECT_OBJECT)
			m.ShowObjects(m_item, false);
		else if (m_state.nselect == SELECT_PART)
			m.ShowParts(m_item, false);
		break;
	case ITEM_ELEM:
	{
		FEMesh* pm = po->GetFEMesh();
		if (pm) pm->ShowElements(m_item, false);
	}
	break;
	case ITEM_FACE:
	{
		FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
		if (pm) pm->ShowFaces(m_item, false);
	}
	break;
	}
}

void CCmdHideUnselected::UnExecute()
{
	GObject* po = m_pDoc->GetActiveObject();
	GModel& m = m_pDoc->GetFEModel()->GetModel();
	m_pDoc->SetItemMode(m_nitem);
	int N = m_item.size();
	if (N == 0) return;
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_state.nselect == SELECT_OBJECT)
			m.ShowObjects(m_item, true);
		else if (m_state.nselect == SELECT_PART)
			m.ShowParts(m_item, true);
		break;
	case ITEM_ELEM:
	{
		FEMesh* pm = po->GetFEMesh();
		if (pm) pm->ShowElements(m_item);
	}
	break;
	case ITEM_FACE:
	{
		FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
		if (pm) pm->ShowFaces(m_item);
	}
	break;
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnhideAll
//////////////////////////////////////////////////////////////////////

CCmdUnhideAll::CCmdUnhideAll() : CCommand("Unhide all")
{
	m_nitem = m_state.nitem;
	GModel& model = m_pDoc->GetFEModel()->GetModel();
	m_bunhide = true;

	if (m_nitem == ITEM_MESH)
	{
		switch (m_state.nselect)
		{
		case SELECT_OBJECT:
		{
			for (int i = 0; i<model.Objects(); ++i)
			{
				GObject* po = model.Object(i);
				if (!po->IsVisible()) m_item.push_back(i);
			}
		}
		break;
		case SELECT_PART:
		case SELECT_FACE:
		case SELECT_EDGE:
		case SELECT_NODE:
		{
			for (int i = 0; i<model.Parts(); ++i)
			{
				GPart* pp = model.Part(i);
				if (!pp->IsVisible()) m_item.push_back(pp->GetID());
			}
		}
		break;
		case SELECT_DISCRETE:
		{
			for (int i = 0; i<model.DiscreteObjects(); ++i)
			{
				GDiscreteObject* pd = model.DiscreteObject(i);
				if (!pd->IsVisible()) m_item.push_back(i);
			}
		}
		break;
		}
	}
	else
	{
		GObject* po = m_pDoc->GetActiveObject();
		if (po == 0) return;
		switch (m_nitem)
		{
		case ITEM_ELEM:
		{
			FEMesh* pm = po->GetFEMesh();
			if (pm)
			{
				for (int i = 0; i<pm->Elements(); ++i) if (!pm->Element(i).IsVisible()) m_item.push_back(i);
			}
		}
		break;
		case ITEM_FACE:
		{
			FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
			if (pm)
			{
				for (int i = 0; i<pm->Faces(); ++i) if (!pm->Face(i).IsVisible()) m_item.push_back(i);
			}
		}
		break;
		}
	}
}

void CCmdUnhideAll::Execute()
{
	m_pDoc->SetItemMode(m_nitem);
	if (m_item.empty()) return;

	GModel& model = m_pDoc->GetFEModel()->GetModel();
	int N = m_item.size();
	if (m_nitem == ITEM_MESH)
	{
		switch (m_state.nselect)
		{
		case SELECT_OBJECT:
		{
			model.ShowObjects(m_item, m_bunhide);
		}
		break;
		case SELECT_PART:
		case SELECT_FACE:
		case SELECT_EDGE:
		case SELECT_NODE:
		{
			model.ShowParts(m_item, m_bunhide);
		}
		break;
		case SELECT_DISCRETE:
		{
			for (int i = 0; i<N; ++i)
			{
				GDiscreteObject* pd = model.DiscreteObject(m_item[i]);
				if (m_bunhide) pd->Show(); else pd->Hide();
			}
		}
		break;
		}
	}
	else
	{
		GObject* po = m_pDoc->GetActiveObject();
		switch (m_nitem)
		{
		case ITEM_ELEM:
		{
			FEMesh* pm = po->GetFEMesh();
			assert(pm);
			pm->ShowElements(m_item, m_bunhide);
		}
		break;
		case ITEM_FACE:
		{
			FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
			assert(pm);
			if (pm) pm->ShowFaces(m_item, m_bunhide);
		}
		break;
		}
	}

	m_bunhide = false;
}

void CCmdUnhideAll::UnExecute()
{
	Execute();
	m_bunhide = true;
}

//=============================================================================
// CCmdApplyFEModifier
//-----------------------------------------------------------------------------

CCmdApplyFEModifier::CCmdApplyFEModifier(FEModifier* pmod, GObject* po, FEGroup* selection) : CCommand(pmod->GetName())
{
	m_pnew = 0;

	m_pobj = po;
	m_psel = selection;

	// store the modifier
	m_pmod = pmod;

	// store the old mesh
	m_pold = po->GetFEMesh();
}

void CCmdApplyFEModifier::Execute()
{
	if (m_pnew == 0)
	{
		// create a new mesh
		if (m_psel)
			m_pnew = m_pmod->Apply(m_psel);
		else
			m_pnew = m_pmod->Apply(m_pold);

		// let's make sure the command worked
		if (m_pnew == 0) throw CCmdFailed(this, m_pmod->GetErrorString());

		// make sure the new mesh is selected
		if (m_pobj) m_pobj->Select();
	}

	if (m_pnew)
	{
		// replace the old mesh with the new
		try
		{
			// This could throw a GObjecException
			m_pobj->ReplaceFEMesh(m_pnew);
		}
		catch (...)
		{
			// swap old and new
			// we do this so that we can always delete m_pnew
			FEMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;

			throw;
		}

		// swap old and new
		// we do this so that we can always delete m_pnew
		FEMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}

void CCmdApplyFEModifier::UnExecute()
{
	// get the FEModel
	if (m_pnew)
	{
		// replace the old mesh with the new
		m_pobj->ReplaceFEMesh(m_pnew);

		// swap old and new
		// we do this so that we can always delete m_pnew
		FEMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}


//=============================================================================
// CCmdApplySurfaceModifier
//-----------------------------------------------------------------------------

CCmdApplySurfaceModifier::CCmdApplySurfaceModifier(FESurfaceModifier* pmod, GObject* po, FEGroup* selection) : CCommand(pmod->GetName())
{
	m_pnew = 0;

	m_pobj = po;
	m_psel = selection;

	// store the modifier
	m_pmod = pmod;

	// store the old mesh
	if (dynamic_cast<GSurfaceMeshObject*>(po))
	{
		m_pold = dynamic_cast<GSurfaceMeshObject*>(po)->GetSurfaceMesh();
	}
	else m_pold = 0;
}

CCmdApplySurfaceModifier::~CCmdApplySurfaceModifier()
{
	delete m_pnew;
	delete m_pmod;
}

void CCmdApplySurfaceModifier::Execute()
{
	if (m_pnew == 0)
	{
		// create a new mesh
		m_pnew = m_pmod->Apply(m_pold, m_psel);

		// let's make sure the command worked
		if (m_pnew == 0) throw CCmdFailed(this, m_pmod->GetErrorString());

		// make sure the new mesh is selected
		if (m_pobj) m_pobj->Select();
	}

	if (m_pnew)
	{
		// replace the old mesh with the new
		try
		{
			// This could throw a GObjecException
			m_pobj->ReplaceSurfaceMesh(m_pnew);
		}
		catch (...)
		{
			// swap old and new
			// we do this so that we can always delete m_pnew
			FESurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;

			throw;
		}

		// swap old and new
		// we do this so that we can always delete m_pnew
		FESurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}

void CCmdApplySurfaceModifier::UnExecute()
{
	// get the FEModel
	if (m_pnew)
	{
		// replace the old mesh with the new
		m_pobj->ReplaceSurfaceMesh(dynamic_cast<FESurfaceMesh*>(m_pnew));

		// swap old and new
		// we do this so that we can always delete m_pnew
		FESurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}

//=============================================================================
// CCmdChangeFEMesh
//-----------------------------------------------------------------------------

CCmdChangeFEMesh::CCmdChangeFEMesh(GObject* po, FEMesh* pm, bool bup) : CCommand("Change mesh")
{
	assert(po);
	m_update = bup;
	m_po = po;
	m_pnew = pm;
}

void CCmdChangeFEMesh::Execute()
{
	FEMesh* pm = m_po->GetFEMesh();
	m_po->ReplaceFEMesh(m_pnew, m_update);

	m_pnew = pm;
}

void CCmdChangeFEMesh::UnExecute()
{
	Execute();
}

//=============================================================================
// CCmdChangeFESurfaceMesh
//-----------------------------------------------------------------------------

CCmdChangeFESurfaceMesh::CCmdChangeFESurfaceMesh(GSurfaceMeshObject* po, FESurfaceMesh* pm, bool up) : CCommand("Change surface mesh")
{
	assert(po);
	assert(pm);

	m_update = up;
	m_po = po;
	m_pnew = pm;
}

CCmdChangeFESurfaceMesh::~CCmdChangeFESurfaceMesh()
{
	delete m_pnew;
}

void CCmdChangeFESurfaceMesh::Execute()
{
	FESurfaceMesh* pm = m_po->GetSurfaceMesh();
	m_po->ReplaceSurfaceMesh(m_pnew);
	m_pnew = pm;
}

void CCmdChangeFESurfaceMesh::UnExecute()
{
	Execute();
}


///////////////////////////////////////////////////////////////////////////////
// CCmdChangeView
///////////////////////////////////////////////////////////////////////////////

CCmdChangeView::CCmdChangeView(CGLView* pview, CGLCamera cam) : CCommand("Change View")
{
	m_cam = cam;
	m_pview = pview;
}

CCmdChangeView::~CCmdChangeView()
{
	m_pview = 0;
}

void CCmdChangeView::Execute()
{
	CGLCamera& cam = m_pview->GetCamera();
	CGLCamera old = cam;
	cam = m_cam;
	m_cam = old;
}

void CCmdChangeView::UnExecute()
{
	Execute();
}

///////////////////////////////////////////////////////////////////////////////
// CCmdInvertElements
///////////////////////////////////////////////////////////////////////////////

CCmdInvertElements::CCmdInvertElements(GMeshObject* po) : CCommand("Invert")
{
	m_po = po;
}

void CCmdInvertElements::Execute()
{
	FEMesh* pm = m_po->GetFEMesh();
	pm->InvertSelectedElements();
	m_po->Update(false);
}

void CCmdInvertElements::UnExecute()
{
	Execute();
}

///////////////////////////////////////////////////////////////////////////////
// CCmdChangeObjectParams
///////////////////////////////////////////////////////////////////////////////

CCmdChangeObjectParams::CCmdChangeObjectParams(GObject *po) : CCommand("Change parameters")
{
	m_po = po;
	m_Param = m_po->GetParamBlock();
}

void CCmdChangeObjectParams::Execute()
{
	ParamBlock pb = m_po->GetParamBlock();
	m_po->GetParamBlock() = m_Param;
	m_po->Update();
	m_Param = pb;
}

void CCmdChangeObjectParams::UnExecute()
{
	ParamBlock pb = m_po->GetParamBlock();
	m_po->GetParamBlock() = m_Param;
	m_po->Update();
	m_Param = pb;
}

///////////////////////////////////////////////////////////////////////////////
// CCmdChangeMesherParams
///////////////////////////////////////////////////////////////////////////////

CCmdChangeMesherParams::CCmdChangeMesherParams(GObject *po) : CCommand("Change meshing parameters")
{
	m_po = po;
	m_Param = m_po->GetFEMesher()->GetParamBlock();
}

void CCmdChangeMesherParams::Execute()
{
	ParamBlock pb = m_po->GetFEMesher()->GetParamBlock();
	m_po->GetFEMesher()->GetParamBlock() = m_Param;
	m_po->BuildMesh();
	m_Param = pb;
}

void CCmdChangeMesherParams::UnExecute()
{
	ParamBlock pb = m_po->GetFEMesher()->GetParamBlock();
	m_po->GetFEMesher()->GetParamBlock() = m_Param;
	m_po->BuildMesh();
	m_Param = pb;
}

//-----------------------------------------------------------------------------
// CCmdConvertToEditableMesh
//-----------------------------------------------------------------------------

CCmdConvertToEditableMesh::CCmdConvertToEditableMesh(GObject* po) : CCommand("Convert")
{
	m_pold = po;
	m_pnew = nullptr;
	m_oml = nullptr;
}

CCmdConvertToEditableMesh::~CCmdConvertToEditableMesh()
{
	if (m_oml)
	{
		MeshLayerManager::Destroy(m_oml);
		delete m_pold;
	}
	else delete m_pnew;
}

void CCmdConvertToEditableMesh::Execute()
{
	// get the model
	FEModel* ps = m_pDoc->GetFEModel();
	GModel& m = ps->GetModel();

	if (m_pnew == 0)
	{
		// make sure the old object has a mesh
		if (m_pold->GetFEMesh() == 0)
		{
			assert(false);
			return;
		}

		// create a new gmeshobject
		m_pnew = new GMeshObject(m_pold);
	}

	// get the old object's meshlist
	m_oml = m.GetObjectMeshList(m_pold);

	// replace the old object with the new one
	m.ReplaceObject(m_pold, m_pnew);
}

void CCmdConvertToEditableMesh::UnExecute()
{
	// get the model
	FEModel* ps = m_pDoc->GetFEModel();
	GModel& m = ps->GetModel();

	// remove the new object
	m.RemoveObject(m_pnew, true);

	// add the oml back
	m.InsertObjectMeshList(m_oml);
	m_oml = nullptr;
}

//-----------------------------------------------------------------------------
// CCmdConvertSurfaceToEditableMesh
//-----------------------------------------------------------------------------

CCmdConvertSurfaceToEditableMesh::CCmdConvertSurfaceToEditableMesh(GObject* po) : CCommand("Convert")
{
	m_pold = po;
	m_pnew = 0;
}

CCmdConvertSurfaceToEditableMesh::~CCmdConvertSurfaceToEditableMesh()
{
	if (m_pnew) delete m_pnew;
}

void CCmdConvertSurfaceToEditableMesh::Execute()
{
	// get the model
	FEModel* ps = m_pDoc->GetFEModel();
	GModel& m = ps->GetModel();

	if (m_pnew == 0)
	{
		// get the surface
		FESurfaceMesh* surfaceMesh = dynamic_cast<GSurfaceMeshObject*>(m_pold)->GetSurfaceMesh();

		// create a new gmeshobject
		m_pnew = new GMeshObject(surfaceMesh);
		m_pnew->SetName(m_pold->GetName());

		// copy data
		m_pnew->CopyTransform(m_pold);
		m_pnew->SetColor(m_pold->GetColor());

		// copy the selection state
		if (m_pold->IsSelected()) m_pnew->Select();
	}

	// replace the old object with the new one
	m.ReplaceObject(m_pold, m_pnew);

	// swap old and new
	GObject* po = m_pold;
	m_pold = m_pnew;
	m_pnew = po;
}

void CCmdConvertSurfaceToEditableMesh::UnExecute()
{
	Execute();
}



//-----------------------------------------------------------------------------
// CCmdConvertToEditableSurface
//-----------------------------------------------------------------------------

CCmdConvertToEditableSurface::CCmdConvertToEditableSurface(GObject* po) : CCommand("Convert")
{
	m_pold = po;
	m_pnew = 0;
}

CCmdConvertToEditableSurface::~CCmdConvertToEditableSurface()
{
	if (m_pnew) delete m_pnew;
}

void CCmdConvertToEditableSurface::Execute()
{
	// get the model
	FEModel* ps = m_pDoc->GetFEModel();
	GModel& m = ps->GetModel();

	if (m_pnew == 0)
	{
		// make sure the old object has a mesh
		if (m_pold->GetFEMesh() == 0)
		{
			return;
		}

		// create a new surface mesh object
		m_pnew = new GSurfaceMeshObject(m_pold);
		m_pnew->SetName(m_pold->GetName());

		// copy data
		m_pnew->CopyTransform(m_pold);
		m_pnew->SetColor(m_pold->GetColor());

		// copy the selection state
		if (m_pold->IsSelected()) m_pnew->Select();
	}

	// replace the old object with the new one
	m.ReplaceObject(m_pold, m_pnew);

	// swap old and new
	GObject* po = m_pold;
	m_pold = m_pnew;
	m_pnew = po;
}

void CCmdConvertToEditableSurface::UnExecute()
{
	Execute();
}

//-----------------------------------------------------------------------------
// CCmdConvertToMultiBlock
//-----------------------------------------------------------------------------

CCmdConvertToMultiBlock::CCmdConvertToMultiBlock(GObject* po) : CCommand("Convert")
{
	m_pold = po;
	m_pnew = 0;
}

CCmdConvertToMultiBlock::~CCmdConvertToMultiBlock()
{
	if (m_pnew) delete m_pnew;
}

void CCmdConvertToMultiBlock::Execute()
{
	// get the model
	FEModel* ps = m_pDoc->GetFEModel();
	GModel& m = ps->GetModel();

	if (m_pnew == 0)
	{
		// make sure we are dealing with a GBox object
		if (dynamic_cast<GBox*>(m_pold) == 0)
		{
			return;
		}

		// create a new gmeshobject
		m_pnew = new GMultiBox(m_pold);
		m_pnew->SetName(m_pold->GetName());

		// copy data
		m_pnew->CopyTransform(m_pold);
		m_pnew->SetColor(m_pold->GetColor());

		// copy the selection state
		if (m_pold->IsSelected()) m_pnew->Select();
	}

	// replace the old object with the new one
	m.ReplaceObject(m_pold, m_pnew);

	// swap old and new
	GObject* po = m_pold;
	m_pold = m_pnew;
	m_pnew = po;
}

void CCmdConvertToMultiBlock::UnExecute()
{
	Execute();
}

//-----------------------------------------------------------------------------
// CCmdAddModifier
//-----------------------------------------------------------------------------

CCmdAddModifier::CCmdAddModifier(GModifiedObject* po, GModifier* pm) : CCommand("Add modifier")
{
	m_po = po;
	m_pm = pm;
}

CCmdAddModifier::~CCmdAddModifier()
{
	if (m_pm) delete m_pm;
}

void CCmdAddModifier::Execute()
{
	// add the modifier to the end of the stack
	assert(m_pm);
	m_po->AddModifier(m_pm);
	m_po->BuildMesh();
	m_pm = 0;
}

void CCmdAddModifier::UnExecute()
{
	// remove the last modifier
	GModifierStack* ps = m_po->GetModifierStack();
	int N = ps->Size();
	assert(N > 0);
	m_pm = ps->Modifier(N - 1);
	m_po->DeleteModifier(m_pm);

	// if there are still some modifiers left, we have to rebuild the mesh
	// otherwise, the original mesh was restored after the removal of the 
	// last modifier
	if (N>1) m_po->BuildMesh();
}

//-----------------------------------------------------------------------------
// CCmdAddStep
//-----------------------------------------------------------------------------

CCmdAddStep::CCmdAddStep(FEStep* pstep) : CCommand("Add step")
{
	m_pstep = pstep;
}

CCmdAddStep::~CCmdAddStep()
{
	if (m_pstep) delete m_pstep;
}

void CCmdAddStep::Execute()
{
	FEModel* ps = m_pDoc->GetFEModel();
	ps->AddStep(m_pstep);
	m_pstep = 0;
}

void CCmdAddStep::UnExecute()
{
	// remove the last step
	FEModel* ps = m_pDoc->GetFEModel();
	int N = ps->Steps();
	assert(N>0);
	m_pstep = ps->GetStep(N - 1);
	ps->DeleteStep(m_pstep);
}

//-----------------------------------------------------------------------------
// CCmdAddMaterial
//-----------------------------------------------------------------------------

CCmdAddMaterial::CCmdAddMaterial(GMaterial* pm) : CCommand("Add material")
{
	m_pm = pm;
}

CCmdAddMaterial::~CCmdAddMaterial()
{
	if (m_pm) delete m_pm;
}

void CCmdAddMaterial::Execute()
{
	FEModel* ps = m_pDoc->GetFEModel();
	ps->AddMaterial(m_pm);
	m_pm = 0;
}

void CCmdAddMaterial::UnExecute()
{
	// remove the last material
	FEModel* ps = m_pDoc->GetFEModel();
	int N = ps->Materials();
	assert(N>0);
	m_pm = ps->GetMaterial(N - 1);
	ps->DeleteMaterial(m_pm);
}

//-----------------------------------------------------------------------------
// CCmdSetModelComponentItemList
//-----------------------------------------------------------------------------

CCmdSetModelComponentItemList::CCmdSetModelComponentItemList(FEModelComponent* pbc, FEItemListBuilder* pl) : CCommand("Assign BC")
{
	m_pbc = pbc;
	m_pl = pl;
}

CCmdSetModelComponentItemList::~CCmdSetModelComponentItemList()
{
	if (m_pl) delete m_pl;
}

void CCmdSetModelComponentItemList::Execute()
{
	FEItemListBuilder* pold = m_pbc->GetItemList();
	m_pbc->SetItemList(m_pl);
	m_pl = pold;
}

void CCmdSetModelComponentItemList::UnExecute()
{
	Execute();
}

//-----------------------------------------------------------------------------
// CCmdUnassignBC
//-----------------------------------------------------------------------------

CCmdUnassignBC::CCmdUnassignBC(FEBoundaryCondition* pbc) : CCommand("Unassign BC")
{
	m_pbc = pbc;
	m_pl = 0;
}

CCmdUnassignBC::~CCmdUnassignBC()
{
	if (m_pl) delete m_pl;
}

void CCmdUnassignBC::Execute()
{
	FEItemListBuilder* pold = m_pbc->GetItemList();
	m_pbc->SetItemList(m_pl);
	m_pl = pold;
}

void CCmdUnassignBC::UnExecute()
{
	Execute();
}

//-----------------------------------------------------------------------------
// CCmdAddToItemListBuilder
//-----------------------------------------------------------------------------

CCmdAddToItemListBuilder::CCmdAddToItemListBuilder(FEItemListBuilder* pold, list<int>& lnew) : CCommand("Add to selection")
{
	m_pold = pold;
	m_lnew = lnew;
	int n = m_pold->size();
	if (n > 0)
	{
		m_tmp.resize(n);
		FEItemListBuilder::Iterator it = m_pold->begin();
		for (int i = 0; i<n; ++i, ++it) m_tmp[i] = *it;
	}
}

void CCmdAddToItemListBuilder::Execute()
{
	m_pold->Merge(m_lnew);
}

void CCmdAddToItemListBuilder::UnExecute()
{
	m_pold->clear();
	int n = (int)m_tmp.size();
	for (int i = 0; i<n; ++i) m_pold->add(m_tmp[i]);
}

//-----------------------------------------------------------------------------
// CCmdRemoveFromItemListBuilder
//-----------------------------------------------------------------------------

CCmdRemoveFromItemListBuilder::CCmdRemoveFromItemListBuilder(FEItemListBuilder* pold, list<int>& lnew) : CCommand("Remove from selection")
{
	m_pold = pold;
	m_lnew = lnew;
	int n = m_pold->size();
	if (n > 0)
	{
		m_tmp.resize(n);
		FEItemListBuilder::Iterator it = m_pold->begin();
		for (int i = 0; i<n; ++i, ++it) m_tmp[i] = *it;
	}
}

void CCmdRemoveFromItemListBuilder::Execute()
{
	m_pold->Subtract(m_lnew);
}

void CCmdRemoveFromItemListBuilder::UnExecute()
{
	m_pold->clear();
	int n = (int)m_tmp.size();
	for (int i = 0; i<n; ++i) m_pold->add(m_tmp[i]);
}

//-----------------------------------------------------------------------------
// CCmdDeleteGObject
//-----------------------------------------------------------------------------

CCmdDeleteGObject::CCmdDeleteGObject(GModel* gm, GObject* po) : CCommand(string("Delete ") + po->GetName())
{
	m_gm = gm;
	m_po = po;
	m_poml = nullptr;
}

CCmdDeleteGObject::~CCmdDeleteGObject()
{
	if (m_poml) 
	{
		MeshLayerManager::Destroy(m_poml);
		delete m_po;
	}
}

void CCmdDeleteGObject::Execute()
{
	m_poml = m_gm->GetObjectMeshList(m_po);
	m_gm->RemoveObject(m_po);
}

void CCmdDeleteGObject::UnExecute()
{
	// re-insert the object and its mesh list
	m_gm->InsertObjectMeshList(m_poml);

	// delete the OML
	m_poml = nullptr;
}

//-----------------------------------------------------------------------------
// CCmdDeleteFSObject
//-----------------------------------------------------------------------------

CCmdDeleteFSObject::CCmdDeleteFSObject(FSObject* po) : CCommand(string("Delete ") + po->GetName())
{
	assert(po->GetParent());
	m_obj = po;
	m_parent = po->GetParent();
	m_delObject = false;
}

CCmdDeleteFSObject::~CCmdDeleteFSObject()
{
	if (m_delObject) delete m_obj;
}

void CCmdDeleteFSObject::Execute()
{
	m_insertPos = m_parent->RemoveChild(m_obj);
	m_obj->SetParent(nullptr);
	m_delObject = true;
}

void CCmdDeleteFSObject::UnExecute()
{
	m_parent->InsertChild(m_insertPos, m_obj);
	assert(m_obj->GetParent() == m_parent);
	m_delObject = false;
}

//-----------------------------------------------------------------------------
// CCmdSetActiveMeshLayer
//-----------------------------------------------------------------------------

CCmdSetActiveMeshLayer::CCmdSetActiveMeshLayer(GModel* mdl, int activeLayer) : CCommand("Change active layer")
{
	m_gm = mdl;
	m_activeLayer = activeLayer;
}

void CCmdSetActiveMeshLayer::Execute()
{
	int currentLayer = m_gm->GetActiveMeshLayer();
	m_gm->SetActiveMeshLayer(m_activeLayer);
	m_activeLayer = currentLayer;
}

void CCmdSetActiveMeshLayer::UnExecute()
{
	Execute();
}

//-----------------------------------------------------------------------------
// CCmdAddMeshLayer
//-----------------------------------------------------------------------------

CCmdAddMeshLayer::CCmdAddMeshLayer(GModel* mdl, const std::string& layerName) : CCommand( string("Add Mesh Layer: ") + layerName)
{
	m_gm = mdl;
	m_layerName = layerName;
	m_layerIndex = -1;
}

void CCmdAddMeshLayer::Execute()
{
	m_layerIndex = m_gm->MeshLayers();
	m_gm->AddMeshLayer(m_layerName);
}

void CCmdAddMeshLayer::UnExecute()
{
	m_gm->DeleteMeshLayer(m_layerIndex);
}

//-----------------------------------------------------------------------------
// CCmdDeleteMeshLayer
//-----------------------------------------------------------------------------


CCmdDeleteMeshLayer::CCmdDeleteMeshLayer(GModel* mdl, int layerIndex) : CCommand("Delete mesh layer")
{
	m_gm = mdl;
	m_layerIndex = layerIndex;
	m_layer = nullptr;
	assert(layerIndex != 0);		// make sure we are not trying to delete the default layer
}

CCmdDeleteMeshLayer::~CCmdDeleteMeshLayer()
{
	if (m_layer) MeshLayerManager::Destroy(m_layer);
}

void CCmdDeleteMeshLayer::Execute()
{
	m_layer = m_gm->RemoveMeshLayer(m_layerIndex);
}

void CCmdDeleteMeshLayer::UnExecute()
{
	m_gm->InsertMeshLayer(m_layerIndex, m_layer);
	m_layer = nullptr;
}

//-----------------------------------------------------------------------------
// CCmdRemoveMeshData
//-----------------------------------------------------------------------------

CCmdRemoveMeshData::CCmdRemoveMeshData(FEMeshData* meshData) : CCommand("Remove mesh data")
{
	m_data = meshData;
	m_index = -1;
}

CCmdRemoveMeshData::~CCmdRemoveMeshData()
{
	if (m_data && (m_index != -1)) delete m_data;
}

void CCmdRemoveMeshData::Execute()
{
	FEMesh* mesh = m_data->GetMesh();
	m_index = mesh->GetMeshDataIndex(m_data); assert(m_index >= 0);
	mesh->RemoveMeshDataField(m_index);
}

void CCmdRemoveMeshData::UnExecute()
{
	FEMesh* mesh = m_data->GetMesh();
	mesh->InsertMeshData(m_index, m_data);
	m_index = -1;
}
