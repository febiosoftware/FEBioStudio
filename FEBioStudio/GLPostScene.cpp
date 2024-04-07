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
#include "GLView.h"
#include "PostDocument.h"
#include <PostGL/GLModel.h>
#include <PostGL/GLPlaneCutPlot.h>
#include <GLLib/glx.h>
#include "PostObject.h"

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

void CGLPostScene::Render(CGLContext& rc)
{
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return;

	CGLView* glview = rc.m_view; assert(glview);
	if (glview == nullptr) return;

	// Update GLWidget string table for post rendering
	GLWidget::addToStringTable("$(filename)", m_doc->GetDocFileName());
	GLWidget::addToStringTable("$(datafield)", m_doc->GetFieldString());
	GLWidget::addToStringTable("$(units)", m_doc->GetFieldUnits());
	GLWidget::addToStringTable("$(time)", m_doc->GetTimeValue());

	// We need this for rendering post docs
	glEnable(GL_COLOR_MATERIAL);

	Post::CGLModel* glm = m_doc->GetGLModel();

	CGLCamera& cam = *rc.m_cam;

	GLViewSettings& vs = glview->GetViewSettings();

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
	int selectionMode = Post::SELECT_ELEMS;
	switch (m_doc->GetItemMode())
	{
	case ITEM_MESH:
	case ITEM_ELEM: selectionMode = Post::SELECT_ELEMS; break;
	case ITEM_FACE: selectionMode = Post::SELECT_FACES; break;
	case ITEM_EDGE: selectionMode = Post::SELECT_EDGES; break;
	case ITEM_NODE: selectionMode = Post::SELECT_NODES; break;
	}
	glm->SetSelectionMode(selectionMode);


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

		vec3d lpv = to_vec3d(glview->GetLightPosition());

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

	GLViewSettings& view = glview->GetViewSettings();
	if (view.m_use_environment_map) ActivateEnvironmentMap();
	glm->Render(rc);
	if (view.m_use_environment_map) DeactivateEnvironmentMap();

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
	if (view.m_bTags) glview->RenderTags();

	Post::CGLPlaneCutPlot::DisableClipPlanes();

	// render the decorations
	glview->RenderDecorations();
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

		CPostObject* po = m_doc->GetPostObject();
		Post::CGLModel* model = m_doc->GetGLModel(); assert(model);

		int m[3] = { -1, -1, -1 };
		int nmode = model->GetSelectionMode();
		FSMeshBase* pm = po->GetFEMesh();
		if (nmode == Post::SELECT_ELEMS)
		{
			const vector<FEElement_*> selElems = model->GetElementSelection();
			if (selElems.size() > 0)
			{
				FEElement_& el = *selElems[0];
				int* n = el.m_node;
				m[0] = n[0]; m[1] = n[1]; m[2] = n[2];
				m_btrack = true;
			}
		}
		else if (nmode == Post::SELECT_NODES)
		{
			int ns = 0;
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				if (pm->Node(i).IsSelected()) m[ns++] = i;
				if (ns == 3)
				{
					m_btrack = true;
					break;
				}
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
