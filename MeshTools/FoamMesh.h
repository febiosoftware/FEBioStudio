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
#include <FSCore/math3d.h>
#include <vector>

class FSModel;
class GObject;
class FSMesh;

// Class describing the foam generator.
class FoamGen
{
private:
	struct NODE
	{
		vec3d	r;	// nodal position
		double	v;	// nodal value
		vec3d	g;	// gradient
	};

	struct CELL
	{
		int		n[8];
		int		ntag;
	};

	struct CELL2D
	{
		int		n[4];
		int		ntag;
	};

	struct EDGE
	{
		int		n[2];
		double	w;
		vec3d	r;
	};

	struct FACE
	{
		int		n[3];
		int		nid;
	};

	struct N2E
	{
		int		e[6];
		int		ne;
	};

public:
	FoamGen();

	// Create the foam object
	FSMesh* Create();

public:
	int		m_nx, m_ny, m_nz;
	double	m_w, m_h, m_d;
	int		m_nseed;
	int		m_nsmooth;
	double	m_ref;

protected:
	double	m_eps;

protected:
	NODE& Node(int i, int j, int k) { return m_Node[k*(m_nx+1)*(m_ny+1)+j*(m_nx+1)+i]; }

	void CreateGrid();
	void EvalGrid();
	void CalcGradient();	// not used (for now; and not finished)
	void SmoothGrid();
	void SmoothMesh(FSMesh* pm, int niter, double w);
	void DistortGrid();
	FSMesh* CreateMesh();
	FSMesh* WeldMesh(FSMesh* pm);

	int FindEdge(int n1, int n2);

	void SelectFace(int i, FSMesh* pm);

protected:
	std::vector<NODE>	m_Node;
	std::vector<CELL>	m_Cell;
	std::vector<CELL2D>	m_Cell2D[6];
	std::vector<NODE>	m_Seed;
	std::vector<EDGE>	m_Edge;
	std::vector<FACE>	m_Face;
	std::vector<N2E>	m_NET;
	
	int		m_nface[8];
};
