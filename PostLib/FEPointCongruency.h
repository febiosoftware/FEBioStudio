#pragma once
#include "math3d.h"
#include "Intersect.h"
#include <set>
using namespace std;

namespace Post {

class FEModel;
class FEFace;

//-----------------------------------------------------------------------------
class FEPointCongruency
{
	enum { GAUSS, MEAN, PRINC1, PRINC2, RMS, DIFF, ANGLE };

public:
	struct CONGRUENCY_DATA
	{
		double	H1, G1;			// curvature on master side
		double	H2, G2;			// curvature on slave side
		double	D;				// "Delta" value
		double	a;				// "alpha" value
		double	Kemin, Kemax;	// effective curvature
		double	Ke;				// congruency
		int		nface;			// face on which the master is projected
	};

public:
	// constructor
	FEPointCongruency();

	// measure the congruency of a point
	CONGRUENCY_DATA Congruency(FEModel* pm, int node, int nstate);

	void SetLevels(int niter) { m_nlevels = niter; }

private:
	bool Project(int nid, int& nface, vec3f& q, double rs[2], vec3f& sn);
	bool Intersect(const Ray& ray, int& nface, int nid, vec3f& q, double rs[2]);

	bool IntersectTri3 (const Ray& ray, FEFace& face, vec3f& q, double rs[2]);
	bool IntersectQuad4(const Ray& ray, FEFace& face, vec3f& q, double rs[2]);
	float nodal_curvature(int nid, vec3f& nn, int m);
	void level(int n, int l, set<int>& nl1);

	float face_curvature(FEFace& face, double rs[2], vec3f& sn, int m);

public:
	int	m_nlevels;
	int	m_bext;
	int	m_nmax;

private:
	FEModel*	m_pfem;
	int			m_nstate;
};
}
