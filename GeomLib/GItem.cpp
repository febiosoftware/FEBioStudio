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

#include "GItem.h"
#include "GBaseObject.h"
#include "GObject.h"
#include "geom.h"
#include "GPartSection.h"
#include <GeomLib/FSGroup.h>
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
// initialize static variables
template <> int GItem_T<GPart  >::m_ncount = 0;
template <> int GItem_T<GFace  >::m_ncount = 0;
template <> int GItem_T<GEdge  >::m_ncount = 0;
template <> int GItem_T<GNode  >::m_ncount = 0;


//=============================================================================
// GNode
//-----------------------------------------------------------------------------

GNode::GNode() : GItem_T<GNode>(0) 
{ 
	m_ntype = NODE_UNKNOWN; 
	m_node = -1;
}

GNode::GNode(GBaseObject* po) : GItem_T<GNode>(po) 
{ 
	m_ntype = NODE_UNKNOWN; 
	m_node = -1;
}

GNode::GNode(const GNode &n)
{
	m_r = n.m_r;
	m_state = n.m_state;
	m_gid = n.m_gid;
	m_lid = n.m_lid;
	m_ntype = n.m_ntype;
	m_ntag = n.m_ntag;
	m_po = n.m_po;
	m_node = n.m_node;
	SetName(n.GetName());
}

//-----------------------------------------------------------------------------
void GNode::operator =(const GNode &n)
{
	m_r = n.m_r;
	m_state = n.m_state;
	m_gid = n.m_gid;
	m_lid = n.m_lid;
	m_ntype = n.m_ntype;
	m_ntag = n.m_ntag;
	m_node = n.m_node;
//	m_po = n.m_po;
	SetName(n.GetName());
}

//-----------------------------------------------------------------------------
vec3d GNode::Position() const
{
	const GBaseObject* po = Object(); assert(po);
	if (po)
	{
		return po->GetTransform().LocalToGlobal(m_r);
	}
	else return m_r;
}

//-----------------------------------------------------------------------------
void GNode::MakeRequired()
{
	SetRequired(true);

	GObject* po = dynamic_cast<GObject*>(Object());
	if (po)
	{
		FSMesh* pm = po->GetFEMesh();
		if (pm && (m_node >= 0) && (m_node < pm->Nodes()))
		{
			FSNode* pn = pm->NodePtr(m_node); assert(pn);
			if (pn) pn->SetRequired(true);
		}
	}
}

//=============================================================================
// GEdge
//-----------------------------------------------------------------------------
GEdge::GEdge(const GEdge& e)
{
	m_node[0] = e.m_node[0];
	m_node[1] = e.m_node[1];
	m_cnode = e.m_cnode;
	m_orient = e.m_orient;
	m_ntype = e.m_ntype;

	m_state = e.m_state;
	m_gid = e.m_gid;
	m_lid = e.m_lid;
	m_po = e.m_po;
	SetName(e.GetName());
}

//-----------------------------------------------------------------------------
void GEdge::operator =(const GEdge &e)
{
	m_node[0] = e.m_node[0];
	m_node[1] = e.m_node[1];
	m_cnode = e.m_cnode;
	m_orient = e.m_orient;
	m_ntype = e.m_ntype;

	m_state = e.m_state;
	m_gid = e.m_gid;
	m_lid = e.m_lid;
//	m_po = e.m_po;
	SetName(e.GetName());
}

//-----------------------------------------------------------------------------
bool GEdge::operator==(const GEdge& e)
{
	if (e.m_ntype != m_ntype) return false;
	switch (m_ntype)
	{
	case EDGE_LINE:
		if ((m_node[0] != e.m_node[0]) && (m_node[0] != e.m_node[1])) return false;
		if ((m_node[1] != e.m_node[0]) && (m_node[1] != e.m_node[1])) return false;
		break;
	case EDGE_YARC:
	case EDGE_ZARC:
		if ((m_node[0] != e.m_node[0]) || (m_node[1] != e.m_node[1])) return false;
		if (m_orient != e.m_orient) return false;
		break;
	case EDGE_3P_CIRC_ARC:
		if ((m_node[0] != e.m_node[0])  || (m_node[1] != e.m_node[1])) return false;
		if (m_cnode[0] != e.m_cnode[0]) return false;
		if (m_orient != e.m_orient) return false;
		break;
	case EDGE_3P_ARC:
		if ((m_node[0] != e.m_node[0]) || (m_node[1] != e.m_node[1])) return false;
		if (m_cnode != e.m_cnode) return false;
		break;
	default:
//		assert(false);
		return false;
	}
	return true;
}

