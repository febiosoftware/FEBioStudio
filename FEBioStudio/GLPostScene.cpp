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
#include "GLModelDocument.h"
#include <GLLib/GLContext.h>
#include <PostGL/GLModel.h>
#include <PostGL/GLPlaneCutPlot.h>
#include <GLLib/glx.h>
#include <MeshTools/FESelection.h>
#include <PostGL/PostObject.h>

#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLMirrorPlane.h>
#include <PostGL/GLModel.h>

using namespace Post;

void GLPostPlaneCutItem::render(GLRenderEngine& re, CGLContext& rc)
{
	CGLModel& gm = *m_scene->GetGLModel();
	GPlotList& PL = gm.GetPlotList();

	// clear all clipping planes
	CGLPlaneCutPlot::ClearClipPlanes();
	for (int i = 0; i < (int)PL.Size(); ++i)
	{
		CGLPlaneCutPlot* p = dynamic_cast<CGLPlaneCutPlot*>(PL[i]);
		if (p && p->IsActive()) p->Activate(true);
	}

	Post::CGLPlaneCutPlot::EnableClipPlanes();

	GLCompositeSceneItem::render(re, rc);

	Post::CGLPlaneCutPlot::DisableClipPlanes();
}

void GLPostMirrorItem::render(GLRenderEngine& re, CGLContext& rc)
{
	int frontFace;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);

	re.pushTransform();
	renderMirror(re, rc, 0, CGLMirrorPlane::MAX_MIRROR_PLANES);
	re.popTransform();

	glFrontFace(frontFace);
}

void GLPostMirrorItem::renderMirror(GLRenderEngine& re, CGLContext& rc, int start, int end)
{
	// pass one
	GLCompositeSceneItem::render(re, rc);

	for (int i = start; i < end; ++i)
	{
		Post::CGLMirrorPlane* plane = CGLMirrorPlane::GetMirrorPlane(i);
		if (plane && plane->IsActive())
		{
			// plane normal
			vec3f scl, norm;
			switch (plane->m_plane)
			{
			case 0: norm = vec3f(1.f, 0.f, 0.f); scl = vec3f(-1.f, 1.f, 1.f); break;
			case 1: norm = vec3f(0.f, 1.f, 0.f); scl = vec3f(1.f, -1.f, 1.f); break;
			case 2: norm = vec3f(0.f, 0.f, 1.f); scl = vec3f(1.f, 1.f, -1.f); break;
			}

			float offset = plane->m_offset;

			glPushMatrix();
			glTranslatef(-offset * norm.x, -offset * norm.y, -offset * norm.z);
			glScalef(scl.x, scl.y, scl.z);

			int frontFace;
			glGetIntegerv(GL_FRONT_FACE, &frontFace);
			glFrontFace(frontFace == GL_CW ? GL_CCW : GL_CW);

			renderMirror(re, rc, 0, i);
			glFrontFace(frontFace);
			glPopMatrix();
		}
	}
}

void GLPostModelItem::render(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel* glm = m_scene->GetGLModel();
	if (glm == nullptr) return;

	GLViewSettings& vs = rc.m_settings;

	glm->m_nrender = vs.m_nrender + 1;
	glm->m_bnorm = vs.m_bnorm;
	glm->m_scaleNormals = vs.m_scaleNormals;
	glm->m_doZSorting = vs.m_bzsorting;

	glDisable(GL_CULL_FACE);

	// match the selection mode
	SelectionType selectionMode = SELECT_FE_ELEMS;
	switch (m_scene->GetItemMode())
	{
	case ITEM_MESH:
	case ITEM_ELEM: selectionMode = SELECT_FE_ELEMS; break;
	case ITEM_FACE: selectionMode = SELECT_FE_FACES; break;
	case ITEM_EDGE: selectionMode = SELECT_FE_EDGES; break;
	case ITEM_NODE: selectionMode = SELECT_FE_NODES; break;
	}
	glm->SetSelectionType(selectionMode);

	CGLPlaneCutPlot::EnableClipPlanes();

	RenderModel(re, rc);

	// Render discrete elements
	RenderDiscrete(rc);

	// render min/max markers
	Post::CGLColorMap* pcm = glm->GetColorMap();
	if (pcm && pcm->ShowMinMaxMarkers())
	{
		RenderMinMaxMarkers(rc);
	}

	CGLPlaneCutPlot::DisableClipPlanes();
}

