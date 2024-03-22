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

GLHighlighter GLHighlighter::m_This;

GLHighlighter::GLHighlighter() : m_view(0), m_activeItem(0), m_btrack(false)
{
	m_activeColor = GLColor(0, 255, 255);
	m_pickColor   = GLColor(0, 0, 255);
}

void GLHighlighter::AttachToView(CGLView* view)
{
	m_This.m_view = view;
	if (view) view->repaint();
}

void GLHighlighter::SetActiveItem(GItem* item)
{
	if (item != m_This.m_activeItem)
	{
		m_This.m_activeItem = item;
		if (m_This.m_view) m_This.m_view->repaint();
		emit m_This.activeItemChanged();
	}
}

void GLHighlighter::PickActiveItem()
{
	GItem* pick = m_This.m_activeItem;
	
	// make sure this item is not already picked
	for (int i=0; i<(int)m_This.m_item.size(); ++i)
		if (m_This.m_item[i] == pick) return;

	m_This.m_activeItem = 0;
	m_This.m_item.push_back(pick);
	if (m_This.m_view) m_This.m_view->repaint();
	emit m_This.itemPicked(pick);
}

void GLHighlighter::PickItem(GItem* item)
{
	if (item == 0) return;
	
	// make sure this item is not already picked
	for (int i = 0; i<(int)m_This.m_item.size(); ++i)
		if (m_This.m_item[i] == item) return;

	m_This.m_activeItem = 0;
	m_This.m_item.push_back(item);
	emit m_This.itemPicked(item);
}


GItem* GLHighlighter::GetActiveItem()
{ 
	return m_This.m_activeItem; 
}

QString GLHighlighter::GetActiveItemName()
{
	if (m_This.m_activeItem) return QString::fromStdString(m_This.m_activeItem->GetName());
	else return QString("");
}

void GLHighlighter::ClearHighlights()
{
	m_This.m_item.clear();
	m_This.m_activeItem = nullptr;
	if (m_This.m_view) m_This.m_view->repaint();
}

void GLHighlighter::setHighlightType(int type)
{
	// TODO: Implement this (for now, this only works with curves)
}

void GLHighlighter::setTracking(bool b)
{
	m_This.m_btrack = b;
	if (m_This.m_view) m_This.m_view->repaint();
}

bool GLHighlighter::IsTracking()
{
	return m_This.m_btrack;
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

void GLHighlighter::draw()
{
	if (m_This.m_item.empty() && (m_This.m_activeItem == 0)) return;

	CGLView* view = m_This.m_view;
	if (view == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	GLfloat line_old;
	glGetFloatv(GL_LINE_WIDTH, &line_old);

	glLineWidth(2.0f);

	GLMeshRender renderer;

    for (GItem* item : m_This.m_item)
	{
		GEdge* edge = dynamic_cast<GEdge*>(item);
		if (edge) drawEdge(renderer, edge, m_This.m_pickColor);

		GNode* node = dynamic_cast<GNode*>(item);
		if (node) drawNode(renderer, node, m_This.m_pickColor);
	}

	if (m_This.m_activeItem)
	{
		GEdge* edge = dynamic_cast<GEdge*>(m_This.m_activeItem);
		if (edge) drawEdge(renderer, edge, m_This.m_activeColor);

		GNode* node = dynamic_cast<GNode*>(m_This.m_activeItem);
		if (node) drawNode(renderer, node, m_This.m_activeColor);
	}
	glLineWidth(line_old);

	glPopAttrib();
}
