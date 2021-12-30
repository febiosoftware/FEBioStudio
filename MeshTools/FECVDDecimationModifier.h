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
#include "FESurfaceModifier.h"

class FSFace;

//-----------------------------------------------------------------------------
//! This class implements a modifier that coarses a plygonal mesh using an
//! approximated centroidal voronoi diagram.
class FECVDDecimationModifier : public FESurfaceModifier
{
private:
	class Cluster
	{
	public:
		Cluster() : m_sgamma(vec3d(0,0,0)), m_srho(0.0), m_val(0) {}
		Cluster(const Cluster& c)
		{
			m_sgamma = c.m_sgamma;
			m_srho = c.m_srho;
			m_fid = c.m_fid;
		}
		void operator = (const Cluster& c)
		{
			m_sgamma = c.m_sgamma;
			m_srho = c.m_srho;
			m_fid = c.m_fid;
		}

		int faces() const { return (int) m_fid.size(); }

	public:
		vec3d	m_sgamma;	// sum of "gamma's"
		double	m_srho;		// sum of "rho's"
		int		m_val;		// valence (i.e. number of cells assigned to this cluster)
		std::vector<int> m_fid; //face ids of the faces in this cluster
		
	};

	struct EDGE
	{
		int face[2];	// the indices of the two faces sharing this edge
		int node[2];	// the nodes of the edge
	};

	// a node which contains a list of clusters it belongs to
	class NODE
	{
	public:
		enum {MAX_CLUSTERS=23};
	public:
		NODE() : nc(0){}

		bool AttachToCluster(int n);
	public:
		int	c[MAX_CLUSTERS];	// cluster ID's
		int	nc;					// nr of clusters
	};

public:
	//! Constructor
	FECVDDecimationModifier();

	//! Apply the decimation modifier
	FSSurfaceMesh* Apply(FSSurfaceMesh* pm);

private:
	//! Initialize data structures
	bool Initialize(FSSurfaceMesh* pm);

	//! Minimize the energy
	bool Minimize(FSSurfaceMesh* pm);

	//! Triangulate
	FSSurfaceMesh* Triangulate(FSSurfaceMesh* pm);
	FSSurfaceMesh* Triangulate2(FSSurfaceMesh* pm);	// uses hole-filling algorithm

	FSSurfaceMesh* CalculateCVD(FSSurfaceMesh* pm);

	//! swap triangles
	bool Swap(FSFace& face, int nface, int ncluster);

public:
	bool	m_bcvd;
	double	m_gradient; //gradiant for curvature

private:
	std::vector<Cluster>	m_Cluster;
	std::vector<int>		m_tag;
	std::vector<double>	m_rho;		// rho for all triangles
	std::vector<vec3d>	m_gamma;	// centroids of all triangles
	std::list<EDGE>		m_Edge;		// edge list
};
