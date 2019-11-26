#include "stdafx.h"
#include "TetOverlap.h"
#include <MeshLib/FEMesh.h>

struct TET
{
	int		n[4];
	vec3d	r[4];
};

bool tet_overlap(TET& a, TET& b);
bool box_test(BOX& a, TET& b);

TetOverlap::TetOverlap()
{

}

bool TetOverlap::Apply(FEMesh* mesh, std::vector<pair<int, int> >& tetList)
{
	if (mesh == nullptr) return false;
	if (mesh->IsType(FE_TET4) == false) return false;

	int NE = mesh->Elements();
	vector<TET> tet(NE);
	TET t;
	for (int i = 0; i < NE; ++i)
	{
		FEElement& e = mesh->Element(i);

		double V = mesh->TetVolume(e);

		if (V >= 0)
		{
			t.n[0] = e.m_node[0]; t.r[0] = mesh->Node(e.m_node[0]).r;
			t.n[1] = e.m_node[1]; t.r[1] = mesh->Node(e.m_node[1]).r;
			t.n[2] = e.m_node[2]; t.r[2] = mesh->Node(e.m_node[2]).r;
			t.n[3] = e.m_node[3]; t.r[3] = mesh->Node(e.m_node[3]).r;
		}
		else
		{
			t.n[2] = e.m_node[0]; t.r[2] = mesh->Node(e.m_node[0]).r;
			t.n[1] = e.m_node[1]; t.r[1] = mesh->Node(e.m_node[1]).r;
			t.n[0] = e.m_node[2]; t.r[0] = mesh->Node(e.m_node[2]).r;
			t.n[3] = e.m_node[3]; t.r[3] = mesh->Node(e.m_node[3]).r;
		}

		tet[i] = t;
	}

	// the list that will store the overlapping pairs
	tetList.clear();
	tetList.reserve(NE / 2);

	for (int i = 0; i < NE; ++i)
	{
		TET& a = tet[i];

		BOX box;
		for (int k = 0; k < 4; ++k) box += a.r[k];
		double R = box.GetMaxExtent();
		box.Inflate(R*0.001);

		for (int j = i+1; j < NE; ++j)
		{
			TET& b = tet[j];
			if (box_test(box, b) == false)
			{
				if (tet_overlap(a, b))
				{
					pair<int, int> tetPair(i, j);
					tetList.push_back(tetPair);
				}
			}
		}
	}

	return true;
}

bool plane_test(const vec3d& q, const vec3d& n, TET& b)
{
	const double eps = -1e-15;
	if (n*(b.r[0] - q) < eps) return false;
	if (n*(b.r[1] - q) < eps) return false;
	if (n*(b.r[2] - q) < eps) return false;
	if (n*(b.r[3] - q) < eps) return false;
	return true;
}

bool box_test(BOX& a, TET& b)
{
	if (plane_test(a.r1(), vec3d(1, 0, 0), b)) return true;
	if (plane_test(a.r1(), vec3d(0, 1, 0), b)) return true;
	if (plane_test(a.r1(), vec3d(0, 0, 1), b)) return true;

	if (plane_test(a.r0(), vec3d(-1, 0, 0), b)) return true;
	if (plane_test(a.r0(), vec3d(0, -1, 0), b)) return true;
	if (plane_test(a.r0(), vec3d(0, 0, -1), b)) return true;

	return false;
}

// We test overlap by finding a plane of "a" that separates all the nodes of "b"
// We return false if we find such a plane, otherwise we return true
bool separates_by_plane(TET& a, TET& b)
{
	const int T[4][3] = {
		{ 0, 1, 3 },
		{ 1, 2, 3 },
		{ 2, 0, 3 },
		{ 0, 2, 1 }
	};

	const double eps = -1e-15;

	// loop over all the planes of a
	for (int i = 0; i < 4; ++i)
	{
		vec3d& p0 = a.r[T[i][0]];
		vec3d& e1 = a.r[T[i][1]] - p0;
		vec3d& e2 = a.r[T[i][2]] - p0;

		// normal
		vec3d n = e1 ^ e2; n.Normalize();

		// see if all the nodes of b are on or in front of this plane
		bool bok = true;
		for (int j = 0; j < 4; ++j)
		{
			int m = b.n[j];
			if ((m != a.n[T[i][0]]) && (m != a.n[T[i][1]]) && (m != a.n[T[i][2]]))
			{
				vec3d r = b.r[j];
				double d = n*(r - p0);

				if (d < eps)
				{
					bok = false;
					break;
				}
			}
		}

		if (bok == true) return true;
	}

	return false;
}

void project_on_line(vec3d& n, TET& t, double& fmin, double& fmax)
{
	fmin = fmax = n*t.r[0];
	for (int i = 1; i < 4; ++i)
	{
		double fi = n*t.r[i];
		if (fi < fmin) fmin = fi;
		if (fi > fmax) fmax = fi;
	}
}

bool separates_by_edge(TET& a, TET& b)
{
	int E[6][2] = { {0,1}, {1,2},{2,0},{0,3},{1,3},{2,3} };

	const double eps = 1e-12;

	// we do line projections for each edge pair
	for (int i = 0; i < 6; ++i)
	{
		vec3d ea = a.r[E[i][1]] - a.r[E[i][0]]; ea.Normalize();
		for (int j = 0; j < 6; ++j)
		{
			vec3d eb = b.r[E[j][1]] - b.r[E[j][0]]; eb.Normalize();

			vec3d n = ea ^ eb;
			if (n.Length() > eps)
			{
				n.Normalize();

				double min_a, max_a;
				project_on_line(n, a, min_a, max_a);

				double min_b, max_b;
				project_on_line(n, b, min_b, max_b);

				if ((max_a <= min_b+eps) || (max_b <= min_a+eps)) return true;
			}
		}
	}

	return false;
}

bool tet_overlap(TET& a, TET& b)
{
	// test for face separation
	if (separates_by_plane(a, b) || separates_by_plane(b, a)) return false;

	// check for edge separation
	if (separates_by_edge(a, b)) return false;

	return true;
}
