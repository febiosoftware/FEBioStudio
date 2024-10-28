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
	GMesh* mesh = GetFERenderMesh(); assert(mesh);
	if (mesh == nullptr) return;

	FSMesh* pm = GetFEMesh();

	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		GMesh::NODE& nd = mesh->Node(i);
		FSNode& ns = pm->Node(nd.nid);

		nd.r = to_vec3f(ns.r);
	}
	mesh->Update();

	vector<double> buf(pm->Nodes());
	for (int i = 0; i < mesh->Faces(); ++i)
	{
		GMesh::FACE& face = mesh->Face(i);
		if (face.pid < Faces())
		{
			assert(face.fid >= 0);
			FSFace& f = pm->Face(face.fid);
			for (int j = 0; j < f.Nodes(); ++j) buf[f.n[j]] = f.m_tex[j];
			
			face.t[0] = buf[face.n[0]];
			face.t[1] = buf[face.n[1]];
			face.t[2] = buf[face.n[2]];
		}
		else
		{
			Post::GLSurface& surf = InteralSurface(face.pid - Faces());
			FSFace& f = surf.Face(face.fid);
			FSElement& e = pm->Element(face.eid);
			for (int j = 0; j < f.Nodes(); ++j) buf[f.n[j]] = f.m_tex[j];

			face.t[0] = buf[face.n[0]];
			face.t[1] = buf[face.n[1]];
			face.t[2] = buf[face.n[2]];
		}
	}
}

void CPostObject::BuildFERenderMesh()
{
	FSMesh* pm = GetFEMesh();
	if (pm == nullptr) return;

	int nsurf = Faces();
	if (nsurf == 0) return;

	GMesh* pgm = new GMesh;
	GMesh& gm = *pgm;
	gm.Create(pm->Nodes(), 0, 0);
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		gm.Node(i).r = to_vec3f(pm->Node(i).r);
		gm.Node(i).nid = i;
	}

	int NF = pm->Faces();
	std::vector< std::deque<int> > faceList(NF);
	for (int i = 0; i < NF; i++)
	{
		const FSFace& face = pm->Face(i);
		assert(face.m_gid >= 0);
		faceList[face.m_gid].push_back(i);
	}

	for (int i = 0; i < nsurf; i++)
	{
		std::deque<int>::iterator it = faceList[i].begin();
		gm.NewPartition();
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
				if (eid >= 0) mid = pm->Element(eid).m_MatID;

				gm.AddFace(face.n, face.Nodes(), face.m_gid, face.m_sid, face.IsExterior(), n, eid, mid);

				int ne = face.Edges();
				for (int j = 0; j < ne; ++j)
				{
					int j1 = (j + 1) % ne;
					if ((face.m_nbr[j] < 0) || (face.n[j] < face.n[j1]))
					{
						int m[2] = { face.n[j], face.n[j1] };
						gm.AddEdge(m, 2);
					}
				}
			}
		}
	}

	nsurf = InternalSurfaces();
	for (int i = 0; i < nsurf; ++i)
	{
		Post::GLSurface& surf = InteralSurface(i);
		gm.NewPartition();
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
					gm.AddEdge(m, 2);
				}
			}
		}
	}

	gm.Update();
	SetFERenderMesh(&gm);
	UpdateMeshData();
}

void CPostObject::BuildInternalSurfaces()
{
	ClearInternalSurfaces();

	Post::FEPostMesh* pmesh = dynamic_cast<Post::FEPostMesh*>(GetFEMesh());
	if (pmesh == nullptr) return;
	Post::FEPostMesh& mesh = *pmesh;

	int ndom = mesh.Domains();
	for (int i = 0; i < ndom; ++i) m_innerSurface.push_back(new Post::GLSurface);

	int nsurf = Faces();
	FSFace face;
	for (int m = 0; m < ndom; ++m)
	{
		Post::MeshDomain& dom = mesh.Domain(m);
		int NE = dom.Elements();

		for (int i = 0; i < NE; ++i)
		{
			FEElement_& el = dom.Element(i);
			if (el.IsVisible())
			{
				for (int j = 0; j < el.Faces(); ++j)
				{
					FEElement_* pen = mesh.ElementPtr(el.m_nbr[j]);
					if (pen && (pen->m_MatID == el.m_MatID) && !pen->IsVisible())
					{
						el.GetFace(j, face);
						face.m_elem[0].eid = el.m_lid; // store the element ID. This is used for selection ???
						face.m_elem[1].eid = pen->m_lid;

						// calculate the face normals
						vec3f r0 = to_vec3f(mesh.Node(face.n[0]).r);
						vec3f r1 = to_vec3f(mesh.Node(face.n[1]).r);
						vec3f r2 = to_vec3f(mesh.Node(face.n[2]).r);

						face.m_fn = (r1 - r0) ^ (r2 - r0);
						for (int k = 0; k < face.Nodes(); ++k) face.m_nn[k] = face.m_fn;
						face.m_fn.Normalize();
						face.m_sid = 0;
						face.m_gid = nsurf + m;

						m_innerSurface[m]->add(face);
					}
				}
			}
		}
	}
	BuildFERenderMesh();
}