void GLPostModelItem::RenderModel(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	// set the render interior nodes flag
	glm.RenderInteriorNodes(rc.m_settings.m_bext == false);

	// get the FE model
	FEPostModel* fem = m_scene->GetFSModel();

	glm.m_bshowMesh = rc.m_settings.m_bmesh;

	int mode = glm.GetSelectionType();

	// render the faces
	if (mode == SELECT_FE_FACES)
	{
		RenderFaces(re, rc);
	}
	else if (mode == SELECT_FE_ELEMS)
	{
		RenderElems(re, rc);
	}
	else
	{
		// for nodes, edges, draw the faces as well
		RenderFaces(re, rc);
	}

	// render outline
	if (rc.m_settings.m_bfeat)
	{
		rc.m_cam->LineDrawMode(true);
		RenderOutline(re, rc);
		rc.m_cam->LineDrawMode(false);
	}

	// render mesh lines
	if (glm.m_bshowMesh && (glm.GetSelectionType() != SELECT_FE_EDGES))
	{
		RenderMeshLines(re, rc);
	}

	// render the selected elements and faces
	RenderSelection(re, rc);

	// render the normals
	if (glm.m_bnorm) RenderNormals(re, rc);

	// render the ghost
	if (glm.m_bghost) RenderGhost(re, rc);

	// render the edges
	if (mode == SELECT_FE_EDGES)
	{
		rc.m_cam->LineDrawMode(true);
		RenderEdges(re, rc);
		rc.m_cam->LineDrawMode(false);
	}

	// render the nodes
	if (mode == SELECT_FE_NODES)
	{
		rc.m_cam->LineDrawMode(true);
		RenderNodes(re, rc);
		rc.m_cam->LineDrawMode(false);
	}
}

void GLPostModelItem::RenderNodes(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	Post::FEPostMesh* pm = glm.GetActiveMesh();

	GMesh* gm = glm.GetPostObject()->GetFERenderMesh();

	const int VISIBLE_FLAG = 1;
	const int SELECT_FLAG = 2;

	// reset tags and check visibility
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& n = pm->Node(i);
		GMesh::NODE& gn = gm->Node(i);
		gn.tag = 0;
		if (n.IsVisible())
		{
			if (n.IsExterior() || glm.m_brenderInteriorNodes) gn.tag = VISIBLE_FLAG;
			if (n.IsSelected()) gn.tag |= SELECT_FLAG;
		}
	}

	// see if backface-culling is enabled or not
	GLboolean bcull;
	glGetBooleanv(GL_CULL_FACE, &bcull);
	if (bcull)
	{
		quatd q = rc.m_cam->GetOrientation();
		vec3f f;
		int NF = pm->Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = pm->Face(i);
			int n = face.Nodes();
			for (int j = 0; j < n; ++j)
			{
				vec3d f = to_vec3d(face.m_nn[j]);
				q.RotateVector(f);
				if (f.z < 0) gm->Node(face.n[j]).tag = 0;
			}
		}
	}

	GMesh pointMesh;
	for (int i = 0; i < gm->Nodes(); ++i)
	{
		GMesh::NODE& gn = gm->Node(i);
		if (gn.tag & VISIBLE_FLAG)
		{
			pointMesh.AddNode(gn.r);
		}
	}

	float fsize = rc.m_settings.m_node_size;
	re.setPointSize(fsize);

	// render all unselected tagged nodes
	GLColor c = glm.m_node_col; c.a = 128;
	re.setMaterial(GLMaterial::CONSTANT, c);
	re.renderGMeshNodes(pointMesh, false);

	// render selected tagged nodes
	if (glm.GetSelectionType() == SELECT_FE_NODES)
	{
		FENodeSelection* sel = dynamic_cast<FENodeSelection*>(m_scene->GetSelection());
		if (sel && sel->Count())
		{
			GMesh selMesh;
			const std::vector<int>& items = sel->Items();
			for (int n : items)
			{
				selMesh.AddNode(gm->Node(n).r);
			}
			re.setMaterial(GLMaterial::OVERLAY, GLColor::Yellow());
			re.renderGMeshNodes(selMesh, false);
		}
	}
}

