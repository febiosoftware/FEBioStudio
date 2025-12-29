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
#include "PostObject.h"
#include "GLModel.h"
#include <array>

CPostObject::CPostObject(Post::CGLModel* glm) : GMeshObject((FSMesh*)nullptr)
{
	// store the model
	m_glm = glm;
}

CPostObject::~CPostObject()
{
	// The mesh is owned by the CGLModel
	// so we have to set the mesh to zero here, otherwise the GObject baseclass will try to 
	// delete it as well
	SetFEMesh(nullptr);
	ClearInternalSurfaces();
}

BOX CPostObject::GetBoundingBox()
{
	FSMesh* mesh = GetFEMesh();
	if (mesh) return mesh->GetBoundingBox();
	else return BOX();
}

void CPostObject::ClearInternalSurfaces()
{
	for (int i = 0; i < (int)m_innerSurface.size(); ++i) delete m_innerSurface[i];
	m_innerSurface.clear();
}

void CPostObject::UpdateMesh()
{
	GLMesh* mesh = GetFERenderMesh(); assert(mesh);
	if (mesh == nullptr) return;

	FSMesh* pm = GetFEMesh();
	if (pm == nullptr)
	{
		SetFERenderMesh(nullptr);
		return;
	}

	if (shellToSolid)
	{
		int N0 = pm->Nodes();
		int NF = pm->Faces();
		int nsurf = Faces();

		// first update the main nodes
		for (int i = 0; i < N0; ++i)
		{
			GLMesh::NODE& nd = mesh->Node(i);
			FSNode& ns = pm->Node(nd.nid);
			nd.r = to_vec3f(ns.r);
		}

		// update the shell offset nodes
		std::vector<vector<vec3d>> faceNodeNormals = pm->FaceNodalNormals();

		std::vector< std::deque<int> > faceList(nsurf);
		for (int i = 0; i < NF; i++)
		{
			const FSFace& face = pm->Face(i);
			if (face.m_gid >= 0 && face.m_gid < nsurf)
				faceList[face.m_gid].push_back(i);
		}

		int n0 = pm->Nodes();
		for (int i = 0; i < nsurf; i++)
		{
			for (auto n : faceList[i])
			{
				const FSFace& face = pm->Face(n);
				if (face.IsVisible())
				{
					int eid = face.m_elem[0].eid;
					if ((eid >= 0) && (!pm->Element(eid).IsVisible()))
					{
						eid = face.m_elem[1].eid;
					}

					bool isShell = false;
					if (eid >= 0)
					{
						isShell = pm->Element(eid).IsShell();
					}

					if (isShell)
					{
						FSElement& el = pm->Element(eid);
						int nf = face.Nodes();
						int nn[FSElement::MAX_NODES] = { 0 };
						int m[FSElement::MAX_NODES] = { 0 };
						for (int j = 0; j < nf; ++j)
						{
							int nj = face.n[j];
							vec3d rn = pm->Node(nj).r;
							vec3d N = faceNodeNormals[n][j];
							switch (shellRefSurface)
							{
							case 0: // mid
							{
								vec3d ra = rn + N * (el.m_h[j] * 0.5);
								vec3d rb = rn - N * (el.m_h[j] * 0.5);
								mesh->Node(n0++).r = to_vec3f(rb);
								mesh->Node(n0++).r = to_vec3f(ra);
							}
							break;
							case 1: // bottom
								rn = rn + N * el.m_h[j];
								mesh->Node(n0++).r = to_vec3f(rn);
								break;
							case 2: // top
								rn = rn - N * el.m_h[j];
								mesh->Node(n0++).r = to_vec3f(rn);
								break;
							}
						}
					}
				}
			}
		}
		assert(n0 == mesh->Nodes());
	}
	else
	{
		assert(mesh->Nodes() == pm->Nodes());
		for (int i = 0; i < mesh->Nodes(); ++i)
		{
			GLMesh::NODE& nd = mesh->Node(i);
			FSNode& ns = pm->Node(nd.nid);

			nd.r = to_vec3f(ns.r);
		}
	}

	mesh->Update();
	mesh->setModified(true);

	// the render mesh is used for rendering the feature edges.
	// TODO: Can I not just use the FE render mesh?
	mesh = GetRenderMesh();
	if (mesh)
	{
		for (int i = 0; i < mesh->Nodes(); ++i)
		{
			GLMesh::NODE& nd = mesh->Node(i);
			if ((nd.nid >= 0) && (nd.nid < pm->Nodes()))
			{
				FSNode& ns = pm->Node(nd.nid);
				nd.r = to_vec3f(ns.r);
			}
		}
		mesh->Update();
		mesh->setModified(true);
	}
}

