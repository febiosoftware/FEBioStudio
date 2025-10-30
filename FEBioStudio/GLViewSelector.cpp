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
#include "GLViewSelector.h"
#include "GLView.h"
#include "GLViewTransform.h"
#include "Document.h"
#include "PostDocument.h"
#include "ModelDocument.h"
#include "Commands.h"
#include "FEBioStudio.h"
#include "MainWindow.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSNodeEdgeList.h>
#include <PostGL/GLModel.h>
#include "GLHighlighter.h"

//=============================================================================
bool SelectRegion::LineIntersects(int x0, int y0, int x1, int y1) const
{
	return (IsInside(x0, y0) || IsInside(x1, y1));
}

bool SelectRegion::TriangleIntersect(int x0, int y0, int x1, int y1, int x2, int y2) const
{
	return (LineIntersects(x0, y0, x1, y1) || LineIntersects(x1, y1, x2, y2) || LineIntersects(x2, y2, x0, y0));
}

//=============================================================================
BoxRegion::BoxRegion(int x0, int x1, int y0, int y1)
{
	m_x0 = (x0 < x1 ? x0 : x1); m_x1 = (x0 < x1 ? x1 : x0);
	m_y0 = (y0 < y1 ? y0 : y1); m_y1 = (y0 < y1 ? y1 : y0);
}

bool BoxRegion::IsInside(int x, int y) const
{
	return ((x >= m_x0) && (x <= m_x1) && (y >= m_y0) && (y <= m_y1));
}

bool BoxRegion::LineIntersects(int x0, int y0, int x1, int y1) const
{
	return intersectsRect(QPoint(x0, y0), QPoint(x1, y1), QRect(m_x0, m_y0, m_x1 - m_x0, m_y1 - m_y0));
}

CircleRegion::CircleRegion(int x0, int x1, int y0, int y1)
{
	m_xc = x0;
	m_yc = y0;

	double dx = (x1 - x0);
	double dy = (y1 - y0);
	m_R = (int)sqrt(dx * dx + dy * dy);
}

bool CircleRegion::IsInside(int x, int y) const
{
	double rx = x - m_xc;
	double ry = y - m_yc;
	int r = rx * rx + ry * ry;
	return (r <= m_R * m_R);
}

bool CircleRegion::LineIntersects(int x0, int y0, int x1, int y1) const
{
	if (IsInside(x0, y0) || IsInside(x1, y1)) return true;

	int tx = x1 - x0;
	int ty = y1 - y0;

	int D = tx * (m_xc - x0) + ty * (m_yc - y0);
	int N = tx * tx + ty * ty;
	if (N == 0) return false;

	if ((D >= 0) && (D <= N))
	{
		int px = x0 + D * tx / N - m_xc;
		int py = y0 + D * ty / N - m_yc;

		if (px * px + py * py <= m_R * m_R) return true;
	}
	else return false;

	return false;
}

FreeRegion::FreeRegion(vector<pair<int, int> >& pl) : m_pl(pl)
{
	if (m_pl.empty() == false)
	{
		vector<pair<int, int> >::iterator pi = m_pl.begin();
		m_x0 = m_x1 = pi->first;
		m_y0 = m_y1 = pi->second;
		for (pi = m_pl.begin(); pi != m_pl.end(); ++pi)
		{
			int x = pi->first;
			int y = pi->second;
			if (x < m_x0) m_x0 = x; if (x > m_x1) m_x1 = x;
			if (y < m_y0) m_y0 = y; if (y > m_y1) m_y1 = y;
		}
	}
}

bool FreeRegion::IsInside(int x, int y) const
{
	if (m_pl.empty()) return false;
	if ((x < m_x0) || (x > m_x1) || (y < m_y0) || (y > m_y1))
	{
		return false;
	}

	int nint = 0;
	int N = (int)m_pl.size();
	for (int i = 0; i < N; ++i)
	{
		int ip1 = (i + 1) % N;
		double x0 = (double)m_pl[i].first;
		double y0 = (double)m_pl[i].second;
		double x1 = (double)m_pl[ip1].first;
		double y1 = (double)m_pl[ip1].second;

		double yc = (double)y + 0.0001;

		if (((y1 > yc) && (y0 < yc)) || ((y0 > yc) && (y1 < yc)))
		{
			double xi = x1 + ((x0 - x1) * (y1 - yc)) / (y1 - y0);
			if (xi > (double)x) nint++;
		}
	}
	return ((nint > 0) && (nint % 2));
}

//-----------------------------------------------------------------------------
GLViewSelector::GLViewSelector(CGLView* glview) : m_glv(glview) 
{
	m_bshift = false;
	m_bctrl = false;
}

//-----------------------------------------------------------------------------
void GLViewSelector::SetStateModifiers(bool shift, bool ctrl)
{
	m_bshift = shift;
	m_bctrl = ctrl;
}

//-----------------------------------------------------------------------------
void GLViewSelector::TagBackfacingNodes(FSMeshBase& mesh)
{
	int NN = mesh.Nodes();
	for (int i = 0; i < NN; ++i) mesh.Node(i).m_ntag = 1;

	// assigns 1 to back-facing faces, and 0 to front-facing
	TagBackfacingFaces(mesh);

	int NF = mesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.m_ntag == 0)
		{
			int nn = f.Nodes();
			for (int i = 0; i < nn; ++i) mesh.Node(f.n[i]).m_ntag = 0;
		}
	}
}

//-----------------------------------------------------------------------------
bool IsBackfacing(const vec3d r[3])
{
	bool b = ((r[1].x - r[0].x) * (r[2].y - r[0].y) - (r[1].y - r[0].y) * (r[2].x - r[0].x)) >= 0.f;
	return b;
}

void GLViewSelector::TagBackfacingFaces(FSMeshBase& mesh)
{
	GLViewTransform transform(m_glv);

	vec3d r[4], p1[3], p2[3];
	int NF = mesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh.Face(i);

		if (f.IsExternal())
		{
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
			{
				r[0] = mesh.Node(f.n[0]).r;
				r[1] = mesh.Node(f.n[1]).r;
				r[2] = mesh.Node(f.n[2]).r;

				p1[0] = transform.WorldToScreen(r[0]);
				p1[1] = transform.WorldToScreen(r[1]);
				p1[2] = transform.WorldToScreen(r[2]);

				if (IsBackfacing(p1)) f.m_ntag = 1;
				else f.m_ntag = 0;
			}
			break;
			case FE_FACE_QUAD4:
			case FE_FACE_QUAD8:
			case FE_FACE_QUAD9:
			{
				r[0] = mesh.Node(f.n[0]).r;
				r[1] = mesh.Node(f.n[1]).r;
				r[2] = mesh.Node(f.n[2]).r;
				r[3] = mesh.Node(f.n[3]).r;

				p1[0] = transform.WorldToScreen(r[0]);
				p1[1] = transform.WorldToScreen(r[1]);
				p1[2] = transform.WorldToScreen(r[2]);

				p2[0] = p1[2];
				p2[1] = transform.WorldToScreen(r[3]);
				p2[2] = p1[0];

				if (IsBackfacing(p1) && IsBackfacing(p2)) f.m_ntag = 1;
				else f.m_ntag = 0;
			}
			break;
			}
		}
		else f.m_ntag = 1;
	}
}

//-----------------------------------------------------------------------------
void GLViewSelector::RegionSelectFENodes(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	Transform& T = po->GetRenderTransform();

	FSMeshBase* pm = nullptr;
	switch (pdoc->GetMeshMode())
	{
	case MESH_MODE_VOLUME: pm = po->GetFEMesh(); break;
	case MESH_MODE_SURFACE: pm = po->GetEditableMesh(); break;
	}
	if (pm == nullptr) return;

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	// ignore exterior option for surface meshes
	if (view.m_bext || (dynamic_cast<FSSurfaceMesh*>(pm)))
	{
		if (view.m_bcullSel)
		{
			// NOTE: This tags front facing nodes. Should rename function. 
			TagBackfacingNodes(*pm);
		}
		else
		{
			// tag all exterior nodes
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				FSNode& node = pm->Node(i);
				if (node.IsExterior()) node.m_ntag = 0;
				else node.m_ntag = -1;
			}
		}
	}
	else
		pm->TagAllNodes(0);