void GLPostModelItem::RenderEdges(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	Post::FEPostMesh& mesh = *glm.GetActiveMesh();
	int NE = mesh.Edges();
	if (NE == 0) return;

	GMesh lineMesh;
	vec3f r[3];
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.IsVisible() && (edge.IsSelected() == false))
		{
			r[0] = to_vec3f(mesh.Node(edge.n[0]).r);
			r[1] = to_vec3f(mesh.Node(edge.n[1]).r);

			switch (edge.Type())
			{
			case FE_EDGE2:
				lineMesh.AddEdge(r, 2, edge.m_gid);
				break;
			case FE_EDGE3:
				r[2] = to_vec3f(mesh.Node(edge.n[2]).r);
				lineMesh.AddEdge(r, 3, edge.m_gid);
				break;
			}
		}
	}
	lineMesh.Update();

	re.setMaterial(GLMaterial::CONSTANT, GLColor::Blue());
	re.renderGMeshEdges(lineMesh, false);

	// render selected edges
	if (glm.GetSelectionType() == SELECT_FE_EDGES)
	{
		FEEdgeSelection* sel = dynamic_cast<FEEdgeSelection*>(m_scene->GetSelection());
		if (sel && sel->Count())
		{
			GMesh selMesh;
			int n = sel->Count();
			for (int i = 0; i < n; ++i)
			{
				FSEdge& edge = *sel->Edge(i);
				r[0] = to_vec3f(mesh.Node(edge.n[0]).r);
				r[1] = to_vec3f(mesh.Node(edge.n[1]).r);

				switch (edge.Type())
				{
				case FE_EDGE2:
					selMesh.AddEdge(r, 2, edge.m_gid);
					break;
				case FE_EDGE3:
					r[2] = to_vec3f(mesh.Node(edge.n[2]).r);
					selMesh.AddEdge(r, 3, edge.m_gid);
					break;
				}
			}
			selMesh.Update();

			re.setMaterial(GLMaterial::OVERLAY, GLColor::Yellow());
			re.renderGMeshEdges(lineMesh, false);
		}
	}
}

void GLPostModelItem::RenderFaces(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();
	Post::FEPostModel* ps = m_scene->GetFSModel();
	bool colorMapEnabled = glm.GetColorMap()->IsActive();

	CPostObject* po = glm.GetPostObject();
	GMesh* mesh = po->GetFERenderMesh();
	if (mesh == nullptr) return;

	for (int i = 0; i < po->Faces(); ++i)
	{
		const GMesh::PARTITION& p = mesh->Partition(i);
		if (p.nf > 0)
		{
			int n0 = p.n0;
			int matID = mesh->Face(n0).mid;

			Post::Material& mat = *ps->GetMaterial(matID);
			if (mat.bvisible)
			{
				if (colorMapEnabled)
				{
					if (mat.benable)
					{
						float alpha = mat.transparency;
						GLColor c = GLColor::White();
						c.a = (uint8_t)(255.f * alpha);
						re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::TEXTURE_1D);
						glm.m_pcol->GetColorMap()->GetTexture().MakeCurrent();
					}
					else
					{
						float alpha = mat.transparency;
						GLColor c = glm.m_pcol->GetInactiveColor();
						c.a = (uint8_t)(255.f * alpha);
						re.setMaterial(GLMaterial::PLASTIC, c);
					}
				}
				else
				{
					float alpha = mat.transparency;
					GLColor c = mat.diffuse;
					c.a = (uint8_t)(255.f * alpha);
					re.setMaterial(GLMaterial::PLASTIC, c);
				}

				re.renderGMesh(*mesh, i, false);
			}
		}
	}
}

// TODO: This is identical to RenderFaces, except that we loop over all the GMesh partitions
//       Maybe I can combine these two functions.
void GLPostModelItem::RenderElems(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();
	Post::FEPostModel* ps = m_scene->GetFSModel();
	bool colorMapEnabled = glm.GetColorMap()->IsActive();

	CPostObject* po = glm.GetPostObject();
	GMesh* mesh = po->GetFERenderMesh();
	if (mesh == nullptr) return;

	for (int i = 0; i < mesh->Partitions(); ++i)
	{
		const GMesh::PARTITION& p = mesh->Partition(i);
		if (p.nf > 0)
		{
			int n0 = p.n0;
			int matID = mesh->Face(n0).mid;

			Post::Material& mat = *ps->GetMaterial(matID);
			if (mat.bvisible)
			{
				if (colorMapEnabled)
				{
					if (mat.benable)
					{
						float alpha = mat.transparency;
						GLColor c = GLColor::White();
						c.a = (uint8_t)(255.f * alpha);
						re.setMaterial(GLMaterial::PLASTIC, c, GLMaterial::TEXTURE_1D);
						glm.m_pcol->GetColorMap()->GetTexture().MakeCurrent();
					}
					else
					{
						float alpha = mat.transparency;
						GLColor c = glm.m_pcol->GetInactiveColor();
						c.a = (uint8_t)(255.f * alpha);
						re.setMaterial(GLMaterial::PLASTIC, c);
					}
				}
				else
				{
					float alpha = mat.transparency;
					GLColor c = mat.diffuse;
					c.a = (uint8_t)(255.f * alpha);
					re.setMaterial(GLMaterial::PLASTIC, c);
				}

				re.renderGMesh(*mesh, i, false);
			}
		}
	}
}

void GLPostModelItem::RenderSelection(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	// render the selection surface
	re.setMaterial(GLMaterial::OVERLAY, glm.m_sel_col);
	re.renderGMesh(glm.m_selectionMesh, false);

	// render the selection outlines
	re.setColor(GLColor::Yellow());
	re.renderGMeshEdges(glm.m_selectionMesh, false);
}