void CPostObject::BuildFERenderMesh()
{
	FSMesh* pm = GetFEMesh();
	if (pm == nullptr) return;

	int nsurf = Faces();
	if (nsurf == 0) return;

	vector<vector<vec3d>> faceNodeNormals;
	if (shellToSolid)
		faceNodeNormals = pm->FaceNodalNormals();

	GLMesh* pgm = new GLMesh;
	GLMesh& gm = *pgm;
	gm.Create(pm->Nodes(), 0, 0);
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		gm.Node(i).r = to_vec3f(pm->Node(i).r);
		gm.Node(i).nid = i;
	}

	int NF = pm->Faces();
	std::vector< std::deque<int> > faceList(nsurf);
	for (int i = 0; i < NF; i++)
	{
		const FSFace& face = pm->Face(i);
		if (face.m_gid >= 0 && face.m_gid < nsurf)
			faceList[face.m_gid].push_back(i);
	}

	for (int i = 0; i < nsurf; i++)
	{
		std::deque<int>::iterator it = faceList[i].begin();
		GFace* pf = Face(i);
		gm.NewSurfacePartition(pf->m_nPID[1] == -1 ? 0 : 1);
		for (auto n : faceList[i])
		{
			const FSFace& face = pm->Face(n);
			assert(face.m_gid == i);
			if (face.IsVisible())
			{
				int eid = face.m_elem[0].eid;
				if ((eid >= 0) && (!pm->Element(eid).IsVisible()))
				{
					eid = face.m_elem[1].eid;
				}
				int mid = -1;

				bool isShell = false;
				if (eid >= 0)
				{
					isShell = pm->Element(eid).IsShell();
					mid = pm->Element(eid).m_MatID;
				}
				if (isShell && shellToSolid)
				{
					FSElement& el = pm->Element(eid);
					int nf = face.Nodes();
					int mtop[FSElement::MAX_NODES] = { 0 };
					int mbot[FSElement::MAX_NODES] = { 0 };
					int m0[FSElement::MAX_NODES] = { 0 };
					int m1[FSElement::MAX_NODES] = { 0 };
					for (int j = 0; j < nf; ++j)
					{
						int nj = face.n[j];
						vec3d rn = pm->Node(nj).r;
						vec3d N = faceNodeNormals[n][j];
						switch (shellRefSurface)
						{
						case 0: // mid
						{
							vec3d ra = rn + N * (el.m_h[j] * 0.5);
							vec3d rb = rn - N * (el.m_h[j] * 0.5);
							m0[j] = gm.AddNode(to_vec3f(rb), nj, -1);
							m1[j] = gm.AddNode(to_vec3f(ra), nj, -1);
							mbot[nf - j - 1] = m0[j];
							mtop[j] = m1[j];
						}
						break;
						case 1: // bottom
							rn = rn + N * el.m_h[j]; 
							m0[j] = nj;
							m1[j] = gm.AddNode(to_vec3f(rn), nj, -1);
							mbot[nf - j - 1] = m0[j];
 							mtop[j] = m1[j];
							break;
						case 2: // top
							rn = rn - N * el.m_h[j];
							m0[j] = gm.AddNode(to_vec3f(rn), nj, -1);
							m1[j] = nj;
							mbot[nf - j - 1] = m0[j];
 							mtop[j] = m1[j];
							break;
						}
					}

					gm.AddFace(mtop, face.Nodes(), face.m_gid, face.m_gid, face.IsExterior(), n, eid, mid);
					gm.AddFace(mbot, face.Nodes(), face.m_gid, face.m_gid, face.IsExterior(), n, eid, mid);

					int ne = face.Edges();
					for (int j = 0; j < ne; ++j)
					{
						int j1 = (j + 1) % ne;
						FSFace* pfn = pm->FacePtr(face.m_nbr[j]);
						if ((pfn == nullptr) || (pfn->m_gid != face.m_gid))
						{
							std::array<int, 4> mm = { m0[j], m0[j1], m1[j1], m1[j] };
							gm.AddFace(mm.data(), 4, face.m_gid, -1, face.IsExterior(), n, eid, mid);
						}
					}
				}
				else
					gm.AddFace(face.n, face.Nodes(), face.m_gid, face.m_gid, face.IsExterior(), n, eid, mid);

				int ne = face.Edges();
				for (int j = 0; j < ne; ++j)
				{
					int j1 = (j + 1) % ne;
					if ((face.m_nbr[j] < 0) || (face.n[j] < face.n[j1]))
					{
						int m[FSEdge::MAX_NODES] = { 0 };
						int l = face.GetEdgeNodes(j, m);
						gm.AddEdge(m, l, mid);
					}
				}
			}
		}
	}

	nsurf = InternalSurfaces();
	for (int i = 0; i < nsurf; ++i)
	{
		Post::GLSurface& surf = InteralSurface(i);
		gm.NewSurfacePartition();
		for (int j = 0; j < surf.Faces(); ++j)
		{
			FSFace& face = surf.Face(j);

			int eid = face.m_elem[0].eid;
			if ((eid >= 0) && (!pm->Element(eid).IsVisible()))
			{
				eid = face.m_elem[1].eid;
			}
			int mid = -1;
			if (eid >= 0) mid = pm->Element(eid).m_MatID;

			gm.AddFace(face.n, face.Nodes(), face.m_gid, -1, false, j, eid, mid);

			int ne = face.Edges();
			for (int j = 0; j < ne; ++j)
			{
				int j1 = (j + 1) % ne;
				if ((face.m_nbr[j] < 0) || (face.n[j] < face.n[j1]))
				{
					int m[2] = { face.n[j], face.n[j1] };
					gm.AddEdge(m, 2, mid);
				}
			}
		}
	}

	// discrete elements
	for (int i=0; i<pm->Edges(); ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.m_elem >= 0)
		{
			FSElement& el = pm->Element(edge.m_elem);
			int m[2] = { edge.n[0], edge.n[1] };
			gm.AddEdge(m, 2, el.m_MatID);
		}
	}

	gm.AutoEdgePartition();
	gm.Update();
	SetFERenderMesh(&gm);
}

