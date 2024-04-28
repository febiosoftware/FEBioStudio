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
#include "GLHighlighter.h"
#include "GLView.h"
#include <GLLib/GLMeshRender.h>
#include <GeomLib/GObject.h>

GLHighlighter GLHighlighter::This;

GLHighlighter::GLHighlighter() : m_view(0), m_activeItem(0), m_btrack(false)
{
	m_activeColor = GLColor(100, 255, 255);
	m_pickColor   = GLColor(0, 200, 200);
}

void GLHighlighter::AttachToView(CGLView* view)
{
	This.m_view = view;
	if (view) view->repaint();
}

void GLHighlighter::SetActiveItem(GItem* item)
{
	if (item != This.m_activeItem)
	{
		This.m_activeItem = item;
		if (This.m_view) This.m_view->repaint();
		emit This.activeItemChanged();
	}
}

void GLHighlighter::PickActiveItem()
{
	GItem* pick = This.m_activeItem;
	
	// make sure this item is not already picked
	for (int i=0; i<(int)This.m_item.size(); ++i)
		if (This.m_item[i] == pick) return;

	This.m_activeItem = 0;
	This.m_item.push_back(pick);
	if (This.m_view) This.m_view->repaint();
	emit This.itemPicked(pick);
}

void GLHighlighter::PickItem(GItem* item)
{
	if (item == 0) return;
	
	// make sure this item is not already picked
	for (int i = 0; i<(int)This.m_item.size(); ++i)
		if (This.m_item[i] == item) return;

	This.m_activeItem = 0;
	This.m_item.push_back(item);
	emit This.itemPicked(item);
}


GItem* GLHighlighter::GetActiveItem()
{ 
	return This.m_activeItem; 
}

QString GLHighlighter::GetActiveItemName()
{
	if (This.m_activeItem) return QString::fromStdString(This.m_activeItem->GetName());
	else return QString("");
}

void GLHighlighter::ClearHighlights()
{
	This.m_item.clear();
	This.m_activeItem = nullptr;
	if (This.m_view) This.m_view->repaint();
}

void GLHighlighter::setHighlightType(int type)
{
	// TODO: Implement this (for now, this only works with curves)
}

void GLHighlighter::setTracking(bool b)
{
	This.m_btrack = b;
	if (This.m_view) This.m_view->repaint();
}

bool GLHighlighter::IsTracking()
{
	return This.m_btrack;
}

void drawEdge(GLMeshRender& renderer, GEdge* edge, GLColor c)
{
	GObject* po = dynamic_cast<GObject*>(edge->Object());
	if (po == 0) return;

	glColor3ub(c.r, c.g, c.b);

	glPushMatrix();
	SetModelView(po);

	GMesh& m = *po->GetRenderMesh();
	renderer.RenderGLEdges(&m, edge->GetLocalID());

	GNode* n0 = po->Node(edge->m_node[0]);
	GNode* n1 = po->Node(edge->m_node[1]);

	if (n0 && n1)
	{
		glBegin(GL_POINTS);
		{
			vec3d r0 = n0->LocalPosition();
			vec3d r1 = n1->LocalPosition();
			glVertex3d(r0.x, r0.y, r0.z);
			glVertex3d(r1.x, r1.y, r1.z);
		}
		glEnd();
	}
	glPopMatrix();
}

void drawNode(GLMeshRender& renderer, GNode* node, GLColor c)
{
	GObject* po = dynamic_cast<GObject*>(node->Object());
	if (po == 0) return;

	glColor3ub(c.r, c.g, c.b);

	glPushMatrix();
	SetModelView(po);

	GMesh& m = *po->GetRenderMesh();

	if (node)
	{
		glBegin(GL_POINTS);
		{
			vec3d r0 = node->LocalPosition();
			glVertex3d(r0.x, r0.y, r0.z);
		}
		glEnd();
	}
	glPopMatrix();
}

void drawFace(CGLContext& rc, GLMeshRender& renderer, GFace* face, GLColor c)
{
	GObject* po = dynamic_cast<GObject*>(face->Object());
	if (po == 0) return;
	GMesh* mesh = po->GetRenderMesh();
	if (mesh == nullptr) return;
	glPushAttrib(GL_ENABLE_BIT);
	{
		glEnable(GL_COLOR_MATERIAL);
		glColor3ub(c.r, c.g, c.b);
		renderer.SetRenderMode(GLMeshRender::RenderMode::SelectionMode);
		renderer.RenderGLMesh(mesh, face->GetLocalID());
		renderer.SetRenderMode(GLMeshRender::RenderMode::OutlineMode);
		renderer.RenderSurfaceOutline(rc, mesh, face->GetLocalID());
	}
	glPopAttrib();
}