void GLPostModelItem::RenderNormals(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	// get the mesh
	FSMeshBase* pm = glm.GetActiveMesh();
	if (pm == 0) return;	

	double R = 0.05 * pm->GetBoundingBox().GetMaxExtent() * glm.m_scaleNormals;

	GMesh lineMesh;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		const FSFace& face = pm->Face(i);

		vec3f p[2];
		p[0] = vec3f(0, 0, 0);
		vec3f fn = face.m_fn;

		vec3d c(0, 0, 0);
		int nf = face.Nodes();
		for (int j = 0; j < nf; ++j) c += pm->Node(face.n[j]).r;
		c /= nf;

		p[0] = to_vec3f(c);
		p[1] = p[0] + fn * R;

		lineMesh.AddEdge(p, 2);

		float r = fabs(fn.x);
		float g = fabs(fn.y);
		float b = fabs(fn.z);

		GMesh::EDGE& edge = lineMesh.Edge(lineMesh.Edges() - 1);
		edge.c[0] = GLColor::White();
		edge.c[1] = GLColor::FromRGBf(r, g, b);
	}

	re.setMaterial(GLMaterial::CONSTANT, GLColor::White(), GLMaterial::VERTEX_COLOR);
	re.renderGMeshEdges(lineMesh, false);
}

void GLPostModelItem::RenderGhost(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	FEPostModel* ps = m_scene->GetFSModel();
	FSMeshBase* pm = glm.GetActiveMesh();
	Post::FERefState* ref = glm.GetActiveState()->m_ref;

	quatd q = rc.m_cam->GetOrientation();

	double eps = cos(glm.GetSmoothingAngleRadians());

	GMesh lineMesh;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.IsVisible())
		{
			int n = f.Edges();
			for (int j = 0; j < n; ++j)
			{
				bool bdraw = false;

				if (f.m_nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					FSFace& f2 = pm->Face(f.m_nbr[j]);
					if (f.m_gid != f2.m_gid)
					{
						bdraw = true;
					}
					else if (f.m_fn * f2.m_fn <= eps)
					{
						bdraw = true;
					}
					else
					{
						vec3d n1 = to_vec3d(f.m_fn);
						vec3d n2 = to_vec3d(f2.m_fn);
						q.RotateVector(n1);
						q.RotateVector(n2);
						if (n1.z * n2.z <= 0)
						{
							bdraw = true;
						}
					}
				}

				if (bdraw)
				{
					int a = f.n[j];
					int b = f.n[(j + 1) % n];

					if (a > b) { a ^= b; b ^= a; a ^= b; }

					vec3f r[2];
					r[0] = ref->m_Node[a].m_rt;
					r[1] = ref->m_Node[b].m_rt;

					lineMesh.AddEdge(r, 2);
				}
			}
		}
	}

	re.setMaterial(GLMaterial::CONSTANT, glm.m_ghost_color);
	re.renderGMeshEdges(lineMesh, false);
}

void GLPostModelItem::RenderOutline(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	FEPostModel* ps = m_scene->GetFSModel();
	CPostObject* po = glm.GetPostObject();
	if (po == nullptr) return;
	GMesh* pm = po->GetFERenderMesh();
	if (pm == nullptr) return;

	Transform T;
	re.setMaterial(GLMaterial::CONSTANT, glm.m_line_col);
	for (int i = 0; i < pm->Partitions(); ++i)
	{
		re.renderGMeshOutline(*rc.m_cam, *pm, T, i);
	}
}

void GLPostModelItem::RenderMeshLines(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& glm = *m_scene->GetGLModel();

	CPostObject* po = glm.GetPostObject();
	if (po == nullptr) return;

	GMesh* mesh = po->GetFERenderMesh();
	if (mesh == nullptr) return;

	GLColor c = rc.m_settings.m_meshColor;
	c.a = 128;
	re.setMaterial(GLMaterial::CONSTANT, c);
	re.renderGMeshEdges(*mesh, false);
}

void GLPostModelItem::RenderDiscrete(CGLContext& rc)
{
	Post::CGLModel& gm = *m_scene->GetGLModel();
	if (gm.ShowBeam2Solid())
	{
		RenderDiscreteAsSolid(rc);
	}
	else
	{
		RenderDiscreteAsLines(rc);
	}
}

