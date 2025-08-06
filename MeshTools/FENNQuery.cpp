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
#include "FENNQuery.h"
#include <stdlib.h>
#include <assert.h>

int cmp_node(const void* e1, const void* e2)
{
	FSNNQuery::NODE& n1 = *((FSNNQuery::NODE*)e1);
	FSNNQuery::NODE& n2 = *((FSNNQuery::NODE*)e2);

	return (n1.d1 > n2.d1 ? 1 : -1);
}

FSNNQuery::FSNNQuery(const std::vector<vec3d>& points) : m_points(points)
{
	m_imin = -1;
}

FSNNQuery::~FSNNQuery()
{

}

void FSNNQuery::Init()
{
	vec3d r0, r;
	int N = (int)m_points.size();

	// pick a random point as pivot
	r0 = m_q1 = m_points[0];

	// find the furtest node of this node
	double dmax = 0, d;
	for (int i=0; i<N; ++i)
	{
		r = m_points[i];
		d = (r - r0)*(r - r0);
		if (d > dmax)
		{
			m_q1 = r;
			dmax = d;
		}
	}

	// let's find the furthest node of this node
	r0 = m_q2 = m_q1;
	dmax = 0;
	for (int i=0; i<N; ++i)
	{
		r = m_points[i];
		d = (r - r0)*(r - r0);
		if (d > dmax)
		{
			m_q2 = r;
			dmax = d;
		}
	}

	// create the BK-"tree"
	m_bk.resize(N);
	for (int i=0; i<N; ++i)
	{
		r = m_points[i];
		m_bk[i].i = i;
		m_bk[i].r = r;
		m_bk[i].d1 = (m_q1 - r)*(m_q1 - r);
		m_bk[i].d2 = (m_q2 - r)*(m_q2 - r);
	}

	// sort the tree
	qsort((NODE*) &m_bk[0], N, sizeof(NODE), cmp_node);

	// set the initial search item
	m_imin = 0;
}

const vec3d& FSNNQuery::Find(const vec3d& x)
{
	return m_points[FindIndex(x)];
}

int FSNNQuery::FindIndex(const vec3d& x)
{
	double rmin1, rmin2, rmax1, rmax2;
	double rmin1s, rmin2s, rmax1s, rmax2s;
	double d, d1, d2, dmin;
	vec3d r;

	// set the initial search radii
	d1 = sqrt((m_q1 - x)*(m_q1 - x)); 
	rmin1 = 0;
	rmax1 = 2*d1;

	d2 = sqrt((m_q2 - x)*(m_q2 - x));
	rmin2 = 0;
	rmax2 = 2*d2;

	// check the last found item
	r = m_points[m_imin];
	dmin = (r - x)*(r - x);
	d = sqrt(dmin);
	
	// adjust search radii
	if (d1 - d > rmin1) rmin1 = d1 - d;
	if (d1 + d < rmax1) rmax1 = d1 + d;
	rmin1s = rmin1*rmin1;
	rmax1s = rmax1*rmax1;

	if (d2 - d > rmin2) rmin2 = d2 - d;
	if (d2 + d < rmax2) rmax2 = d2 + d;
	rmin2s = rmin2*rmin2;
	rmax2s = rmax2*rmax2;

	// find the first item that satisfies d(i, q1) >= rmin1
	int i0 = FindRadius(rmin1s);

	for (int i=i0; i<(int) m_bk.size(); ++i)
	{
		NODE& n = m_bk[i];
		if (n.d1 <= rmax1s)
		{
			if ((n.d2 >= rmin2s) && (n.d2 <= rmax2s))
			{
				r = n.r;
				d = (r - x)*(r - x);
				if (d < dmin)
				{
					dmin = d;
					d = sqrt(dmin);
					m_imin = n.i;

//					if (d1 - d > rmin1) rmin1 = d1 - d;
					if (d1 + d < rmax1) rmax1 = d1 + d;
//					rmin1s = rmin1*rmin1;
					rmax1s = rmax1*rmax1;

					if (d2 - d > rmin2) rmin2 = d2 - d;
					if (d2 + d < rmax2) rmax2 = d2 + d;
					rmin2s = rmin2*rmin2;
					rmax2s = rmax2*rmax2;
				}
			}
		}
		else break;
	}

/*
	// do it the hard way
	int imin = 0;
	r = m_points[imin];
	double d0 = (r - x)*(r - x);
	for (int i=0; i<m_points.size(); ++i)
	{
		r = m_points[i];
		d = (r - x)*(r - x);
		if (d < d0)
		{
			d0 = d;
			imin = i;
		}
	}
	assert(dmin == d0);
*/
	return m_imin;
}

//-----------------------------------------------------------------------------

int FSNNQuery::FindRadius(double r)
{
	int N = (int) m_bk.size();
	int L = N - 1;
	int i0 = 0;
	int i1 = L;
	if (m_bk[i1].d1 < r) return N;
	int i = i1 / 2;
	do
	{
		if (m_bk[i].d1 < r)
		{
			i0 = i;
			if (m_bk[i+1].d1 >= r) { ++i; break; }
		}
		else
		{
			i1 = i;
			if ((i==0) || (m_bk[i-1].d1 < r)) break;
		}
		i = (i1 + i0) / 2;
	}
	while(i0 != i1);

	return i;
}
