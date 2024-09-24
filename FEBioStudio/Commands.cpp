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

#include "stdafx.h"
#include "Commands.h"
#include "ModelDocument.h"
#include "GLView.h"
#include <GeomLib/GObject.h>
#include <MeshTools/FEMesher.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMultiBox.h>
#include <GeomLib/GModel.h>
#include <GeomLib/MeshLayer.h>
#include <MeshLib/FEMeshBuilder.h>
#include <ImageLib/ImageAnalysis.h>

//////////////////////////////////////////////////////////////////////
// CCmdAddObject
//////////////////////////////////////////////////////////////////////

CCmdAddObject::CCmdAddObject(GModel* model, GObject* po) : CCommand("Add object") 
{ 
	m_model = model;
	m_pobj = po; 
	m_bdel = true; 
}

CCmdAddObject::~CCmdAddObject() 
{ 
	if (m_bdel) delete m_pobj; 
}

void CCmdAddObject::Execute()
{
	// add the mesh to the model
	m_model->AddObject(m_pobj);

	m_bdel = false;
}

void CCmdAddObject::UnExecute()
{
	// remove the mesh from the model
	m_model->RemoveObject(m_pobj, true);

	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddDiscreteObject
//////////////////////////////////////////////////////////////////////

CCmdAddDiscreteObject::CCmdAddDiscreteObject(GModel* model, GDiscreteObject* po) : CCommand("Add discrete object") 
{ 
	m_model = model;
	m_pobj = po; 
	m_bdel = true; 
}

CCmdAddDiscreteObject::~CCmdAddDiscreteObject() 
{ 
	if (m_bdel) delete m_pobj; 
}

void CCmdAddDiscreteObject::Execute()
{
	// add the mesh to the model
	m_model->AddDiscreteObject(m_pobj);
	m_bdel = false;
}

void CCmdAddDiscreteObject::UnExecute()
{
	// remove the mesh from the model
	m_model->RemoveDiscreteObject(m_pobj);
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
// CCmdAddConstraint
//////////////////////////////////////////////////////////////////////

CCmdAddConstraint::CCmdAddConstraint(FSStep* ps, FSModelConstraint* pmc) : CCommand("Add constraint") 
{ 
	m_ps = ps; 
	m_pmc = pmc;
	m_bdel = true; 
}

CCmdAddConstraint::~CCmdAddConstraint() 
{ 
	if (m_bdel) delete m_pmc; 
}

void CCmdAddConstraint::Execute()
{
	// add the connector to the step
	m_ps->AddConstraint(m_pmc);
	m_bdel = false;
}

void CCmdAddConstraint::UnExecute()
{
	// remove the connector from the step
	m_ps->RemoveConstraint(m_pmc);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddPart
//////////////////////////////////////////////////////////////////////

void CCmdAddPart::Execute()
{
	// add the group to the mesh
	m_po->AddFEElemSet(m_pg);
	m_bdel = false;
}

void CCmdAddPart::UnExecute()
{
	// remove the mesh from the model
	m_po->RemoveFEElemSet(m_pg);
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

CCmdAddGPartGroup::CCmdAddGPartGroup(GModel* model, GPartList* pg) : CCommand("Add Part")
{ 
	m_model = model;
	m_pg = pg; 
	m_bdel = true; 
}

CCmdAddGPartGroup::~CCmdAddGPartGroup()
{ 
	if (m_bdel) delete m_pg; 
}

void CCmdAddGPartGroup::Execute()
{
	m_model->AddPartList(m_pg);
	m_bdel = false;
}

void CCmdAddGPartGroup::UnExecute()
{
	m_model->RemovePartList(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddGFaceGroup
//////////////////////////////////////////////////////////////////////

CCmdAddGFaceGroup::CCmdAddGFaceGroup(GModel* model, GFaceList* pg) : CCommand("Add Surface")
{ 
	m_model = model;
	m_pg = pg; 
	m_bdel = true; 
}

CCmdAddGFaceGroup::~CCmdAddGFaceGroup() 
{ 
	if (m_bdel) delete m_pg; 
}

void CCmdAddGFaceGroup::Execute()
{
	m_model->AddFaceList(m_pg);
	m_bdel = false;
}

void CCmdAddGFaceGroup::UnExecute()
{
	m_model->RemoveFaceList(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddGEdgeGroup
//////////////////////////////////////////////////////////////////////

CCmdAddGEdgeGroup::CCmdAddGEdgeGroup(GModel* model, GEdgeList* pg) : CCommand("Add Edge")
{ 
	m_model = model;
	m_pg = pg; 
	m_bdel = true; 
}

CCmdAddGEdgeGroup::~CCmdAddGEdgeGroup()
{ 
	if (m_bdel) delete m_pg; 
}

void CCmdAddGEdgeGroup::Execute()
{
	m_model->AddEdgeList(m_pg);
	m_bdel = false;
}

void CCmdAddGEdgeGroup::UnExecute()
{
	m_model->RemoveEdgeList(m_pg);
	m_bdel = true;
}

//////////////////////////////////////////////////////////////////////
// CCmdAddGNodeGroup
//////////////////////////////////////////////////////////////////////

CCmdAddGNodeGroup::CCmdAddGNodeGroup(GModel* model, GNodeList* pg) : CCommand("Add Nodeset")
{ 
	m_model = model;
	m_pg = pg; m_bdel = true; 
}

CCmdAddGNodeGroup::~CCmdAddGNodeGroup()
{ 
	if (m_bdel) delete m_pg; 
}

void CCmdAddGNodeGroup::Execute()
{
	m_model->AddNodeList(m_pg);
	m_bdel = false;
}

void CCmdAddGNodeGroup::UnExecute()
{
	m_model->RemoveNodeList(m_pg);
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
// CCmdDeleteDiscreteObject
//////////////////////////////////////////////////////////////////////

CCmdDeleteDiscreteObject::CCmdDeleteDiscreteObject(GModel* model, GDiscreteObject* po) : CCommand("Remove discrete object")
{ 
	m_model = model;
	m_pobj = po; 
	m_bdel = false; 
}

CCmdDeleteDiscreteObject::~CCmdDeleteDiscreteObject() 
{ 
	if (m_bdel) delete m_pobj; 
}

void CCmdDeleteDiscreteObject::Execute()
{
	// remove the mesh to the model
	m_npos = m_model->RemoveDiscreteObject(m_pobj);
	m_bdel = true;
}

void CCmdDeleteDiscreteObject::UnExecute()
{
	// remove the mesh from the model
	m_model->InsertDiscreteObject(m_pobj, m_npos);
	m_bdel = false;
}

//////////////////////////////////////////////////////////////////////
// CCmdTransformObject
//////////////////////////////////////////////////////////////////////

CCmdTransformObject::CCmdTransformObject(GObject* po, const Transform& Q) : CCommand("Transform")
{
	m_po = po;
	m_oldQ = Q;
}

void CCmdTransformObject::Execute()
{
	Transform Q = m_po->GetTransform();
	m_po->GetTransform() = m_oldQ;
	m_oldQ = Q;
}

void CCmdTransformObject::UnExecute() { Execute(); }

//////////////////////////////////////////////////////////////////////
// CCmdTranslateSelection
//////////////////////////////////////////////////////////////////////

CCmdTranslateSelection::CCmdTranslateSelection(CGLDocument* doc, vec3d dr) : CCommand("Translate")
{
	m_doc = doc;
	m_dr = dr;
}

void CCmdTranslateSelection::Execute()
{
	m_doc->GetCurrentSelection()->Translate(m_dr);
	m_doc->Update();
	GObject* po = m_doc->GetActiveObject();
	if (po) po->UpdateFERenderMesh();
}

void CCmdTranslateSelection::UnExecute()
{
	m_doc->GetCurrentSelection()->Translate(-m_dr);
	m_doc->Update();
	GObject* po = m_doc->GetActiveObject();
	if (po) po->UpdateFERenderMesh();
}

//////////////////////////////////////////////////////////////////////
// CCmdRotateSelection
//////////////////////////////////////////////////////////////////////

CCmdRotateSelection::CCmdRotateSelection(CGLDocument* doc, quatd q, vec3d rc) : CCommand("Rotate")
{
	m_doc = doc;
	m_q = q;
	m_rc = rc;
}

void CCmdRotateSelection::Execute()
{
	m_doc->GetCurrentSelection()->Rotate(m_q, m_rc);
	m_doc->Update();
	GObject* po = m_doc->GetActiveObject();
	if (po) po->UpdateFERenderMesh();
}

void CCmdRotateSelection::UnExecute()
{
	m_doc->GetCurrentSelection()->Rotate(m_q.Inverse(), m_rc);
	m_doc->Update();
	GObject* po = m_doc->GetActiveObject();
	if (po) po->UpdateFERenderMesh();
}

//////////////////////////////////////////////////////////////////////
// CCmdScaleSelection
//////////////////////////////////////////////////////////////////////

CCmdScaleSelection::CCmdScaleSelection(CGLDocument* doc, double s, vec3d dr, vec3d rc) : CCommand("Scale")
{
	m_doc = doc;
	m_s = s;
	m_dr = dr;
	m_rc = rc;
}

void CCmdScaleSelection::Execute()
{
	m_doc->GetCurrentSelection()->Scale(m_s, m_dr, m_rc);
	m_doc->Update();
	GObject* po = m_doc->GetActiveObject();
	if (po) po->UpdateFERenderMesh();
}

void CCmdScaleSelection::UnExecute()
{
	m_doc->GetCurrentSelection()->Scale(1 / m_s, m_dr, m_rc);
	m_doc->Update();
	GObject* po = m_doc->GetActiveObject();
	if (po) po->UpdateFERenderMesh();
}

//=============================================================================
CCmdToggleObjectVisibility::CCmdToggleObjectVisibility(GModel* model) : CCommand("Toggle visibility")
{
	m_model = model;
}

void CCmdToggleObjectVisibility::Execute()
{
	int N = m_model->Objects();
	for (int i = 0; i<N; ++i)
	{
		GObject* po = m_model->Object(i);
		if (po->IsVisible()) po->Hide(); else po->Show();
	}
}

void CCmdToggleObjectVisibility::UnExecute()
{
	Execute();
}

//=============================================================================
CCmdTogglePartVisibility::CCmdTogglePartVisibility(GModel* model) : CCommand("Toggle visibility")
{
	m_model = model;
}

void CCmdTogglePartVisibility::Execute()
{
	int N = m_model->Objects();
	for (int i = 0; i<N; ++i)
	{
		GObject* po = m_model->Object(i);
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
CCmdToggleDiscreteVisibility::CCmdToggleDiscreteVisibility(GModel* model) : CCommand("Toggle visibility")
{
	m_model = model;
}

void CCmdToggleDiscreteVisibility::Execute()
{
	int N = m_model->DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* po = m_model->DiscreteObject(i);
		if (po->IsVisible()) po->Hide(); else po->Show();
	}
}

void CCmdToggleDiscreteVisibility::UnExecute()
{
	Execute();
}

//=============================================================================
CCmdToggleElementVisibility::CCmdToggleElementVisibility(GObject* po) : CCommand("Toggle visibility")
{
	m_po = po;
	m_mesh = (m_po ? m_po->GetFEMesh() : nullptr);
}

void CCmdToggleElementVisibility::Execute()
{
	if (m_mesh)
	{
		for (int i = 0; i < m_mesh->Elements(); ++i)
		{
			FSElement& el = m_mesh->Element(i);
			if (el.IsHidden()) el.Unhide(); else el.Hide();
		}
		m_mesh->UpdateItemVisibility();
		if (m_po) m_po->BuildFERenderMesh();
	}
}

void CCmdToggleElementVisibility::UnExecute()
{
	Execute();
}

//=============================================================================
CCmdToggleFEFaceVisibility::CCmdToggleFEFaceVisibility(FSMeshBase* mesh) : CCommand("Toggle visibility")
{
	m_mesh = mesh;
}

void CCmdToggleFEFaceVisibility::Execute()
{
	for (int i = 0; i<m_mesh->Faces(); ++i)
	{
		FSFace& face = m_mesh->Face(i);
		if (face.IsVisible()) face.Hide(); else face.Show();
	}
	m_mesh->UpdateItemVisibility();
}

void CCmdToggleFEFaceVisibility::UnExecute()
{
	Execute();
}


//////////////////////////////////////////////////////////////////////
// CCmdSelectObject
//////////////////////////////////////////////////////////////////////

CCmdSelectObject::CCmdSelectObject(GModel* model, GObject* po, bool badd) : CCommand("Select Object")
{
	m_badd = badd;

	m_model = model;

	// store the meshes selection state

	int M = model->Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = model->Object(i)->IsSelected();

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

CCmdSelectObject::CCmdSelectObject(GModel* model, GObject** ppo, int N, bool badd) : CCommand("Select Object")
{
	m_model = model;
	m_badd = badd;

	// store the meshes selection state
	int M = m_model->Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m_model->Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = ppo[i];
}

CCmdSelectObject::CCmdSelectObject(GModel* model, const vector<GObject*>& po, bool badd) : CCommand("Select Object")
{
	m_model = model;
	m_badd = badd;
	int N = (int)po.size();

	// store the meshes selection state
	int M = m_model->Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = m_model->Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = po[i];
}


void CCmdSelectObject::Execute()
{
	if (!m_badd)
	{
		for (int i = 0; i<m_model->Objects(); ++i) m_model->Object(i)->UnSelect();
	}

	for (int i = 0; i<m_N; ++i) m_ppo[i]->Select();
}

void CCmdSelectObject::UnExecute()
{
	for (int i = 0; i<m_model->Objects(); ++i)
	{
		GObject* po = m_model->Object(i);
		if (m_ptag[i])
			po->Select();
		else
			po->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectObject
//////////////////////////////////////////////////////////////////////

CCmdUnselectObject::CCmdUnselectObject(GModel* model, GObject* po) : CCommand("Unselect Object")
{
	m_model = model;

	// store the meshes selection state
	int M = model->Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = model->Object(i)->IsSelected();

	// store the meshes we need to unselect
	m_N = 1;
	m_ppo = new GObject*[m_N];
	m_ppo[0] = po;
}

CCmdUnselectObject::CCmdUnselectObject(GModel* model, GObject** ppo, int N) : CCommand("Unselect Object")
{
	m_model = model;

	// store the meshes selection state
	int M = model->Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = model->Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = ppo[i];
}

CCmdUnselectObject::CCmdUnselectObject(GModel* model, const vector<GObject*>& po) : CCommand("Unselect Object")
{
	m_model = model;

	// store the meshes selection state
	int N = (int)po.size();
	int M = model->Objects(), i;
	m_ptag = new bool[M];
	for (i = 0; i<M; ++i) m_ptag[i] = model->Object(i)->IsSelected();

	// store the meshes we need to select
	m_N = N;
	m_ppo = new GObject*[m_N];
	for (i = 0; i<N; ++i) m_ppo[i] = po[i];
}


void CCmdUnselectObject::Execute()
{
	for (int i = 0; i<m_N; ++i) m_ppo[i]->UnSelect();
}

void CCmdUnselectObject::UnExecute()
{
	for (int i = 0; i<m_model->Objects(); ++i)
	{
		GObject* po = m_model->Object(i);
		if (m_ptag[i])
			po->Select();
		else
			po->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectPart
//////////////////////////////////////////////////////////////////////

CCmdSelectPart::CCmdSelectPart(GModel* model, int* npart, int n, bool badd) : CCommand("Select Part")
{
	m_model = model;

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = npart[i];
	}

	m_bold.resize(model->Parts());
	int N = model->Parts();
	for (int i = 0; i<N; ++i) m_bold[i] = model->Part(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectPart::CCmdSelectPart(GModel* model, const vector<int>& part, bool badd) : CCommand("Select Part")
{
	m_model = model;
	int n = (int)part.size();

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = part[i];
	}

	m_bold.resize(model->Parts());
	int N = model->Parts();
	for (int i = 0; i<N; ++i) m_bold[i] = model->Part(i)->IsSelected();
	m_badd = badd;
}

void CCmdSelectPart::Execute()
{
	int i, n;
	if (!m_badd)
	{
		n = m_model->Parts();
		for (i = 0; i<n; ++i) m_model->Part(i)->UnSelect();
	}

	n = (int)m_npart.size();
	for (i = 0; i<n; ++i)
	{
		GPart* pg = m_model->FindPart(m_npart[i]);
		if (pg) pg->Select();
	}
}

void CCmdSelectPart::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GPart* pg = m_model->Part(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectPart
//////////////////////////////////////////////////////////////////////

CCmdUnSelectPart::CCmdUnSelectPart(GModel* model, int* npart, int n) : CCommand("Unselect Part")
{
	m_model = model;

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = npart[i];
	}

	int N = model->Parts();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Part(i)->IsSelected();
}

CCmdUnSelectPart::CCmdUnSelectPart(GModel* model, const vector<int>& part) : CCommand("Unselect Part")
{
	m_model = model;
	int n = (int)part.size();

	if (n > 0)
	{
		m_npart.resize(n);
		for (int i = 0; i<n; ++i) m_npart[i] = part[i];
	}

	int N = model->Parts();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Part(i)->IsSelected();
}

void CCmdUnSelectPart::Execute()
{
	int i, n;
	n = m_npart.size();
	for (i = 0; i<n; ++i)
	{
		GPart* pg = m_model->FindPart(m_npart[i]);
		if (pg) pg->UnSelect();
	}
}

void CCmdUnSelectPart::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GPart* pg = m_model->Part(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectSurface
//////////////////////////////////////////////////////////////////////

CCmdSelectSurface::CCmdSelectSurface(GModel* model, int* nsurf, int n, bool badd) : CCommand("Select Surface")
{
	m_model = model;

	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = nsurf[i];
	}

	int N = model->Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m_model->Surface(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectSurface::CCmdSelectSurface(GModel* ps, const vector<int>& surf, bool badd) : CCommand("Select Surface")
{
	m_model = ps;
	int n = (int)surf.size();

	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = surf[i];
	}

	int N = m_model->Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m_model->Surface(i)->IsSelected();
	m_badd = badd;
}

void CCmdSelectSurface::Execute()
{
	int i, n;
	if (!m_badd)
	{
		n = m_model->Surfaces();
		for (i = 0; i<n; ++i) m_model->Surface(i)->UnSelect();
	}

	n = m_nsurf.size();
	for (i = 0; i<n; ++i)
	{
		GFace* ps = m_model->FindSurface(m_nsurf[i]);
		if (ps) ps->Select();
		else
		{
//			flx_error("Reference to undefined surface in CCmdSelectSurface::Execute().");
			break;
		}
	}
}

void CCmdSelectSurface::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GFace* pg = m_model->Surface(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectSurface
//////////////////////////////////////////////////////////////////////

CCmdUnSelectSurface::CCmdUnSelectSurface(GModel* model, int* nsurf, int n) : CCommand("Unselect Surface")
{
	// get the model
	m_model = model;
	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = nsurf[i];
	}

	int N = model->Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Surface(i)->IsSelected();
}

CCmdUnSelectSurface::CCmdUnSelectSurface(GModel* model, const vector<int>& surf) : CCommand("Unselect Surface")
{
	int n = (int)surf.size();

	m_model = model;
	if (n > 0)
	{
		m_nsurf.resize(n);
		for (int i = 0; i<n; ++i) m_nsurf[i] = surf[i];
	}

	int N = model->Surfaces();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Surface(i)->IsSelected();
}

void CCmdUnSelectSurface::Execute()
{
	int i, n;
	n = m_nsurf.size();
	for (i = 0; i<n; ++i)
	{
		GFace* ps = m_model->FindSurface(m_nsurf[i]);
		if (ps) ps->UnSelect();
	}
}

void CCmdUnSelectSurface::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GFace* pg = m_model->Surface(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectEdge
//////////////////////////////////////////////////////////////////////

CCmdSelectEdge::CCmdSelectEdge(GModel* model, int* nedge, int n, bool badd) : CCommand("Select Edge")
{
	m_model = model;

	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = nedge[i];
	}

	int N = model->Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Edge(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectEdge::CCmdSelectEdge(GModel* model, const vector<int>& edge, bool badd) : CCommand("Select Edge")
{
	m_model = model;
	int n = (int)edge.size();

	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = edge[i];
	}

	int N = model->Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Edge(i)->IsSelected();
	m_badd = badd;
}

void CCmdSelectEdge::Execute()
{
	int i, n;
	if (!m_badd)
	{
		n = m_model->Edges();
		for (i = 0; i<n; ++i) m_model->Edge(i)->UnSelect();
	}

	n = m_nedge.size();
	for (i = 0; i<n; ++i)
	{
		GEdge* ps = m_model->FindEdge(m_nedge[i]);
		if (ps) ps->Select();
	}
}

void CCmdSelectEdge::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GEdge* pg = m_model->Edge(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectEdge
//////////////////////////////////////////////////////////////////////

CCmdUnSelectEdge::CCmdUnSelectEdge(GModel* model, int* nedge, int n) : CCommand("Unselect Edge")
{
	m_model = model;
	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = nedge[i];
	}

	int N = model->Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Edge(i)->IsSelected();
}

CCmdUnSelectEdge::CCmdUnSelectEdge(GModel* model, const vector<int>& edge) : CCommand("Unselect Edge")
{
	m_model = model;
	int n = (int)edge.size();
	if (n > 0)
	{
		m_nedge.resize(n);
		for (int i = 0; i<n; ++i) m_nedge[i] = edge[i];
	}

	int N = model->Edges();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Edge(i)->IsSelected();
}


void CCmdUnSelectEdge::Execute()
{
	int i, n;
	n = m_nedge.size();
	for (i = 0; i<n; ++i)
	{
		GEdge* ps = m_model->FindEdge(m_nedge[i]);
		if (ps) ps->UnSelect();
	}
}

void CCmdUnSelectEdge::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GEdge* pg = m_model->Edge(i);
		if (m_bold[i]) pg->Select(); else pg->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectNode
//////////////////////////////////////////////////////////////////////

CCmdSelectNode::CCmdSelectNode(GModel* model, int* node, int n, bool badd) : CCommand("Select Node")
{
	m_model = model;
	if (n > 0)
	{
		m_node.resize(n);
		for (int i = 0; i<n; ++i) m_node[i] = node[i];
	}

	int N = model->Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Node(i)->IsSelected();
	m_badd = badd;
}

CCmdSelectNode::CCmdSelectNode(GModel* model, const vector<int>& node, bool badd) : CCommand("Select Node")
{
	m_model = model;

	int n = (int)node.size();
	m_node = node;

	int N = model->Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = model->Node(i)->IsSelected();
	m_badd = badd;
}


void CCmdSelectNode::Execute()
{
	int i, n;
	if (!m_badd)
	{
		n = m_model->Nodes();
		for (i = 0; i<n; ++i) m_model->Node(i)->UnSelect();
	}

	n = m_node.size();
	for (i = 0; i<n; ++i)
	{
		GNode* pn = m_model->FindNode(m_node[i]);
		if (pn) pn->Select();
	}
}

void CCmdSelectNode::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GNode* pn = m_model->Node(i);
		if (m_bold[i]) pn->Select(); else pn->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectNode
//////////////////////////////////////////////////////////////////////

CCmdUnSelectNode::CCmdUnSelectNode(GModel* model, int* node, int n) : CCommand("Unselect Node")
{
	m_model = model;

	if (n > 0)
	{
		m_node.resize(n);
		for (int i = 0; i<n; ++i) m_node[i] = node[i];
	}

	int N = m_model->Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m_model->Node(i)->IsSelected();
}

CCmdUnSelectNode::CCmdUnSelectNode(GModel* model, const vector<int>& node) : CCommand("Unselect Node")
{
	m_model = model;
	int n = (int)node.size();
	if (n > 0)
	{
		m_node.resize(n);
		for (int i = 0; i<n; ++i) m_node[i] = node[i];
	}

	int N = m_model->Nodes();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m_model->Node(i)->IsSelected();
}

void CCmdUnSelectNode::Execute()
{
	int i, n;
	n = m_node.size();
	for (i = 0; i<n; ++i)
	{
		GNode* pn = m_model->FindNode(m_node[i]);
		if (pn) pn->UnSelect();
	}
}

void CCmdUnSelectNode::UnExecute()
{
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GNode* pn = m_model->Node(i);
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
// CCmdUnSelectDiscreteElements
//////////////////////////////////////////////////////////////////////

CCmdUnSelectDiscreteElements::CCmdUnSelectDiscreteElements(GDiscreteElementSet* set, const vector<int>& elemList) : CCommand("Unselect Discrete")
{
	m_ps = set;
	m_elemList = elemList;

	int N = set->size();
	m_bold.resize(N);
	for (int i = 0; i < N; ++i) m_bold[i] = m_ps->element(i).IsSelected();
}

void CCmdUnSelectDiscreteElements::Execute()
{
	int N = m_elemList.size();
	for (int i = 0; i < N; ++i)
	{
		GDiscreteElement& de = m_ps->element(m_elemList[i]);
		de.UnSelect();
	}
}

void CCmdUnSelectDiscreteElements::UnExecute()
{
	int N = m_bold.size();
	for (int i = 0; i < N; ++i)
	{
		GDiscreteElement& de = m_ps->element(i);
		if (m_bold[i]) de.Select(); else de.UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnSelectNode
//////////////////////////////////////////////////////////////////////

CCmdUnSelectDiscrete::CCmdUnSelectDiscrete(GModel* ps, int* pobj, int n) : CCommand("Unselect Discrete")
{
	m_model = ps;
	if (n > 0)
	{
		m_obj.resize(n);
		for (int i = 0; i<n; ++i) m_obj[i] = m_model->DiscreteObject(pobj[i]);
	}

	int N = m_model->DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m_model->DiscreteObject(i)->IsSelected();
}

CCmdUnSelectDiscrete::CCmdUnSelectDiscrete(GModel* ps, const vector<int>& obj) : CCommand("Unselect Discrete")
{
	m_model = ps;

	int n = (int)obj.size();
	if (n > 0)
	{
		m_obj.resize(n);
		for (int i = 0; i<n; ++i) m_obj[i] = m_model->DiscreteObject(obj[i]);
	}

	int N = m_model->DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i<N; ++i) m_bold[i] = m_model->DiscreteObject(i)->IsSelected();
}

CCmdUnSelectDiscrete::CCmdUnSelectDiscrete(GModel* ps, const vector<GDiscreteObject*>& obj) : CCommand("Unselect Discrete")
{
	m_model = ps;

	m_obj = obj;

	int N = m_model->DiscreteObjects();
	m_bold.resize(N);
	for (int i = 0; i < N; ++i) m_bold[i] = m_model->DiscreteObject(i)->IsSelected();
}


void CCmdUnSelectDiscrete::Execute()
{
	int ND = m_model->DiscreteObjects();
	int i, n;
	n = m_obj.size();
	for (i = 0; i<n; ++i)
	{
		GDiscreteObject* pn = m_obj[i];
		if (pn) pn->UnSelect();
	}
}

void CCmdUnSelectDiscrete::UnExecute()
{
	int ND = m_model->DiscreteObjects();
	int n = m_bold.size();
	for (int i = 0; i<n; ++i)
	{
		GDiscreteObject* pn = m_model->DiscreteObject(i);
		if (m_bold[i]) pn->Select(); else pn->UnSelect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdInvertSelection
//////////////////////////////////////////////////////////////////////

CCmdInvertSelection::CCmdInvertSelection(CGLDocument* doc) : CCommand("Invert selection")
{
	m_doc = doc;
}

void CCmdInvertSelection::Execute()
{
	m_doc->GetCurrentSelection()->Invert();
}

void CCmdInvertSelection::UnExecute()
{
	m_doc->GetCurrentSelection()->Invert();
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectElements
//////////////////////////////////////////////////////////////////////

CCmdSelectElements::CCmdSelectElements(FSCoreMesh* pm, int* pe, int N, bool badd) : CCommand("Select Elements")
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
	for (i = 0; i<M; ++i) m_ptag[i] = pm->ElementRef(i).IsSelected();

	// store the elements we need to select
	if (N != 0)
	{
		m_N = N;
		m_pel = new int[m_N];
		for (i = 0; i<N; ++i) m_pel[i] = pe[i];
	}
}

CCmdSelectElements::CCmdSelectElements(FSCoreMesh* pm, const std::vector<int>& el, bool badd) : CCommand("Select Elements")
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
	for (i = 0; i<M; ++i) m_ptag[i] = pm->ElementRef(i).IsSelected();

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

	if (!m_badd)
	{
		for (i = 0; i<m_pm->Elements(); ++i) m_pm->ElementRef(i).Unselect();
	}

	int NE = m_pm->Elements();
	for (i = 0; i<m_N; ++i)
	{
		int n = m_pel[i];
		if ((n >= 0) && (n<NE)) m_pm->ElementRef(n).Select();
	}
}

void CCmdSelectElements::UnExecute()
{
	for (int i = 0; i<m_pm->Elements(); ++i)
	{
		FEElement_& el = m_pm->ElementRef(i);
		if (m_ptag[i])
			el.Select();
		else
			el.Unselect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectElements
//////////////////////////////////////////////////////////////////////

CCmdUnselectElements::CCmdUnselectElements(FSMesh* mesh, int* pe, int N) : CCommand("Unselect")
{
	// get the current mesh
	m_mesh = mesh;
	FSMesh* pm = mesh;
	int M = pm->Elements();

	// store the elements selection state
	m_ptag = new bool[M];
	for (int i = 0; i<M; ++i) m_ptag[i] = pm->Element(i).IsSelected();

	// store the elements we need to select
	m_N = N;
	m_pel = new int[m_N];
	for (int i = 0; i<N; ++i) m_pel[i] = pe[i];
}

CCmdUnselectElements::CCmdUnselectElements(FSMesh* mesh, const vector<int>& elem) : CCommand("Unselect")
{
	int N = (int)elem.size();

	// get the current mesh
	m_mesh = mesh;
	FSMesh* pm = m_mesh;
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
	FSMesh* pm = m_mesh;
	for (int i = 0; i<m_N; ++i) pm->Element(m_pel[i]).Unselect();
}

void CCmdUnselectElements::UnExecute()
{
	FSMesh* pm = m_mesh;
	for (int i = 0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (m_ptag[i])
			el.Select();
		else
			el.Unselect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectFaces
//////////////////////////////////////////////////////////////////////

CCmdSelectFaces::CCmdSelectFaces(FSMeshBase* pm, int* pf, int N, bool badd) : CCommand("Select Faces")
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

CCmdSelectFaces::CCmdSelectFaces(FSMeshBase* pm, const vector<int>& fl, bool badd) : CCommand("Select Faces")
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
}

void CCmdSelectFaces::UnExecute()
{
	for (int i = 0; i<m_pm->Faces(); ++i)
	{
		FSFace& face = m_pm->Face(i);
		if (m_ptag[i])
			face.Select();
		else
			face.Unselect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectFaces
//////////////////////////////////////////////////////////////////////

CCmdUnselectFaces::CCmdUnselectFaces(FSMeshBase* pm, int* pf, int N) : CCommand("Unselect")
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

CCmdUnselectFaces::CCmdUnselectFaces(FSMeshBase* pm, const vector<int>& face) : CCommand("Unselect")
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
	for (int i = 0; i<m_N; ++i) m_pm->Face(m_pface[i]).Unselect();
}

void CCmdUnselectFaces::UnExecute()
{
	for (int i = 0; i<m_pm->Faces(); ++i)
	{
		FSFace& face = m_pm->Face(i);
		if (m_ptag[i])
			face.Select();
		else
			face.Unselect();
	}
}


//////////////////////////////////////////////////////////////////////
// CCmdSelectFEEdges
//////////////////////////////////////////////////////////////////////

CCmdSelectFEEdges::CCmdSelectFEEdges(FSLineMesh* pm, int* pe, int N, bool badd) : CCommand("Select Edges")
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

CCmdSelectFEEdges::CCmdSelectFEEdges(FSLineMesh* pm, const vector<int>& el, bool badd) : CCommand("Select Edges")
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
}

void CCmdSelectFEEdges::UnExecute()
{
	for (int i = 0; i<m_pm->Edges(); ++i)
	{
		FSEdge& edge = m_pm->Edge(i);
		if (m_ptag[i])
			edge.Select();
		else
			edge.Unselect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectFEEdges
//////////////////////////////////////////////////////////////////////

CCmdUnselectFEEdges::CCmdUnselectFEEdges(FSLineMesh* pm, int* pe, int N) : CCommand("Unselect")
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

CCmdUnselectFEEdges::CCmdUnselectFEEdges(FSLineMesh* pm, const vector<int>& edge) : CCommand("Unselect")
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
	for (int i = 0; i<m_N; ++i) m_pm->Edge(m_pedge[i]).Unselect();
}

void CCmdUnselectFEEdges::UnExecute()
{
	for (int i = 0; i<m_pm->Edges(); ++i)
	{
		FSEdge& edge = m_pm->Edge(i);
		if (m_ptag[i])
			edge.Select();
		else
			edge.Unselect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdSelectFENodes
//////////////////////////////////////////////////////////////////////

CCmdSelectFENodes::CCmdSelectFENodes(FSLineMesh* pm, int* pn, int N, bool badd) : CCommand("Select Nodes")
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

CCmdSelectFENodes::CCmdSelectFENodes(FSLineMesh* pm, const vector<int>& nl, bool badd) : CCommand("Select Nodes")
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
}

void CCmdSelectFENodes::UnExecute()
{
	for (int i = 0; i<m_pm->Nodes(); ++i)
	{
		FSNode& node = m_pm->Node(i);
		if (m_ptag[i])
			node.Select();
		else
			node.Unselect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnselectNodes
//////////////////////////////////////////////////////////////////////

CCmdUnselectNodes::CCmdUnselectNodes(FSLineMesh* pm, int* pn, int N) : CCommand("Unselect")
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

CCmdUnselectNodes::CCmdUnselectNodes(FSLineMesh* pm, const vector<int>& node) : CCommand("Unselect")
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
	FSLineMesh* pm = m_mesh;
	for (int i = 0; i<m_N; ++i) pm->Node(m_pn[i]).Unselect();
}

void CCmdUnselectNodes::UnExecute()
{
	FSLineMesh* pm = m_mesh;
	for (int i = 0; i<pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if (m_ptag[i])
			node.Select();
		else
			node.Unselect();
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdAssignPartMaterial
//////////////////////////////////////////////////////////////////////

CCmdAssignPartMaterial::CCmdAssignPartMaterial(GModel* model, vector<int> npart, int nmat) : CCommand("Assign material")
{
	m_model = model;

	int n = npart.size();

	m_part.resize(n);
	m_old.resize(n);
	for (int i = 0; i<n; ++i)
	{
		GPart* pg = model->FindPart(npart[i]);
		m_part[i] = npart[i];
		m_old[i] = pg->GetMaterialID();
	}
	m_mat = nmat;
}

void CCmdAssignPartMaterial::Execute()
{
	int N = m_part.size();
	for (int i = 0; i<N; ++i)
	{
		GPart* pg = m_model->FindPart(m_part[i]);
		GObject* po = dynamic_cast<GObject*>(pg->Object());
		po->AssignMaterial(m_part[i], m_mat);
	}
}

void CCmdAssignPartMaterial::UnExecute()
{
	int N = m_part.size();
	for (int i = 0; i<N; ++i)
	{
		GPart* pg = m_model->FindPart(m_part[i]);
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
	m_po->AssignMaterial(m_mat);
}

void CCmdAssignObjectMaterial::UnExecute()
{
	int N = m_po->Parts();
	for (int i = 0; i < N; ++i)
	{
		m_po->AssignMaterial(m_po->Part(i), m_old[i]);
	}
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

CCmdDeleteFESelection::CCmdDeleteFESelection(GMeshObject* po, int nitem) : CCommand("Delete")
{
	m_nitem = nitem;
	m_pnew = 0;
	m_pobj = po;
	m_pold = po->GetFEMesh();
}

void CCmdDeleteFESelection::Execute()
{
	// create a copy of the old mesh
	if (m_pnew == 0)
	{
		m_pnew = new FSMesh(*m_pold);
		FEMeshBuilder meshBuilder(*m_pnew);

		if      (m_nitem == ITEM_ELEM) meshBuilder.DeleteSelectedElements();
		else if (m_nitem == ITEM_FACE) meshBuilder.DeleteSelectedFaces();
		else if (m_nitem == ITEM_NODE) meshBuilder.DeleteSelectedNodes();

		// make sure you select the object
		m_pobj->Select();
	}

	// set the object's mesh
	m_pobj->ReplaceFEMesh(m_pnew, false);

	// swap meshes
	FSMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
}

//-----------------------------------------------------------------------------
//! Undo the delete FE selection
//! \todo this function does not restore the original GMeshObject
void CCmdDeleteFESelection::UnExecute()
{
	// set the object's mesh
	m_pobj->ReplaceFEMesh(m_pnew);

	// swap meshes
	FSMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
}

//=============================================================================
// CCmdDeleteFESurfaceSelection
//-----------------------------------------------------------------------------

CCmdDeleteFESurfaceSelection::CCmdDeleteFESurfaceSelection(GSurfaceMeshObject* po, int nitem) : CCommand("Delete")
{
	m_pnew = 0;
	m_pobj = po;
	m_pold = po->GetSurfaceMesh();
	m_item = nitem;
}

void CCmdDeleteFESurfaceSelection::Execute()
{
	// create a copy of the old mesh
	if (m_pnew == 0)
	{
		m_pnew = new FSSurfaceMesh(*m_pold);

		if (m_item == ITEM_FACE) m_pnew->DeleteSelectedFaces();
		else if (m_item == ITEM_EDGE) m_pnew->DeleteSelectedEdges();
		else if (m_item == ITEM_NODE) m_pnew->DeleteSelectedNodes();

		// make sure you select the object
		m_pobj->Select();
	}

	// set the object's mesh
	m_pobj->ReplaceSurfaceMesh(m_pnew);

	// swap meshes
	FSSurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
}

//-----------------------------------------------------------------------------
//! Undo the delete FE selection
//! \todo this function does not restore the original GMeshObject
void CCmdDeleteFESurfaceSelection::UnExecute()
{
	// set the object's mesh
	m_pobj->ReplaceSurfaceMesh(m_pnew);

	// swap meshes
	FSSurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
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

CCmdHideParts::CCmdHideParts(GModel* model, GPart* part) : CCommand("Hide")
{
	m_model = model;
	m_partList.push_back(part);
}

CCmdHideParts::CCmdHideParts(GModel* model, std::list<GPart*> partList) : CCommand("Hide")
{
	m_model = model;
	m_partList = partList;
}

void CCmdHideParts::Execute()
{
	m_model->ShowParts(m_partList, false);
}

void CCmdHideParts::UnExecute()
{
	m_model->ShowParts(m_partList, true);
}

//////////////////////////////////////////////////////////////////////
// CCmdShowParts
//////////////////////////////////////////////////////////////////////

CCmdShowParts::CCmdShowParts(GModel* model, std::list<GPart*> partList) : CCommand("Show parts")
{
	m_model = model;
	m_partList = partList;
}

void CCmdShowParts::Execute()
{
	m_model->ShowParts(m_partList, true);
}

void CCmdShowParts::UnExecute()
{
	m_model->ShowParts(m_partList, false);
}

//////////////////////////////////////////////////////////////////////
// CCmdHideElements
//////////////////////////////////////////////////////////////////////

CCmdHideElements::CCmdHideElements(GObject* po, const vector<int>& elemList) : CCommand("Hide")
{
	m_po = po;
	m_mesh = po->GetFEMesh();
	m_elemList = elemList;
}

CCmdHideElements::CCmdHideElements(FSMesh* mesh, const vector<int>& elemList) : CCommand("Hide")
{
	m_po = nullptr;
	m_mesh = mesh;
	m_elemList = elemList;
}

void CCmdHideElements::Execute()
{
	if (m_po) m_po->ShowElements(m_elemList, false);
	else m_mesh->ShowElements(m_elemList, false);
}

void CCmdHideElements::UnExecute()
{
	if (m_po) m_po->ShowElements(m_elemList, true);
	else m_mesh->ShowElements(m_elemList, true);
}

//////////////////////////////////////////////////////////////////////
// CCmdHideFaces
//////////////////////////////////////////////////////////////////////

CCmdHideFaces::CCmdHideFaces(FSSurfaceMesh* mesh, const vector<int>& faceList) : CCommand("Hide")
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

CCmdHideSelection::CCmdHideSelection(CModelDocument* doc) : CCommand("Hide")
{
	m_doc = doc;
	FESelection* ps = m_doc->GetCurrentSelection();
	assert(ps->Size());
	m_item.resize(ps->Size());
	int m = 0;

	// get the model
	GModel& model = m_doc->GetFSModel()->GetModel();
	GObject* po = m_doc->GetActiveObject();

	m_nitem = doc->GetItemMode();
	m_nselect = doc->GetSelectionMode();

	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_nselect == SELECT_OBJECT)
		{
			for (int i = 0; i<model.Objects(); ++i)
				if (model.Object(i)->IsSelected()) m_item[m++] = i;
		}
		else if (m_nselect == SELECT_PART)
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
		FSMesh* mesh = (po ? po->GetFEMesh() : 0);
		if (mesh)
		{
			for (int i = 0; i<mesh->Elements(); ++i)
			{
				FSElement& el = mesh->Element(i);
				if (el.IsSelected()) m_item[m++] = i;
			}
		}
	}
	break;
	case ITEM_FACE:
	{
		FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
		if (pm)
		{
			for (int i = 0; i<pm->Faces(); ++i)
			{
				FSFace& face = pm->Face(i);
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
	GModel& m = m_doc->GetFSModel()->GetModel();
	GObject* po = m_doc->GetActiveObject();
	int N = m_item.size();
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_nselect == SELECT_OBJECT)
			m.ShowObjects(m_item, false);
		else if (m_nselect == SELECT_PART)
			m.ShowParts(m_item, false);
		break;
	case ITEM_ELEM:
	{
		po->ShowElements(m_item, false);
	}
	break;
	case ITEM_FACE:
	{
		FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
		if (pm) pm->ShowFaces(m_item, false);
	}
	break;
	}
}

void CCmdHideSelection::UnExecute()
{
	GObject* po = m_doc->GetActiveObject();
	GModel& m = m_doc->GetFSModel()->GetModel();
	m_doc->SetItemMode(m_nitem);
	int N = m_item.size();
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_nselect == SELECT_OBJECT)
		{
			m.ShowObjects(m_item);
			m.SelectObjects(m_item);
		}
		else if (m_nselect == SELECT_PART)
		{
			m.ShowParts(m_item, true, true);
		}
		break;
	case ITEM_ELEM:
	{
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			po->ShowElements(m_item, true);
			pm->SelectElements(m_item);
		}
	}
	break;
	case ITEM_FACE:
	{
		FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
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

CCmdHideUnselected::CCmdHideUnselected(CModelDocument* doc) : CCommand("Hide")
{
	m_doc = doc;
	FESelection* ps = m_doc->GetCurrentSelection();
	assert(ps->Size());
	m_item.resize(ps->Size());
	int m = 0;

	GObject* po = m_doc->GetActiveObject();
	GModel& model = m_doc->GetFSModel()->GetModel();
	m_nitem = doc->GetItemMode();
	m_nselect = doc->GetSelectionMode();

	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_nselect == SELECT_OBJECT)
		{
			for (int i = 0; i<model.Objects(); ++i)
			{
				po = model.Object(i);
				if (!po->IsSelected()) m_item[m++] = i;
			}
		}
		else if (m_nselect == SELECT_PART)
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
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			for (int i = 0; i<pm->Elements(); ++i) if (!pm->Element(i).IsSelected()) m_item[m++] = i;
		}
	}
	break;
	case ITEM_FACE:
	{
		FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
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
	GObject* po = m_doc->GetActiveObject();
	GModel& m = m_doc->GetFSModel()->GetModel();
	int N = m_item.size();
	if (N == 0) return;
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_nselect == SELECT_OBJECT)
			m.ShowObjects(m_item, false);
		else if (m_nselect == SELECT_PART)
			m.ShowParts(m_item, false);
		break;
	case ITEM_ELEM:
	{
		FSMesh* pm = po->GetFEMesh();
		if (pm) pm->ShowElements(m_item, false);
	}
	break;
	case ITEM_FACE:
	{
		FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
		if (pm) pm->ShowFaces(m_item, false);
	}
	break;
	}
}

void CCmdHideUnselected::UnExecute()
{
	GObject* po = m_doc->GetActiveObject();
	GModel& m = m_doc->GetFSModel()->GetModel();
	int N = m_item.size();
	if (N == 0) return;
	switch (m_nitem)
	{
	case ITEM_MESH:
		if (m_nselect == SELECT_OBJECT)
			m.ShowObjects(m_item, true);
		else if (m_nselect == SELECT_PART)
			m.ShowParts(m_item, true);
		break;
	case ITEM_ELEM:
	{
		FSMesh* pm = po->GetFEMesh();
		if (pm) pm->ShowElements(m_item);
	}
	break;
	case ITEM_FACE:
	{
		FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
		if (pm) pm->ShowFaces(m_item);
	}
	break;
	}
}

//////////////////////////////////////////////////////////////////////
// CCmdUnhideAll
//////////////////////////////////////////////////////////////////////

CCmdUnhideAll::CCmdUnhideAll(CModelDocument* doc) : CCommand("Unhide all")
{
	m_doc = doc;
	m_nitem = doc->GetItemMode();
	m_nselect = doc->GetSelectionMode();
	GModel& model = m_doc->GetFSModel()->GetModel();
	m_bunhide = true;

	if (m_nitem == ITEM_MESH)
	{
		switch (m_nselect)
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
		GObject* po = m_doc->GetActiveObject();
		if (po == 0) return;
		switch (m_nitem)
		{
		case ITEM_ELEM:
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				for (int i = 0; i<pm->Elements(); ++i) if (!pm->Element(i).IsVisible()) m_item.push_back(i);
			}
		}
		break;
		case ITEM_FACE:
		{
			FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
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
	if (m_item.empty()) return;

	GModel& model = m_doc->GetFSModel()->GetModel();
	int N = m_item.size();
	if (m_nitem == ITEM_MESH)
	{
		switch (m_nselect)
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
		GObject* po = m_doc->GetActiveObject();
		switch (m_nitem)
		{
		case ITEM_ELEM:
		{
			FSMesh* pm = po->GetFEMesh();
			assert(pm);
			pm->ShowElements(m_item, m_bunhide);
		}
		break;
		case ITEM_FACE:
		{
			FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
			assert(pm);
			if (pm) pm->ShowFaces(m_item, m_bunhide);
		}
		break;
		}
		po->UpdateItemVisibility();
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

CCmdApplyFEModifier::CCmdApplyFEModifier(FEModifier* pmod, GObject* po, FSGroup* selection) : CCommand(pmod->GetName())
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
			FSMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;

			throw;
		}

		// swap old and new
		// we do this so that we can always delete m_pnew
		FSMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}

void CCmdApplyFEModifier::UnExecute()
{
	// get the FSModel
	if (m_pnew)
	{
		// replace the old mesh with the new
		m_pobj->ReplaceFEMesh(m_pnew);

		// swap old and new
		// we do this so that we can always delete m_pnew
		FSMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}


//=============================================================================
// CCmdApplySurfaceModifier
//-----------------------------------------------------------------------------

CCmdApplySurfaceModifier::CCmdApplySurfaceModifier(FESurfaceModifier* pmod, GObject* po, FSGroup* selection) : CCommand(pmod->GetName())
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
			FSSurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;

			throw;
		}

		// swap old and new
		// we do this so that we can always delete m_pnew
		FSSurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}

void CCmdApplySurfaceModifier::UnExecute()
{
	// get the FSModel
	if (m_pnew)
	{
		// replace the old mesh with the new
		m_pobj->ReplaceSurfaceMesh(dynamic_cast<FSSurfaceMesh*>(m_pnew));

		// swap old and new
		// we do this so that we can always delete m_pnew
		FSSurfaceMesh* pm = m_pnew; m_pnew = m_pold; m_pold = pm;
	}
}

//=============================================================================
// CCmdChangeFEMesh
//-----------------------------------------------------------------------------

CCmdChangeFEMesh::CCmdChangeFEMesh(GObject* po, FSMesh* pm, bool bup) : CCommand("Change mesh")
{
	assert(po);
	m_update = bup;
	m_po = po;
	m_pnew = pm;
}

void CCmdChangeFEMesh::Execute()
{
	FSMesh* pm = m_po->GetFEMesh();
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

CCmdChangeFESurfaceMesh::CCmdChangeFESurfaceMesh(GSurfaceMeshObject* po, FSSurfaceMesh* pm, bool up) : CCommand("Change surface mesh")
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
	FSSurfaceMesh* pm = m_po->GetSurfaceMesh();
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

CCmdChangeView::CCmdChangeView(CGView* pview, CGLCamera cam) : CCommand("Change View")
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
	FSMesh* pm = m_po->GetFEMesh();
	FEMeshBuilder meshBuilder(*pm);
	meshBuilder.InvertSelectedElements();
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
	m_po->GetParamBlock().Copy(m_Param);
	m_po->Update();
	m_Param = pb;
}

void CCmdChangeObjectParams::UnExecute()
{
	ParamBlock pb = m_po->GetParamBlock();
	m_po->GetParamBlock().Copy(m_Param);
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
// CCmdSwapObjects
//-----------------------------------------------------------------------------

CCmdSwapObjects::CCmdSwapObjects(GModel* model, GObject* pold, GObject* pnew) : CCommand("Convert")
{
	m_model = model;
	m_pold = pold;
	m_pnew = pnew;
	m_oml = nullptr;
}

CCmdSwapObjects::~CCmdSwapObjects()
{
	if (m_oml)
	{
		MeshLayerManager::Destroy(m_oml);
		delete m_pold;
	}
	else delete m_pnew;
}

void CCmdSwapObjects::Execute()
{
	// get the old object's meshlist
	m_oml = m_model->GetObjectMeshList(m_pold);

	// replace the old object with the new one
	m_model->ReplaceObject(m_pold, m_pnew);
}

void CCmdSwapObjects::UnExecute()
{
	// remove the new object
	m_model->RemoveObject(m_pnew, true);

	// add the oml back
	m_model->InsertObjectMeshList(m_oml);
	m_oml = nullptr;
}

//-----------------------------------------------------------------------------
// CCmdConvertToMultiBlock
//-----------------------------------------------------------------------------

CCmdConvertToMultiBlock::CCmdConvertToMultiBlock(GModel* model, GObject* po) : CCommand("Convert")
{
	m_model = model;
	m_pold = po;
	m_pnew = 0;
}

CCmdConvertToMultiBlock::~CCmdConvertToMultiBlock()
{
	if (m_pnew) delete m_pnew;
}

void CCmdConvertToMultiBlock::Execute()
{
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
	m_model->ReplaceObject(m_pold, m_pnew);

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

CCmdAddStep::CCmdAddStep(FSModel* fem, FSStep* pstep, int insertAfter) : CCommand("Add step")
{
	m_fem = fem;
	m_pstep = pstep;
	m_pos = insertAfter;
}

CCmdAddStep::~CCmdAddStep()
{
	if (m_pstep) delete m_pstep;
}

void CCmdAddStep::Execute()
{
	if (m_pos == -1) m_pos = m_fem->Steps() - 1;
	m_fem->InsertStep(m_pos + 1, m_pstep);
	m_pstep = 0;
}

void CCmdAddStep::UnExecute()
{
	m_pstep = m_fem->GetStep(m_pos + 1);
	m_fem->DeleteStep(m_pstep);
}

//-----------------------------------------------------------------------------
// CCmdSwapSteps
//-----------------------------------------------------------------------------

CCmdSwapSteps::CCmdSwapSteps(FSModel* fem, FSStep* step0, FSStep* step1) : CCommand("Swap steps")
{
	m_fem = fem;
	m_step0 = step0;
	m_step1 = step1;
}

void CCmdSwapSteps::Execute() 
{
	m_fem->SwapSteps(m_step0, m_step1);
}

void CCmdSwapSteps::UnExecute()
{
	Execute();
}


//-----------------------------------------------------------------------------
// CCmdAddMaterial
//-----------------------------------------------------------------------------

CCmdAddMaterial::CCmdAddMaterial(FSModel* fem, GMaterial* pm) : CCommand("Add material")
{
	m_fem = fem;
	m_pm = pm;
}

CCmdAddMaterial::~CCmdAddMaterial()
{
	if (m_pm) delete m_pm;
}

void CCmdAddMaterial::Execute()
{
	m_fem->AddMaterial(m_pm);
	m_pm = 0;
}

void CCmdAddMaterial::UnExecute()
{
	// remove the last material
	int N = m_fem->Materials();
	assert(N>0);
	m_pm = m_fem->GetMaterial(N - 1);
	m_fem->DeleteMaterial(m_pm);
}

//-----------------------------------------------------------------------------
// CCmdSetItemList
//-----------------------------------------------------------------------------

CCmdSetItemList::CCmdSetItemList(IHasItemLists* pbc, FEItemListBuilder* pl, int n) : CCommand("Assign selection")
{
	m_pbc = pbc;
	m_pl = pl;
	m_index = n;
}

CCmdSetItemList::~CCmdSetItemList()
{
	
}

void CCmdSetItemList::Execute()
{
	FEItemListBuilder* pold = m_pbc->GetItemList(m_index);
	m_pbc->SetItemList(m_pl, m_index);
	m_pl = pold;
}

void CCmdSetItemList::UnExecute()
{
	Execute();
}

//-----------------------------------------------------------------------------
// CCmdAddToItemListBuilder
//-----------------------------------------------------------------------------

CCmdAddToItemListBuilder::CCmdAddToItemListBuilder(FEItemListBuilder* pold, vector<int>& lnew) : CCommand("Add to selection")
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

CCmdRemoveFromItemListBuilder::CCmdRemoveFromItemListBuilder(FEItemListBuilder* pold, vector<int>& lnew) : CCommand("Remove from selection")
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
// CCmdRemoveItemListBuilder
//-----------------------------------------------------------------------------

CCmdRemoveItemListBuilder::CCmdRemoveItemListBuilder(IHasItemLists* pmc, int n) : CCommand("Remove selection")
{
	m_pmc = pmc;
	m_pitem = nullptr;
	m_index = n;
}

CCmdRemoveItemListBuilder::~CCmdRemoveItemListBuilder()
{
	
}

void CCmdRemoveItemListBuilder::Execute()
{
	m_pitem = m_pmc->GetItemList(m_index);
	m_pmc->SetItemList(nullptr, m_index);
}

void CCmdRemoveItemListBuilder::UnExecute()
{
	m_pmc->SetItemList(m_pitem, m_index);
	m_pitem = nullptr;
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
// CCmdDeleteFSModelComponent
//-----------------------------------------------------------------------------

CCmdDeleteFSModelComponent::CCmdDeleteFSModelComponent(FSModelComponent* po) : CCommand(string("Delete ") + po->GetName())
{
	assert(po->GetParent());
	m_obj = po;
	m_parent = po->GetParent();
	m_delObject = false;
}

CCmdDeleteFSModelComponent::~CCmdDeleteFSModelComponent()
{
	if (m_delObject) delete m_obj;
}

void CCmdDeleteFSModelComponent::Execute()
{
	m_insertPos = m_parent->RemoveChild(m_obj);
	m_obj->SetParent(nullptr);
	m_delObject = true;

	IHasItemLists* pil = dynamic_cast<IHasItemLists*>(m_obj);
	if (pil)
	{
		for (int i = 0; i < pil->ItemLists(); ++i)
		{
			FEItemListBuilder* pl = pil->GetItemList(i);
			if (pl) pl->DecRef();
		}
	}
}

void CCmdDeleteFSModelComponent::UnExecute()
{
	m_parent->InsertChild(m_insertPos, m_obj);
	assert(m_obj->GetParent() == m_parent);
	m_delObject = false;

	IHasItemLists* pil = dynamic_cast<IHasItemLists*>(m_obj);
	if (pil)
	{
		for (int i = 0; i < pil->ItemLists(); ++i)
		{
			FEItemListBuilder* pl = pil->GetItemList(i);
			if (pl) pl->IncRef();
		}
	}
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
	FSMesh* mesh = m_data->GetMesh();
	m_index = mesh->GetMeshDataIndex(m_data); assert(m_index >= 0);
	mesh->RemoveMeshDataField(m_index);
	if (m_data->GetItemList()) m_data->GetItemList()->DecRef();
}

void CCmdRemoveMeshData::UnExecute()
{
	FSMesh* mesh = m_data->GetMesh();
	mesh->InsertMeshData(m_index, m_data);
	if (m_data->GetItemList()) m_data->GetItemList()->IncRef();
	m_index = -1;
}

CCmdToggleActiveParts::CCmdToggleActiveParts(const std::vector<GPart*>& partList) : CCommand("toggle active")
{
	m_partList = partList;
}

void CCmdToggleActiveParts::Execute()
{
	for (GPart* pg : m_partList)
	{
		pg->SetActive(!pg->IsActive());
	}
}

void CCmdToggleActiveParts::UnExecute()
{
	Execute();
}

//-----------------------------------------------------------------------------
// CCmdAddImageAnalysis
//-----------------------------------------------------------------------------

CCmdAddImageAnalysis::CCmdAddImageAnalysis(CImageAnalysis* analysis)
    : CCommand("Add image analysis"), m_analysis(analysis), m_del(false)
{

}

CCmdAddImageAnalysis::~CCmdAddImageAnalysis()
{
    if(m_analysis && m_del) delete m_analysis;
}

void CCmdAddImageAnalysis::Execute()
{
    m_analysis->GetImageModel()->AddImageAnalysis(m_analysis);
    m_del = false;
}

void CCmdAddImageAnalysis::UnExecute()
{
    m_analysis->GetImageModel()->RemoveAnalysis(m_analysis);
    m_analysis->OnDelete();
    m_del = true;
}


//-----------------------------------------------------------------------------
// CCmdDeleteImageAnalysis
//-----------------------------------------------------------------------------

CCmdDeleteImageAnalysis::CCmdDeleteImageAnalysis(CImageAnalysis* analysis)
    : CCommand("Delete image analysis"), m_analysis(analysis), m_del(false)
{

}

CCmdDeleteImageAnalysis::~CCmdDeleteImageAnalysis()
{
    if(m_analysis && m_del) delete m_analysis;
}

void CCmdDeleteImageAnalysis::Execute()
{
    m_analysis->GetImageModel()->RemoveAnalysis(m_analysis);
    m_analysis->OnDelete();
    m_del = true;
}

void CCmdDeleteImageAnalysis::UnExecute()
{
    m_analysis->GetImageModel()->AddImageAnalysis(m_analysis);
    m_del = false;
}