void GLPostModelItem::RenderDiscreteAsLines(CGLContext& rc)
{
	Post::FEPostModel* ps = m_scene->GetFSModel();
	if (ps == nullptr) return;

	float lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(rc.m_settings.m_line_size);

	Post::CGLModel& gm = *m_scene->GetGLModel();
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	Post::FEPostMesh& mesh = *gm.GetActiveMesh();
	int curMat = -1;
	bool bvisible = true;

	// render un-selected, active elements
	Post::CGLColorMap* colmap = gm.GetColorMap();

	if (colmap && colmap->IsActive())
	{
		colmap->GetColorMap()->GetTexture().MakeCurrent();

		glEnable(GL_TEXTURE_1D);

		glColor3ub(255, 255, 255);
		glBegin(GL_LINES);
		for (int i = 0; i < gm.DiscreteEdges(); ++i)
		{
			GLEdge::EDGE& edge = gm.DiscreteEdge(i);
			FEElement_* pe = mesh.ElementPtr(edge.elem);
			if (pe && !pe->IsSelected() && pe->IsVisible())
			{
				int mat = edge.mat;
				if (mat != curMat)
				{
					Material* pmat = ps->GetMaterial(mat);
					curMat = mat;
					bvisible = pmat->bvisible;
					if (!pmat->benable) bvisible = false;
				}

				if (bvisible) RenderDiscreteElement(i);
			}
		}
		glEnd();
	}

	// turn-off texturing for the rest
	glDisable(GL_TEXTURE_1D);

	// loop over un-selected, inactive elements
	curMat = -1;
	glBegin(GL_LINES);
	for (int i = 0; i < gm.DiscreteEdges(); ++i)
	{
		GLEdge::EDGE& edge = gm.DiscreteEdge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && !pe->IsSelected() && pe->IsVisible())
		{
			int mat = edge.mat;
			if (mat != curMat)
			{
				Material* pmat = ps->GetMaterial(mat);
				GLColor c = pmat->diffuse;
				glColor3ub(c.r, c.g, c.b);
				curMat = mat;
				bvisible = pmat->bvisible;
				if (colmap->IsActive() && pmat->benable) bvisible = false;
			}

			if (bvisible) RenderDiscreteElement(i);
		}
	}
	glEnd();

	// loop over selected elements
	glColor3ub(255, 0, 0);
	glBegin(GL_LINES);
	for (int i = 0; i < gm.DiscreteEdges(); ++i)
	{
		GLEdge::EDGE& edge = gm.DiscreteEdge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && pe->IsSelected() && pe->IsVisible())
		{
			RenderDiscreteElement(i);
		}
	}
	glEnd();

	glPopAttrib();

	glLineWidth(lineWidth);
}

void GLPostModelItem::RenderDiscreteAsSolid(CGLContext& rc)
{
	Post::FEPostModel* ps = m_scene->GetFSModel();
	if (ps == nullptr) return;
	Post::CGLModel& gm = *m_scene->GetGLModel();

	glPushAttrib(GL_ENABLE_BIT);
	Post::FEPostMesh& mesh = *gm.GetActiveMesh();
	int curMat = -1;
	bool bvisible = true;

	// find the shortest edge (that's not zero)
	double L2min = 0.0;
	for (int i = 0; i < gm.DiscreteEdges(); ++i)
	{
		GLEdge::EDGE& edge = gm.DiscreteEdge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe)
		{
			vec3d r0 = mesh.Node(edge.n0).r;
			vec3d r1 = mesh.Node(edge.n1).r;
			double L = (r1 - r0).norm2();

			if ((L2min == 0.0) || (L < L2min))
			{
				L2min = L;
			}
		}
	}
	if (L2min == 0.0) return;
	double Lmin = sqrt(L2min);

	double f = gm.SolidBeamRadius();
	double W = f;// Lmin * 0.25 * f;

	// render un-selected, active elements
	Post::CGLColorMap* colmap = gm.GetColorMap();
	if (colmap->IsActive())
	{
		colmap->GetColorMap()->GetTexture().MakeCurrent();
		glEnable(GL_TEXTURE_1D);

		glColor3ub(255, 255, 255);
		for (int i = 0; i < gm.DiscreteEdges(); ++i)
		{
			GLEdge::EDGE& edge = gm.DiscreteEdge(i);
			FEElement_* pe = mesh.ElementPtr(edge.elem);
			if (pe && !pe->IsSelected() && pe->IsVisible())
			{
				int mat = edge.mat;
				if (mat != curMat)
				{
					Material* pmat = ps->GetMaterial(mat);
					curMat = mat;
					bvisible = pmat->bvisible;
					if (!pmat->benable) bvisible = false;
				}

				if (bvisible) RenderDiscreteElementAsSolid(i, W);
			}
		}
	}

	// turn-off texturing for the rest
	glDisable(GL_TEXTURE_1D);
	glEnable(GL_CULL_FACE);

	// loop over un-selected, inactive elements, non-transparent
	curMat = -1;
	for (int i = 0; i < gm.DiscreteEdges(); ++i)
	{
		GLEdge::EDGE& edge = gm.DiscreteEdge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && !pe->IsSelected() && pe->IsVisible())
		{
			int mat = edge.mat;
			if (mat != curMat)
			{
				Material* pmat = ps->GetMaterial(mat);
				bvisible = pmat->bvisible && (pmat->transparency > 0.999f);
				GLColor c = pmat->diffuse;
				unsigned char a = (unsigned char)(255.f * pmat->transparency);
				glColor4ub(c.r, c.g, c.b, a);
				curMat = mat;
				if (colmap->IsActive() && pmat->benable) bvisible = false;
			}

			if (bvisible) RenderDiscreteElementAsSolid(i, W);
		}
	}

	// loop over un-selected, inactive elements, transparent
	curMat = -1;
	for (int i = 0; i < gm.DiscreteEdges(); ++i)
	{
		GLEdge::EDGE& edge = gm.DiscreteEdge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && !pe->IsSelected() && pe->IsVisible())
		{
			int mat = edge.mat;
			if (mat != curMat)
			{
				Material* pmat = ps->GetMaterial(mat);
				bvisible = pmat->bvisible && (pmat->transparency < 0.999f);
				GLColor c = pmat->diffuse;
				unsigned char a = (unsigned char)(255.f * pmat->transparency);
				glColor4ub(c.r, c.g, c.b, a);
				curMat = mat;
				if (colmap->IsActive() && pmat->benable) bvisible = false;
			}

			if (bvisible)
			{
				vec3d r0 = mesh.Node(edge.n0).r;
				vec3d r1 = mesh.Node(edge.n1).r;
				glx::drawCappedCylinder(r0, r1, W);
			}
		}
	}


	// loop over selected elements
	glColor3ub(255, 0, 0);
	for (int i = 0; i < gm.DiscreteEdges(); ++i)
	{
		GLEdge::EDGE& edge = gm.DiscreteEdge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && pe->IsSelected() && pe->IsVisible())
		{
			RenderDiscreteElementAsSolid(i, W);
		}
	}

	glPopAttrib();
}

