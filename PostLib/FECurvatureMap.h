#pragma once
#include "FEModel.h"
#include "math3d.h"
#include "FEMeshData_T.h"

namespace Post {

//-----------------------------------------------------------------------------
// This class measures the congruency between two surfaces
class FECongruencyMap
{
private:
	class Surface
	{
	public:
		struct NODE {
			int		node;
			int		ntag;
			float	val;
		};

	public:
		int Faces() { return (int) m_face.size(); }
		void BuildNodeList(FEMeshBase& m);

		int Nodes() { return (int) m_node.size(); }

	public:
		vector<int>		m_face;		// face list
		vector<NODE>	m_node;		// node list
		vector<int>		m_lnode;	// local node list

		vector<vector<int> >	m_NLT;	// node-facet look-up table
	};

public:
	FECongruencyMap() { m_tol = 0.01; }

	// assign selections
	void SetSelection1(vector<int>& s) { m_surf1.m_face = s; }
	void SetSelection2(vector<int>& s) { m_surf2.m_face = s; }

	// apply the map
	void Apply(FEModel& fem);

protected:
	// project r onto the surface
	float project(Surface& surf, vec3f& r, int ntime);

	// project r onto a facet
	bool ProjectToFacet(Surface& surf, int iface, vec3f& r, int ntime, vec3f& q, float& v);

	// project onto triangular facet
	bool ProjectToTriangle(Surface& surf, int iface, vec3f& r, int ntime, vec3f& q, float& val);

	// project onto quad facet
	bool ProjectToQuad(Surface& surf, int iface, vec3f& r, int ntime, vec3f& q, float& val);

	// evaluate the surface
	void EvalSurface(Surface& surf, FEState* ps);

protected:
	Surface		m_surf1;
	Surface		m_surf2;
	FEModel*	m_pfem;

	double	m_tol;	// projection tolerance
};
}
