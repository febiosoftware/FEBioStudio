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

#include "MeshMetrics.h"
#include "triangulate.h"
#include "MeshTools.h"
#include "FENodeFaceList.h"
#include "hex.h"
#include "tet.h"
#include "penta.h"
#include "pyra.h"
#include "tri3.h"
#include "quad4.h"
#include <FECore/matrix.h>
using namespace std;

int FTHEX8[6][4] = {
	{ 0, 1, 5, 4 },
	{ 1, 2, 6, 5 },
	{ 2, 3, 7, 6 },
	{ 3, 0, 4, 7 },
	{ 3, 2, 1, 0 },
	{ 4, 5, 6, 7 } };

int FTHEX20[6][8] = {
	{ 0, 1, 5, 4, 8, 17, 12, 16 },
	{ 1, 2, 6, 5, 9, 18, 13, 17 },
	{ 2, 3, 7, 6, 10, 19, 14, 18 },
	{ 3, 0, 4, 7, 11, 16, 15, 19 },
	{ 3, 2, 1, 0, 10, 9, 8, 11 },
	{ 4, 5, 6, 7, 12, 13, 14, 15 } };

int FTHEX27[6][9] = {
	{ 0, 1, 5, 4, 8, 17, 12, 16, 20 },
	{ 1, 2, 6, 5, 9, 18, 13, 17, 21 },
	{ 2, 3, 7, 6, 10, 19, 14, 18, 22 },
	{ 3, 0, 4, 7, 11, 16, 15, 19, 23 },
	{ 3, 2, 1, 0, 10, 9, 8, 11, 24 },
	{ 4, 5, 6, 7, 12, 13, 14, 15, 25 } };

int FTPENTA[5][4] = {
	{ 0, 1, 4, 3 },
	{ 1, 2, 5, 4 },
	{ 0, 3, 5, 2 },
	{ 0, 2, 1, -1 },
	{ 3, 4, 5, -1 } };

int FTPENTA15[5][8] = {
	{ 0, 1, 4, 3, 6, 13, 9, 12 },
	{ 1, 2, 5, 4, 7, 14, 10, 13 },
	{ 0, 3, 5, 2, 12, 11, 14, 8 },
	{ 0, 2, 1, 8, 7, 6, -1, -1 },
	{ 3, 4, 5, 9, 10, 11, -1, -1 } };

int FTTET[4][3] = {
	{ 0, 1, 3 },
	{ 1, 2, 3 },
	{ 0, 3, 2 },
	{ 0, 2, 1 } };

int FTTET10[4][6] = {
	{ 0, 1, 3, 4, 8, 7 },
	{ 1, 2, 3, 5, 9, 8 },
	{ 2, 0, 3, 6, 7, 9 },
	{ 2, 1, 0, 5, 4, 6 } };

int FTTET15[4][7] = {
	{ 0, 1, 3, 4, 8, 7, 11 },
	{ 1, 2, 3, 5, 9, 8, 12 },
	{ 2, 0, 3, 6, 7, 9, 13 },
	{ 2, 1, 0, 5, 4, 6, 10 } };

int FTTET20[4][10] = {
	{ 0, 1, 3, 4, 5, 12, 13, 10, 11, 16 },
	{ 1, 2, 3, 6, 7, 14, 15, 12, 13, 17 },
	{ 2, 0, 3, 9, 8, 10, 11, 14, 15, 18 },
	{ 2, 1, 0, 7, 6,  5,  4,  9,  8, 19 }};

int FTPYRA5[5][4] = {
	{ 0, 1, 4, -1 },
	{ 1, 2, 4, -1 },
	{ 2, 3, 4, -1 },
	{ 3, 0, 4, -1 },
	{ 3, 2, 1, 0 }
};

int FTPYRA13[5][8] = {
    { 0, 1, 4, 5, 10,  9, -1, -1 },
    { 1, 2, 4, 6, 11, 10, -1, -1 },
    { 2, 3, 4, 7, 12, 11, -1, -1 },
    { 3, 0, 4, 8,  9, 12, -1, -1 },
    { 3, 2, 1, 0,  7,  6,  5,  8 }
};

//-----------------------------------------------------------------------
// in MeshTools\lut.cpp
extern int ET_HEX[12][2];
extern int ET_TET[6][2];
extern int ET_PENTA[9][2];
extern int ET_PYRA5[8][2];

// in FSElement.cpp
extern int ET_LINE[1][2];
extern int ET_TRI[3][2];
extern int ET_QUAD[4][2];

double GQUAD[4][4][2] = {
	{ { -0.5, -0.5 }, { 0.5, -0 }, { 0, 0 }, { -0, 0.5 } },
	{ { -0.5, -0 }, { 0.5, -0.5 }, { 0, 0.5 }, { -0, 0 } },
	{ { -0, -0 }, { 0, -0.5 }, { 0.5, 0.5 }, { -0.5, 0 } },
	{ { -0, -0.5 }, { 0, -0 }, { 0.5, 0 }, { -0.5, 0.5 } }
};

double GTRI[3][3][2] = {
	{ { -1, -1 }, { 1, 0 }, { 0, 1 } },
	{ { -1, -1 }, { 1, 0 }, { 0, 1 } },
	{ { -1, -1 }, { 1, 0 }, { 0, 1 } }
};

double GQUAD8[8][8][2] = {
	{ { -1.5, -1.5 }, { -0.5, 0. }, { 0., 0. }, { 0., -0.5 }, { 2., 0. }, { 0., 0. }, { 0., 0. }, { 0., 2. } },
	{ { 0.5, 0. }, { 1.5, -1.5 }, { 0., -0.5 }, { 0., 0. }, { -2., 0. }, { 0., 2. }, { 0., 0. }, { 0., 0. } },
	{ { 0., 0. }, { 0., 0.5 }, { 1.5, 1.5 }, { 0.5, 0. }, { 0., 0. }, { 0., -2. }, { -2., 0. }, { 0., 0. } },
	{ { 0., 0.5 }, { 0., 0. }, { -0.5, 0. }, { -1.5, 1.5 }, { 0., 0. }, { 0., 0. }, { 2., 0. }, { 0., -2. } },
	{ { -0.5, -0.5 }, { 0.5, -0.5 }, { 0., -0.5 }, { 0., -0.5 }, { 0., -0.5 }, { 0., 1. }, { 0., 0.5 }, { 0., 1. } },
	{ { 0.5, 0. }, { 0.5, -0.5 }, { 0.5, 0.5 }, { 0.5, 0. }, { -1., 0. }, { 0.5, 0. }, { -1., 0. }, { -0.5, 0. } },
	{ { 0., 0.5 }, { 0., 0.5 }, { 0.5, 0.5 }, { -0.5, 0.5 }, { 0., -0.5 }, { 0., -1. }, { 0., 0.5 }, { 0., -1. } },
	{ { -0.5, -0.5 }, { -0.5, 0. }, { -0.5, 0. }, { -0.5, 0.5 }, { 1., 0. }, { 0.5, 0. }, { 1., 0. }, { -0.5, 0. } }
};