//	double* a = m_glv->PlaneCoordinates();

	vector<int> selectedNodes;
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsVisible() && (node.m_ntag == 0))
		{
			vec3d r = T.LocalToGlobal(node.r);

//			if ((m_glv->ShowPlaneCut() == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] >= 0.0))
			{
				vec3d p = transform.WorldToScreen(r);

				if (region.IsInside((int)p.x, (int)p.y))
				{
					selectedNodes.push_back(i);
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, selectedNodes);
	else pcmd = new CCmdSelectFENodes(pm, selectedNodes, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
void GLViewSelector::TagBackfacingElements(FSMesh& mesh)
{
	GLViewTransform transform(m_glv);
	vec3d r[4], p1[3], p2[3];
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		el.m_ntag = 0;

		// make sure the element is visible
		if (el.IsExterior())
		{
			// get the number of faces
			// Note that NF = 0 for shells so shells are never considered back facing
			int NF = el.Faces();

			// check each face
			// an element is backfacing if all its visible faces are back facing
			bool backFacing = true;
			el.m_ntag = 1;
			for (int j = 0; j < NF; ++j)
			{
				FSElement* pj = (el.m_nbr[j] != -1 ? &mesh.Element(el.m_nbr[j]) : 0);
				if ((pj == 0) || (pj->IsVisible() == false))
				{
					FSFace f = el.GetFace(j);
					switch (f.Type())
					{
					case FE_FACE_TRI3:
					case FE_FACE_TRI6:
					case FE_FACE_TRI7:
					case FE_FACE_TRI10:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					case FE_FACE_QUAD4:
					case FE_FACE_QUAD8:
					case FE_FACE_QUAD9:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;
						r[3] = mesh.Node(f.n[3]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						p2[0] = p1[2];
						p2[1] = transform.WorldToScreen(r[3]);
						p2[2] = p1[0];

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					}
				}

				if (backFacing == false)
				{
					el.m_ntag = 0;
					break;
				}
			}

			// shells 
			if (el.IsShell())
			{
				FSFace* pf = mesh.FacePtr(el.m_face[0]);
				if (pf)
				{
					FSFace& f = *pf;
					switch (f.Type())
					{
					case FE_FACE_TRI3:
					case FE_FACE_TRI6:
					case FE_FACE_TRI7:
					case FE_FACE_TRI10:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					case FE_FACE_QUAD4:
					case FE_FACE_QUAD8:
					case FE_FACE_QUAD9:
					{
						r[0] = mesh.Node(f.n[0]).r;
						r[1] = mesh.Node(f.n[1]).r;
						r[2] = mesh.Node(f.n[2]).r;
						r[3] = mesh.Node(f.n[3]).r;

						p1[0] = transform.WorldToScreen(r[0]);
						p1[1] = transform.WorldToScreen(r[1]);
						p1[2] = transform.WorldToScreen(r[2]);

						p2[0] = p1[2];
						p2[1] = transform.WorldToScreen(r[3]);
						p2[2] = p1[0];

						if (IsBackfacing(p1) == false) backFacing = false;
					}
					break;
					}
				}

				if (backFacing == false)
				{
					el.m_ntag = 0;
				}
			}

			// we should always be able to select beam elements
			if (el.IsBeam())
			{
				el.m_ntag = 0;
			}
		}
	}
}

void GLViewSelector::RegionSelectFEElems(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	Transform& T = po->GetRenderTransform();

	FSMesh* pm = po->GetFEMesh();

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	if (view.m_bcullSel)
	{
		TagBackfacingElements(*pm);
	}
	else pm->TagAllElements(0);

//	double* a = m_glv->PlaneCoordinates();

	vector<int> selectedElements;
	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = pm->Element(i);

		// if the exterior-only flag is off, make sure all solids are selectable
		if ((view.m_bext == false) && el.IsSolid()) el.m_ntag = 0;

		if ((el.m_ntag == 0) && el.IsVisible() && po->Part(el.m_gid)->IsVisible())
		{
			bool process = false;
			process = ((view.m_bext == false) || el.IsExterior());
			if ((process == false) && view.m_bext && el.IsSolid())
			{
				// we'll also allow elements to be region-selected if they are visible
				for (int j = 0; j < el.Faces(); ++j)
				{
					int nbr = el.m_nbr[j];
					FSElement_* pej = (nbr >= 0 ? pm->ElementPtr(nbr) : nullptr);
					if ((pej == nullptr) || (pej->IsVisible() == false)) {
						process = true; break;
					}
				}
			}

			if (process)
			{
				int ne = el.Nodes();
				bool binside = false;

				for (int j = 0; j < ne; ++j)
				{
					vec3d r = T.LocalToGlobal(pm->Node(el.m_node[j]).r);

//					if ((m_glv->ShowPlaneCut()  == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] > 0))
					{
						vec3d p = transform.WorldToScreen(r);
						if (region.IsInside((int)p.x, (int)p.y))
						{
							binside = true;
							break;
						}
					}
				}

				if (binside)
				{
					selectedElements.push_back(i);
				}
			}
		}
	}


	CCommand* pcmd = 0;
	if (view.m_selectAndHide)
	{
		pcmd = new CCmdHideElements(po, selectedElements);
	}
	else
	{
		if (m_bctrl) pcmd = new CCmdUnselectElements(pm, selectedElements);
		else pcmd = new CCmdSelectElements(pm, selectedElements, m_bshift);
	}
	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
bool regionFaceIntersect(GLViewTransform& transform, const SelectRegion& region, FSFace& face, FSMeshBase* pm)
{
	if (pm == 0) return false;

	vec3d r[4], p[4];
	bool binside = false;
	switch (face.Type())
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		r[0] = pm->NodePosition(face.n[0]);
		r[1] = pm->NodePosition(face.n[1]);
		r[2] = pm->NodePosition(face.n[2]);

		p[0] = transform.WorldToScreen(r[0]);
		p[1] = transform.WorldToScreen(r[1]);
		p[2] = transform.WorldToScreen(r[2]);

		if (region.TriangleIntersect((int)p[0].x, (int)p[0].y, (int)p[1].x, (int)p[1].y, (int)p[2].x, (int)p[2].y))
		{
			binside = true;
		}
		break;

	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		r[0] = pm->NodePosition(face.n[0]);
		r[1] = pm->NodePosition(face.n[1]);
		r[2] = pm->NodePosition(face.n[2]);
		r[3] = pm->NodePosition(face.n[3]);

		p[0] = transform.WorldToScreen(r[0]);
		p[1] = transform.WorldToScreen(r[1]);
		p[2] = transform.WorldToScreen(r[2]);
		p[3] = transform.WorldToScreen(r[3]);

		if ((region.TriangleIntersect((int)p[0].x, (int)p[0].y, (int)p[1].x, (int)p[1].y, (int)p[2].x, (int)p[2].y)) ||
			(region.TriangleIntersect((int)p[2].x, (int)p[2].y, (int)p[3].x, (int)p[3].y, (int)p[0].x, (int)p[0].y)))
		{
			binside = true;
		}
		break;
	}
	return binside;
}

void GLViewSelector::RegionSelectFEFaces(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	FSMeshBase* pm = nullptr;
	switch (pdoc->GetMeshMode())
	{
	case MESH_MODE_VOLUME: pm = po->GetFEMesh(); break;
	case MESH_MODE_SURFACE: pm = po->GetEditableMesh(); break;
	}
	if (pm == nullptr) return;

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	// tag back facing items so they won't get selected.
	if (view.m_bcullSel)
	{
		// NOTE: This actually tags front-facing faces. Should rename function.
		TagBackfacingFaces(*pm);
	}
	else if (view.m_bext)
	{
		// tag exterior faces only 
		for (int i = 0; i < pm->Faces(); ++i)
		{
			FSFace& f = pm->Face(i);
			if (f.IsExternal()) f.m_ntag = 0;
			else f.m_ntag = -1;
		}
	}
	else
		pm->TagAllFaces(0);

	int NS = po->Faces();
	vector<bool> vis(NS);
	for (int i = 0; i < NS; ++i)
	{
		vis[i] = po->IsFaceVisible(po->Face(i));
	}

	vector<int> selectedFaces;
	int NF = pm->Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.IsVisible() && vis[face.m_gid] && (face.m_ntag == 0))
		{
			bool b = regionFaceIntersect(transform, region, face, pm);

			if (b)
//			if (b && m_glv->ShowPlaneCut())
			{
//				double* a = m_glv->PlaneCoordinates();
				b = false;
				vec3d r[FSFace::MAX_NODES];
				pm->FaceNodePosition(face, r);
				for (int j = 0; j < face.Nodes(); ++j)
				{
					vec3d p = pm->LocalToGlobal(r[j]);
//					if (p.x * a[0] + p.y * a[1] + p.z * a[2] + a[3] > 0)
					{
						b = true;
						break;
					}
				}
			}

			if (b)
			{
				selectedFaces.push_back(i);
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, selectedFaces);
	else pcmd = new CCmdSelectFaces(pm, selectedFaces, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
void GLViewSelector::TagBackfacingEdges(FSMeshBase& mesh)
{
	int NE = mesh.Edges();
	for (int i = 0; i < NE; ++i) mesh.Edge(i).m_ntag = 1;

	TagBackfacingNodes(mesh);

	for (int i = 0; i < NE; ++i)
	{
		FSEdge& e = mesh.Edge(i);
		if ((mesh.Node(e.n[0]).m_ntag == 0) && (mesh.Node(e.n[1]).m_ntag == 0))
			e.m_ntag = 0;
	}
}

void GLViewSelector::RegionSelectFEEdges(const SelectRegion& region)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	Transform& T = po->GetRenderTransform();

	FSMeshBase* pm = nullptr;
	switch (pdoc->GetMeshMode())
	{
	case MESH_MODE_VOLUME : pm = po->GetFEMesh(); break;
	case MESH_MODE_SURFACE: pm = po->GetEditableMesh(); break;
	}
	if (pm == nullptr) return;

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	if (view.m_bcullSel)
		TagBackfacingEdges(*pm);
	else
		pm->TagAllEdges(0);

//	double* a = m_glv->PlaneCoordinates();
	vector<int> selectedEdges;
	int NE = pm->Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.IsVisible() && (edge.m_ntag == 0))
		{
			vec3d r0 = T.LocalToGlobal(pm->Node(edge.n[0]).r);
			vec3d r1 = T.LocalToGlobal(pm->Node(edge.n[1]).r);

//			double d0 = r0.x * a[0] + r0.y * a[1] + r0.z * a[2] + a[3];
//			double d1 = r1.x * a[0] + r1.y * a[1] + r1.z * a[2] + a[3];

//			if ((m_glv->ShowPlaneCut() == false) || ((d0 >= 0) || (d1 >= 0)))
			{
				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);

				int x0 = (int)p0.x;
				int y0 = (int)p0.y;
				int x1 = (int)p1.x;
				int y1 = (int)p1.y;

				if (region.LineIntersects(x0, y0, x1, y1))
				{
					selectedEdges.push_back(i);
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, selectedEdges);
	else pcmd = new CCmdSelectFEEdges(pm, selectedEdges, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
void GLViewSelector::BrushSelectFaces(int x, int y, bool badd, bool binit)
{
	CGLDocument* pdoc = m_glv->GetDocument();
	int item = pdoc->GetItemMode();
	if (item != ITEM_FACE) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the active object
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	// get the FE mesh
	FSMeshBase* pm = po->GetEditableMesh();
	if (pm == 0) return;
	FSMeshBase& mesh = *pm;

	// convert the point to a ray
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	Ray ray = transform.PointToRay(x, y);

	// convert ray to local coordinates
	Transform& T = po->GetRenderTransform();
	ray.origin = T.GlobalToLocal(ray.origin);
	ray.direction = T.GlobalToLocalNormal(ray.direction);

	double R = view.m_brushSize;

	// First, store the current face selection state
	int faces = mesh.Faces();
	if (binit)
	{
		m_selFaces0.assign(faces, 0);
		for (int i = 0; i < faces; ++i)
		{
			const FSFace& face = mesh.Face(i);
			m_selFaces0[i] = (face.IsSelected() ? 1 : 0);
		}
	}

	// find all nodes that fall within the selection region
	int nodes = mesh.Nodes();
	std::vector<int> tag(nodes, 0);
	for (int i = 0; i < nodes; ++i)
	{
		vec3d r0 = mesh.Node(i).pos();
		vec3d rt = mesh.NodePosition(i);
		vec3d p = transform.WorldToScreen(rt);

		double z = ray.direction * (r0 - ray.origin);

		double L2 = (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y);
		if ((L2 <= R * R) && (z >= 0))
			tag[i] = 1;
	}

	// collect all faces that should be selected
	vector<int> faceList; faceList.reserve(100);
	if (badd)
	{
		Intersection q;
		if (FindFaceIntersection(ray, mesh, q))
		{
			assert((q.m_index >= 0) && (q.m_index < faces));
			mesh.TagAllFaces(0);
			std::stack<int> S;
			S.push(q.m_index);
			while (S.empty() == false)
			{
				int n = S.top(); S.pop();
				FSFace& f = mesh.Face(n);
				assert((f.m_ntag == 0) || (f.m_ntag == 2));
				f.m_ntag = 1;
				faceList.push_back(n);
				for (int j = 0; j < f.Edges(); ++j)
				{
					if (f.m_nbr[j] >= 0)
					{
						FSFace& fj = pm->Face(f.m_nbr[j]);
						vec3d fnj = to_vec3d(fj.m_fn);
						if ((fj.m_ntag == 0))
						{
							bool baddFace = false;
							if ((fj.IsVisible()) && (fnj * ray.direction < 0))
							{
								for (int k = 0; k < fj.Nodes(); ++k)
								{
									if (tag[fj.n[k]] > 0)
									{
										baddFace = true;
										break;
									}
								}
							}

							if (baddFace)
							{
								fj.m_ntag = 2;
								S.push(f.m_nbr[j]);
							}
							else fj.m_ntag = -1;
						}
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < faces; ++i)
		{
			const FSFace& face = mesh.Face(i);
			vec3d fn = to_vec3d(face.m_fn);
			if (face.IsVisible() && (fn * ray.direction < 0))
			{
				bool baddFace = false;
				for (int j = 0; j < face.Nodes(); ++j)
				{
					if (tag[face.n[j]])
					{
						baddFace = true;
						break;
					}
				}
				if (baddFace == false)
				{
					vec3d rn[FSFace::MAX_NODES];
					mesh.FaceNodeLocalPositions(face, rn);
					Intersection q;
					baddFace = RayIntersectFace(ray, face.Type(), rn, q);
				}

				if (baddFace) faceList.push_back(i);
			}
		}
	}
	if (faceList.empty()) return;

	// TODO: eliminate faces that cannot be seen because of the plane cut

	// select the faces
	for (int i : faceList)
	{
		FSFace& face = mesh.Face(i);
		if (badd) face.Select(); else face.Unselect();
	}
	
	pdoc->UpdateSelection();
}

void GLViewSelector::Finish()
{
	CGLDocument* pdoc = m_glv->GetDocument();
	GObject* po = pdoc->GetActiveObject();
	if (po)
	{
		FSMeshBase* pm = po->GetEditableMesh();
		if (pm && (m_selFaces0.empty() == false))
		{
			int faces = pm->Faces();
			assert(m_selFaces0.size() == faces);
			vector<int> faceList; faceList.reserve(100);
			int changes = 0;
			for (int i = 0; i < faces; ++i)
			{
				FSFace& f = pm->Face(i);
				int m = (f.IsSelected() ? 1 : 0);
				if (m != m_selFaces0[i]) changes++;
				if (f.IsSelected()) faceList.push_back(i);
			}

			// restore selection
			for (int i = 0; i < faces; ++i)
			{
				if (m_selFaces0[i]) pm->Face(i).Select(); else pm->Face(i).Unselect();
			}
			m_selFaces0.clear();

			if (changes > 0)
			{
				pdoc->DoCommand(new CCmdSelectFaces(pm, faceList, false));
			}
		}
	}
}

//-----------------------------------------------------------------------------
int FindBeamIntersection(int x, int y, GObject* po, GLViewTransform& transform, Intersection& q)
{
	FSMesh* pm = po->GetFEMesh();

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	// try to select discrete elements
	vec3d o(0, 0, 0);
	vec3d O = transform.WorldToScreen(o);

	Transform& T = po->GetRenderTransform();

	int index = -1;
	float zmin = 0.f;
	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& del = pm->Element(i);
		if (del.IsBeam() && del.IsVisible())
		{
			vec3d r0 = T.LocalToGlobal(pm->Node(del.m_node[0]).r);
			vec3d r1 = T.LocalToGlobal(pm->Node(del.m_node[1]).r);

			vec3d p0 = transform.WorldToScreen(r0);
			vec3d p1 = transform.WorldToScreen(r1);

			// make sure p0, p1 are in front of the camera
			if (((p0.x >= 0) || (p1.x >= 0)) && ((p0.y >= 0) || (p1.y >= 0)) &&
				(p0.z > -1) && (p0.z < 1) && (p1.z > -1) && (p1.z < 1))
			{
				// see if the edge intersects
				if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
				{
					if ((index == -1) || (p0.z < zmin))
					{
						index = i;
						zmin = p0.z;
						q.point = p0;
					}
				}
			}
		}
	}

	return index;
}

//-----------------------------------------------------------------------------
void GLViewSelector::SelectFEElements(int x, int y)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	// convert the point to a ray
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);
	Ray ray = transform.PointToRay(x, y);

	Transform& T = po->GetRenderTransform();

	// convert ray to local coordinates
	Ray localRay;
	localRay.origin = T.GlobalToLocal(ray.origin);
	localRay.direction = T.GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	bool bfound = FindElementIntersection(localRay, *pm, q, m_bctrl);

	// see if the intersection with the plane cut is closer
/*
	if (bfound && m_glv->ShowPlaneCut())
	{
		vec3d p = T.LocalToGlobal(q.point);
		GLPlaneCut& planeCut = m_glv->GetPlaneCut();
		bool bintersect = planeCut.Intersect(p, ray, q);
		if (bintersect) bfound = bintersect;
	}
*/

	// the selection command that will be executed
	CCommand* pcmd = nullptr;

	// see if there is a beam element that is closer
	vec3d p = transform.WorldToScreen(q.point);
	Intersection q2;
	int index = FindBeamIntersection(x, y, po, transform, q2);
	if ((index >= 0) && ((bfound == false) || (q2.point.z < p.z)))
	{
		if (index >= 0)
		{
			if (m_bctrl) pcmd = new CCmdUnselectElements(pm, &index, 1);
			else pcmd = new CCmdSelectElements(pm, &index, 1, m_bshift);
			bfound = false;
		}
	}

	if (bfound)
	{
		int index = q.m_index;
		if (view.m_bconn)
		{
			vector<int> elemList = MeshTools::GetConnectedElements(pm, index, view.m_fconn, view.m_bpart, view.m_bext, view.m_bmax);

			if (!elemList.empty())
			{
				if (view.m_selectAndHide)
				{
					pcmd = new CCmdHideElements(po, elemList);
				}
				else
				{
					if (m_bctrl) pcmd = new CCmdUnselectElements(pm, elemList);
					else pcmd = new CCmdSelectElements(pm, elemList, m_bshift);
				}
			}
		}
		else
		{
			int num = (int)index;
			if (view.m_selectAndHide)
			{
				pcmd = new CCmdHideElements(po, { num });
			}
			else
			{
				if (m_bctrl)
					pcmd = new CCmdUnselectElements(pm, &num, 1);
				else
				{
					pcmd = new CCmdSelectElements(pm, &num, 1, m_bshift);

					// print value of currently selected element
					CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
					if (postDoc && postDoc->IsValid())
					{
						Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
						if (cmap && cmap->IsActive())
						{
							Post::FEPostModel* fem = postDoc->GetFSModel();
							Post::FEState* state = fem->CurrentState();
							double val = state->m_ELEM[num].m_val;
							FSElement& el = pm->Element(num);
							QString txt = QString("Element %1 : %2\n").arg(el.m_nid).arg(val);
                            
							FBS::getMainWindow()->AddLogEntry(txt);
						}
					}
				}
			}
		}
	}

	if ((pcmd == nullptr) && (!m_bshift) && (!m_bctrl))
	{
		// clear selection
		int nsel = pm->CountSelectedElements();
		if (nsel > 0)
		{
			pcmd = new CCmdSelectElements(pm, 0, 0, false);
		}
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::SelectFEFaces(int x, int y)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the active object
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	// get the FE mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return;

	// convert the point to a ray
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);
	Ray ray = transform.PointToRay(x, y);

	// convert ray to local coordinates
	Transform& T = po->GetRenderTransform();
	ray.origin = T.GlobalToLocal(ray.origin);
	ray.direction = T.GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	CCommand* pcmd = 0;

	bool bfound = FindFaceIntersection(ray, *pm, q);

	if (bfound) // && m_glv->ShowPlaneCut())
	{
		vec3d p = T.LocalToGlobal(q.point);

		// see if the intersection lies behind the plane cut. 
//		double* a = m_glv->PlaneCoordinates();
//		double d = p.x * a[0] + p.y * a[1] + p.z * a[2] + a[3];
//		if (d < 0)
		{
//			bfound = false;
		}
	}

	if (bfound)
	{
		static int lastIndex = -1;
		int index = q.m_index;
		if (view.m_bconn)
		{
			// get the list of connected faces
			vector<int> faceList = MeshTools::GetConnectedFaces(pm, index, (view.m_bmax ? view.m_fconn : 0.0), view.m_bpart);

			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, faceList);
			else pcmd = new CCmdSelectFaces(pm, faceList, m_bshift);

			lastIndex = -1;
		}
		else if (view.m_bselpath)
		{
			if (lastIndex != -1)
			{
				vector<int> faceList = MeshTools::GetConnectedFacesByPath(pm, lastIndex, index);

				if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, faceList);
				else pcmd = new CCmdSelectFaces(pm, faceList, m_bshift);
			}
			else
			{
				if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, &index, 1);
				else pcmd = new CCmdSelectFaces(pm, &index, 1, m_bshift);
			}
			lastIndex = index;
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, &index, 1);
			else
			{
				pcmd = new CCmdSelectFaces(pm, &index, 1, m_bshift);

				// print value of currently selected face
				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
				if (postDoc && postDoc->IsValid())
				{
					Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
					if (cmap && cmap->IsActive())
					{
						Post::FEPostModel* fem = postDoc->GetFSModel();
						Post::FEState* state = fem->CurrentState();
						double val = state->m_FACE[index].m_val;
						FSFace& face = pm->Face(index);
						QString txt = QString("Face %1 : %2\n").arg(face.m_nid).arg(val);
						FBS::getMainWindow()->AddLogEntry(txt);
					}
				}
			}
		}
	}
	else if (!m_bshift)
	{
		int nsel = pm->CountSelectedFaces();
		if (nsel > 0)
		{
			pcmd = new CCmdSelectFaces(pm, 0, 0, false);
		}
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::SelectFEEdges(int x, int y)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	Transform& T = po->GetRenderTransform();

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	vec3d o(0, 0, 0);
	vec3d O = transform.WorldToScreen(o);
//	double* a = m_glv->PlaneCoordinates();

	int index = -1;
	float zmin = 0.f;
	int NE = pm->Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		vec3d r0 = T.LocalToGlobal(pm->Node(edge.n[0]).r);
		vec3d r1 = T.LocalToGlobal(pm->Node(edge.n[1]).r);

		vec3d p0 = transform.WorldToScreen(r0);
		vec3d p1 = transform.WorldToScreen(r1);

		// make sure p0, p1 are in front of the camera
		if (((p0.x >= 0) || (p1.x >= 0)) && ((p0.y >= 0) || (p1.y >= 0)) &&
			(p0.z > -1) && (p0.z < 1) && (p1.z > -1) && (p1.z < 1))
		{
			// see if the edge intersects
			bool bfound = intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt);

/*			if (bfound && m_glv->ShowPlaneCut())
			{
				// make sure one point is in front of plane
				double d0 = r0.x * a[0] + r0.y * a[1] + r0.z * a[2] + a[3];
				double d1 = r1.x * a[0] + r1.y * a[1] + r1.z * a[2] + a[3];
				if ((d0 < 0) && (d1 < 0)) bfound = false;
			}
*/
			if (bfound)
			{
				if ((index == -1) || (p0.z < zmin))
				{
					index = i;
					zmin = p0.z;
				}
			}
		}
	}

	// parse the selection buffer
	CCommand* pcmd = 0;
	static int lastIndex = -1; // used by select path tool
	if (index >= 0)
	{
		if (view.m_bconn)
		{
			vector<int> edgeList = MeshTools::GetConnectedEdges(pm, index, view.m_fconn, view.m_bmax);
			if (!edgeList.empty())
			{
				if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, edgeList);
				else pcmd = new CCmdSelectFEEdges(pm, edgeList, m_bshift);
			}
			lastIndex = -1;
		}
		else if (view.m_bselpath)
		{
			vector<int> edgeList;
			if ((lastIndex != -1) && (lastIndex != index))
			{
				edgeList = MeshTools::GetConnectedEdgesByPath(pm, lastIndex, index);
				lastIndex = index;
			}
			else
			{
				edgeList.push_back(index);
				lastIndex = index;
			}
			if (!edgeList.empty())
			{
				if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, edgeList);
				else pcmd = new CCmdSelectFEEdges(pm, edgeList, m_bshift);
			}
		}
		else
		{
			int num = (int)index;
			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &num, 1);
			else
			{
				pcmd = new CCmdSelectFEEdges(pm, &num, 1, m_bshift);

				// print value of currently selected edge
				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
				if (postDoc && postDoc->IsValid())
				{
					Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
					if (cmap && cmap->IsActive())
					{
						Post::FEPostModel* fem = postDoc->GetFSModel();
						Post::FEState* state = fem->CurrentState();
						double val = state->m_EDGE[num].m_val;
						FSEdge& ed = pm->Edge(num);
						QString txt = QString("Edge %1 : %2\n").arg(ed.m_nid).arg(val);
						FBS::getMainWindow()->AddLogEntry(txt);
					}
				}
			}
		}
	}
	else if (!m_bshift)
	{
		int nsel = pm->CountSelectedEdges();
		if (nsel)
		{
			pcmd = new CCmdSelectFEEdges(pm, 0, 0, false);
		}
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

//-----------------------------------------------------------------------------
void GLViewSelector::SelectPostObject(int x, int y)
{
	CPostDocument* postDoc = dynamic_cast<CPostDocument*>(m_glv->GetDocument());
	if (postDoc == nullptr) return;

	// convert the point to a ray
	GLViewTransform transform(m_glv);
	Ray ray = transform.PointToRay(x, y);

	// pass selection to plots
	postDoc->SetTransformMode(TRANSFORM_NONE);
	postDoc->SetCurrentSelection(nullptr);
	Intersection q;
	Post::CGLModel* glm = postDoc->GetGLModel();
	int plots = glm->Plots();
	for (int i = 0; i < plots; ++i)
	{
		Post::CGLPlot* plt = glm->Plot(i);
		if (plt->Intersects(ray, q))
		{
			FESelection* sel = plt->SelectComponent(q.m_index);
			postDoc->SetTransformMode(TRANSFORM_MOVE);
			postDoc->SetCurrentSelection(sel);
		}
		else plt->ClearSelection();
	}
}

//-----------------------------------------------------------------------------
bool IntersectObject(GObject* po, const Ray& ray, Intersection& q)
{
	GLMesh* mesh = po->GetRenderMesh();
	if (mesh == nullptr) return false;

	Transform& T = po->GetRenderTransform();

	Intersection qtmp;
	double distance = 0.0, minDist = 1e34;
	int NF = mesh->Faces();
	bool intersect = false;
	for (int j = 0; j < NF; ++j)
	{
		GLMesh::FACE& face = mesh->Face(j);

		if (po->Face(face.pid)->IsVisible())
		{
			vec3d r0 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[0]).r));
			vec3d r1 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[1]).r));
			vec3d r2 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[2]).r));

			Triangle tri = { r0, r1, r2 };
			if (IntersectTriangle(ray, tri, qtmp))
			{
				double distance = ray.direction * (qtmp.point - ray.origin);
				if ((distance >= 0.0) && (distance < minDist))
				{
					minDist = distance;
					q = qtmp;
					intersect = true;
				}
			}
		}
	}

	return intersect;
}