GNode* GEdge::Node(int i)
{
	return m_po->Node(m_node[i]);
}

double GEdge::Length()
{
	double L = 0;
	switch (m_ntype)
	{
	case EDGE_UNKNOWN: assert(false); L = 0; break;
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y);
			vec2d b(r1.x, r1.y);

			GM_LINE l(a, b);

			L = l.Length();
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_CIRCLE_ARC ca(a, b, c);
			L = ca.Length();
		}
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_ARC ca(a, b, c);
			L = ca.Length();
		}
		break;
	case EDGE_YARC:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();
			vec2d c(0, 0);
			vec2d a(r0.x, r0.z);
			vec2d b(r1.x, r1.z);

			// create an arc object
			GM_CIRCLE_ARC ca(c, a, b, -m_orient);
			L = ca.Length();
		}
		break;
	case EDGE_BEZIER:
		{
			std::vector<vec3d> P;
			P.push_back(m_po->Node(m_node[0])->LocalPosition());
			for (int i = 0; i < m_cnode.size(); ++i)
			{
				P.push_back(m_po->Node(m_cnode[i])->LocalPosition());
			}
			P.push_back(m_po->Node(m_node[1])->LocalPosition());
			GM_BEZIER bez(P);
			L = bez.Length();
		}
	break;
	default:
		assert(false);
	}
	return L;
}

//-----------------------------------------------------------------------------
vec3d GEdge::Point(double l)
{
	assert((l>=0)&&(l<=1.0));
	if (l<0.0) l = 0.0;
	if (l>1.0) l = 1.0;
	vec3d p;
	switch (m_ntype)
	{
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();
			p = r0*(1.0 - l) + r1*l;
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition() - r0;
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition() - r0;
			vec3d n = r1^r2; n.Normalize();
			quatd q(n, vec3d(0,0,1)), qi = q.Inverse();
			q.RotateVector(r1);
			q.RotateVector(r2);
			GM_CIRCLE_ARC c(vec2d(0,0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y));
			vec2d a = c.Point(l);
			p.x = a.x();
			p.y = a.y();
			p.z = 0;
			qi.RotateVector(p);
			p += r0;
		};
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition() - r0;
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition() - r0;
			vec3d n = r1^r2; n.Normalize();
			quatd q(n, vec3d(0,0,1)), qi = q.Inverse();
			q.RotateVector(r1);
			q.RotateVector(r2);
			GM_ARC c(vec2d(0,0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y));
			vec2d a = c.Point(l);
			p.x = a.x();
			p.y = a.y();
			p.z = 0;
			qi.RotateVector(p);
			p += r0;
		};
		break;
	case EDGE_YARC:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();
			double y = r0.y;

			vec2d a(r0.x, r0.z);
			vec2d b(r1.x, r1.z);

			GM_CIRCLE_ARC c(vec2d(0, 0), a, b, -m_orient);
			vec2d q = c.Point(l);
			p = vec3d(q.x(), y, q.y());
		}
		break;
	case EDGE_BEZIER:
		{
			std::vector<vec3d> P;
			P.push_back(m_po->Node(m_node[0])->LocalPosition());
			for (int i = 0; i < m_cnode.size(); ++i)
			{
				P.push_back(m_po->Node(m_cnode[i])->LocalPosition());
			}
			P.push_back(m_po->Node(m_node[1])->LocalPosition());
			GM_BEZIER bez(P);
			p = bez.Point(l);
		}
		break;
	default:
		assert(false);
	}
	return p;
}