void CPostObject::BuildInternalSurfaces()
{
	ClearInternalSurfaces();

	FSMesh* pmesh = dynamic_cast<FSMesh*>(GetFEMesh());
	if (pmesh == nullptr) return;
	FSMesh& mesh = *pmesh;

	int ndom = mesh.MeshPartitions();
	for (int i = 0; i < ndom; ++i) m_innerSurface.push_back(new Post::GLSurface);

	int nsurf = Faces();
	FSFace face;
	for (int m = 0; m < ndom; ++m)
	{
		FSMeshPartition& dom = mesh.MeshPartition(m);
		int NE = dom.Elements();

		for (int i = 0; i < NE; ++i)
		{
			FSElement_& el = dom.Element(i);
			if (el.IsVisible())
			{
				for (int j = 0; j < el.Faces(); ++j)
				{
					FSElement_* pen = mesh.ElementPtr(el.m_nbr[j]);
					if (pen && (pen->m_MatID == el.m_MatID) && !pen->IsVisible())
					{
						el.GetFace(j, face);
						face.m_elem[0].eid = el.m_lid; // store the element ID. This is used for selection ???
						face.m_elem[1].eid = pen->m_lid;
						face.m_gid = nsurf + m;
						m_innerSurface[m]->add(face);
					}
				}
			}
		}
	}
	BuildFERenderMesh();
}