double GTRI6[6][6][2] = {
	{ { -3., -3. }, { -1., 0 }, { 0, -1. }, { 4., 0. }, { 0., 0. }, { 0., 4. } },
	{ { 1., 1. }, { 3., 0 }, { 0, -1. }, { -4., -4. }, { 0., 4. }, { 0., 0. } },
	{ { 1., 1. }, { -1., 0 }, { 0, 3. }, { 0., 0. }, { 4., 0. }, { -4., -4. } },
	{ { -1., -1. }, { 1., 0 }, { 0, -1. }, { 0., -2. }, { 0., 2. }, { 0., 2. } },
	{ { 1., 1. }, { 1., 0 }, { 0, 1. }, { -2., -2. }, { 2., 2. }, { -2., -2. } },
	{ { -1., -1. }, { -1., 0 }, { 0, 1. }, { 2., 0. }, { 2., 0. }, { -2., 0. } }
};

namespace FEMeshMetrics {

//-----------------------------------------------------------------------------
// Calculate the shortest edge or diagonal for all the elements of the mesh.
double ShortestEdge(const FSMesh& mesh)
{
	double Lmin = 1e99;
	const int NE = mesh.Elements();
	for (int k = 0; k<NE; ++k)
	{
		const FSElement& ek = mesh.Element(k);
		int* en = ek.m_node;
		int n = ek.Nodes();
		for (int i = 0; i<n; ++i)
			for (int j = i + 1; j<n; ++j)
			{
				vec3d ri = mesh.NodePosition(en[i]);
				vec3d rj = mesh.NodePosition(en[j]);
				double L = (ri - rj).SqrLength();
				if (L < Lmin) Lmin = L;
			}
	}
	return sqrt(Lmin);
}

//-----------------------------------------------------------------------------
// Calculate the longest edge or diagonal.
//
double LongestEdge(const FSMesh& mesh, const FSElement &el)
{
	int* en = el.m_node;
	double Lmax = 0, L;
	int n = el.Nodes();
	for (int i = 0; i<n; ++i)
		for (int j = 1; j<n; ++j)
		{
			vec3d ri = mesh.NodePosition(en[i]);
			vec3d rj = mesh.NodePosition(en[j]);
			L = (ri - rj).SqrLength();
			if (L > Lmax) Lmax = L;
		}
	return sqrt(Lmax);
}

//-----------------------------------------------------------------------------
// Calculate the shortest edge or diagonal for this element.
//
double ShortestEdge(const FSMesh& mesh, const FSElement &el)
{
	int* en = el.m_node;
	double Lmin = 1e99, L;
	int n = el.Nodes();
	for (int i = 0; i<n; ++i)
		for (int j = i + 1; j<n; ++j)
		{
			vec3d ri = mesh.NodePosition(en[i]);
			vec3d rj = mesh.NodePosition(en[j]);
			L = (ri - rj).SqrLength();
			if (L < Lmin) Lmin = L;
		}
	return sqrt(Lmin);
}


//-----------------------------------------------------------------------------
// Calculate the min jacobian of a shell element
//
double ShellJacobian(const FSMesh& mesh, const FSElement& el, int flag)
{
	assert(el.IsShell());

	int i, j, k;
	int n = el.Nodes();
	double d, dmin = 1e99;
	vec3d r[9], D[9];
	double h[9];
	if (flag == 1)
	{
		for (i = 0; i<n; ++i) r[i] = mesh.NodePosition(el.m_node[i]);
	}
	else
	{
		for (i = 0; i<n; ++i) r[i] = mesh.Node(el.m_node[i]).r;
	}
	const FSFace& face = mesh.Face(el.m_face[0]);
	for (i = 0; i<n; ++i) D[i] = to_vec3d(face.m_nn[i]);//normal node
	for (i = 0; i<n; ++i) h[i] = el.m_h[i];//shell thickness

	for (k = 0; k<2; ++k)
	{
		for (i = 0; i<n; ++i)
		{
			double J[3][3] = { 0 };
			double z = (k == 0 ? 0.5*h[i] : -0.5*h[i]);
			if (el.IsType(FE_QUAD4))
			{
				for (j = 0; j<n; ++j)
				{
					J[0][0] += (r[j].x + z*D[j].x)*GQUAD[i][j][0]; J[1][0] += (r[j].y + z*D[j].y)*GQUAD[i][j][0]; J[2][0] += (r[j].z + z*D[j].z)*GQUAD[i][j][0];
					J[0][1] += (r[j].x + z*D[j].x)*GQUAD[i][j][1]; J[1][1] += (r[j].y + z*D[j].y)*GQUAD[i][j][1]; J[2][1] += (r[j].z + z*D[j].z)*GQUAD[i][j][1];
				}
				J[0][2] += 0.5*h[i] * D[i].x; J[1][2] += 0.5*h[i] * D[i].y; J[2][2] += 0.5*h[i] * D[i].z;
			}
			else if (el.IsType(FE_TRI3))
			{
				for (j = 0; j<3; ++j)
				{
					J[0][0] += (r[j].x + z*D[j].x)*GTRI[i][j][0]; J[1][0] += (r[j].y + z*D[j].y)*GTRI[i][j][0]; J[2][0] += (r[j].z + z*D[j].z)*GTRI[i][j][0];
					J[0][1] += (r[j].x + z*D[j].x)*GTRI[i][j][1]; J[1][1] += (r[j].y + z*D[j].y)*GTRI[i][j][1]; J[2][1] += (r[j].z + z*D[j].z)*GTRI[i][j][1];
				}
				J[0][2] += 0.5*h[i] * D[i].x; J[1][2] += 0.5*h[i] * D[i].y; J[2][2] += 0.5*h[i] * D[i].z;
			}
			if (el.IsType(FE_QUAD8))
			{
				for (j = 0; j<n; ++j)
				{
					J[0][0] += (r[j].x + z*D[j].x)*GQUAD8[i][j][0]; J[1][0] += (r[j].y + z*D[j].y)*GQUAD8[i][j][0]; J[2][0] += (r[j].z + z*D[j].z)*GQUAD8[i][j][0];
					J[0][1] += (r[j].x + z*D[j].x)*GQUAD8[i][j][1]; J[1][1] += (r[j].y + z*D[j].y)*GQUAD8[i][j][1]; J[2][1] += (r[j].z + z*D[j].z)*GQUAD8[i][j][1];
				}
				J[0][2] += 0.5*h[i] * D[i].x; J[1][2] += 0.5*h[i] * D[i].y; J[2][2] += 0.5*h[i] * D[i].z;
			}
			if (el.IsType(FE_TRI6))
			{
				for (j = 0; j<n; ++j)
				{
					J[0][0] += (r[j].x + z*D[j].x)*GTRI6[i][j][0]; J[1][0] += (r[j].y + z*D[j].y)*GTRI6[i][j][0]; J[2][0] += (r[j].z + z*D[j].z)*GTRI6[i][j][0];
					J[0][1] += (r[j].x + z*D[j].x)*GTRI6[i][j][1]; J[1][1] += (r[j].y + z*D[j].y)*GTRI6[i][j][1]; J[2][1] += (r[j].z + z*D[j].z)*GTRI6[i][j][1];
				}
				J[0][2] += 0.5*h[i] * D[i].x; J[1][2] += 0.5*h[i] * D[i].y; J[2][2] += 0.5*h[i] * D[i].z;
			}

			d = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1]) + J[0][1] * (J[1][2] * J[2][0] - J[1][0] * J[2][2]) + J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);
			if (d < dmin) dmin = d;
		}
	}

	return dmin;
}

