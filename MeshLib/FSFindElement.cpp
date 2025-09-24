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

#include "FSFindElement.h"
#include "FSCoreMesh.h"
#include "MeshTools.h"

FSFindElement::OCTREE_BOX::OCTREE_BOX()
{
	m_elem = -1;
	m_level = -1;
}

FSFindElement::OCTREE_BOX::~OCTREE_BOX()
{
	for (size_t i = 0; i<m_child.size(); ++i) delete m_child[i];
	m_child.clear();
}

void FSFindElement::OCTREE_BOX::split(int levels)
{
	m_level = levels;
	if (m_level == 0) return;

	double x0 = m_box.x0, x1 = m_box.x1;
	double y0 = m_box.y0, y1 = m_box.y1;
	double z0 = m_box.z0, z1 = m_box.z1;

	double dx = x1 - x0;
	double dy = y1 - y0;
	double dz = z1 - z0;

	m_child.clear();
	for (int i = 0; i<2; i++)
		for (int j = 0; j<2; j++)
			for (int k = 0; k <2; k++)
			{
				double xa = x0 + i*dx*0.5, xb = x0 + (i + 1)*dx*0.5;
				double ya = y0 + j*dy*0.5, yb = y0 + (j + 1)*dy*0.5;
				double za = z0 + k*dz*0.5, zb = z0 + (k + 1)*dz*0.5;

				BOX b(xa, ya, za, xb, yb, zb);
				double R = b.GetMaxExtent();
				b.Inflate(R*0.0001);

				OCTREE_BOX* box = new OCTREE_BOX;
				box->m_box = b;

				m_child.push_back(box);
			}

	for (int i = 0; i<(int)m_child.size(); ++i)
	{
		m_child[i]->split(levels - 1);
	}
}

void FSFindElement::OCTREE_BOX::Add(BOX& b, int nelem)
{
	if (m_level == 0)
	{
		if (m_box.Intersects(b))
		{
			OCTREE_BOX* box = new OCTREE_BOX;
			box->m_box = b;
			box->m_elem = nelem;
			box->m_level = -1;
			m_child.push_back(box);
			return;
		}
	}
	else
	{
		for (size_t i = 0; i<m_child.size(); ++i)
		{
			m_child[i]->Add(b, nelem);
		}
	}
}


FSFindElement::OCTREE_BOX* FSFindElement::OCTREE_BOX::Find(const vec3f& r)
{
	if (m_level == 0)
	{
		bool inside = m_box.IsInside(to_vec3d(r));
		return (inside ? this : 0);
	}

	// try to find the child
	for (size_t i = 0; i<m_child.size(); ++i)
	{
		OCTREE_BOX* c = m_child[i];
		OCTREE_BOX* ret = c->Find(r);
		if (ret) return ret;
	}

	return 0;
}


FSFindElement::OCTREE_BOX* FSFindElement::FindBox(const vec3f& r)
{
	// make sure it's in the master box
	if (m_bound.IsInside(r) == false) return 0;

	// try to find the child
	OCTREE_BOX* b = &m_bound;
	do
	{
		if (b->m_level == 0)
		{
			bool inside = b->IsInside(r);
			return (inside ? b : 0);
		}

		bool bfound = false;
		for (size_t i = 0; i<b->m_child.size(); ++i)
		{
			OCTREE_BOX* c = b->m_child[i];
			if (c->IsInside(r))
			{
				b = c;
				bfound = true;
				break;
			}
		}

		assert(bfound);
		if (bfound == false) break;
	} while (1);

	return 0;
}

FSFindElement::FSFindElement(FSCoreMesh& mesh) : m_mesh(mesh)
{
	m_nframe = -1;
}

void FSFindElement::InitReferenceFrame(std::vector<bool>& flags)
{
	assert(m_nframe == 0);

	// calculate bounding box for the entire mesh
	int NN = m_mesh.Nodes();
	int NE = m_mesh.Elements();
	if ((NN == 0) || (NE == 0)) return;

	vec3d r = m_mesh.Node(0).r;
	BOX box(r, r);
	for (int i = 1; i<m_mesh.Nodes(); ++i)
	{
		r = m_mesh.Node(i).r;
		box += r;
	}
	double R = box.GetMaxExtent();
	box.Inflate(R*0.001);

	// split this box recursively
	m_bound.m_box = box;

	int l = (int)(log(NE) / log(8.0));
	if (l < 0) l = 0;
	if (l > 3) l = 3;
	m_bound.split(l);

	// calculate bounding boxes for all elements
	int cflags = (int)flags.size();
	for (int i = 0; i<NE; ++i)
	{
		FSElement_& e = m_mesh.ElementRef(i);

		bool badd = true;
		if (flags.empty() == false)
		{
			int mid = e.m_MatID;
			if ((mid >= 0) && (mid < cflags)) badd = flags[mid];
		}

		if (badd)
		{
			int ne = e.Nodes();

			// do a quick bounding box test
			vec3d r0 = m_mesh.Node(e.m_node[0]).r;
			vec3d r1 = r0;
			BOX box(r0, r1);
			for (int j = 1; j<ne; ++j)
			{
				vec3d rj = m_mesh.Node(e.m_node[j]).r;
				box += rj;
			}
			double R = box.GetMaxExtent();
			box.Inflate(R*0.001);

			// add it to the octree
			m_bound.Add(box, i);
		}
	}
}