void GLPostModelItem::RenderDiscreteElement(int i)
{
	Post::CGLModel& gm = *m_scene->GetGLModel();
	GLEdge::EDGE& edge = gm.DiscreteEdge(i);

	Post::FEPostMesh& mesh = *gm.GetActiveMesh();
	FEElement_* pe = mesh.ElementPtr(edge.elem);
	if (pe == nullptr) return;

	if (pe->Type() == FE_BEAM2)
	{
		float t0 = edge.tex[0];
		float t1 = edge.tex[1];
		vec3d r0 = mesh.Node(edge.n0).r;
		vec3d r1 = mesh.Node(edge.n1).r;
		glTexCoord1d(t0); glVertex3d(r0.x, r0.y, r0.z);
		glTexCoord1d(t1); glVertex3d(r1.x, r1.y, r1.z);
	}
	else if (pe->Type() == FE_BEAM3)
	{
		vec3d r[3];
		r[0] = mesh.Node(pe->m_node[0]).r;
		r[1] = mesh.Node(pe->m_node[1]).r;
		r[2] = mesh.Node(pe->m_node[2]).r;
		float t[3];
		t[0] = edge.tex[0];
		t[1] = edge.tex[1];
		t[2] = 0.5f * (t[0] + t[1]);
		vec3d rp = r[0];
		float tp = t[0];
		const int NDIV = 12;
		for (int n = 1; n <= NDIV; ++n)
		{
			float w = -1.f + (n / (float)NDIV) * 2.f;
			float H[3] = { 0.5f * w * (w - 1.f), 0.5f * w * (w + 1.f), 1.f - w * w };
			vec3d rn = r[0] * H[0] + r[1] * H[1] + r[2] * H[2];
			float tn = t[0] * H[0] + t[1] * H[1] + t[2] * H[2];

			glTexCoord1d(tp); glVertex3d(rp.x, rp.y, rp.z);
			glTexCoord1d(tn); glVertex3d(rn.x, rn.y, rn.z);

			rp = rn;
			tp = tn;
		}
	}
}

