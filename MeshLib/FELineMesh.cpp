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

#include "FELineMesh.h"
#include <GeomLib/GObject.h>

FSLineMesh::FSLineMesh() : m_pobj(0)
{
}

//-----------------------------------------------------------------------------
// Tag all nodes
void FSLineMesh::TagAllNodes(int ntag)
{
	const int NN = Nodes();
	for (int i = 0; i<NN; ++i) Node(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
// Tag all edges
void FSLineMesh::TagAllEdges(int ntag)
{
	const int NE = Edges();
	for (int i = 0; i<NE; ++i) Edge(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
void FSLineMesh::SetGObject(GObject* po) { m_pobj = po; }

//-----------------------------------------------------------------------------
GObject* FSLineMesh::GetGObject() { return m_pobj; }

//-----------------------------------------------------------------------------
// convert local coordinates to global coordinates. This uses the transform
// info from the parent object.
vec3d FSLineMesh::LocalToGlobal(const vec3d& r) const
{
	if (m_pobj) return m_pobj->GetTransform().LocalToGlobal(r);
	else return r;
}

//-----------------------------------------------------------------------------
vec3d FSLineMesh::GlobalToLocal(const vec3d& r) const
{
	if (m_pobj) return m_pobj->GetTransform().GlobalToLocal(r);
	else return r;
}

//-----------------------------------------------------------------------------
// return the position of a node in global coordinates
vec3d FSLineMesh::NodePosition(int i) const
{
	return LocalToGlobal(Node(i).r);
}

//-----------------------------------------------------------------------------
// return the local position of a node
vec3d FSLineMesh::NodeLocalPosition(int i) const
{
	return Node(i).r;
}

vec3d FSLineMesh::EdgeCenter(FSEdge& e) const
{
	return (m_Node[e.n[0]].r + m_Node[e.n[1]].r) * 0.5f;
}

//-----------------------------------------------------------------------------
// Updates the bounding box (in local coordinates)
void FSLineMesh::UpdateBoundingBox()
{
	FSNode* pn = NodePtr();
	if (pn == 0)
	{
		m_box.x0 = m_box.y0 = m_box.z0 = 0;
		m_box.x1 = m_box.y1 = m_box.z1 = 0;
		return;
	}

	m_box.x0 = m_box.x1 = pn->r.x;
	m_box.y0 = m_box.y1 = pn->r.y;
	m_box.z0 = m_box.z1 = pn->r.z;
	for (int i = 0; i<Nodes(); i++, pn++)
	{
		vec3d& r = pn->r;
		if (r.x < m_box.x0) m_box.x0 = r.x;
		if (r.y < m_box.y0) m_box.y0 = r.y;
		if (r.z < m_box.z0) m_box.z0 = r.z;
		if (r.x > m_box.x1) m_box.x1 = r.x;
		if (r.y > m_box.y1) m_box.y1 = r.y;
		if (r.z > m_box.z1) m_box.z1 = r.z;
	}
}