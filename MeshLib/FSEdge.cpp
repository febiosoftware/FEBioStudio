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

#include "FSEdge.h"
#include <assert.h>

//-----------------------------------------------------------------------------
FSEdge::FSEdge()
{
	m_elem = -1;
	n[0] = n[1] = n[2] = n[3] = -1;
	m_nbr[0] = m_nbr[1] = -1;
	m_face[0] = m_face[1] = -1;
	m_type = FE_EDGE_INVALID;
	m_gid = -1;
}

//-----------------------------------------------------------------------------
FSEdge::FSEdge(const FSEdge& e) : FSMeshItem(e)
{
	m_type = e.m_type;
	m_elem = e.m_elem;
	n[0] = e.n[0];
	n[1] = e.n[1];
	n[2] = e.n[2];
	n[3] = e.n[3];
	m_nbr[0] = e.m_nbr[0];
	m_nbr[1] = e.m_nbr[1];
	m_face[0] = e.m_face[0];
	m_face[1] = e.m_face[1];
}

//-----------------------------------------------------------------------------
void FSEdge::operator = (const FSEdge& e)
{
	m_type = e.m_type;
	m_elem = e.m_elem;
	n[0] = e.n[0];
	n[1] = e.n[1];
	n[2] = e.n[2];
	n[3] = e.n[3];
	m_nbr[0] = e.m_nbr[0];
	m_nbr[1] = e.m_nbr[1];
	m_face[0] = e.m_face[0];
	m_face[1] = e.m_face[1];
	FSMeshItem::operator=(e);
}

//-----------------------------------------------------------------------------
// Tests equality between edges
bool FSEdge::operator == (const FSEdge& e) const
{
	if (e.m_type != m_type) return false;
	assert(m_type != FE_EDGE_INVALID);
	if ((n[0] != e.n[0]) && (n[0] != e.n[1])) return false;
	if ((n[1] != e.n[0]) && (n[1] != e.n[1])) return false;

	if (m_type == FE_EDGE3) 
	{
		if (n[2] != e.n[2]) return false;
	}
	else if (m_type == FE_EDGE4)
	{
		if ((n[2] != e.n[2]) && (n[2] != e.n[3])) return false;
		if ((n[3] != e.n[2]) && (n[3] != e.n[3])) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Returns the local index for a given node number.
int FSEdge::FindNodeIndex(int node) const
{
	assert(m_type != FE_EDGE_INVALID);
	if (node == n[0]) return 0;
	if (node == n[1]) return 1;
	if (node == n[2]) return 2;
	if (node == n[3]) return 3;
	return -1;
}

//-----------------------------------------------------------------------------
bool FSEdge::HasNode(int node) const
{
	return (FindNodeIndex(node) != -1);
}

//-----------------------------------------------------------------------------
void FSEdge::SetType(FSEdgeType type)
{ 
	assert(m_type == FE_EDGE_INVALID);
	assert(type != FE_EDGE_INVALID);
	m_type = type;
}

//-----------------------------------------------------------------------------
int FSEdge::Nodes() const
{ 
	static int nodeCount[] = {2, 3, 4, 0};
	assert(m_type != FE_EDGE_INVALID);
	return nodeCount[m_type];
}

//-----------------------------------------------------------------------------
//! Evaluate the shape function values at the iso-parametric point r = [0,1]
void FSEdge::shape(double* H, double r)
{
	switch (m_type)
	{
	case FE_EDGE2:
		H[0] = 1.0 - r;
		H[1] = r;
		break;
	case FE_EDGE3:
		H[0] = (1 - r)*(2 * (1 - r) - 1);
		H[1] = r*(2 * r - 1);
		H[2] = 4 * (1 - r)*r;
		break;
	case FE_EDGE4:
		H[0] = 0.5f*(1.f - r)*(3.f*r - 1.f)*(3.f*r - 2.f);
		H[1] = 0.5f*r*(3.f*r - 1.f)*(3.f*r - 2.f);
		H[2] = 9.f / 2.f*r*(r - 1.f)*(3.f*r - 2.f);
		H[3] = 9.f / 2.f*r*(1.f - r)*(3.f*r - 1.f);
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
double FSEdge::eval(double* d, double r)
{
	double H[FSEdge::MAX_NODES];
	shape(H, r);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return a;
}

//-----------------------------------------------------------------------------
vec3f FSEdge::eval(vec3f* d, double r)
{
	double H[FSEdge::MAX_NODES];
	shape(H, r);
	vec3f a(0, 0, 0);
	for (int i = 0; i<Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}

//-----------------------------------------------------------------------------
vec3d FSEdge::eval(vec3d* d, double r)
{
	double H[FSEdge::MAX_NODES];
	shape(H, r);
	vec3d a(0, 0, 0);
	for (int i = 0; i < Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}
