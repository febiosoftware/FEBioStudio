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

#pragma once
#include <MeshLib/FEMesh.h>
#include <MeshLib/Intersect.h>
#include <vector>
#include <string>
#include "FEDataField.h"

namespace Post {

class FEPostModel;

class FEAreaCoverage : public ModelDataField
{
	class Surface
	{
	public:
		int Faces() { return (int)m_face.size(); }

		void Create(FSMesh& m);

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
	FEAreaCoverage(FEPostModel* fem, int flags);

	ModelDataField* Clone() const override;

	FEMeshData* CreateData(FEState* pstate) override;

	void AllowBackIntersection(bool b);
	bool AllowBackIntersection() const;

	void SetAngleThreshold(double w);
	double GetAngleThreshold() const;

	void SetBackSearchRadius(double R);
	double GetBackSearchRadius() const;

	void InitSurface(int n);

	int GetSurfaceSize(int i);

	// apply the map
	void Apply();
	void Apply(FEPostModel* fem);

	// assign selections
	void SetSelection1(vector<int>& s) { m_surf1.m_face = s; }
	void SetSelection2(vector<int>& s) { m_surf2.m_face = s; }

protected:
	// build node normal list
	void UpdateSurface(FEAreaCoverage::Surface& s, int nstate);

	// see if a ray intersects with a surface
	bool intersect(const vec3f& r, const vec3f& N, FEAreaCoverage::Surface& surf, Intersection& q);
	bool faceIntersect(FEAreaCoverage::Surface& surf, const Ray& ray, int nface, Intersection& q);

	// project a surface onto another surface
	void projectSurface(FEAreaCoverage::Surface& surf1, FEAreaCoverage::Surface& surf2, vector<float>& a);

protected:
	Surface		m_surf1;
	Surface		m_surf2;

	bool		m_ballowBackIntersections;	// include back intersections
	double		m_angleThreshold;			// angular threshold (between 0 and 1)
	double		m_backSearchRadius;			// search radius for back intersections (set to 0 to ignore)
};
}
