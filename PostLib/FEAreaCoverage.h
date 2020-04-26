#pragma once
#include "FEPostMesh.h"
#include <MeshLib/Intersect.h>
#include <vector>
#include <string>
#include "FEDataField.h"
using namespace std;

namespace Post {

class FEPostModel;

class FEAreaCoverage : public FEDataField
{
	class Surface
	{
	public:
		int Faces() { return (int)m_face.size(); }

		void Create(FEPostMesh& m);

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
	FEAreaCoverage(FEPostModel* fem);

	FEDataField* Clone() const override;

	FEMeshData* CreateData(FEState* pstate) override;

	void InitSurface(int n);

	int GetSurfaceSize(int i);

	// apply the map
	void Apply();

protected:
	// assign selections
	void SetSelection1(vector<int>& s) { m_surf1.m_face = s; }
	void SetSelection2(vector<int>& s) { m_surf2.m_face = s; }

	// build node normal list
	void UpdateSurface(FEAreaCoverage::Surface& s, int nstate);

	// see if a ray intersects with a surface
	bool intersect(const vec3f& r, const vec3f& N, FEAreaCoverage::Surface& surf);
	bool faceIntersect(FEAreaCoverage::Surface& surf, const Ray& ray, int nface);

protected:
	Surface		m_surf1;
	Surface		m_surf2;
	FEPostModel*	m_fem;
};
}
