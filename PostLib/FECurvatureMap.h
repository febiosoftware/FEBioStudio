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
#include "FEPostModel.h"
#include <FSCore/math3d.h>
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
		void BuildNodeList(FSMesh& m);

		int Nodes() { return (int) m_node.size(); }

	public:
		std::vector<int>	m_face;		// face list
		std::vector<NODE>	m_node;		// node list
		std::vector<int>	m_lnode;	// local node list

		std::vector< std::vector<int> >	m_NLT;	// node-facet look-up table
	};

public:
	FECongruencyMap() { m_tol = 0.01; }

	// assign selections
	void SetSelection1(std::vector<int>& s) { m_surf1.m_face = s; }
	void SetSelection2(std::vector<int>& s) { m_surf2.m_face = s; }

	// apply the map
	void Apply(FEPostModel& fem);

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
	Surface			m_surf1;
	Surface			m_surf2;
	FEPostModel*	m_pfem;

	double	m_tol;	// projection tolerance
};
}