//-----------------------------------------------------------------------------
// Select Objects
void GLViewSelector::SelectObjects(int x, int y)
{
	CPostDocument* postDoc = dynamic_cast<CPostDocument*>(m_glv->GetDocument());
	if (postDoc)
	{
		SelectPostObject(x, y);
		return;
	}

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	m_glv->makeCurrent();

	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// convert the point to a ray
	GLViewTransform transform(m_glv);
	Ray ray = transform.PointToRay(x, y);

	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	GObject* closestObject = 0;
	Intersection q;
	double minDist = 0;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			if (IntersectObject(po, ray, q))
			{
				double distance = ray.direction * (q.point - ray.origin);
				if ((closestObject == 0) || ((distance >= 0.0) && (distance < minDist)))
				{
					closestObject = po;
					minDist = distance;
				}
			}
			else
			{
				// if this is a line object, we'll need to use a different strategy
				double zmin;
				if (SelectClosestEdge(po, transform, rt, zmin))
				{
					if ((closestObject == nullptr) || (zmin < minDist))
					{
						closestObject = po;
						minDist = zmin;
					}
				}
			}
		}
	}

	// parse the selection buffer
	CCommand* pcmd = 0;
	string objName;
	if (closestObject != 0)
	{
		objName = closestObject->GetName();
		GLViewSettings& vs = m_glv->GetViewSettings();
		if (vs.m_selectAndHide)
		{
			pcmd = new CCmdHideObject(closestObject, true);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectObject(&model, closestObject);
			else pcmd = new CCmdSelectObject(&model, closestObject, m_bshift);
		}
	}
	else if ((m_bctrl == false) && (m_bshift == false))
	{
		// this clears the selection, but we only do this when there is an object currently selected
		FESelection* sel = pdoc->GetCurrentSelection();
		if (sel && sel->Size()) pcmd = new CCmdSelectObject(&model, 0, false);
		objName = "<Empty>";
	}

	// (un)select the mesh(es)
	if (pcmd) pdoc->DoCommand(pcmd, objName);
}

