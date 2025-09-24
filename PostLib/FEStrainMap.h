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
#include <vector>
#include <FSCore/math3d.h>

class FSMesh;

namespace Post {

class FEPostModel;

class FEStrainMap
{
	class Surface
	{
	public:
		int Faces() { return (int)m_face.size(); }
		void BuildNodeList(FSMesh& m);

		int Nodes() { return (int)m_node.size(); }

	public:
		std::vector<int>	m_face;		// face list
		std::vector<int>	m_node;		// node list
		std::vector<int>	m_lnode;	// local node list
		std::vector<vec3f>	m_norm;		// node normals
		std::vector<vec3f>	m_pos;		// node positions

		std::vector<std::vector<int> >	m_NLT;	// node-facet look-up table
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
	void Apply(FEPostModel& fem);

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

	FEPostModel*	m_fem;

	double	m_tol;
};
}