void GLPostModelItem::RenderDiscreteElementAsSolid(int i, double W)
{
	Post::CGLModel& gm = *m_scene->GetGLModel();
	GLEdge::EDGE& edge = gm.DiscreteEdge(i);

	Post::FEPostMesh& mesh = *gm.GetActiveMesh();
	FEElement_* pe = mesh.ElementPtr(edge.elem);
	if (pe == nullptr) return;

	if (pe->Type() == FE_BEAM2)
	{
		vec3d r0 = mesh.Node(edge.n0).r;
		vec3d r1 = mesh.Node(edge.n1).r;
		float t0 = edge.tex[0];
		float t1 = edge.tex[1];

		int leftCap = (pe->m_nbr[0] == -1 ? 1 : 0);
		int rightCap = (pe->m_nbr[1] == -1 ? 1 : 0);

		glx::drawCappedCylinder(r0, r1, W, t0, t1, 16, leftCap, rightCap);
	}
	else if (pe->Type() == FE_BEAM3)
	{
		vec3d r[3];
		r[0] = mesh.Node(pe->m_node[0]).r;
		r[1] = mesh.Node(pe->m_node[1]).r;
		r[2] = mesh.Node(pe->m_node[2]).r;
		const int NDIV = 12;
		vector<vec3d> p(NDIV + 1);
		for (int n = 0; n <= NDIV; ++n)
		{
			float w = -1.f + (n / (float)NDIV) * 2.f;
			float H[3] = { 0.5f * w * (w - 1.f), 0.5f * w * (w + 1.f), 1.f - w * w };
			p[n] = r[0] * H[0] + r[1] * H[1] + r[2] * H[2];
		}

		int leftCap = (pe->m_nbr[0] == -1 ? 1 : 0);
		int rightCap = (pe->m_nbr[1] == -1 ? 1 : 0);

		glx::drawSmoothPath(p, W, edge.tex[0], edge.tex[1], leftCap, rightCap);
	}
}

void GLPostModelItem::RenderMinMaxMarkers(CGLContext& rc)
{
	Post::CGLModel& gm = *m_scene->GetGLModel();

	Post::CGLColorMap* pcm = gm.GetColorMap();
	if ((pcm == nullptr) || (pcm->IsActive() == false)) return;

	vec3d rmin = pcm->GetMinPosition();
	vec3d rmax = pcm->GetMaxPosition();

	Post::CColorTexture* tex = pcm->GetColorMap();
	CColorMap& map = tex->ColorMap();

	GLColor c0 = map.GetColor(0);
	GLColor c1 = map.GetColor(map.Colors() - 1);

	// TODO: Can I render this as tags instead of here

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	float pointSize;
	glGetFloatv(GL_POINT_SIZE, &pointSize);

	glPointSize(15.f);
	glBegin(GL_POINTS);
	glColor3ub(255, 255, 255);
	glVertex3d(rmin.x, rmin.y, rmin.z);
	glVertex3d(rmax.x, rmax.y, rmax.z);
	glEnd();

	glPointSize(10.f);
	glBegin(GL_POINTS);
	glColor3ub(c0.r, c0.g, c0.b); glVertex3d(rmin.x, rmin.y, rmin.z);
	glColor3ub(c1.r, c1.g, c1.b); glVertex3d(rmax.x, rmax.y, rmax.z);
	glEnd();

	glPointSize(pointSize);

	glPopAttrib();
}

void GLPostPlotItem::render(GLRenderEngine& re, CGLContext& rc)
{
	Post::CGLModel& gm = *m_scene->GetGLModel();
	Post::GPlotList& plotList = gm.GetPlotList();
	for (int i = 0; i < plotList.Size(); ++i)
	{
		Post::CGLPlot& plt = *plotList[i];
		if (plt.IsActive())
		{
			if (plt.AllowClipping()) CGLPlaneCutPlot::EnableClipPlanes();
			else CGLPlaneCutPlot::DisableClipPlanes();

			plt.Render(re, rc);
		}
	}
	CGLPlaneCutPlot::DisableClipPlanes();
}

void GLPostObjectItem::render(GLRenderEngine& re, CGLContext& rc)
{
	Post::FEPostModel* fem = m_scene->GetFSModel();
	if (fem == nullptr) return;

	if ((fem->PointObjects() == 0) && (fem->LineObjects() == 0)) return;

	double scale = 0.05 * (double)rc.m_cam->GetTargetDistance();
	double R = 0.5 * scale;

	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	bool renderRB = rc.m_settings.m_brigid;
	bool renderRJ = rc.m_settings.m_bjoint;

	for (int i = 0; i < fem->PointObjects(); ++i)
	{
		Post::FEPostModel::PointObject& ob = *fem->GetPointObject(i);
		if (ob.IsActive())
		{
			glPushMatrix();
			glx::translate(ob.m_pos);
			glx::rotate(ob.m_rot);

			glx::translate(ob.m_rt);

			double size = R * ob.Scale();

			GLColor c = ob.Color();
			glColor3ub(c.r, c.g, c.b);
			switch (ob.m_tag)
			{
			case 1: if (renderRB) glx::renderRigidBody(size); break;
			case 2: if (renderRJ) glx::renderJoint(size); break;
			case 3: if (renderRJ) glx::renderJoint(size); break;
			case 4: if (renderRJ) glx::renderPrismaticJoint(size); break;
			case 5: if (renderRJ) glx::renderRevoluteJoint(size); break;
			case 6: if (renderRJ) glx::renderCylindricalJoint(size); break;
			case 7: if (renderRJ) glx::renderPlanarJoint(size); break;
			default:
				if (renderRB) glx::renderAxis(size);
			}
			glPopMatrix();
		}
	}

	for (int i = 0; i < fem->LineObjects(); ++i)
	{
		Post::FEPostModel::LineObject& ob = *fem->GetLineObject(i);
		if (ob.IsActive() && renderRJ)
		{
			glPushMatrix();
			glx::translate(ob.m_pos);
			glx::rotate(ob.m_rot);

			vec3d a = ob.m_r1;
			vec3d b = ob.m_r2;
			double Lt = sqrt((a - b) * (a - b));

			double L0 = sqrt((ob.m_r01 - ob.m_r02) * (ob.m_r01 - ob.m_r02));
			if (L0 == 0) L0 = Lt;

			GLColor c = ob.Color();
			glColor3ub(c.r, c.g, c.b);
			switch (ob.m_tag)
			{
			case 1: glx::renderSpring(a, b, R, (R == 0 ? 25 : L0 / R)); break;
			case 2: glx::renderDamper(a, b, R); break;
			case 4: glx::renderContractileForce(a, b, R); break;
			default:
				glx::drawLine(a, b);
			}
			glPopMatrix();
		}
	}

	glPopAttrib();
}