//-----------------------------------------------------------------------------
// Select parts
void GLViewSelector::SelectParts(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	if (model.Parts() == 0) return;

	// convert the point to a ray
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);
	Ray ray = transform.PointToRay(x, y);

	GPart* closestPart = 0;
	Intersection q;
	double minDist = 0;
//	double* a = m_glv->PlaneCoordinates();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			Transform& T = po->GetRenderTransform();

			GLMesh* mesh = po->GetRenderMesh();
			if (mesh)
			{
				int NF = mesh->Faces();
				for (int j = 0; j < NF; ++j)
				{
					GLMesh::FACE& face = mesh->Face(j);

					vec3d r0 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[0]).r));
					vec3d r1 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[1]).r));
					vec3d r2 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[2]).r));

					Triangle tri = { r0, r1, r2 };
					if (IntersectTriangle(ray, tri, q))
					{
	//					if ((m_glv->ShowPlaneCut() == false) || (q.point.x * a[0] + q.point.y * a[1] + q.point.z * a[2] + a[3] > 0))
						{
							double distance = ray.direction * (q.point - ray.origin);
							if ((closestPart == 0) || ((distance >= 0.0) && (distance < minDist)))
							{
								GFace* gface = po->Face(face.pid);
								int pid = gface->m_nPID[0];
								GPart* part = po->Part(pid);
								if (part->IsVisible() && ((part->IsSelected() == false) || (m_bctrl)))
								{
									closestPart = part;
									minDist = distance;
								}
								else if (gface->m_nPID[1] >= 0)
								{
									pid = gface->m_nPID[1];
									part = po->Part(pid);
									if (part->IsVisible() && ((part->IsSelected() == false) || (m_bctrl)))
									{
										closestPart = part;
										minDist = distance;
									}
								}
								else if (gface->m_nPID[2] >= 0)
								{
									pid = gface->m_nPID[2];
									part = po->Part(pid);
									if (part->IsVisible() && ((part->IsSelected() == false) || (m_bctrl)))
									{
										closestPart = part;
										minDist = distance;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	CCommand* pcmd = 0;
	string partName;
	if (closestPart != 0)
	{
		partName = closestPart->GetName();
		int index = closestPart->GetID();
		GLViewSettings& vs = m_glv->GetViewSettings();
		if (vs.m_selectAndHide)
		{
			pcmd = new CCmdHideParts(&model, closestPart);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnSelectPart(&model, &index, 1);
			else pcmd = new CCmdSelectPart(&model, &index, 1, m_bshift);
		}
	}
	else if ((m_bctrl == false) && (m_bshift == false))
	{
		pcmd = new CCmdSelectPart(&model, 0, 0, false);
		partName = "<Empty>";
	}

	// execute command
	GLHighlighter::ClearHighlights();
	if (pcmd) pdoc->DoCommand(pcmd, partName);
}

//-----------------------------------------------------------------------------
// select faces
void GLViewSelector::SelectSurfaces(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	if (model.Surfaces() == 0) return;

	// convert the point to a ray
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);
	Ray ray = transform.PointToRay(x, y);

//	double* a = m_glv->PlaneCoordinates();
	GFace* closestSurface = 0;
	Intersection q;
	double minDist = 0;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			Transform& T = po->GetRenderTransform();

			GLMesh* mesh = po->GetRenderMesh();
			if (mesh)
			{
				int NF = mesh->Faces();
				for (int j = 0; j < NF; ++j)
				{
					GLMesh::FACE& face = mesh->Face(j);
					GFace* gface = po->Face(face.pid);
					if (po->IsFaceVisible(gface))
					{
						// NOTE: Note sure why I have a scale factor here. It was originally to 0.99, but I
						//       had to increase it. I suspect it is to overcome some z-fighting for overlapping surfaces, but not sure. 
						vec3d r0 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[0]).r * 0.99999));
						vec3d r1 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[1]).r * 0.99999));
						vec3d r2 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[2]).r * 0.99999));

						Triangle tri = { r0, r1, r2 };
						if (IntersectTriangle(ray, tri, q))
						{
//							if ((m_glv->ShowPlaneCut() == false) || (q.point.x * a[0] + q.point.y * a[1] + q.point.z * a[2] + a[3] > 0))
							{
								double distance = ray.direction * (q.point - ray.origin);
								if ((closestSurface == 0) || ((distance >= 0.0) && (distance < minDist)))
								{
									if ((gface->IsSelected() == false) || (m_bctrl))
									{
										closestSurface = po->Face(face.pid);
										minDist = distance;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	string surfName = "<Empty>";
	if (closestSurface != 0)
	{
		int index = closestSurface->GetID();
		if (m_bctrl) pcmd = new CCmdUnSelectSurface(&model, &index, 1);
		else pcmd = new CCmdSelectSurface(&model, &index, 1, m_bshift);
		surfName = closestSurface->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectSurface(&model, 0, 0, false);

	// execute command
	GLHighlighter::ClearHighlights();
	if (pcmd) pdoc->DoCommand(pcmd, surfName);
}

GEdge* GLViewSelector::SelectClosestEdge(GObject* po, GLViewTransform& transform, QRect& rt, double& zmin)
{
	GLMesh* mesh = po->GetRenderMesh(); assert(mesh);
	if (mesh == nullptr) return nullptr;

	Transform& T = po->GetRenderTransform();

//	double* a = m_glv->PlaneCoordinates();

	GEdge* closestEdge = nullptr;
	zmin = 0.0;

	int edges = mesh->Edges();
	for (int j = 0; j < edges; ++j)
	{
		GLMesh::EDGE& edge = mesh->Edge(j);

		vec3d r0 = T.LocalToGlobal(to_vec3d(mesh->Node(edge.n[0]).r));
		vec3d r1 = T.LocalToGlobal(to_vec3d(mesh->Node(edge.n[1]).r));

//		double d0 = r0.x * a[0] + r0.y * a[1] + r0.z * a[2] + a[3];
//		double d1 = r1.x * a[0] + r1.y * a[1] + r1.z * a[2] + a[3];

//		if ((m_glv->ShowPlaneCut() == false) || ((d0 > 0) || (d1 > 0)))
		{
			vec3d p0 = transform.WorldToScreen(r0);
			vec3d p1 = transform.WorldToScreen(r1);

			if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
			{
				if ((closestEdge == nullptr) || (p0.z < zmin))
				{
					closestEdge = po->Edge(edge.pid);
					zmin = p0.z;
				}
			}
		}
	}
	return closestEdge;
}

//-----------------------------------------------------------------------------
// select edges
void GLViewSelector::SelectEdges(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int NE = model.Edges();
	if (NE == 0) return;

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

//	double* a = m_glv->PlaneCoordinates();

	int Objects = model.Objects();
	GEdge* closestEdge = 0;
	double zmin = 0.0;
	for (int i = 0; i < Objects; ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			double z;
			GEdge* pe = SelectClosestEdge(po, transform, rt, z);
			if (pe)
			{
				if ((closestEdge == nullptr) || (z < zmin))
				{
					closestEdge = pe;
					zmin = z;
				}
			}
		}
	}

	CCommand* pcmd = 0;
	string edgeName = "<Empty>";
	if (closestEdge != nullptr)
	{
		int index = closestEdge->GetID();
		if (m_bctrl) pcmd = new CCmdUnSelectEdge(&model, &index, 1);
		else pcmd = new CCmdSelectEdge(&model, &index, 1, m_bshift);
		edgeName = closestEdge->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectEdge(&model, 0, 0, false);

	// execute command
	GLHighlighter::ClearHighlights();
	if (pcmd) pdoc->DoCommand(pcmd, edgeName);
}

//-----------------------------------------------------------------------------
// select nodes
void GLViewSelector::SelectNodes(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	int NN = model.Nodes();
	if (NN == 0) return;
	GNode* closestNode = 0;
	double zmin = 0.0;
//	double* a = m_glv->PlaneCoordinates();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			Transform& T = po->GetRenderTransform();

			int nodes = po->Nodes();
			for (int j = 0; j < nodes; ++j)
			{
				GNode& node = *po->Node(j);

				// don't select shape nodes
				if (node.Type() != NODE_SHAPE)
				{
					vec3d r0 = node.LocalPosition();
					vec3d r = T.LocalToGlobal(r0);

//					if ((m_glv->ShowPlaneCut() == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] >= 0))
					{
						vec3d p = transform.WorldToScreen(r);
						if (rt.contains(QPoint((int)p.x, (int)p.y)))
						{
							if ((closestNode == 0) || (p.z < zmin))
							{
								closestNode = &node;
								zmin = p.z;
							}
						}
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	string nodeName = "<Empty>";
	if (closestNode != 0)
	{
		int index = closestNode->GetID();
		assert(closestNode->Type() != NODE_SHAPE);
		if (m_bctrl) pcmd = new CCmdUnSelectNode(&model, &index, 1);
		else pcmd = new CCmdSelectNode(&model, &index, 1, m_bshift);
		nodeName = closestNode->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectNode(&model, 0, 0, false);

	// execute command
	GLHighlighter::ClearHighlights();
	if (pcmd) pdoc->DoCommand(pcmd, nodeName);
}

//-----------------------------------------------------------------------------
// select nodes
void GLViewSelector::SelectDiscrete(int x, int y)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// get the fe model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int ND = model.DiscreteObjects();
	if (ND == 0) return;


	int X = x;
	int Y = y;
	int S = 4;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	int index = -1;
	int comp = -1;
	float zmin = 0.f;
	for (int i = 0; i < ND; ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);

		if (dynamic_cast<GLinearSpring*>(po))
		{
			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(po);
			GNode* node0 = model.FindNode(ps->m_node[0]);
			GNode* node1 = model.FindNode(ps->m_node[1]);
			if (node0 && node1)
			{
				vec3d r0 = node0->Position();
				vec3d r1 = node1->Position();

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);

				if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
				{
					if ((index == -1) || (p0.z < zmin))
					{
						index = i;
						zmin = p0.z;
					}
				}
			}
		}
		else if (dynamic_cast<GGeneralSpring*>(po))
		{
			GGeneralSpring* ps = dynamic_cast<GGeneralSpring*>(po);
			GNode* node0 = model.FindNode(ps->m_node[0]);
			GNode* node1 = model.FindNode(ps->m_node[1]);
			if (node0 && node1)
			{
				vec3d r0 = node0->Position();
				vec3d r1 = node1->Position();

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);

				if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
				{
					if ((index == -1) || (p0.z < zmin))
					{
						index = i;
						zmin = p0.z;
					}
				}
			}
		}
		else if (dynamic_cast<GDiscreteElementSet*>(po))
		{
			GDiscreteElementSet* ps = dynamic_cast<GDiscreteElementSet*>(po);
			int NE = ps->size();
			for (int j = 0; j < NE; ++j)
			{
				GDiscreteElement& el = ps->element(j);

				GNode* node0 = model.FindNode(el.Node(0));
				GNode* node1 = model.FindNode(el.Node(1));
				if (node0 && node1)
				{
					vec3d r0 = node0->Position();
					vec3d r1 = node1->Position();

					vec3d p0 = transform.WorldToScreen(r0);
					vec3d p1 = transform.WorldToScreen(r1);

					if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
					{
						if ((index == -1) || (p0.z < zmin))
						{
							index = i;
							zmin = p0.z;
							comp = j;
						}
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (index >= 0)
	{
		GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(model.DiscreteObject(index));
		if (pds)
		{
			// TODO: Turn this into a command
			if (m_bctrl)
			{
				vector<int> elemList{ comp };
				pcmd = new CCmdUnSelectDiscreteElements(pds, elemList);
			}
			else
			{
				vector<int> elemList{ comp };
				pcmd = new CCmdSelectDiscreteElements(pds, elemList, m_bshift);
			}
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnSelectDiscrete(&model, &index, 1);
			else pcmd = new CCmdSelectDiscrete(&model, &index, 1, m_bshift);
		}
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectDiscrete(&model, 0, 0, false);

	// execute command
	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::SelectSurfaceFaces(int x, int y)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the active object
	GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(m_glv->GetActiveObject());
	if (po == 0) return;

	// get the surface mesh
	FSMeshBase* pm = po->GetSurfaceMesh();
	if (pm == 0) return;

	// convert the point to a ray
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);
	Ray ray = transform.PointToRay(x, y);

	// convert ray to local coordinates
	Transform& T = po->GetRenderTransform();
	ray.origin = T.GlobalToLocal(ray.origin);
	ray.direction = T.GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	CCommand* pcmd = 0;
	if (FindFaceIntersection(ray, *pm, q))
	{
		int index = q.m_index;
		if (view.m_bconn)
		{
			// get the list of connected faces
			vector<int> faceList = MeshTools::GetConnectedFaces(pm, index, (view.m_bmax ? view.m_fconn : 0.0), view.m_bpart);

			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, faceList);
			else pcmd = new CCmdSelectFaces(pm, faceList, m_bshift);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectFaces(pm, &index, 1);
			else pcmd = new CCmdSelectFaces(pm, &index, 1, m_bshift);
		}
	}
	else if (!m_bshift) pcmd = new CCmdSelectFaces(pm, 0, 0, false);

	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::SelectSurfaceEdges(int x, int y)
{
	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	Transform& T = po->GetRenderTransform();

	FSMeshBase* pmesh = po->GetEditableMesh();
	FSLineMesh* pm = po->GetEditableLineMesh();

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	vec3d o(0, 0, 0);
	vec3d O = transform.WorldToScreen(o);

	int index = -1;
	float zmin = 0.f;
	int NE = pm->Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		vec3d r0 = T.LocalToGlobal(pm->Node(edge.n[0]).r);
		vec3d r1 = T.LocalToGlobal(pm->Node(edge.n[1]).r);

		vec3d p0 = transform.WorldToScreen(r0);
		vec3d p1 = transform.WorldToScreen(r1);

		// make sure p0, p1 are in front of the camers
		if (((p0.x >= 0) || (p1.x >= 0)) && ((p0.y >= 0) || (p1.y >= 0)) &&
			(p0.z > -1) && (p0.z < 1) && (p1.z > -1) && (p1.z < 1))
		{
			// see if the edge intersects
			if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt))
			{
				if ((index == -1) || (p0.z < zmin))
				{
					index = i;
					zmin = p0.z;
				}
			}
		}
	}

	// parse the selection buffer
	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn)
		{
			vector<int> edgeList = MeshTools::GetConnectedEdgesOnLineMesh(pm, index, view.m_fconn, view.m_bmax);
			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, edgeList);
			else pcmd = new CCmdSelectFEEdges(pm, edgeList, m_bshift);
		}
		else if (view.m_bselpath)
		{
			static int lastIndex = -1;
			vector<int> edgeList;
			if ((lastIndex != -1) && (lastIndex != index))
			{
				edgeList = MeshTools::GetConnectedEdgesByPath(pmesh, lastIndex, index);
				lastIndex = index;
			}
			else
			{
				edgeList.push_back(index);
				lastIndex = index;
			}
			if (!edgeList.empty())
			{
				if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, edgeList);
				else pcmd = new CCmdSelectFEEdges(pm, edgeList, m_bshift);
			}
		}
		else
		{
			int num = (int)index;
			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &num, 1);
			else pcmd = new CCmdSelectFEEdges(pm, &num, 1, m_bshift);
		}
	}
	else if (!m_bshift) pcmd = new CCmdSelectFEEdges(pm, 0, 0, false);

	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::SelectSurfaceNodes(int x, int y)
{
	static int lastIndex = -1;

	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	Transform& T = po->GetRenderTransform();

	FSMeshBase* pm = po->GetEditableMesh();
	FSLineMesh* lineMesh = po->GetEditableLineMesh();
	if (lineMesh == 0) return;

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	int index = -1;
	float zmin = 0.f;
	int NN = lineMesh->Nodes();
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = lineMesh->Node(i);
		if (node.IsVisible() && ((view.m_bext == false) || node.IsExterior()))
		{
			vec3d r = T.LocalToGlobal(lineMesh->Node(i).r);

			vec3d p = transform.WorldToScreen(r);

			if (rt.contains(QPoint((int)p.x, (int)p.y)))
			{
				if ((index == -1) || (p.z < zmin))
				{
					index = i;
					zmin = p.z;
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn && pm)
		{
			MeshTools::TagConnectedNodes(pm, index, view.m_fconn, view.m_bmax, view.m_bpart, 1);
			lastIndex = -1;

			// fill the pint array
			int m = 0;
			vector<int> pint(pm->Nodes(), 0);
			for (int i = 0; i < pm->Nodes(); ++i)
				if (pm->Node(i).m_ntag == 1) pint[m++] = i;

			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, &pint[0], m);
			else pcmd = new CCmdSelectFENodes(pm, &pint[0], m, m_bshift);
		}
		else if (view.m_bselpath && pm)
		{
			vector<int> nodeList;
			if ((lastIndex != -1) && (lastIndex != index))
			{
				nodeList = MeshTools::GetConnectedNodesByPath(pm, lastIndex, index);
				lastIndex = index;
			}
			else
			{
				nodeList.push_back(index);
				lastIndex = index;
			}

			// fill the pint array
			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, nodeList);
			else pcmd = new CCmdSelectFENodes(pm, nodeList, m_bshift);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectNodes(lineMesh, &index, 1);
			else pcmd = new CCmdSelectFENodes(lineMesh, &index, 1, m_bshift);
			lastIndex = -1;
		}
	}
	else if (!m_bshift)
	{
		pcmd = new CCmdSelectFENodes(lineMesh, 0, 0, false);
		lastIndex = -1;
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::RegionSelectObjects(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	// get the document
	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();
	if (model.Objects() == 0) return;

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	vector<GObject*> selectedObjects;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		GLMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			Transform& T = po->GetRenderTransform();
			bool intersect = false;
			for (int j = 0; j < mesh->Faces(); ++j)
			{
				GLMesh::FACE& face = mesh->Face(j);

				vec3d r0 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[0]).r));
				vec3d r1 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[1]).r));
				vec3d r2 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[2]).r));

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);
				vec3d p2 = transform.WorldToScreen(r2);

				if (region.TriangleIntersect((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y))
				{
					selectedObjects.push_back(po);
					intersect = true;
					break;
				}
			}

			// check nodes too
			if (intersect == false)
			{
				for (int j = 0; j < mesh->Nodes(); ++j)
				{
					GLMesh::NODE& node = mesh->Node(j);

					vec3d r = T.LocalToGlobal(to_vec3d(node.r));
					vec3d p = transform.WorldToScreen(r);
					if (region.IsInside((int)p.x, (int)p.y))
					{
						selectedObjects.push_back(po);
						intersect = true;
						break;
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnselectObject(&model, selectedObjects);
	else pcmd = new CCmdSelectObject(&model, selectedObjects, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::RegionSelectParts(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	// get the document
	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	if (model.Parts() == 0) return;

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	std::list<GPart*> selectedParts;
	vector<int> selectedPartIds;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		GLMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			Transform& T = po->GetRenderTransform();

			for (int j = 0; j < mesh->Faces(); ++j)
			{
				GLMesh::FACE& face = mesh->Face(j);

				vec3d r0 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[0]).r));
				vec3d r1 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[1]).r));
				vec3d r2 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[2]).r));

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);
				vec3d p2 = transform.WorldToScreen(r2);

				if (region.TriangleIntersect((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y))
				{
					GFace* gface = po->Face(face.pid);
					GPart* part = po->Part(gface->m_nPID[0]);
					if (part && part->IsVisible())
					{
						int pid = part->GetID();

						// make sure that this surface is not added yet
						bool bfound = false;
						for (int k = 0; k < selectedParts.size(); ++k)
						{
							if (selectedPartIds[k] == pid)
							{
								bfound = true;
								break;
							}
						}

						if (bfound == false)
						{
							selectedPartIds.push_back(pid);
							selectedParts.push_back(part);
						}
					}
				}
			}
		}
	}

	if (!selectedParts.empty())
	{
		CCommand* pcmd = 0;
		if (view.m_selectAndHide)
		{
			pcmd = new CCmdHideParts(&model, selectedParts);
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnSelectPart(&model, selectedPartIds);
			else pcmd = new CCmdSelectPart(&model, selectedPartIds, m_bshift);
		}
		if (pcmd) pdoc->DoCommand(pcmd);
	}
}