//-----------------------------------------------------------------------------
double ShellArea(const FSMesh& mesh, const FSElement& el)
{
	if (el.IsShell() == false) return 0.0;

	double val = 0.0;
	vec3d ra[3], rb[3];
	if (el.Nodes() == 3)
	{
		ra[0] = mesh.NodePosition(el.m_node[0]);
		ra[1] = mesh.NodePosition(el.m_node[1]);
		ra[2] = mesh.NodePosition(el.m_node[2]);
		val = area_triangle(ra);
	}
	else if (el.Nodes() == 6)
	{
		ra[0] = mesh.NodePosition(el.m_node[0]);
		ra[1] = mesh.NodePosition(el.m_node[3]);
		ra[2] = mesh.NodePosition(el.m_node[5]);
		val = area_triangle(ra);

		ra[0] = mesh.NodePosition(el.m_node[3]);
		ra[1] = mesh.NodePosition(el.m_node[1]);
		ra[2] = mesh.NodePosition(el.m_node[4]);
		val += area_triangle(ra);

		ra[0] = mesh.NodePosition(el.m_node[4]);
		ra[1] = mesh.NodePosition(el.m_node[2]);
		ra[2] = mesh.NodePosition(el.m_node[5]);
		val += area_triangle(ra);

		ra[0] = mesh.NodePosition(el.m_node[5]);
		ra[1] = mesh.NodePosition(el.m_node[3]);
		ra[2] = mesh.NodePosition(el.m_node[4]);
		val += area_triangle(ra);
	}
	else
	{
		ra[0] = mesh.NodePosition(el.m_node[0]);
		ra[1] = mesh.NodePosition(el.m_node[1]);
		ra[2] = mesh.NodePosition(el.m_node[2]);

		rb[0] = mesh.NodePosition(el.m_node[2]);
		rb[1] = mesh.NodePosition(el.m_node[3]);
		rb[2] = mesh.NodePosition(el.m_node[0]);
		val = area_triangle(ra) + area_triangle(rb);
	}

	return val;
}

//-----------------------------------------------------------------------------
// Calculate min jacobian of a solid element
// Evaluates the jacobian at the element's nodes and find the smallest (or most negative) value
double SolidJacobian(const FSMesh& mesh, const FSElement& el)
{
	assert(el.IsSolid());

    // nodal coordinates
    vec3d r[FSElement::MAX_NODES];
    mesh.ElementNodeLocalPositions(el, r);

	// calculate jacobian based on element type
    // use flag 'true' to evaluate min Jacobian
    switch (el.Type())
    {
        case FE_TET4: return tet4_volume(r, true); break;
        case FE_TET5: return tet5_volume(r, true); break;
        case FE_TET10: return tet10_volume(r, true); break;
        case FE_TET15: return tet15_volume(r, true); break;
        case FE_TET20: return tet20_volume(r, true); break;
        case FE_HEX8: return hex8_volume(r, true); break;
        case FE_HEX20: return hex20_volume(r, true); break;
        case FE_HEX27: return hex27_volume(r, true); break;
        case FE_PENTA6: return penta6_volume(r, true); break;
        case FE_PENTA15: return penta15_volume(r, true); break;
        case FE_PYRA5: return pyra5_volume(r, true); break;
        case FE_PYRA13: return pyra13_volume(r, true); break;
        default: return 0;
    }
}

