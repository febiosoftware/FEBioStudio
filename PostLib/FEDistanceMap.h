#pragma once
#include "FEModel.h"

namespace Post {

//-----------------------------------------------------------------------------
// This class maps the distance between two surfaces and adds a field variable
// to the mesh
class FEDistanceMap
{
private:
	class Surface
	{
	public:
		int Faces() { return (int) m_face.size(); }
		void BuildNodeList(FEMeshBase& m);

		int Nodes() { return (int) m_node.size(); }

	public:
		vector<int>	m_face;		// face list
		vector<int>	m_node;		// node list
		vector<int>	m_lnode;	// local node list
		vector<vec3f> m_norm;	// node normals

		vector<vector<int> >	m_NLT;	// node-facet look-up table
	};

public:
	FEDistanceMap() { m_tol = 0.01; m_bsigned = false; }

	// assign selections
	void SetSelection1(vector<int>& s) { m_surf1.m_face = s; }
	void SetSelection2(vector<int>& s) { m_surf2.m_face = s; }

	// apply the map
	void Apply(FEModel& fem);

protected:
	// build node normal list
	void BuildNormalList(FEDistanceMap::Surface& s);

	// project r onto the surface
	vec3f project(Surface& surf, vec3f& r, int ntime);

	// project r onto a facet
	bool ProjectToFacet(FEFace& face, vec3f& r, int ntime, vec3f& q);

protected:
	Surface		m_surf1;
	Surface		m_surf2;
	FEModel*	m_pfem;

public:
	double	m_tol;			//!< projection tolerance
	bool	m_bsigned;		//!< signed or non-signed distance
};
}