void GLViewSelector::RegionSelectSurfaces(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	// get the document
	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int nSurfaces = model.Surfaces();
	if (nSurfaces == 0) return;

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	vector<int> selectedSurfaces;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		GLMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			Transform& T = po->GetRenderTransform();

			for (int j = 0; j < mesh->Faces(); ++j)
			{
				GLMesh::FACE& face = mesh->Face(j);

				vec3d r0 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[0]).r));
				vec3d r1 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[1]).r));
				vec3d r2 = T.LocalToGlobal(to_vec3d(mesh->Node(face.n[2]).r));

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);
				vec3d p2 = transform.WorldToScreen(r2);

				if (region.TriangleIntersect((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y))
				{
					int pid = po->Face(face.pid)->GetID();

					// make sure that this surface is not added yet
					bool bfound = false;
					for (int k = 0; k < selectedSurfaces.size(); ++k)
					{
						if (selectedSurfaces[k] == pid)
						{
							bfound = true;
							break;
						}
					}

					if (bfound == false) selectedSurfaces.push_back(pid);
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectSurface(&model, selectedSurfaces);
	else pcmd = new CCmdSelectSurface(&model, selectedSurfaces, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}


void GLViewSelector::RegionSelectEdges(const SelectRegion& region)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = pdoc->GetSelectionStyle();

	// Get the model
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	vector<int> selectedEdges;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			Transform& T = po->GetRenderTransform();

			for (int j = 0; j < po->Edges(); ++j)
			{
				GEdge* edge = po->Edge(j);
				int* n = edge->m_node;

				if ((n[0] >= 0) && (n[1] >= 0))
				{
					vec3d r0 = T.LocalToGlobal(po->Node(n[0])->LocalPosition());
					vec3d r1 = T.LocalToGlobal(po->Node(n[1])->LocalPosition());

					vec3d p0 = transform.WorldToScreen(r0);
					vec3d p1 = transform.WorldToScreen(r1);

					int x0 = (int)p0.x;
					int y0 = (int)p0.y;
					int x1 = (int)p1.x;
					int y1 = (int)p1.y;

					if (region.LineIntersects(x0, y0, x1, y1))
					{
						selectedEdges.push_back(edge->GetID());
					}
				}
			}
		}
	}


	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectEdge(&model, selectedEdges);
	else pcmd = new CCmdSelectEdge(&model, selectedEdges, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
}