//-----------------------------------------------------------------------------
// Calculate (approximate) surface of a face
//
double SurfaceArea(const FSMesh& mesh, const FSFace& f)
{
	vec3d ra[3], rb[3];
	if (f.Nodes() == 3)
	{
		ra[0] = mesh.NodePosition(f.n[0]);
		ra[1] = mesh.NodePosition(f.n[1]);
		ra[2] = mesh.NodePosition(f.n[2]);
		return area_triangle(ra);
	}
	else
	{
		ra[0] = mesh.NodePosition(f.n[0]);
		ra[1] = mesh.NodePosition(f.n[1]);
		ra[2] = mesh.NodePosition(f.n[2]);

		rb[0] = mesh.NodePosition(f.n[2]);
		rb[1] = mesh.NodePosition(f.n[3]);
		rb[2] = mesh.NodePosition(f.n[0]);
		return area_triangle(ra) + area_triangle(rb);
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Evaluate volume of an element
//
double ElementVolume(const FSMesh& mesh, const FSElement &e)
{
	// nodal coordinates
    vec3d r[FSElement::MAX_NODES];
	mesh.ElementNodeLocalPositions(e, r);

    switch (e.Type())
    {
        case FE_TET4: return tet4_volume(r); break;
        case FE_TET5: return tet5_volume(r); break;
        case FE_TET10: return tet10_volume(r); break;
        case FE_TET15: return tet15_volume(r); break;
        case FE_TET20: return tet20_volume(r); break;
        case FE_HEX8: return hex8_volume(r); break;
        case FE_HEX20: return hex20_volume(r); break;
        case FE_HEX27: return hex27_volume(r); break;
        case FE_PENTA6: return penta6_volume(r); break;
        case FE_PENTA15: return penta15_volume(r); break;
        case FE_PYRA5: return pyra5_volume(r); break;
        case FE_PYRA13: return pyra13_volume(r); break;
        default: return 0;
    }
}

//-----------------------------------------------------------------------------
// calculates the maximum distance of the midside nodes to the plane of 
// the triangle facets.
double Tet10MidsideNodeOffset(const FSMesh& mesh, const FSElement& el, bool brel)
{
	if (el.IsType(FE_TET10) == false) throw 0;

	// max distance
	double maxd = 0;

	// loop over all neighbors
	vec3d a[3];
	bool ok = false;
	for (int i = 0; i<4; ++i)
	{
		if (el.m_nbr[i] == -1)
		{
			ok = true;
			int* n = FTTET10[i];
			a[0] = mesh.Node(el.m_node[n[0]]).r;
			a[1] = mesh.Node(el.m_node[n[1]]).r;
			a[2] = mesh.Node(el.m_node[n[2]]).r;

			vec3d N = (a[1] - a[0]) ^ (a[2] - a[0]);
			N.Normalize();

			for (int j = 0; j<3; ++j)
			{
				vec3d r = mesh.Node(el.m_node[n[3 + j]]).r;
				vec3d dr = r - a[j];
				vec3d t = a[(j + 1) % 3] - a[j];
				vec3d e = t.Normalized();

				double d = (dr - e*(dr*e)).Length();

				if (brel)
				{
					double l = 0.5*t.Length();
					d = 2.0*d/(l*l + d*d);
				}

				double D = r.Length();

				if (d > maxd) maxd = d;
			}
		}
	}

	if (ok == false) throw 0;

	return maxd;
}

//-----------------------------------------------------------------------------
double TriQuality(const FSMesh& mesh, const FSElement& el)
{
	if (el.IsType(FE_TRI3) == false) return 0.;

	// get the tet's nodal coordinates
	vec3d r[3];
	for (int i = 0; i<3; ++i) r[i] = mesh.Node(el.m_node[i]).r;

	return TriangleQuality(r);
}

//-----------------------------------------------------------------------------
double TriMaxDihedralAngle(const FSMesh& mesh, const FSElement& el)
{
	if (el.IsType(FE_TRI3) == false) return 0.;

	if (el.m_face[0] < 0) return 0;
	const FSFace& f = mesh.Face(el.m_face[0]);

	double maxAngle = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (f.m_nbr[i] >= 0)
		{
			const FSFace& fi = mesh.Face(f.m_nbr[i]);
			double a = acos(f.m_fn * fi.m_fn);
			if (a > maxAngle) maxAngle = a;
		}
	}
	return maxAngle * 180.0 / PI;
}

//-----------------------------------------------------------------------------
//! Calculate tet-element quality
//! This calculates the radius-edge ratio
double TetQuality(const FSMesh& mesh, const FSElement& el)
{
	if ((el.IsType(FE_TET4) == false) && (el.IsType(FE_TET10) == false)) throw 0;

	// get the tet's nodal coordinates
	vec3d p[4];
	for (int i = 0; i<4; ++i) p[i] = mesh.Node(el.m_node[i]).r;

	// setup system of equation
	mat3d A;
	A[0][0] = p[1].x - p[0].x; A[0][1] = p[1].y - p[0].y; A[0][2] = p[1].z - p[0].z;
	A[1][0] = p[2].x - p[0].x; A[1][1] = p[2].y - p[0].y; A[1][2] = p[2].z - p[0].z;
	A[2][0] = p[3].x - p[0].x; A[2][1] = p[3].y - p[0].y; A[2][2] = p[3].z - p[0].z;
	A = A.inverse();

	// setup RHS
	vec3d b;
	b.x = 0.5*(p[1].x*p[1].x - p[0].x*p[0].x + p[1].y*p[1].y - p[0].y*p[0].y + p[1].z*p[1].z - p[0].z*p[0].z);
	b.y = 0.5*(p[2].x*p[2].x - p[0].x*p[0].x + p[2].y*p[2].y - p[0].y*p[0].y + p[2].z*p[2].z - p[0].z*p[0].z);
	b.z = 0.5*(p[3].x*p[3].x - p[0].x*p[0].x + p[3].y*p[3].y - p[0].y*p[0].y + p[3].z*p[3].z - p[0].z*p[0].z);

	// find the center of the circum sphere
	vec3d c = A*b;

	// find the radius of the circum sphere
	double R2 = (p[0].x - c.x)*(p[0].x - c.x) + (p[0].y - c.y)*(p[0].y - c.y) + (p[0].z - c.z)*(p[0].z - c.z);
	double R = sqrt(R2);

	// find the shortest edge
	const int ET[6][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 0, 3 }, { 1, 3 }, { 2, 3 } };

	double L2, L2min = 1e99;
	for (int i = 0; i<6; ++i)
	{
		int j = ET[i][0];
		int k = ET[i][1];
		L2 = (p[j].x - p[k].x)*(p[j].x - p[k].x) + (p[j].y - p[k].y)*(p[j].y - p[k].y) + (p[j].z - p[k].z)*(p[j].z - p[k].z);
		if (L2 < L2min) L2min = L2;
	}
	double L = sqrt(L2min);

	return R / L;
}

