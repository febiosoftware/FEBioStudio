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
#include "Logger.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FENodeEdgeList.h>
#include <PostGL/GLModel.h>

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

	double* a = m_glv->PlaneCoordinates();

	vector<int> selectedNodes;
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsVisible() && (node.m_ntag == 0))
		{
			vec3d r = po->GetTransform().LocalToGlobal(node.r);

			if ((m_glv->ShowPlaneCut() == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] >= 0.0))
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

	FSMesh* pm = po->GetFEMesh();

	// activate the gl rendercontext
	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	if (view.m_bcullSel)
	{
		TagBackfacingElements(*pm);
	}
	else pm->TagAllElements(0);

	double* a = m_glv->PlaneCoordinates();

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
					FEElement_* pej = (nbr >= 0 ? pm->ElementPtr(nbr) : nullptr);
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
					vec3d r = po->GetTransform().LocalToGlobal(pm->Node(el.m_node[j]).r);

					if ((m_glv->ShowPlaneCut()  == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] > 0))
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
	if (m_bctrl) pcmd = new CCmdUnselectElements(pm, selectedElements);
	else pcmd = new CCmdSelectElements(pm, selectedElements, m_bshift);
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

			if (b && m_glv->ShowPlaneCut())
			{
				double* a = m_glv->PlaneCoordinates();
				b = false;
				vec3d r[FSFace::MAX_NODES];
				pm->FaceNodePosition(face, r);
				for (int j = 0; j < face.Nodes(); ++j)
				{
					vec3d p = pm->LocalToGlobal(r[j]);
					if (p.x * a[0] + p.y * a[1] + p.z * a[2] + a[3] > 0)
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

	double* a = m_glv->PlaneCoordinates();
	vector<int> selectedEdges;
	int NE = pm->Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.IsVisible() && (edge.m_ntag == 0))
		{
			vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[0]).r);
			vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[1]).r);

			double d0 = r0.x * a[0] + r0.y * a[1] + r0.z * a[2] + a[3];
			double d1 = r1.x * a[0] + r1.y * a[1] + r1.z * a[2] + a[3];

			if ((m_glv->ShowPlaneCut() == false) || ((d0 >= 0) || (d1 >= 0)))
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
	ray.origin = po->GetTransform().GlobalToLocal(ray.origin);
	ray.direction = po->GetTransform().GlobalToLocalNormal(ray.direction);

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
			stack<int> S;
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
	mesh.UpdateSelection();
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
vector<int> FindConnectedElements(FSMesh* pm, int startIndex, double fconn, bool bpart, bool exteriorOnly, bool bmax)
{
	FEElement_* pe, * pe2;
	int elems = pm->Elements();
	vector<int> elemList; elemList.reserve(elems);

	for (int i = 0; i < pm->Elements(); ++i) pm->Element(i).m_ntag = i;
	std::stack<FEElement_*> stack;

	// push the first element to the stack
	pe = pm->ElementPtr(startIndex);
	pe->m_ntag = -1;
	elemList.push_back(startIndex);
	stack.push(pe);

	double tr = -2;
	vec3d t(0, 0, 0);
	if (pe->IsShell())
	{
		assert(pe->m_face[0] >= 0);
		t = to_vec3d(pm->Face(pe->m_face[0]).m_fn); tr = cos(PI * fconn / 180.0);
	}

	// get the respect partition boundary flag
	int gid = pe->m_gid;

	// now push the rest
	int n;
	while (!stack.empty())
	{
		pe = stack.top(); stack.pop();

		// solid elements
		n = pe->Faces();
		for (int i = 0; i < n; ++i)
			if (pe->m_nbr[i] >= 0)
			{
				pe2 = pm->ElementPtr(pe->m_nbr[i]);
				if (pe2->m_ntag >= 0 && pe2->IsVisible())
				{
					if ((exteriorOnly == false) || pe2->IsExterior())
					{
						int fid2 = -1;
						if (pe->m_face[i] >= 0)
						{
							FSFace& f2 = pm->Face(pe->m_face[i]);
							fid2 = f2.m_gid;
						}

						if ((bpart == false) || ((pe2->m_gid == gid) && (fid2 == -1)))
						{
							elemList.push_back(pe2->m_ntag);
							pe2->m_ntag = -1;
							stack.push(pe2);
						}
					}
				}
			}

		// shell elements
		n = pe->Edges();
		for (int i = 0; i < n; ++i)
			if (pe->m_nbr[i] >= 0)
			{
				pe2 = pm->ElementPtr(pe->m_nbr[i]);
				if (pe2->m_ntag >= 0 && pe2->IsVisible())
				{
					int eface = pe2->m_face[0]; assert(eface >= 0);
					if (eface >= 0)
					{
						if ((bmax == false) || (pm->Face(eface).m_fn * to_vec3f(t) >= tr))
						{
							if ((bpart == false) || (pe2->m_gid == gid))
							{
								elemList.push_back(pe2->m_ntag);
								pe2->m_ntag = -1;
								stack.push(pe2);
							}
						}
					}
				}
			}
	}

	return elemList;
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

	int index = -1;
	float zmin = 0.f;
	int NE = pm->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& del = pm->Element(i);
		if (del.IsBeam() && del.IsVisible())
		{
			vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(del.m_node[0]).r);
			vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(del.m_node[1]).r);

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

	// convert ray to local coordinates
	Ray localRay;
	localRay.origin = po->GetTransform().GlobalToLocal(ray.origin);
	localRay.direction = po->GetTransform().GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	bool bfound = FindElementIntersection(localRay, *pm, q, m_bctrl);

	// see if the intersection with the plane cut is closer
	if (bfound && m_glv->ShowPlaneCut())
	{
		vec3d p = po->GetTransform().LocalToGlobal(q.point);
		GLPlaneCut& planeCut = m_glv->GetPlaneCut();
		bool bintersect = planeCut.Intersect(p, ray, q);
		if (bintersect) bfound = bintersect;
	}

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
			vector<int> pint = FindConnectedElements(pm, index, view.m_fconn, view.m_bpart, view.m_bext, view.m_bmax);
			int N = (int)pint.size();

			if (m_bctrl) pcmd = new CCmdUnselectElements(pm, &pint[0], N);
			else pcmd = new CCmdSelectElements(pm, &pint[0], N, m_bshift);
		}
		else
		{
			int num = (int)index;
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
						CLogger::AddLogEntry(txt);
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

	m_glv->DeletePlaneCutMesh();
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
	ray.origin = po->GetTransform().GlobalToLocal(ray.origin);
	ray.direction = po->GetTransform().GlobalToLocalNormal(ray.direction);

	// find the intersection
	Intersection q;
	CCommand* pcmd = 0;

	bool bfound = FindFaceIntersection(ray, *pm, q);

	if (bfound && m_glv->ShowPlaneCut())
	{
		vec3d p = po->GetTransform().LocalToGlobal(q.point);

		// see if the intersection lies behind the plane cut. 
		double* a = m_glv->PlaneCoordinates();
		double d = p.x * a[0] + p.y * a[1] + p.z * a[2] + a[3];
		if (d < 0)
		{
			bfound = false;
		}
	}

	if (bfound)
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
						CLogger::AddLogEntry(txt);
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
	double* a = m_glv->PlaneCoordinates();

	int index = -1;
	float zmin = 0.f;
	int NE = pm->Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[0]).r);
		vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[1]).r);

		vec3d p0 = transform.WorldToScreen(r0);
		vec3d p1 = transform.WorldToScreen(r1);

		// make sure p0, p1 are in front of the camera
		if (((p0.x >= 0) || (p1.x >= 0)) && ((p0.y >= 0) || (p1.y >= 0)) &&
			(p0.z > -1) && (p0.z < 1) && (p1.z > -1) && (p1.z < 1))
		{
			// see if the edge intersects
			bool bfound = intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt);

			if (bfound && m_glv->ShowPlaneCut())
			{
				// make sure one point is in front of plane
				double d0 = r0.x * a[0] + r0.y * a[1] + r0.z * a[2] + a[3];
				double d1 = r1.x * a[0] + r1.y * a[1] + r1.z * a[2] + a[3];
				if ((d0 < 0) && (d1 < 0)) bfound = false;
			}

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
	if (index >= 0)
	{
		if (view.m_bconn)
		{
			vector<int> pint(pm->Edges());
			int m = 0;

			for (int i = 0; i < pm->Edges(); ++i) pm->Edge(i).m_ntag = i;
			std::stack<FSEdge*> stack;

			FSNodeEdgeList NEL(pm);

			// push the first face to the stack
			FSEdge* pe = pm->EdgePtr(index);
			pint[m++] = index;
			pe->m_ntag = -1;
			stack.push(pe);

			int gid = pe->m_gid;

			// setup the direction vector
			vec3d& r0 = pm->Node(pe->n[0]).r;
			vec3d& r1 = pm->Node(pe->n[1]).r;
			vec3d t1 = r1 - r0; t1.Normalize();

			// angle tolerance
			double wtol = 1.000001 * cos(PI * view.m_fconn / 180.0); // scale factor to address some numerical round-off issue when selecting 180 degrees

			// now push the rest
			while (!stack.empty())
			{
				pe = stack.top(); stack.pop();

				for (int i = 0; i < 2; ++i)
				{
					int n = NEL.Edges(pe->n[i]);
					for (int j = 0; j < n; ++j)
					{
						int edgeID = NEL.Edge(pe->n[i], j)->m_ntag;
						if (edgeID >= 0)
						{
							FSEdge* pe2 = pm->EdgePtr(edgeID);
							vec3d& r0 = pm->Node(pe2->n[0]).r;
							vec3d& r1 = pm->Node(pe2->n[1]).r;
							vec3d t2 = r1 - r0; t2.Normalize();
							if (pe2->IsVisible() && ((view.m_bmax == false) || (fabs(t1 * t2) >= wtol)) && ((gid == -1) || (pe2->m_gid == gid)))
							{
								pint[m++] = pe2->m_ntag;
								pe2->m_ntag = -1;
								stack.push(pe2);
							}
						}
					}
				}
			}

			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &pint[0], m);
			else pcmd = new CCmdSelectFEEdges(pm, &pint[0], m, m_bshift);
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
						CLogger::AddLogEntry(txt);
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
	GMesh* mesh = po->GetRenderMesh();
	if (mesh == nullptr) return false;

	Intersection qtmp;
	double distance = 0.0, minDist = 1e34;
	int NF = mesh->Faces();
	bool intersect = false;
	for (int j = 0; j < NF; ++j)
	{
		GMesh::FACE& face = mesh->Face(j);

		if (po->Face(face.pid)->IsVisible())
		{
			vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
			vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
			vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

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
		if (m_bctrl) pcmd = new CCmdUnselectObject(&model, closestObject);
		else pcmd = new CCmdSelectObject(&model, closestObject, m_bshift);
		objName = closestObject->GetName();
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
	double* a = m_glv->PlaneCoordinates();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			GMesh* mesh = po->GetRenderMesh();
			if (mesh)
			{
				int NF = mesh->Faces();
				for (int j = 0; j < NF; ++j)
				{
					GMesh::FACE& face = mesh->Face(j);

					vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
					vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
					vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

					Triangle tri = { r0, r1, r2 };
					if (IntersectTriangle(ray, tri, q))
					{
						if ((m_glv->ShowPlaneCut() == false) || (q.point.x * a[0] + q.point.y * a[1] + q.point.z * a[2] + a[3] > 0))
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
		int index = closestPart->GetID();
		if (m_bctrl) pcmd = new CCmdUnSelectPart(&model, &index, 1);
		else pcmd = new CCmdSelectPart(&model, &index, 1, m_bshift);
		partName = closestPart->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false))
	{
		pcmd = new CCmdSelectPart(&model, 0, 0, false);
		partName = "<Empty>";
	}

	// execute command
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

	double* a = m_glv->PlaneCoordinates();
	GFace* closestSurface = 0;
	Intersection q;
	double minDist = 0;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			GMesh* mesh = po->GetRenderMesh();
			if (mesh)
			{
				int NF = mesh->Faces();
				for (int j = 0; j < NF; ++j)
				{
					GMesh::FACE& face = mesh->Face(j);
					GFace* gface = po->Face(face.pid);
					if (po->IsFaceVisible(gface))
					{
						// NOTE: Note sure why I have a scale factor here. It was originally to 0.99, but I
						//       had to increase it. I suspect it is to overcome some z-fighting for overlapping surfaces, but not sure. 
						vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r * 0.99999);
						vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r * 0.99999);
						vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r * 0.99999);

						Triangle tri = { r0, r1, r2 };
						if (IntersectTriangle(ray, tri, q))
						{
							if ((m_glv->ShowPlaneCut() == false) || (q.point.x * a[0] + q.point.y * a[1] + q.point.z * a[2] + a[3] > 0))
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
		surfName = ps->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectSurface(&model, 0, 0, false);

	// execute command
	if (pcmd) pdoc->DoCommand(pcmd, surfName);
}

GEdge* GLViewSelector::SelectClosestEdge(GObject* po, GLViewTransform& transform, QRect& rt, double& zmin)
{
	GMesh* mesh = po->GetRenderMesh(); assert(mesh);
	if (mesh == nullptr) return nullptr;

	Transform& T = po->GetTransform();

	double* a = m_glv->PlaneCoordinates();

	GEdge* closestEdge = nullptr;
	zmin = 0.0;

	int edges = mesh->Edges();
	for (int j = 0; j < edges; ++j)
	{
		GMesh::EDGE& edge = mesh->Edge(j);

		vec3d r0 = T.LocalToGlobal(mesh->Node(edge.n[0]).r);
		vec3d r1 = T.LocalToGlobal(mesh->Node(edge.n[1]).r);

		double d0 = r0.x * a[0] + r0.y * a[1] + r0.z * a[2] + a[3];
		double d1 = r1.x * a[0] + r1.y * a[1] + r1.z * a[2] + a[3];

		if ((m_glv->ShowPlaneCut() == false) || ((d0 > 0) || (d1 > 0)))
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

	double* a = m_glv->PlaneCoordinates();

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
		edgeName = ps->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectEdge(&model, 0, 0, false);

	// execute command
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
	double* a = m_glv->PlaneCoordinates();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			int nodes = po->Nodes();
			for (int j = 0; j < nodes; ++j)
			{
				GNode& node = *po->Node(j);

				// don't select shape nodes
				if (node.Type() != NODE_SHAPE)
				{
					vec3d r = node.Position();

					if ((m_glv->ShowPlaneCut() == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] >= 0))
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
		nodeName = ps->GetName();
	}
	else if ((m_bctrl == false) && (m_bshift == false)) pcmd = new CCmdSelectNode(&model, 0, 0, false);

	// execute command
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
	ray.origin = po->GetTransform().GlobalToLocal(ray.origin);
	ray.direction = po->GetTransform().GlobalToLocalNormal(ray.direction);

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
		vec3d r0 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[0]).r);
		vec3d r1 = po->GetTransform().LocalToGlobal(pm->Node(edge.n[1]).r);

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
			vector<int> pint(pm->Edges());
			int m = 0;

			for (int i = 0; i < pm->Edges(); ++i) pm->Edge(i).m_ntag = i;
			std::stack<FSEdge*> stack;

			FSNodeEdgeList NEL(pm);

			// push the first face to the stack
			FSEdge* pe = pm->EdgePtr(index);
			pint[m++] = index;
			pe->m_ntag = -1;
			stack.push(pe);

			int gid = pe->m_gid;

			// setup the direction vector
			vec3d& r0 = pm->Node(pe->n[0]).r;
			vec3d& r1 = pm->Node(pe->n[1]).r;
			vec3d t1 = r1 - r0; t1.Normalize();

			// angle tolerance
			double wtol = 1.000001 * cos(PI * view.m_fconn / 180.0); // scale factor to address some numerical round-off issue when selecting 180 degrees

			// now push the rest
			while (!stack.empty())
			{
				pe = stack.top(); stack.pop();

				for (int i = 0; i < 2; ++i)
				{
					int n = NEL.Edges(pe->n[i]);
					for (int j = 0; j < n; ++j)
					{
						int edgeID = NEL.Edge(pe->n[i], j)->m_ntag;
						if (edgeID >= 0)
						{
							FSEdge* pe2 = pm->EdgePtr(edgeID);
							vec3d& r0 = pm->Node(pe2->n[0]).r;
							vec3d& r1 = pm->Node(pe2->n[1]).r;
							vec3d t2 = r1 - r0; t2.Normalize();
							if (pe2->IsVisible() && ((view.m_bmax == false) || (fabs(t1 * t2) >= wtol)) && ((gid == -1) || (pe2->m_gid == gid)))
							{
								pint[m++] = pe2->m_ntag;
								pe2->m_ntag = -1;
								stack.push(pe2);
							}
						}
					}
				}
			}

			if (m_bctrl) pcmd = new CCmdUnselectFEEdges(pm, &pint[0], m);
			else pcmd = new CCmdSelectFEEdges(pm, &pint[0], m, m_bshift);
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
			vec3d r = po->GetTransform().LocalToGlobal(lineMesh->Node(i).r);

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
			vector<int> pint(pm->Nodes(), 0);

			if (view.m_bselpath == false)
			{
				MeshTools::TagConnectedNodes(pm, index, view.m_fconn, view.m_bmax);
				lastIndex = -1;
			}
			else
			{
				if ((lastIndex != -1) && (lastIndex != index))
				{
					MeshTools::TagNodesByShortestPath(pm, lastIndex, index);
					lastIndex = index;
				}
				else
				{
					pm->TagAllNodes(0);
					pm->Node(index).m_ntag = 1;
					lastIndex = index;
				}
			}

			// fill the pint array
			int m = 0;
			for (int i = 0; i < pm->Nodes(); ++i)
				if (pm->Node(i).m_ntag == 1) pint[m++] = i;

			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, &pint[0], m);
			else pcmd = new CCmdSelectFENodes(pm, &pint[0], m, m_bshift);
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
		GMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			bool intersect = false;
			for (int j = 0; j < mesh->Faces(); ++j)
			{
				GMesh::FACE& face = mesh->Face(j);

				vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
				vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
				vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

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
					GMesh::NODE& node = mesh->Node(j);

					vec3d r = po->GetTransform().LocalToGlobal(node.r);
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

	vector<int> selectedParts;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		GMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			for (int j = 0; j < mesh->Faces(); ++j)
			{
				GMesh::FACE& face = mesh->Face(j);

				vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
				vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
				vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

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
							if (selectedParts[k] == pid)
							{
								bfound = true;
								break;
							}
						}

						if (bfound == false) selectedParts.push_back(pid);
					}
				}
			}
		}
	}

	CCommand* pcmd = 0;
	if (m_bctrl) pcmd = new CCmdUnSelectPart(&model, selectedParts);
	else pcmd = new CCmdSelectPart(&model, selectedParts, m_bshift);
	if (pcmd) pdoc->DoCommand(pcmd);
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
		GMesh* mesh = po->GetRenderMesh();
		if (po->IsVisible() && mesh)
		{
			for (int j = 0; j < mesh->Faces(); ++j)
			{
				GMesh::FACE& face = mesh->Face(j);

				vec3d r0 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[0]).r);
				vec3d r1 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[1]).r);
				vec3d r2 = po->GetTransform().LocalToGlobal(mesh->Node(face.n[2]).r);

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
			for (int j = 0; j < po->Edges(); ++j)
			{
				GEdge* edge = po->Edge(j);
				int* n = edge->m_node;

				if ((n[0] >= 0) && (n[1] >= 0))
				{
					vec3d r0 = po->Node(n[0])->Position();
					vec3d r1 = po->Node(n[1])->Position();

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
	double* a = m_glv->PlaneCoordinates();

	vector<int> selectedNodes;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsVisible())
		{
			for (int j = 0; j < po->Nodes(); ++j)
			{
				GNode* node = po->Node(j);

				// don't select shape nodes
				if (node->Type() != NODE_SHAPE)
				{
					vec3d r = node->Position();

					if ((m_glv->ShowPlaneCut() == false) || (r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3] >= 0.0))
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

	vector<int> selectedObjects;

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
				selectedObjects.push_back(i);
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

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	int X = x;
	int Y = y;
	int S = 6;
	QRect rt(X - S, Y - S, 2 * S, 2 * S);

	m_glv->makeCurrent();
	GLViewTransform transform(m_glv);

	double* a = m_glv->PlaneCoordinates();
	int index = -1;
	float zmin = 0.f;
	int NN = pm->Nodes();
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsVisible() && ((view.m_bext == false) || node.IsExterior()))
		{
			vec3d r = po->GetTransform().LocalToGlobal(pm->Node(i).r);
			double D = r.x * a[0] + r.y * a[1] + r.z * a[2] + a[3];

			if ((m_glv->ShowPlaneCut() == false) || (D >= 0))
			{
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
	}

	CCommand* pcmd = 0;
	if (index >= 0)
	{
		if (view.m_bconn && pm)
		{
			vector<int> pint(pm->Nodes(), 0);

			if (view.m_bselpath == false)
			{
				MeshTools::TagConnectedNodes(pm, index, view.m_fconn, view.m_bmax);
				lastIndex = -1;
			}
			else
			{
				if ((lastIndex != -1) && (lastIndex != index))
				{
					MeshTools::TagNodesByShortestPath(pm, lastIndex, index);
					lastIndex = index;
				}
				else
				{
					pm->TagAllNodes(0);
					pm->Node(index).m_ntag = 1;
					lastIndex = index;
				}
			}

			// fill the pint array
			int m = 0;
			for (int i = 0; i < pm->Nodes(); ++i)
				if (pm->Node(i).m_ntag == 1) pint[m++] = i;

			if (m_bctrl) pcmd = new CCmdUnselectNodes(pm, &pint[0], m);
			else pcmd = new CCmdSelectFENodes(pm, &pint[0], m, m_bshift);
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

					CLogger::AddLogEntry(txt + QString("\n"));
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
