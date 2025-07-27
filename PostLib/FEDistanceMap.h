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
#include "FEDataField.h"

namespace Post {

	class FEPostModel;

//-----------------------------------------------------------------------------
// This class maps the distance between two surfaces and adds a field variable
// to the mesh
class FEDistanceMap : public ModelDataField
{
private:
	class Surface
	{
	public:
		int Faces() { return (int) m_face.size(); }
		void BuildNodeList(FEPostMesh& m);

		int Nodes() { return (int) m_node.size(); }

	public:
		std::vector<int>	m_face;		// face list
		std::vector<int>	m_node;		// node list
		std::vector<int>	m_lnode;	// local node list
		std::vector<vec3f> m_norm;	// node normals

		std::vector< std::vector<int> >	m_NLT;	// node-facet look-up table
	};

	struct Projection
	{
		vec3f q; // position
		vec3f n; // normal
	};

public:
	FEDistanceMap(FEPostModel* fem, int flags);

	ModelDataField* Clone() const override;

	FEMeshData* CreateData(FEState* pstate) override;

	void Apply();

	void InitSurface(int n);

	int GetSurfaceSize(int i);

public:
	// assign selections
	void SetSelection1(std::vector<int>& s) { m_surf1.m_face = s; }

	void SetSelection2(std::vector<int>& s) { m_surf2.m_face = s; }

protected:
	// build node normal list
	void BuildNormalList(FEDistanceMap::Surface& s);

	// project r onto the surface
	Projection project(Surface& surf, vec3f& r, int ntime);

	// project r onto a facet
	bool ProjectToFacet(FSFace& face, vec3f& r, int ntime, Projection& P);

protected:
	Surface			m_surf1;
	Surface			m_surf2;

public:
	double	m_tol;			//!< projection tolerance
	bool	m_bsigned;		//!< signed or non-signed distance
};
}