//-----------------------------------------------------------------------------
//! Calculates the smallest dihedral angle for a tet element
double TetMinDihedralAngle(const FSMesh& mesh, const FSElement& el)
{
	if (el.Type() != FE_TET4) return 0.0;

	// get the nodal coordinates
	vec3d r[4];
	for (int i = 0; i<4; ++i) r[i] = mesh.Node(el.m_node[i]).r;

	// find the normals of all four faces
	vec3d fn[4];
	for (int i = 0; i<4; ++i)
	{
		int* m = FTTET[i];
		fn[i] = (r[m[1]] - r[m[0]]) ^ (r[m[2]] - r[m[0]]);
		fn[i].Normalize();
	}

	const int LT[6][2] = { { 0, 1 }, { 1, 2 }, { 0, 2 }, { 0, 3 }, { 1, 3 }, { 2, 3 } };
	double cwmin = -1.0;
	for (int i = 0; i<6; ++i)
	{
		double cw = -fn[LT[i][0]] * fn[LT[i][1]];
		if (cw > cwmin) cwmin = cw;
	}

	double w = 180.0*acos(cwmin) / PI;
	return w;
}

//-----------------------------------------------------------------------------
//! Calculates the largest dihedral angle for a tet element
double TetMaxDihedralAngle(const FSMesh& mesh, const FSElement& el)
{
	if (el.Type() != FE_TET4) return 0.0;

	// get the nodal coordinates
	vec3d r[4];
	for (int i = 0; i<4; ++i) r[i] = mesh.Node(el.m_node[i]).r;

	// find the normals of all four faces
	vec3d fn[4];
	for (int i = 0; i<4; ++i)
	{
		int* m = FTTET[i];
		fn[i] = (r[m[1]] - r[m[0]]) ^ (r[m[2]] - r[m[0]]);
		fn[i].Normalize();
	}

	const int LT[6][2] = { { 0, 1 }, { 1, 2 }, { 0, 2 }, { 0, 3 }, { 1, 3 }, { 2, 3 } };
	double cwmin = 1.0;
	for (int i = 0; i<6; ++i)
	{
		double cw = -fn[LT[i][0]] * fn[LT[i][1]];
		if (cw < cwmin) cwmin = cw;
	}

	double w = 180.0*acos(cwmin) / PI;
	return w;
}

//-----------------------------------------------------------------------------
vec3d GradientSolid(const FSMesh& mesh, const FSElement& el, int node, double* v);
vec3d GradientShell(const FSMesh& mesh, const FSElement& el, int node, double* v);

vec3d Gradient(const FSMesh& mesh, const FSElement& el, int node, double* v)
{
	if (el.IsSolid()) return GradientSolid(mesh, el, node, v);
	else if (el.IsShell()) return GradientShell(mesh, el, node, v);
	assert(false);
	return vec3d(0, 0, 0);
}