void drawPart(CGLContext& rc, GLMeshRender& renderer, GPart* part, GLColor c)
{
	GObject* po = dynamic_cast<GObject*>(part->Object());
	if (po == 0) return;
	GMesh* mesh = po->GetRenderMesh();
	if (mesh == nullptr) return;

	int pid = part->GetLocalID();

	// TODO: Make sure that the part's face list is populated!
//	if (part->Faces() == 0) return;
	vector<int> faceList; faceList.reserve(po->Faces());
	for (int i = 0; i < po->Faces(); ++i)
	{
		GFace* face = po->Face(i);
		if ((face->m_nPID[0] == pid) || (face->m_nPID[1] == pid)) faceList.push_back(i);
	}
	if (faceList.empty()) return;

	glPushAttrib(GL_ENABLE_BIT);
	{
		glEnable(GL_COLOR_MATERIAL);
		glColor3ub(c.r, c.g, c.b);
		for (int surfID : faceList)
		{
			renderer.SetRenderMode(GLMeshRender::RenderMode::SelectionMode);
			renderer.RenderGLMesh(mesh, surfID);
			renderer.SetRenderMode(GLMeshRender::RenderMode::OutlineMode);
			renderer.RenderSurfaceOutline(rc, mesh, surfID);
		}
	}
	glPopAttrib();
}

void GLHighlighter::draw()
{
	if (This.m_item.empty() && (This.m_activeItem == 0)) return;

	CGLView* view = This.m_view;
	if (view == 0) return;

	CGLContext rc;
	rc.m_view = view;
	rc.m_cam = view->GetCamera();
	rc.m_settings = view->GetViewSettings();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	GLfloat line_old;
	glGetFloatv(GL_LINE_WIDTH, &line_old);

	glLineWidth(2.0f);

	GLMeshRender renderer;

	for (GItem* item : This.m_item)
	{
		GEdge* edge = dynamic_cast<GEdge*>(item);
		if (edge) drawEdge(renderer, edge, This.m_pickColor);

		GNode* node = dynamic_cast<GNode*>(item);
		if (node) drawNode(renderer, node, This.m_pickColor);

		GFace* face = dynamic_cast<GFace*>(item);
		if (face) drawFace(rc, renderer, face, This.m_pickColor);

		GPart* part = dynamic_cast<GPart*>(item);
		if (part) drawPart(rc, renderer, part, This.m_pickColor);
	}

	if (This.m_activeItem)
	{
		GEdge* edge = dynamic_cast<GEdge*>(This.m_activeItem);
		if (edge) drawEdge(renderer, edge, This.m_activeColor);

		GNode* node = dynamic_cast<GNode*>(This.m_activeItem);
		if (node) drawNode(renderer, node, This.m_activeColor);

		GFace* face = dynamic_cast<GFace*>(This.m_activeItem);
		if (face) drawFace(rc, renderer, face, This.m_activeColor);

		GPart* part = dynamic_cast<GPart*>(This.m_activeItem);
		if (part) drawPart(rc, renderer, part, This.m_activeColor);
	}
	glLineWidth(line_old);

	glPopAttrib();
}

BOX GLHighlighter::GetBoundingBox()
{
	BOX box;
	for (GItem* item : This.m_item)
	{
		GNode* node = dynamic_cast<GNode*>(item);
		if (node)
		{
			box += node->Position();
		}

		GEdge* edge = dynamic_cast<GEdge*>(item);
		if (edge)
		{
			GNode* n0 = edge->Node(0);
			GNode* n1 = edge->Node(1);
			if (n0) box += n0->Position();
			if (n1) box += n1->Position();
		}

		GFace* face = dynamic_cast<GFace*>(item);
		if (face)
		{
			GObject* po = dynamic_cast<GObject*>(face->Object());
			GMesh* m = po->GetRenderMesh();
			if (m)
			{
				for (int i=0; i<m->Faces(); ++i)
				{ 
					GMesh::FACE& f = m->Face(i);
					if (f.pid == face->GetLocalID())
					{
						vec3d r0 = m->Node(f.n[0]).r; box += po->GetTransform().LocalToGlobal(r0);
						vec3d r1 = m->Node(f.n[1]).r; box += po->GetTransform().LocalToGlobal(r1);
						vec3d r2 = m->Node(f.n[2]).r; box += po->GetTransform().LocalToGlobal(r2);
					}
				}
			}
		}

		GPart* part = dynamic_cast<GPart*>(item);
		if (part)
		{
			GObject* po = dynamic_cast<GObject*>(part->Object());
			GMesh* m = po->GetRenderMesh();
			if (m)
			{
				int pid = part->GetLocalID();
				for (int i = 0; i < m->Faces(); ++i)
				{
					GMesh::FACE& f = m->Face(i);
					GFace* face = po->Face(f.pid);
					if ((face->m_nPID[0] == pid) || 
						(face->m_nPID[1] == pid) ||
						(face->m_nPID[2] == pid))
					{
						vec3d r0 = m->Node(f.n[0]).r; box += po->GetTransform().LocalToGlobal(r0);
						vec3d r1 = m->Node(f.n[1]).r; box += po->GetTransform().LocalToGlobal(r1);
						vec3d r2 = m->Node(f.n[2]).r; box += po->GetTransform().LocalToGlobal(r2);
					}
				}
			}
		}
	}

	return box;
}

std::vector<GItem*> GLHighlighter::GetItems()
{
	return This.m_item;
}