void GLPost3DImageItem::render(GLRenderEngine& re, CGLContext& rc)
{
	if (m_img && m_img->IsActive())
	{
		m_img->Render(re, rc);
	}
}

CGLPostScene::CGLPostScene(CGLModelDocument* doc) : m_doc(doc)
{
	m_btrack = false;
	m_ntrack[0] = m_ntrack[1] = m_ntrack[2] = -1;
	m_trackScale = 1.0;
	m_buildScene = true;
}

Post::CGLModel* CGLPostScene::GetGLModel()
{
	if (m_doc && m_doc->IsValid()) return m_doc->GetGLModel();
	return nullptr;
}

Post::FEPostModel* CGLPostScene::GetFSModel()
{
	if (m_doc && m_doc->IsValid()) return m_doc->GetFSModel();
	return nullptr;
}

FESelection* CGLPostScene::GetSelection()
{
	if (m_doc && m_doc->IsValid()) return m_doc->GetCurrentSelection();
	return nullptr;
}

int CGLPostScene::GetItemMode() const
{
	return m_doc->GetItemMode();
}

void CGLPostScene::Update()
{
	m_buildScene = true;
	CGLScene::Update();
}

BOX CGLPostScene::GetBoundingBox()
{
	BOX box;
	if (m_doc && m_doc->IsValid())
	{
		CGLModel& gm = *m_doc->GetGLModel();
		box = gm.GetPostObject()->GetBoundingBox();
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

	// build the scene
	if (m_buildScene)
	{
		BuildScene(rc);
		m_buildScene = false;
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// we need to update the tracking target before we position the camera
	if (m_btrack) UpdateTracking();

	GLViewSettings& vs = rc.m_settings;

	if (vs.m_use_environment_map) ActivateEnvironmentMap(engine);

	CGLCamera& cam = *rc.m_cam;
	engine.positionCamera(cam);

	// now render it
	CGLScene::Render(engine, rc);

	if (vs.m_use_environment_map) DeactivateEnvironmentMap(engine);

	// update and render the tracking
	if (m_btrack)
	{
		glx::renderAxes(m_trackScale, m_trgPos, m_trgRot, GLColor(255, 0, 255));
	}

	ClearTags();
	if (rc.m_settings.m_bTags) CreateTags(rc);
}

void CGLPostScene::BuildScene(CGLContext& rc)
{
	clear();
	if ((m_doc == nullptr) || (m_doc->IsValid() == false)) return;
	CGLModel* gm = m_doc->GetGLModel();
	if (gm == nullptr) return;

	GLPostPlaneCutItem* root = new GLPostPlaneCutItem(this);

	GLPostMirrorItem* mirror = new GLPostMirrorItem(this);
	root->addItem(mirror);

	mirror->addItem(new GLPostPlotItem(this));

	mirror->addItem(new GLPostModelItem(this));

	mirror->addItem(new GLPostObjectItem(this));

	for (int i = 0; i < m_doc->ImageModels(); ++i)
	{
		CImageModel* img = m_doc->GetImageModel(i);
		root->addItem(new GLPost3DImageItem(img, this));
	}

	addItem(root);
}

void CGLPostScene::CreateTags(CGLContext& rc)
{
	GLViewSettings& view = rc.m_settings;

	CGLModel& gm = *m_doc->GetGLModel();
	GObject* po = gm.GetPostObject();
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
		CGLModel& gm = *m_doc->GetGLModel();
		FSMeshBase* pm = gm.GetPostObject()->GetFEMesh();
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
