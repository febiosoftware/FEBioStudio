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
#include "GLPostScene.h"
#include "PostDocument.h"
#include <GLLib/GLContext.h>
#include <PostGL/GLModel.h>
#include <PostGL/GLPlaneCutPlot.h>
#include <GLLib/glx.h>
#include <MeshTools/FESelection.h>
#include <PostGL/PostObject.h>
#include "GLHighlighter.h"

CGLPostScene::CGLPostScene(CPostDocument* doc) : m_doc(doc)
{
	m_btrack = false;
	m_ntrack[0] = m_ntrack[1] = m_ntrack[2] = -1;
	m_trackScale = 1.0;
}

BOX CGLPostScene::GetBoundingBox()
{
	BOX box;
	if (m_doc && m_doc->IsValid())
	{
		box = m_doc->GetPostObject()->GetBoundingBox();
	}
	return box;
}

BOX CGLPostScene::GetSelectionBox()
{
	BOX box;
	if (m_doc && m_doc->IsValid())
	{
		box = m_doc->GetSelectionBox();
	}
	return box;
}

void CGLPostScene::Render(GLRenderEngine& engine, CGLContext& rc)
{
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return;

	// Update GLWidget string table for post rendering
	GLWidget::addToStringTable("$(filename)", m_doc->GetDocFileName());
	GLWidget::addToStringTable("$(datafield)", m_doc->GetFieldString());
	GLWidget::addToStringTable("$(units)", m_doc->GetFieldUnits());
	GLWidget::addToStringTable("$(time)", m_doc->GetTimeValue());

	// We need this for rendering post docs
	glEnable(GL_COLOR_MATERIAL);

	Post::CGLModel* glm = m_doc->GetGLModel();

	CGLCamera& cam = *rc.m_cam;

	GLViewSettings& vs = rc.m_settings;

	glm->m_nrender = vs.m_nrender + 1;
	glm->m_bnorm = vs.m_bnorm;
	glm->m_scaleNormals = vs.m_scaleNormals;
	glm->m_doZSorting = vs.m_bzsorting;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// we need to update the tracking target before we position the camera
	if (m_btrack) UpdateTracking();

	cam.PositionInScene();

	glDisable(GL_CULL_FACE);

	// match the selection mode
	SelectionType selectionMode = SELECT_FE_ELEMS;
	switch (m_doc->GetItemMode())
	{
	case ITEM_MESH:
	case ITEM_ELEM: selectionMode = SELECT_FE_ELEMS; break;
	case ITEM_FACE: selectionMode = SELECT_FE_FACES; break;
	case ITEM_EDGE: selectionMode = SELECT_FE_EDGES; break;
	case ITEM_NODE: selectionMode = SELECT_FE_NODES; break;
	}
	glm->SetSelectionType(selectionMode);


	if (vs.m_bShadows)
	{
		BOX box = m_doc->GetBoundingBox();

		float a = vs.m_shadow_intensity;
		GLfloat shadow[] = { a, a, a, 1 };
		GLfloat zero[] = { 0, 0, 0, 1 };
		GLfloat ones[] = { 1,1,1,1 };
		GLfloat lp[4] = { 0 };

		glEnable(GL_STENCIL_TEST);

		float inf = box.Radius() * 100.f;

		vec3d lpv = to_vec3d(vs.m_light);

		quatd q = cam.GetOrientation();
		q.Inverse().RotateVector(lpv);

		lp[0] = lpv.x;
		lp[1] = lpv.y;
		lp[2] = lpv.z;

		// set coloring for shadows
		glLightfv(GL_LIGHT0, GL_DIFFUSE, shadow);
		glLightfv(GL_LIGHT0, GL_SPECULAR, zero);

		glStencilFunc(GL_ALWAYS, 0x00, 0xff);
		glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

		// render the scene
		glm->Render(rc);

		// Create mask in stencil buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);

		Post::FEPostModel* fem = glm->GetFSModel();
		glm->RenderShadows(fem, lpv, inf);

		glCullFace(GL_BACK);
		glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);

		glm->RenderShadows(fem, lpv, inf);

		// Render the scene in light
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		GLfloat d = vs.m_diffuse;
		GLfloat dv[4] = { d, d, d, 1.f };

		glLightfv(GL_LIGHT0, GL_DIFFUSE, dv);
		glLightfv(GL_LIGHT0, GL_SPECULAR, ones);

		glStencilFunc(GL_EQUAL, 0, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glDisable(GL_CULL_FACE);

		glClear(GL_DEPTH_BUFFER_BIT);
	}

	if (vs.m_use_environment_map) ActivateEnvironmentMap(engine);
	glm->Render(rc);
	if (vs.m_use_environment_map) DeactivateEnvironmentMap(engine);

	// update and render the tracking
	if (m_btrack)
	{
		glx::renderAxes(m_trackScale, m_trgPos, m_trgRot, GLColor(255, 0, 255));
	}

	// render the image data
	RenderImageData(rc);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// render the tags
	ClearTags();
	if (vs.m_bTags) RenderTags(rc);

	Post::CGLPlaneCutPlot::DisableClipPlanes();
}

