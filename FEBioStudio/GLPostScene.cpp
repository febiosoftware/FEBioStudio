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
#include "PostObject.h"

CGLPostScene::CGLPostScene(CPostDocument* doc) : m_doc(doc)
{

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

	glview->PositionCamera();

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

	glm->Render(rc);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// render the tracking
	if (glview->TrackModeActive()) glview->RenderTrack();

	// render the tags
	GLViewSettings& view = glview->GetViewSettings();
	if (view.m_bTags) glview->RenderTags();

	Post::CGLPlaneCutPlot::DisableClipPlanes();

	// render the image data
	glview->RenderImageData();

	// render the decorations
	glview->RenderDecorations();
}