void FSFindElement::InitCurrentFrame(std::vector<bool>& flags)
{
	assert(m_nframe == 1);

	// calculate bounding box for the entire mesh
	int NN = m_mesh.Nodes();
	int NE = m_mesh.Elements();
	if ((NN == 0) || (NE == 0)) return;

	vec3d r = m_mesh.Node(0).r;
	BOX box(r, r);
	for (int i = 1; i<m_mesh.Nodes(); ++i)
	{
		r = m_mesh.Node(i).r;
		box += r;
	}
	double R = box.GetMaxExtent();
	box.Inflate(R*0.001);

	// split this box recursively
	m_bound.m_box = box;

	int l = (int)(log(NE) / log(8.0));
	if (l < 0) l = 0;
	if (l > 3) l = 3;
	m_bound.split(l);

	// calculate bounding boxes for all elements
	int cflags = (int)flags.size();
	for (int i = 0; i<NE; ++i)
	{
		FSElement_& e = m_mesh.ElementRef(i);

		bool badd = true;
		if (flags.empty() == false)
		{
			int mid = e.m_MatID;
			if ((mid >= 0) && (mid < cflags)) badd = flags[mid];
		}

		if (badd)
		{
			int ne = e.Nodes();

			// do a quick bounding box test
			vec3d r0 = m_mesh.Node(e.m_node[0]).r;
			vec3d r1 = r0;
			BOX box(r0, r1);
			for (int j = 1; j<ne; ++j)
			{
				vec3d rj = m_mesh.Node(e.m_node[j]).r;
				box += rj;
			}
			double R = box.GetMaxExtent();
			box.Inflate(R*0.001);

			// add it to the octree
			m_bound.Add(box, i);
		}
	}
}

void FSFindElement::Init(int nframe)
{
	std::vector<bool> dummy;
	m_nframe = nframe;
	if (m_nframe == 0) InitReferenceFrame(dummy);
	else InitCurrentFrame(dummy);
}

void FSFindElement::Init(std::vector<bool>& flags, int nframe)
{
	m_nframe = nframe;
	if (m_nframe == 0) InitReferenceFrame(flags);
	else InitCurrentFrame(flags);
}

bool FSFindElement::FindInReferenceFrame(const vec3f& x, int& nelem, double r[3])
{
	assert(m_nframe == 0);

	vec3f y[FSElement::MAX_NODES];
	OCTREE_BOX* b = FindBox(x);
	if (b == 0) return false;
	assert(b->m_level == 0);

	int NE = (int)b->m_child.size();
	for (int i = 0; i<NE; ++i)
	{
		OCTREE_BOX* c = b->m_child[i];
		assert(c->m_level == -1);

		int nid = c->m_elem; assert(nid >= 0);

		FSElement_& e = m_mesh.ElementRef(nid);
		nelem = nid;

		// do a quick bounding box test
		if (c->m_box.IsInside(to_vec3d(x)))
		{
			// do a more complete search
			if (ProjectInsideReferenceElement(m_mesh, e, x, r)) return true;
		}
	}

	nelem = -1;
	return false;
}

bool FSFindElement::FindInCurrentFrame(const vec3f& x, int& nelem, double r[3])
{
	assert(m_nframe == 1);

	vec3f y[FSElement::MAX_NODES];
	OCTREE_BOX* b = FindBox(x);
	if (b == 0) return false;
	assert(b->m_level == 0);

	int NE = (int)b->m_child.size();
	for (int i = 0; i<NE; ++i)
	{
		OCTREE_BOX* c = b->m_child[i];
		assert(c->m_level == -1);

		int nid = c->m_elem; assert(nid >= 0);

		FSElement_& e = m_mesh.ElementRef(nid);
		nelem = nid;

		// do a quick bounding box test
		if (c->m_box.IsInside(to_vec3d(x)))
		{
			// do a more complete search
			if (ProjectInsideElement(m_mesh, e, x, r)) return true;
		}
	}

	nelem = -1;
	return false;
}

//================================================================================================
bool FindElement2D(const vec2d& r, int& elem, double q[2], FSMesh* mesh)
{
	vec3d x[FSElement::MAX_NODES];
	int NE = mesh->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh->Element(i);
		if (el.IsShell())
		{
			int nn = el.Nodes();
			BOX box;
			for (int j = 0; j < nn; ++j)
			{
				x[j] = mesh->Node(el.m_node[j]).r;
				box += x[j];
			}

			double R = box.GetMaxExtent();
			box.Inflate(R * 1e-5);

			if ((box.x0 < r.x()) && (box.x1 > r.x()) &&
				(box.y0 < r.y()) && (box.y1 > r.y()))
			{
				q[0] = q[1] = 0.0;
				if (project_inside_element2d(el, x, r, q))
				{
					elem = i;
					return true;
				}
			}
		}
	}

	elem = -1;
	return false;
}