void CGLPostScene::RenderImageData(CGLContext& rc)
{
	if (m_doc->IsValid() == false) return;
	for (int i = 0; i < m_doc->ImageModels(); ++i)
	{
		CImageModel* img = m_doc->GetImageModel(i);
		if (img->IsActive()) img->Render(rc);
	}
}

void CGLPostScene::RenderTags(CGLContext& rc)
{
	GLViewSettings& view = rc.m_settings;

	GObject* po = m_doc->GetPostObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();

	// create the tag array.
	// We add a tag for each selected item
	GLTAG tag;

	int mode = m_doc->GetItemMode();

	GLColor extcol(255, 255, 0);
	GLColor intcol(255, 0, 0);

	// process elements
	FESelection* currentSelection = m_doc->GetCurrentSelection();
	if ((view.m_ntagInfo > TagInfoOption::NO_TAG_INFO) && currentSelection)
	{
		FSLineMesh* mesh = nullptr;
		if (mode == ITEM_ELEM)
		{
			FEElementSelection* selection = dynamic_cast<FEElementSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSMesh* pm = selection->GetMesh(); mesh = pm;
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NE = selection->Count();
				for (int i = 0; i < NE; i++)
				{
					FEElement_& el = *selection->Element(i); assert(el.IsSelected());
					tag.r = pm->LocalToGlobal(pm->ElementCenter(el));
					tag.c = extcol;
					int nid = el.GetID();
					if (nid < 0) nid = selection->ElementIndex(i) + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "E%d", nid);
					AddTag(tag);

					if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
					{
						int ne = el.Nodes();
						for (int j = 0; j < ne; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
					}
				}
			}
		}

		// process faces
		if (mode == ITEM_FACE)
		{
			FEFaceSelection* selection = dynamic_cast<FEFaceSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSMeshBase* pm = selection->GetMesh(); mesh = pm;
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NF = selection->Count();
				for (int i = 0; i < NF; ++i)
				{
					FSFace& f = *selection->Face(i); assert(f.IsSelected());
					tag.r = pm->LocalToGlobal(pm->FaceCenter(f));
					tag.c = (f.IsExternal() ? extcol : intcol);
					int nid = f.GetID();
					if (nid < 0) nid = selection->FaceIndex(i) + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "F%d", nid);
					AddTag(tag);

					if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
					{
						int nf = f.Nodes();
						for (int j = 0; j < nf; ++j) pm->Node(f.n[j]).m_ntag = 1;
					}
				}
			}
		}

		// process edges
		if (mode == ITEM_EDGE)
		{
			FEEdgeSelection* selection = dynamic_cast<FEEdgeSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSLineMesh* pm = selection->GetMesh(); mesh = pm;
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NC = selection->Size();
				for (int i = 0; i < NC; i++)
				{
					FSEdge& edge = *selection->Edge(i);
					tag.r = pm->LocalToGlobal(pm->EdgeCenter(edge));
					tag.c = extcol;
					int nid = edge.GetID();
					if (nid < 0) nid = selection->EdgeIndex(i) + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "L%d", nid);
					AddTag(tag);

					if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES)
					{
						int ne = edge.Nodes();
						for (int j = 0; j < ne; ++j) pm->Node(edge.n[j]).m_ntag = 1;
					}
				}
			}
		}

		// process nodes
		if (mode == ITEM_NODE)
		{
			FENodeSelection* selection = dynamic_cast<FENodeSelection*>(currentSelection);
			if (selection && selection->Count())
			{
				FSLineMesh* pm = selection->GetMesh(); mesh = pm;
				if (view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) pm->TagAllNodes(0);
				int NN = selection->Size();
				for (int i = 0; i < NN; i++)
				{
					FSNode* node = selection->Node(i); assert(node);
					if (node)
					{
						tag.r = pm->LocalToGlobal(node->r);
						tag.c = (node->IsExterior() ? extcol : intcol);
						int nid = node->GetID();
						if (nid < 0) nid = selection->NodeIndex(i) + 1;
						snprintf(tag.sztag, sizeof tag.sztag, "N%d", nid);
						AddTag(tag);
					}
				}
			}
		}

		// add additional nodes
		if ((view.m_ntagInfo == TagInfoOption::TAG_ITEM_AND_NODES) && mesh)
		{
			int NN = mesh->Nodes();
			for (int i = 0; i < NN; i++)
			{
				FSNode& node = mesh->Node(i);
				if (node.m_ntag == 1)
				{
					tag.r = mesh->LocalToGlobal(node.r);
					tag.c = (node.IsExterior() ? extcol : intcol);
					int n = node.GetID();
					if (n < 0) n = i + 1;
					snprintf(tag.sztag, sizeof tag.sztag, "N%d", n);
					AddTag(tag);
				}
			}
		}
	}

	// render object labels
	if (view.m_showRigidLabels)
	{
		bool renderRB = view.m_brigid;
		bool renderRJ = view.m_bjoint;
		Post::FEPostModel* fem = m_doc->GetFSModel();
		for (int i = 0; i < fem->PointObjects(); ++i)
		{
			Post::FEPostModel::PointObject& ob = *fem->GetPointObject(i);
			if (ob.IsActive())
			{
				if (((ob.m_tag == 1) && renderRB) ||
					((ob.m_tag > 1) && renderRJ))
				{
					tag.r = ob.m_pos;
					tag.c = ob.Color();
					snprintf(tag.sztag, sizeof tag.sztag, ob.GetName().c_str());
					AddTag(tag);
				}
			}
		}

		for (int i = 0; i < fem->LineObjects(); ++i)
		{
			Post::FEPostModel::LineObject& ob = *fem->GetLineObject(i);
			if (ob.IsActive() && renderRJ)
			{
				vec3d a = ob.m_r1;
				vec3d b = ob.m_r2;

				tag.r = (a + b) * 0.5;
				tag.c = ob.Color();
				snprintf(tag.sztag, sizeof tag.sztag, ob.GetName().c_str());
				AddTag(tag);
			}
		}
	}
}