vec3d GradientSolid(const FSMesh& mesh, const FSElement& el, int node, double* v)
{
	const int MN = FSElement::MAX_NODES;
	vec3d r[MN];
	const int ne = el.Nodes();
	mesh.ElementNodeLocalPositions(el, r);

	// shape function derivatives at node
	double G[3][MN] = { 0 };
    double q[3];
    switch (el.Type())
    {
        case FE_TET4:
        {
            TET4::iso_coord(node,q);
            TET4::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET5:
        {
            TET5::iso_coord(node,q);
            TET5::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET10:
        {
            TET10::iso_coord(node,q);
            TET10::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET15:
        {
            TET15::iso_coord(node,q);
            TET15::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET20:
        {
            TET20::iso_coord(node,q);
            TET20::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_HEX8:
        {
            HEX8::iso_coord(node,q);
            HEX8::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_HEX20:
        {
            HEX20::iso_coord(node,q);
            HEX20::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_HEX27:
        {
            HEX27::iso_coord(node,q);
            HEX27::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PENTA6:
        {
            PENTA6::iso_coord(node,q);
            PENTA6::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PENTA15:
        {
            PENTA15::iso_coord(node,q);
            PENTA15::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PYRA5:
        {
            PYRA5::iso_coord(node,q);
            PYRA5::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PYRA13:
        {
            PYRA13::iso_coord(node,q);
            PYRA13::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        default:
            return vec3d(0, 0, 0);
    }

	// Jacobian
	mat3d J; J.zero();
	for (int i = 0; i<ne; ++i)
	{
		J[0][0] += G[0][i] * r[i].x; J[0][1] += G[1][i] * r[i].x; J[0][2] += G[2][i] * r[i].x;
		J[1][0] += G[0][i] * r[i].y; J[1][1] += G[1][i] * r[i].y; J[1][2] += G[2][i] * r[i].y;
		J[2][0] += G[0][i] * r[i].z; J[2][1] += G[1][i] * r[i].z; J[2][2] += G[2][i] * r[i].z;
	}
	J = J.inverse();
	J = J.transpose();

	// shape function gradients
	double Gx[MN] = { 0 }, Gy[MN] = { 0 }, Gz[MN] = { 0 };
	for (int i = 0; i<ne; ++i)
	{
		Gx[i] += J[0][0] * G[0][i] + J[0][1] * G[1][i] + J[0][2] * G[2][i];
		Gy[i] += J[1][0] * G[0][i] + J[1][1] * G[1][i] + J[1][2] * G[2][i];
		Gz[i] += J[2][0] * G[0][i] + J[2][1] * G[1][i] + J[2][2] * G[2][i];
	}

	// gradient
	vec3d g(0, 0, 0);
	for (int j = 0; j<ne; ++j)
	{
		g.x += Gx[j] * v[j];
		g.y += Gy[j] * v[j];
		g.z += Gz[j] * v[j];
	}

	return g;
}

vec3d GradientShell(const FSMesh& mesh, const FSElement& el, int node, double* v)
{
	const int MN = FSElement::MAX_NODES;
	vec3d r[MN];
	const int ne = el.Nodes();
	mesh.ElementNodeLocalPositions(el, r);

	// shape function derivatives at node
	double G[2][MN] = { 0 };
	double q[2];
	switch (el.Type())
	{
	case FE_TRI3:
	{
		TRI3::iso_coord(node, q);
		TRI3::shape_deriv(G[0], G[1], q[0], q[1]);
	}
	break;
	case FE_QUAD4:
	{
		QUAD4::iso_coord(node, q);
		QUAD4::shape_deriv(G[0], G[1], q[0], q[1]);
	}
	break;
	default:
		return vec3d(0, 0, 0);
	}

	// get the surface normal
	int n0 = node;
	int n1 = (node + 1) % ne;
	int n2 = (node + (ne-1)) % ne;
	vec3d e1 = r[n1] - r[n0];
	vec3d e2 = r[n2] - r[n0];
	vec3d e3 = e1 ^ e2;
	e3.Normalize();

	// shell thickness
	double h = 1.0;

	// Jacobian
	mat3d J; J.zero();
	for (int i = 0; i < ne; ++i)
	{
		J[0][0] += G[0][i] * r[i].x; J[0][1] += G[1][i] * r[i].x;
		J[1][0] += G[0][i] * r[i].y; J[1][1] += G[1][i] * r[i].y;
		J[2][0] += G[0][i] * r[i].z; J[2][1] += G[1][i] * r[i].z;
	}
	J[0][2] += h*e3.x;
	J[1][2] += h*e3.y;
	J[2][2] += h*e3.z;

	J = J.inverse();
	J = J.transpose();

	// shape function gradients
	double Gx[MN] = { 0 }, Gy[MN] = { 0 }, Gz[MN] = { 0 };
	for (int i = 0; i < ne; ++i)
	{
		Gx[i] += J[0][0] * G[0][i] + J[0][1] * G[1][i];
		Gy[i] += J[1][0] * G[0][i] + J[1][1] * G[1][i];
		Gz[i] += J[2][0] * G[0][i] + J[2][1] * G[1][i];
	}

	// gradient
	vec3d g(0, 0, 0);
	for (int j = 0; j < ne; ++j)
	{
		g.x += Gx[j] * v[j];
		g.y += Gy[j] * v[j];
		g.z += Gz[j] * v[j];
	}

	return g;
}

//-----------------------------------------------------------------------------
// evaluate gradient at element nodes (i.e. Grad{Na(x_b)})

vec3d ShapeGradientSolid(const FSMesh& mesh, const FEElement_& el, int na, int nb);
vec3d ShapeGradientShell(const FSMesh& mesh, const FEElement_& el, int na, int nb);

vec3d ShapeGradient(const FSMesh& mesh, const FEElement_& el, int na, int nb)
{
	if (el.IsSolid()) return ShapeGradientSolid(mesh, el, na, nb);
	else if (el.IsShell()) return ShapeGradientShell(mesh, el, na, nb);
	assert(false);
	return vec3d(0, 0, 0);
}

vec3d ShapeGradientSolid(const FSMesh& mesh, const FEElement_& el, int na, int nb)
{
	const int MN = FSElement::MAX_NODES;
	vec3d r[MN];
	const int ne = el.Nodes();
	mesh.ElementNodeLocalPositions(el, r);

	// shape function derivatives at node
	double G[3][FSElement::MAX_NODES] = { 0 };
    double q[3];
    switch (el.Type())
    {
        case FE_TET4:
        {
            TET4::iso_coord(nb,q);
            TET4::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET5:
        {
            TET5::iso_coord(nb,q);
            TET5::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET10:
        {
            TET10::iso_coord(nb,q);
            TET10::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET15:
        {
            TET15::iso_coord(nb,q);
            TET15::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_TET20:
        {
            TET20::iso_coord(nb,q);
            TET20::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_HEX8:
        {
            HEX8::iso_coord(nb,q);
            HEX8::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_HEX20:
        {
            HEX20::iso_coord(nb,q);
            HEX20::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_HEX27:
        {
            HEX27::iso_coord(nb,q);
            HEX27::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PENTA6:
        {
            PENTA6::iso_coord(nb,q);
            PENTA6::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PENTA15:
        {
            PENTA15::iso_coord(nb,q);
            PENTA15::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PYRA5:
        {
            PYRA5::iso_coord(nb,q);
            PYRA5::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        case FE_PYRA13:
        {
            PYRA13::iso_coord(nb,q);
            PYRA13::shape_deriv(G[0], G[1], G[2], q[0], q[1], q[2]);
        }
            break;
        default:
            return vec3d(0, 0, 0);
    }

	// Jacobian at node b
	mat3d J; J.zero();
	for (int i = 0; i<ne; ++i)
	{
		J[0][0] += G[0][i] * r[i].x; J[0][1] += G[1][i] * r[i].x; J[0][2] += G[2][i] * r[i].x;
		J[1][0] += G[0][i] * r[i].y; J[1][1] += G[1][i] * r[i].y; J[1][2] += G[2][i] * r[i].y;
		J[2][0] += G[0][i] * r[i].z; J[2][1] += G[1][i] * r[i].z; J[2][2] += G[2][i] * r[i].z;
	}
	J = J.inverse();
	J = J.transpose();

	// shape function gradient
	vec3d grad(0, 0, 0);
	grad.x = J[0][0] * G[0][na] + J[0][1] * G[1][na] + J[0][2] * G[2][na];
	grad.y = J[1][0] * G[0][na] + J[1][1] * G[1][na] + J[1][2] * G[2][na];
	grad.z = J[2][0] * G[0][na] + J[2][1] * G[1][na] + J[2][2] * G[2][na];

	return grad;
}

// NOTE: This is a work in progress and was implemented to apply the scalar field tool to shells.
vec3d ShapeGradientShell(const FSMesh& mesh, const FEElement_& el, int na, int nb)
{
	const int MN = FSElement::MAX_NODES;
	vec3d r[MN];
	const int ne = el.Nodes();
	mesh.ElementNodeLocalPositions(el, r);

	// shape function derivatives at node
	double G[2][FSElement::MAX_NODES] = { 0 };
	double q[2];
	switch (el.Type())
	{
	case FE_TRI3:
	{
		TRI3::iso_coord(nb, q);
		TRI3::shape_deriv(G[0], G[1], q[0], q[1]);
	}
	break;
	case FE_QUAD4:
	{
		QUAD4::iso_coord(nb, q);
		QUAD4::shape_deriv(G[0], G[1], q[0], q[1]);
	}
	break;
	default:
		return vec3d(0, 0, 0);
	}

	// get the surface normal
	int n0 = nb;
	int n1 = (nb + 1) % ne;
	int n2 = (nb + (ne - 1)) % ne;
	vec3d e1 = r[n1] - r[n0];
	vec3d e2 = r[n2] - r[n0];
	vec3d e3 = e1 ^ e2;
	e3.Normalize();

	// shell thickenss
	double h = 1.0;

	// Jacobian at node b
	mat3d J; J.zero();
	for (int i = 0; i < ne; ++i)
	{
		J[0][0] += G[0][i] * r[i].x; J[0][1] += G[1][i] * r[i].x;
		J[1][0] += G[0][i] * r[i].y; J[1][1] += G[1][i] * r[i].y;
		J[2][0] += G[0][i] * r[i].z; J[2][1] += G[1][i] * r[i].z;
	}
	J[0][2] += h*e3.x;
	J[1][2] += h*e3.y;
	J[2][2] += h*e3.z;

	J = J.inverse();
	J = J.transpose();

	// shape function gradient
	vec3d grad(0, 0, 0);
	grad.x = J[0][0] * G[0][na] + J[0][1] * G[1][na];
	grad.y = J[1][0] * G[0][na] + J[1][1] * G[1][na];
	grad.z = J[2][0] * G[0][na] + J[2][1] * G[1][na];

	return grad;
}

// get the min edge length of an element
double MinEdgeLength(const FSMesh& mesh, const FSElement& e)
{
	// get the number of edges and edge table
	// TODO: do ELEM_PYRA
	int edges = 0;
	const int(*ET)[2] = 0;
	int shape = e.Shape();
	switch (shape)
	{
	case ELEM_HEX  : edges = 12; ET = ET_HEX  ; break;
	case ELEM_TET  : edges =  6; ET = ET_TET  ; break;
	case ELEM_PENTA: edges =  9; ET = ET_PENTA; break;
    case ELEM_PYRA : edges =  8; ET = ET_PYRA5; break;
	case ELEM_TRI  : edges =  3; ET = ET_TRI  ; break;
	case ELEM_QUAD : edges =  4; ET = ET_QUAD ; break;
	case ELEM_LINE : edges =  1; ET = ET_LINE ; break;
	default:
		assert(false);
		return 0;
	}
	
	// find the smallest edge
	double Lmin = 1e99;
	for (int i = 0; i < edges; ++i)
	{
		vec3d r1 = mesh.Node(e.m_node[ET[i][0]]).pos();
		vec3d r2 = mesh.Node(e.m_node[ET[i][1]]).pos();

		double L = (r2 - r1).Length();
		if (L < Lmin) Lmin = L;
	}

	return Lmin;
}

// get the max edge length of an element
double MaxEdgeLength(const FSMesh& mesh, const FSElement& e)
{
	// get the number of edges and edge table
	// TODO: do ELEM_PYRA
	int edges = 0;
	const int(*ET)[2] = 0;
	int shape = e.Shape();
	switch (shape)
	{
	case ELEM_HEX  : edges = 12; ET = ET_HEX  ; break;
	case ELEM_TET  : edges =  6; ET = ET_TET  ; break;
	case ELEM_PENTA: edges =  9; ET = ET_PENTA; break;
    case ELEM_PYRA : edges =  8; ET = ET_PYRA5; break;
	case ELEM_TRI  : edges =  3; ET = ET_TRI  ; break;
	case ELEM_QUAD : edges =  4; ET = ET_QUAD ; break;
	case ELEM_LINE : edges =  1; ET = ET_LINE ; break;
	default:
		assert(false);
		return 0;
	}
	
	// find the smallest edge
	double Lmax = 0.0;
	for (int i = 0; i < edges; ++i)
	{
		vec3d r1 = mesh.Node(e.m_node[ET[i][0]]).pos();
		vec3d r2 = mesh.Node(e.m_node[ET[i][1]]).pos();

		double L = (r2 - r1).Length();
		if (L > Lmax) Lmax = L;
	}

	return Lmax;
}

// get the min edge length of a face
double MinEdgeLength(const FSMeshBase& mesh, const FSFace& f)
{
	int edges = 0;
	const int(*ET)[2] = 0;
	int shape = f.Shape();
	switch (shape)
	{
	case FE_FACE_TRI: edges = 3; ET = ET_TRI; break;
	case FE_FACE_QUAD: edges = 4; ET = ET_QUAD; break;
	default:
		assert(false);
		return 0;
	}

	// find the smallest edge
	double Lmin = 1e99;
	for (int i = 0; i < edges; ++i)
	{
		vec3d r1 = mesh.Node(f.n[ET[i][0]]).pos();
		vec3d r2 = mesh.Node(f.n[ET[i][1]]).pos();

		double L = (r2 - r1).Length();
		if (L < Lmin) Lmin = L;
	}

	return Lmin;
}

// get the max edge length of a face
double MaxEdgeLength(const FSMeshBase& mesh, const FSFace& f)
{
	int edges = 0;
	const int(*ET)[2] = 0;
	int shape = f.Shape();
	switch (shape)
	{
	case FE_FACE_TRI : edges = 3; ET = ET_TRI; break;
	case FE_FACE_QUAD: edges = 4; ET = ET_QUAD; break;
	default:
		assert(false);
		return 0;
	}

	// find the smallest edge
	double Lmax = 0.0;
	for (int i = 0; i < edges; ++i)
	{
		vec3d r1 = mesh.Node(f.n[ET[i][0]]).pos();
		vec3d r2 = mesh.Node(f.n[ET[i][1]]).pos();

		double L = (r2 - r1).Length();
		if (L > Lmax) Lmax = L;
	}

	return Lmax;
}

double Curvature(FSMesh& mesh, int node, int measure, int levels, int maxIters, bool extQuad)
{
	// get the reference nodal position
	vec3f r0 = to_vec3f(mesh.Node(node).pos());

	// get the node-face list
	const vector<NodeFaceRef>& nfl = mesh.NodeFaceList(node);
	int NF = nfl.size();

	// estimate surface normal
	vec3f sn(0, 0, 0);
	for (int i = 0; i < NF; ++i)
	{
		const FSFace& f = mesh.Face(nfl[i].fid);
		sn += f.m_fn;
	}
	sn.Normalize();

	// array of nodal points
	vector<vec3f> x; x.reserve(128);

	// number of levels
	int nlevels = levels;
	if (nlevels < 0) nlevels = 0;
	if (nlevels > 10) nlevels = 10;

	// get the neighbors
	set<int> nl1;
	mesh.GetNodeNeighbors(node, nlevels, nl1);

	// get the node coordinates
	set<int>::iterator it;
	for (it = nl1.begin(); it != nl1.end(); ++it)
	{
		if (*it != node) x.push_back(to_vec3f(mesh.Node(*it).pos()));
	}

	// evaluate curvature
	float K = eval_curvature(x, r0, sn, measure, extQuad, maxIters);

	return K;
}

float eval_curvature(const vector<vec3f>& x, const vec3f& r0, vec3f sn, int measure, bool useExtendedFit, int maxIter)
{
	vector<vec3f> y; y.reserve(128);
	y.resize(x.size());
	int nn = x.size();

	// for less than three neighbors, we assume K = 0
	double K = 0.0;
	if (nn < 3) K = 0;
	else if ((nn < 5) || (useExtendedFit == false))
	{
		// for less than 5 points, or if the extended quadric flag is zero,
		// we use the quadric method

		// construct local coordinate system
		vec3f e3 = sn;

		vec3f qx(1.f - sn.x * sn.x, -sn.y * sn.x, -sn.z * sn.x);
		if (qx.Length() < 1e-5) qx = vec3f(-sn.x * sn.y, 1.f - sn.y * sn.y, -sn.z * sn.y);
		qx.Normalize();
		vec3f e1 = qx;

		vec3f e2 = e3 ^ e1;

		mat3f Q;
		Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
		Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
		Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
		mat3f Qt = Q.transpose();

		// map coordinates
		for (int i = 0; i < nn; ++i)
		{
			vec3f tmp = x[i] - r0;
			y[i] = Q * tmp;
		}

		// setup the linear system
		matrix R(nn, 3);
		vector<double> r(nn);
		for (int i = 0; i < nn; ++i)
		{
			vec3f& p = y[i];
			R[i][0] = p.x * p.x;
			R[i][1] = p.x * p.y;
			R[i][2] = p.y * p.y;
			r[i] = p.z;
		}

		// solve for quadric coefficients
		vector<double> q(3);
		R.lsq_solve(q, r);
		double a = q[0], b = q[1], c = q[2];

		// calculate curvature
		switch (measure)
		{
		case 0: // Gaussian
			K = 4 * a * c - b * b;
			break;
		case 1:	// Mean
			K = a + c;
			break;
		case 2:	// 1-principal
			K = a + c + sqrt((a - c) * (a - c) + b * b);
			break;
		case 3:	// 2-principal
			K = a + c - sqrt((a - c) * (a - c) + b * b);
			break;
		case 4:	// rms
		{
			double k1 = a + c + sqrt((a - c) * (a - c) + b * b);
			double k2 = a + c - sqrt((a - c) * (a - c) + b * b);
			K = sqrt(0.5 * (k1 * k1 + k2 * k2));
		}
		break;
		case 5: // diff
		{
			double k1 = a + c + sqrt((a - c) * (a - c) + b * b);
			double k2 = a + c - sqrt((a - c) * (a - c) + b * b);
			K = k1 - k2;
		}
		break;
		default:
			assert(false);
			K = 0;
		}
	}
	else
	{
		// loop until converged
		const int NMAX = 100;
		int nmax = maxIter;
		if (nmax > NMAX) nmax = NMAX;
		if (nmax < 1) nmax = 1;
		int niter = 0;
		while (niter < nmax)
		{
			// construct local coordinate system
			vec3f e3 = sn;

			vec3f qx(1.f - sn.x * sn.x, -sn.y * sn.x, -sn.z * sn.x);
			if (qx.Length() < 1e-5) qx = vec3f(-sn.x * sn.y, 1.f - sn.y * sn.y, -sn.z * sn.y);
			qx.Normalize();
			vec3f e1 = qx;

			vec3f e2 = e3 ^ e1;

			mat3f Q;
			Q[0][0] = e1.x; Q[1][0] = e2.x; Q[2][0] = e3.x;
			Q[0][1] = e1.y; Q[1][1] = e2.y; Q[2][1] = e3.y;
			Q[0][2] = e1.z; Q[1][2] = e2.z; Q[2][2] = e3.z;
			mat3f Qt = Q.transpose();

			// map coordinates
			for (int i = 0; i < nn; ++i)
			{
				vec3f tmp = x[i] - r0;
				y[i] = Q * tmp;
			}

			// setup the linear system
			matrix R(nn, 5);
			vector<double> r(nn);
			for (int i = 0; i < nn; ++i)
			{
				vec3f& p = y[i];
				R[i][0] = p.x * p.x;
				R[i][1] = p.x * p.y;
				R[i][2] = p.y * p.y;
				R[i][3] = p.x;
				R[i][4] = p.y;
				r[i] = p.z;
			}

			// solve for quadric coefficients
			vector<double> q(5);
			R.lsq_solve(q, r);
			double a = q[0], b = q[1], c = q[2], d = q[3], e = q[4];

			// calculate curvature
			double D = 1.0 + d * d + e * e;
			switch (measure)
			{
			case 0: // Gaussian
				K = (4 * a * c - b * b) / (D * D);
				break;
			case 1: // Mean
				K = (a + c + a * e * e + c * d * d - b * d * e) / pow(D, 1.5);
				break;
			case 2: // 1-principal
			{
				double H = (a + c + a * e * e + c * d * d - b * d * e) / pow(D, 1.5);
				double G = (4 * a * c - b * b) / (D * D);
				K = H + sqrt(H * H - G);
			}
			break;
			case 3: // 2-principal
			{
				double H = (a + c + a * e * e + c * d * d - b * d * e) / pow(D, 1.5);
				double G = (4 * a * c - b * b) / (D * D);
				K = H - sqrt(H * H - G);
			}
			break;
			case 4: // RMS
			{
				double H = (a + c + a * e * e + c * d * d - b * d * e) / pow(D, 1.5);
				double G = (4 * a * c - b * b) / (D * D);
				double k1 = H + sqrt(H * H - G);
				double k2 = H - sqrt(H * H - G);
				K = sqrt((k1 * k1 + k2 * k2) * 0.5);
			}
			break;
			case 5:
			{
				double H = (a + c + a * e * e + c * d * d - b * d * e) / pow(D, 1.5);
				double G = (4 * a * c - b * b) / (D * D);
				double k1 = H + sqrt(H * H - G);
				double k2 = H - sqrt(H * H - G);
				K = k1 - k2;
			}
			break;
			default:
				assert(false);
				K = 0;
			}

			// setup the new normal
			sn = vec3f(-(float)d, -(float)e, 1.f);
			sn.Normalize();
			sn = Qt * sn;

			// increase counter
			niter++;
		}
	}

	return K;
}

}
