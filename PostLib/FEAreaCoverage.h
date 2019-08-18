#pragma once
#include "FEMesh.h"
#include "Intersect.h"
#include <vector>
#include <string>
using namespace std;

namespace Post {

class FEModel;

class FEAreaCoverage
{
	class Surface
	{
	public:
		int Faces() { return (int)m_face.size(); }

		void Create(FEMeshBase& m);

		int Nodes() { return (int)m_node.size(); }

	public:
		vector<int>		m_face;		// face list
		vector<int>		m_node;		// node list
		vector<vec3f>	m_pos;		// node positions
		vector<int>		m_lnode;	// local node list
		vector<vec3f>	m_norm;		// node normals
		vector<vec3f>	m_fnorm;	// face normals

		vector<vector<int> >	m_NLT;	// node-facet look-up table
	};

public:
	FEAreaCoverage();

	// set the name of the data field that will be created
	void SetDataFieldName(const std::string& name) { m_name = name; }

	// assign selections
	void SetSelection1(vector<int>& s) { m_surf1.m_face = s; }
	void SetSelection2(vector<int>& s) { m_surf2.m_face = s; }

	// apply the map
	void Apply(FEModel& fem);

protected:
	// build node normal list
	void UpdateSurface(FEAreaCoverage::Surface& s, int nstate);

	// see if a ray intersects with a surface
	bool intersect(const vec3f& r, const vec3f& N, FEAreaCoverage::Surface& surf);
	bool faceIntersect(FEAreaCoverage::Surface& surf, const Ray& ray, int nface);

protected:
	Surface		m_surf1;
	Surface		m_surf2;
	FEModel*	m_fem;
	string  	m_name;
};
}