void CGLPostScene::UpdateTracking()
{
	if ((m_ntrack[0] >= 0) && (m_ntrack[1] >= 0) && (m_ntrack[2] >= 0))
	{
		// calculate new tracking position and orientation
		FSMeshBase* pm = m_doc->GetPostObject()->GetFEMesh();
		int* nt = m_ntrack;
		vec3d a = pm->Node(nt[0]).r;
		vec3d b = pm->Node(nt[1]).r;
		vec3d c = pm->Node(nt[2]).r;
		m_trgPos = a;

		vec3d e1 = (b - a);
		vec3d e3 = e1 ^ (c - a);
		vec3d e2 = e3 ^ e1;
		m_trackScale = e1.Length();
		e1.Normalize();
		e2.Normalize();
		e3.Normalize();
		mat3d Q = mat3d(e1, e2, e3);
		m_trgRot = quatd(Q);

		// update camera's position and orientation
		CGLCamera& cam = GetCamera();
		quatd currentRot = cam.GetOrientation();
		quatd q0 = currentRot*m_trgRotDelta.Inverse();

		m_trgRotDelta = m_trgRot0 * m_trgRot.Inverse();

		quatd R = q0*m_trgRotDelta;

		cam.SetOrientation(R);
		cam.SetTarget(m_trgPos);
		cam.Update(true);
	}
	else m_btrack = false;
}

void CGLPostScene::ToggleTrackSelection()
{
	CGLCamera& cam = GetCamera();

	if (m_btrack)
	{
		m_btrack = false;
	}
	else
	{
		m_btrack = false;

		Post::CGLModel* model = m_doc->GetGLModel(); assert(model);
		CPostObject* po = model->GetPostObject(); assert(po);

		int m[3] = { -1, -1, -1 };
		int nmode = model->GetSelectionType();
		FSMeshBase* pm = po->GetFEMesh();
		if (nmode == SELECT_FE_ELEMS)
		{
			FEElementSelection* selElems = dynamic_cast<FEElementSelection*>(m_doc->GetCurrentSelection());
			if (selElems && (selElems->Size() > 0))
			{
				FEElement_& el = *selElems->Element(0);
				int* n = el.m_node;
				m[0] = n[0]; m[1] = n[1]; m[2] = n[2];
				m_btrack = true;
			}
		}
		else if (nmode == SELECT_FE_NODES)
		{
			FENodeSelection* selNodes = dynamic_cast<FENodeSelection*>(m_doc->GetCurrentSelection());
			if (selNodes && (selNodes->Count() >= 3))
			{
				for (int i = 0; i < 3; ++i) m[i] = selNodes->NodeIndex(i);
				m_btrack = true;
			}
		}

		if (m_btrack)
		{
			// store the nodes to track
			m_ntrack[0] = m[0];
			m_ntrack[1] = m[1];
			m_ntrack[2] = m[2];

			// get the current nodal positions
			FSMeshBase* pm = po->GetFEMesh();
			int NN = pm->Nodes();
			int* nt = m_ntrack;
			if ((nt[0] >= NN) || (nt[1] >= NN) || (nt[2] >= NN)) { assert(false); return; }

			vec3d a = pm->Node(nt[0]).r;
			vec3d b = pm->Node(nt[1]).r;
			vec3d c = pm->Node(nt[2]).r;

			// setup orthogonal basis
			vec3d e1 = (b - a);
			vec3d e3 = e1 ^ (c - a);
			vec3d e2 = e3 ^ e1;
			e1.Normalize();
			e2.Normalize();
			e3.Normalize();

			// create matrix form
			mat3d Q(e1, e2, e3);

			// store as quat
			m_trgRot0 = Q;
			m_trgRot = Q;
			m_trgRotDelta = quatd(0, vec3d(0, 0, 1));
		}
	}
}