void GLViewSelector::RegionSelectNodes(const SelectRegion& region)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (doc == nullptr) return;

	// get the document
	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = doc->GetSelectionStyle();

	// Get the model
	FSModel* ps = doc->GetFSModel();
	GModel& model = ps->GetModel();

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);
//	double* a = m_glv->PlaneCoordinates();

	vector<int> selectedNodes;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			Transform& T = po->GetRenderTransform();
			for (int j = 0; j < po->Nodes(); ++j)
			{
				GNode* node = po->Node(j);

				// don't select shape nodes
				if (node->Type() != NODE_SHAPE)
				{
					vec3d r = T.LocalToGlobal(node->LocalPosition());

//					if ((m_glv->ShowPlaneCut() == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] >= 0.0))
					{
						vec3d p = transform.WorldToScreen(r);

						if (region.IsInside((int)p.x, (int)p.y))
						{
							selectedNodes.push_back(node->GetID());
						}
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectNode(&model, selectedNodes);
	else pcmd = new CCmdSelectNode(&model, selectedNodes, m_bshift);
	if (pcmd) doc->DoCommand(pcmd);
}

void GLViewSelector::RegionSelectDiscrete(const SelectRegion& region)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_glv->GetDocument());
	if (doc == nullptr) return;

	// get the document
	GLViewSettings& view = m_glv->GetViewSettings();
	int nsel = doc->GetSelectionStyle();

	// Get the model
	FSModel* ps = doc->GetFSModel();
	GModel& model = ps->GetModel();

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	vector<GDiscreteObject*> selectedObjects;

	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);

		if (dynamic_cast<GLinearSpring*>(po))
		{
			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(po);

			vec3d r0 = model.FindNode(ps->m_node[0])->Position();
			vec3d r1 = model.FindNode(ps->m_node[1])->Position();

			vec3d p0 = transform.WorldToScreen(r0);
			vec3d p1 = transform.WorldToScreen(r1);

			int x0 = (int)p0.x;
			int y0 = (int)p0.y;
			int x1 = (int)p1.x;
			int y1 = (int)p1.y;

			if (region.LineIntersects(x0, y0, x1, y1))
			{
				selectedObjects.push_back(ps);
			}
		}
		else if (dynamic_cast<GDiscreteSpringSet*>(po))
		{
			GDiscreteSpringSet* set = dynamic_cast<GDiscreteSpringSet*>(po);
			for (int n = 0; n < set->size(); ++n)
			{
				GDiscreteElement& el = set->element(n);
				vec3d r0 = model.FindNode(el.Node(0))->Position();
				vec3d r1 = model.FindNode(el.Node(1))->Position();

				vec3d p0 = transform.WorldToScreen(r0);
				vec3d p1 = transform.WorldToScreen(r1);

				int x0 = (int)p0.x;
				int y0 = (int)p0.y;
				int x1 = (int)p1.x;
				int y1 = (int)p1.y;

				if (region.LineIntersects(x0, y0, x1, y1))
				{
					selectedObjects.push_back(&el);
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectDiscrete(&model, selectedObjects);
	else pcmd = new CCmdSelectDiscrete(&model, selectedObjects, m_bshift);
	if (pcmd) doc->DoCommand(pcmd);
}

void GLViewSelector::SelectFENodes(int x, int y)
{
	static int lastIndex = -1;

	// get the document
	CGLDocument* pdoc = m_glv->GetDocument();
	if (pdoc == nullptr) return;

	GLViewSettings& view = m_glv->GetViewSettings();

	// Get the mesh
	GObject* po = m_glv->GetActiveObject();
	if (po == 0) return;

	Transform& T = po->GetRenderTransform();

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

//	double* a = m_glv->PlaneCoordinates();
	int index = -1;
	int globalIndex = -1;
	float zmin = 0.f;
	int NN = pm->Nodes();
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsVisible() && ((view.m_bext == false) || node.IsExterior()))
		{
			vec3d r = T.LocalToGlobal(pm->Node(i).r);
//			double D = r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3];

//			if ((m_glv->ShowPlaneCut() == false) || (D >= 0))
			{
				vec3d p = transform.WorldToScreen(r);

				if (rt.contains(QPoint((int)p.x, (int)p.y)))
				{
					if ((index == -1) || (p.z < zmin))
					{
						index = i;
						globalIndex = node.m_nid;
						zmin = p.z;
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn && pm)
		{
			vector<int> nodeList;
			nodeList = MeshTools::GetConnectedNodes(pm, index, view.m_fconn, view.m_bmax, view.m_bpart);
			lastIndex = -1;
			if (!nodeList.empty())
			{
				if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, nodeList);
				else pcmd = new CCmdSelectFENodes(pm, nodeList, m_bshift);
			}
		}
		else if (view.m_bselpath && pm)
		{
			vector<int> nodeList;
			if ((lastIndex != -1) && (lastIndex != index))
			{
				nodeList = MeshTools::GetConnectedNodesByPath(pm, lastIndex, index);
				lastIndex = index;
			}
			else
			{
				nodeList.push_back(index);
				lastIndex = index;
			}
			if (!nodeList.empty())
			{
				if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, nodeList);
				else
				{
					pcmd = new CCmdSelectFENodes(pm, nodeList, m_bshift);
				}
			}
		}
		else
		{
			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, &index, 1);
			else
			{
				pcmd = new CCmdSelectFENodes(pm, &index, 1, m_bshift);

				// print value of currently selected node
				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(pdoc);
				if (postDoc && postDoc->IsValid())
				{
					Post::FEPostModel* fem = postDoc->GetFSModel();
					Post::FEState* state = fem->CurrentState();
					FSNode& node = pm->Node(index);
					vec3f r = state->m_NODE[index].m_rt;
					QString txt = QString("Node %1 : position = (%2, %3, %4)").arg(node.m_nid).arg(r.x).arg(r.y).arg(r.z);

					Post::CGLColorMap* cmap = postDoc->GetGLModel()->GetColorMap();
					if (cmap && cmap->IsActive())
					{
						double val = state->m_NODE[index].m_val;
						txt += QString(", value = %1").arg(val);
					}

					FBS::getMainWindow()->AddLogEntry(txt + QString("\n"));
				}
			}
			lastIndex = -1;
		}
	}
	else if (!m_bshift)
	{
		int nsel = pm->CountSelectedNodes();
		if (nsel > 0)
		{
			pcmd = new CCmdSelectFENodes(pm, 0, 0, false);
		}
		lastIndex = -1;
	}

	if (pcmd) pdoc->DoCommand(pcmd);
}

