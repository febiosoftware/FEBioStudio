#pragma once
#include "FEModel.h"
#include <vector>

namespace Post {

class FEModel;

class FEStrainMap
{
	class Surface
	{
	public:
		int Faces() { return (int)m_face.size(); }
		void BuildNodeList(FEMeshBase& m);

		int Nodes() { return (int)m_node.size(); }

	public:
		std::vector<int>	m_face;		// face list
		std::vector<int>	m_node;		// node list
		std::vector<int>	m_lnode;	// local node list
		std::vector<vec3f>	m_norm;		// node normals
		std::vector<vec3f>	m_pos;		// node positions

		std::vector<vector<int> >	m_NLT;	// node-facet look-up table
	};

public:
	// constructor
	FEStrainMap();

	// assign selections
	void SetFrontSurface1(std::vector<int>& s);
	void SetBackSurface1(std::vector<int>& s);
	void SetFrontSurface2(std::vector<int>& s);
	void SetBackSurface2(std::vector<int>& s);

	// apply the map
	void Apply(FEModel& fem);

protected:
	// update the surface normal positions
	void UpdateNodePositions(FEStrainMap::Surface& s, int ntime);

	// build node normal list
	void BuildNormalList(FEStrainMap::Surface& s);

	// project r onto the surface
	bool project(Surface& surf, vec3f& r, vec3f& t, vec3f& q);

	// project r onto a facet
	bool ProjectToFacet(vec3f* y, int nf, vec3f& r, vec3f& t, vec3f& q);

protected:
	Surface		m_front1;
	Surface		m_back1;
	Surface		m_front2;
	Surface		m_back2;

	FEModel*	m_fem;

	double	m_tol;
};
}