//-----------------------------------------------------------------------------
vec2d GEdge::Tangent(double l)
{
	vec2d t;
	switch (m_ntype)
	{
	case EDGE_UNKNOWN: assert(false); break;
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y);
			vec2d b(r1.x, r1.y);

			GM_LINE line(a, b);

			t = line.Tangent(l);
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_CIRCLE_ARC ca(a, b, c);

			t = ca.Tangent(l);
		}
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_ARC ca(a, b, c);

			t = ca.Tangent(l);
		}
		break;
	case EDGE_BEZIER:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();

			if (m_cnode.empty())
			{
				vec3d e = r1 - r0;
				return vec2d(e.x, e.y);
			}
			else
			{
				int n = m_cnode.size();
				vec3d a, b;
				if (l <= 0)
				{
					a = m_po->Node(m_node[0])->LocalPosition();
					b = m_po->Node(m_cnode[0])->LocalPosition();
				}
				else if (l >= 1)
				{
					a = m_po->Node(m_cnode[n-1])->LocalPosition();
					b = m_po->Node(m_node[1])->LocalPosition();
				}
				else
				{
					assert(false);
				}
				vec3d e = b - a;
				return vec2d(e.x, e.y);
			}
		}
		break;
	default:
		assert(false);
	}

	return t;
}

//-----------------------------------------------------------------------------
FSEdgeSet* GEdge::GetFEEdgeSet() const
{
	GObject* po = dynamic_cast<GObject*>(m_po);
	if (m_po == nullptr) return nullptr;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return nullptr;

	FSEdgeSet* edge = new FSEdgeSet(pm);
	int eid = GetLocalID();
	for (int i = 0; i < pm->Edges(); ++i)
	{
		const FSEdge& ei = pm->Edge(i);
		if (ei.m_gid == eid) edge->add(i);
	}

	return edge;
}


//=============================================================================
// GFace
//-----------------------------------------------------------------------------

GFace::GFace() : GItem_T<GFace>(0) 
{ 
	m_nPID[0] = m_nPID[1] = m_nPID[2] = -1;
	m_ntype = FACE_UNKNOWN; 
}

GFace::GFace(GBaseObject* po) : GItem_T<GFace>(po) 
{ 
	m_nPID[0] = m_nPID[1] = m_nPID[2] = -1; 
	m_ntype = FACE_UNKNOWN; 
}

GFace::GFace(const GFace& f) 
{
	m_ntype = f.m_ntype;
	m_nPID[0] = f.m_nPID[0];
	m_nPID[1] = f.m_nPID[1];
	m_nPID[2] = f.m_nPID[2];
	m_node = f.m_node;
	m_edge = f.m_edge;

	m_state = f.m_state;
	m_gid = f.m_gid;
	m_lid = f.m_lid;
	m_po = f.m_po;
	SetName(f.GetName());
}

//-----------------------------------------------------------------------------
void GFace::operator = (const GFace& f)
{
	m_ntype = f.m_ntype;
	m_nPID[0] = f.m_nPID[0];
	m_nPID[1] = f.m_nPID[1];
	m_nPID[2] = f.m_nPID[2];
	m_node = f.m_node;
	m_edge = f.m_edge;

	m_state = f.m_state;
	m_gid = f.m_gid;
	m_lid = f.m_lid;
	SetName(f.GetName());
//	m_po = f.m_po;
}

//-----------------------------------------------------------------------------
bool GFace::HasEdge(int nid)
{
	for (int i=0; i<(int) m_edge.size(); ++i)
	{
		if (m_edge[i].nid == nid) return true;
	}
	return false;
}


