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
#include <GeomLib/GObject.h>
#include <GLLib/GLMesh.h>
#include <MeshLib/FSMesh.h>

GLHighlighter GLHighlighter::This;

GLHighlighter::GLHighlighter() : m_view(0), m_activeItem(0), m_btrack(false)
{
}

void GLHighlighter::AttachToView(CGLView* view)
{
	This.m_view = view;
	if (view) view->update();
}

void GLHighlighter::SetActiveItem(GItem* item)
{
	if (item != This.m_activeItem)
	{
		This.m_activeItem = item;
		if (This.m_view) This.m_view->update();
//		emit This.activeItemChanged(); // doesn't look anything is listening so why send it?
	}
}

void GLHighlighter::PickActiveItem()
{
	GItem* pick = This.m_activeItem;
	
	// make sure this item is not already picked
	for (int i=0; i<(int)This.m_item.size(); ++i)
		if (This.m_item[i].item == pick) return;

	This.m_activeItem = nullptr;
	This.m_item.push_back({ pick, 0 });
	if (This.m_view) This.m_view->update();
	emit This.itemPicked(pick);
}

void GLHighlighter::PickItem(GItem* item, int colorMode)
{
	if (item == 0) return;
	
	// make sure this item is not already picked
	for (int i = 0; i<(int)This.m_item.size(); ++i)
		if (This.m_item[i].item == item) return;

	This.m_activeItem = 0;
	This.m_item.push_back({ item, colorMode });
	emit This.itemPicked(item);
}

void GLHighlighter::PickItem(FSGroup* item, int colorMode)
{
	if (item == nullptr) return;

	// make sure this item is not already picked
	for (int i = 0; i < (int)This.m_item.size(); ++i)
		if (This.m_item[i].item == item) return;

	This.m_activeItem = nullptr;
	This.m_item.push_back({ item, colorMode });
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
	if (This.m_view) This.m_view->update();
}

void GLHighlighter::setHighlightType(int type)
{
	// TODO: Implement this (for now, this only works with curves)
}

void GLHighlighter::setTracking(bool b)
{
	This.m_btrack = b;
	if (This.m_view) This.m_view->update();
}

bool GLHighlighter::IsTracking()
{
	return This.m_btrack;
}

BOX GLHighlighter::GetBoundingBox()
{
	BOX box;
	for (Item& item : This.m_item)
	{
		FSObject* it = item.item;
		GNode* node = dynamic_cast<GNode*>(it);
		if (node)
		{
			box += node->Position();
		}

		GEdge* edge = dynamic_cast<GEdge*>(it);
		if (edge)
		{
			GNode* n0 = edge->Node(0);
			GNode* n1 = edge->Node(1);
			if (n0) box += n0->Position();
			if (n1) box += n1->Position();
		}

		GFace* face = dynamic_cast<GFace*>(it);
		if (face)
		{
			GObject* po = dynamic_cast<GObject*>(face->Object());
			GLMesh* m = po->GetRenderMesh();
			if (m)
			{
				for (int i=0; i<m->Faces(); ++i)
				{ 
					GLMesh::FACE& f = m->Face(i);
					if (f.pid == face->GetLocalID())
					{
						vec3d r0 = to_vec3d(m->Node(f.n[0]).r); box += po->GetTransform().LocalToGlobal(r0);
						vec3d r1 = to_vec3d(m->Node(f.n[1]).r); box += po->GetTransform().LocalToGlobal(r1);
						vec3d r2 = to_vec3d(m->Node(f.n[2]).r); box += po->GetTransform().LocalToGlobal(r2);
					}
				}
			}
		}

		GPart* part = dynamic_cast<GPart*>(it);
		if (part)
		{
			GObject* po = dynamic_cast<GObject*>(part->Object());
			GLMesh* m = po->GetRenderMesh();
			if (m)
			{
				int pid = part->GetLocalID();
				for (int i = 0; i < m->Faces(); ++i)
				{
					GLMesh::FACE& f = m->Face(i);
					GFace* face = po->Face(f.pid);
					if ((face->m_nPID[0] == pid) || 
						(face->m_nPID[1] == pid) ||
						(face->m_nPID[2] == pid))
					{
						vec3d r0 = to_vec3d(m->Node(f.n[0]).r); box += po->GetTransform().LocalToGlobal(r0);
						vec3d r1 = to_vec3d(m->Node(f.n[1]).r); box += po->GetTransform().LocalToGlobal(r1);
						vec3d r2 = to_vec3d(m->Node(f.n[2]).r); box += po->GetTransform().LocalToGlobal(r2);
					}
				}
			}
		}
	}

	return box;
}

std::vector<GLHighlighter::Item> GLHighlighter::GetItems()
{
	return This.m_item;
}