void GFace::Invert()
{
	// swap block IDs
	if (m_nPID[1] != -1)
	{
		int tmp = m_nPID[0]; m_nPID[0] = m_nPID[1]; m_nPID[1] = tmp;
	}

	// we also need to invert the edges and nodes
	int nn = m_node.size();
	int ne = m_edge.size();
	std::vector<int> fn(nn);
	std::vector<int> fe(ne);
	std::vector<int> fw(ne);

	for (int j = 0; j < nn; ++j) {
		fn[j] = m_node[j];
	}
	for (int j = 0; j < ne; ++j) {
		fe[j] = m_edge[j].nid;
		fw[j] = m_edge[j].nwn;
	}

	for (int j = 0; j < nn; ++j)
	{
		m_node[j] = fn[(nn - j) % nn];
	}
	for (int j = 0; j < ne; ++j)
	{
		m_edge[j].nid = fe[ne - j - 1];
		m_edge[j].nwn = -fw[ne - j - 1];
	}
}

//=============================================================================
// GPart
//-----------------------------------------------------------------------------
GPart::GPart() : GItem_T<GPart>(0) 
{ 
	m_matid = -1;
	m_section = nullptr;
}
GPart::GPart(GBaseObject* po) : GItem_T<GPart>(po) 
{ 
	m_matid = -1;
	m_section = nullptr;
}

GPart::~GPart()
{
	delete m_section;
}

void GPart::SetSection(GPartSection* section) 
{ 
	if (m_section == section) return;
	delete m_section;
	m_section = section; 
	section->SetParent(this);
}

GPartSection* GPart::GetSection() const { return m_section; }

GPart::GPart(const GPart& p)
{
	if (p.GetSection()) m_section = p.GetSection()->Copy();

	m_matid = p.m_matid;

	m_state = p.m_state;
	m_gid = p.m_gid;
	m_lid = p.m_lid;
	m_po = p.m_po;
	SetName(p.GetName());
}

bool GPart::IsSolid() const
{
//	assert(m_section);
	return (dynamic_cast<GSolidSection*>(m_section));
}

bool GPart::IsShell() const
{
//	assert(m_section);
	return (dynamic_cast<GShellSection*>(m_section));
}

bool GPart::IsBeam() const
{
//	assert(m_section);
	return (dynamic_cast<GBeamSection*>(m_section));
}

//-----------------------------------------------------------------------------
void GPart::operator =(const GPart &p)
{
	m_matid = p.m_matid;
	CopyParams(p);

	m_state = p.m_state;
	m_gid = p.m_gid;
	m_lid = p.m_lid;
	SetName(p.GetName());
//	m_po = p.m_po;
}

//-----------------------------------------------------------------------------
bool GPart::Update(bool b)
{
	int id = GetLocalID();
	if (m_node.empty())
	{
		GBaseObject* po = Object();
		for (int i = 0; i < po->Nodes(); ++i) po->Node(i)->m_ntag = 0;
		for (int i = 0; i < po->Faces(); ++i)
		{
			GFace* pf = po->Face(i);
			if ((pf->m_nPID[0] == id) || (pf->m_nPID[1] == id))
			{
				for (int k : pf->m_node)
				{
					GNode* pn = po->Node(k);
					if (pn) po->Node(k)->m_ntag = 1;
				}
			}
		}

		for (int i = 0; i < po->Nodes(); ++i)
			if (po->Node(i)->m_ntag == 1) m_node.push_back(i);
	}
	UpdateBoundingBox();
	return true;
}

//-----------------------------------------------------------------------------
void GPart::UpdateBoundingBox()
{
	GBaseObject* po = Object();
	if (po == nullptr) return;

	BOX box;
	for (int i = 0; i < m_node.size(); ++i)
	{
		GNode* pn = po->Node(m_node[i]); assert(pn);
		if (pn) box += pn->LocalPosition();
	}
	m_box = box;
}

//-----------------------------------------------------------------------------
BOX GPart::GetLocalBox() const
{
	return m_box;
}

BOX GPart::GetGlobalBox() const
{
	BOX box = GetLocalBox();
	const GBaseObject* po = Object();
	if (po == nullptr) return box;

	vec3d r0 = vec3d(box.x0, box.y0, box.z0);
	vec3d r1 = vec3d(box.x1, box.y1, box.z1);
	r0 = po->GetTransform().LocalToGlobal(r0);
	r1 = po->GetTransform().LocalToGlobal(r1);
	return BOX(r0.x, r0.y, r0.z, r1.x, r1.y, r1.z);
}
