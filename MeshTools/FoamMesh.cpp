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

#include "stdafx.h"
#include "FoamMesh.h"
#include <GeomLib/GMeshObject.h>
#include "FEWeldModifier.h"
#include <MeshLib/FSNodeNodeList.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// declared in lut.cpp
extern int LUT[256][15];
extern int ET_HEX[12][2];
extern int F2D[16][16];
extern int ET2D[4][2];

//-----------------------------------------------------------------------------
// return a random number between zero and one
double frand() { return (double) rand() / (double) RAND_MAX; }

//-----------------------------------------------------------------------------
FoamGen::FoamGen()
{
	m_nx = 10;
	m_ny = 10;
	m_nz = 10;

	m_w = m_h = m_d = 1.0;

	m_nseed = 5;
	m_nsmooth = 5;

	m_ref = 0.5;
	m_eps = 0.5;
}

//-----------------------------------------------------------------------------
FSMesh* FoamGen::Create()
{
	// data check
	if (m_nx < 1) return 0;
	if (m_ny < 1) return 0;
	if (m_nz < 1) return 0;
	if (m_nseed <= 0) return 0;

	// create the grid
	CreateGrid();
	EvalGrid();
/*	for (int i=0; i<5; ++i)
	{
		DistortGrid();
		EvalGrid();
	}
*/
	// Extract the mesh
	FSMesh* pm = CreateMesh();
	if (pm == 0) return 0;

	// apply a weld modifier
	FSMesh* pmw = WeldMesh(pm);

	// remove duplicate elements
	int F0 = pmw->Elements();
	pm = pmw;
	FERemoveDuplicateElements dup;
	pmw = dup.Apply(pm);
	int F1 = pmw->Elements();
	for (int i=1; i<8; ++i) m_nface[i] -= (F0 - F1);

	// smooth the mesh
	SelectFace(0, pmw);
	SmoothMesh(pmw, 1, 0.5);

	return pmw;
}

//-----------------------------------------------------------------------------
void FoamGen::SelectFace(int i, FSMesh* pm)
{
	int j;

	if (i==0)
	{
		// select all nodes
		int N = pm->Nodes();
		for (j=0; j<N; ++j) pm->Node(j).Select();

		// unselect all faces, except the first one
		for (int n=m_nface[1]; n<m_nface[7]; ++n)
		{
			FSElement& f = pm->Element(n);
			pm->Node(f.m_node[0]).Unselect();
			pm->Node(f.m_node[1]).Unselect();
			pm->Node(f.m_node[2]).Unselect();
		}
	}
	else
	{
		// unselect all nodes
		int N = pm->Nodes();
		for (j=0; j<N; ++j) pm->Node(j).Unselect();

		// select the face
		for (int n=m_nface[i]; n<m_nface[i+1]; ++n)
		{
			FSElement& f = pm->Element(n);
			pm->Node(f.m_node[0]).Select();
			pm->Node(f.m_node[1]).Select();
			pm->Node(f.m_node[2]).Select();
		}
	}
}

//-----------------------------------------------------------------------------
FSMesh* FoamGen::WeldMesh(FSMesh* pm)
{
	// find a good welding tolerance
	double dx = m_w/m_nx;
	double dy = m_h/m_ny;
	double dz = m_d/m_nz;
	double R = dx;
	if (dy<R) R = dy;
	if (dz<R) R = dz;

	// get a weld modifier
	FEWeldNodes mod;
	mod.SetThreshold(R/3.0);

	// loop over all seven faces
	for (int i=0; i<7; ++i)
	{
		// select the face
		SelectFace(i, pm);

		// remember the nr of faces
		int F0 = pm->Elements();

		// apply weld modifier
		FSMesh* pmn = mod.Apply(pm);

		// adjust face counts
		int F1 = pmn->Elements();
		for (int j=i+1; j<8; ++j) m_nface[j] -= (F0 - F1);

		// get ready for next iteration
		delete pm; pm = pmn;
	}

	return pm;
}

//-----------------------------------------------------------------------------
void FoamGen::SmoothMesh(FSMesh* pm, int niter, double w)
{
	// set up the node-element table
	FSNodeNodeList NNL(pm);

	// smooth node positions
	for (int n=0; n<niter; ++n)
	{
		int N = pm->Nodes();
		for (int i=0; i<N; ++i)
		{
			FSNode& ni = pm->Node(i);
			if (ni.IsSelected())
			{
				int nn = NNL.Valence(i);
				if (nn > 0)
				{
					vec3d v;
					for (int j=0; j<nn; ++j)
					{
						FSNode& nj = pm->Node(NNL.Node(i, j));
						v += nj.r;
					}
					v /= (double) nn;

					ni.r = ni.r*w + v*(1.0-w);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FoamGen::CreateGrid()
{
	int i, j, k;

	// create the grid
	int nx = m_nx+1;
	int ny = m_ny+1;
	int nz = m_nz+1;
	int nn = nx*ny*nz;
	m_Node.resize(nn);

	for (k=0; k<nz; ++k)
	{
		for (j=0; j<ny; ++j)
		{
			for (i=0; i<nx; ++i)
			{
				NODE& n = Node(i, j, k);
				n.r.x = i*m_w/(nx-1);
				n.r.y = j*m_h/(ny-1);
				n.r.z = k*m_d/(nz-1);
				n.v = 0.0;
			}
		}
	}

	// create the seeds
	m_Seed.resize(m_nseed);
	for (i=0; i<m_nseed; ++i)
	{
		NODE& s = m_Seed[i];
		s.r.x = -m_w*m_ref + (frand() + m_ref)*m_w;
		s.r.y = -m_h*m_ref + (frand() + m_ref)*m_h;
		s.r.z = -m_d*m_ref + (frand() + m_ref)*m_d;
		s.v = 1.0;
	}

	// create the cells
	int ne = m_nx*m_ny*m_nz;
	m_Cell.resize(ne);
	ne = 0;
	for (k=0; k<m_nz; ++k)
		for (j=0; j<m_ny; ++j)
			for (i=0; i<m_nx; ++i)
			{
				CELL& e = m_Cell[ne++];
				e.ntag = 0;
				e.n[0] =  k   *(nx*ny)+ j   *nx+i;
				e.n[1] =  k   *(nx*ny)+ j   *nx+i+1;
				e.n[2] =  k   *(nx*ny)+(j+1)*nx+i+1;
				e.n[3] =  k   *(nx*ny)+(j+1)*nx+i;
				e.n[4] = (k+1)*(nx*ny)+ j   *nx+i;
				e.n[5] = (k+1)*(nx*ny)+ j   *nx+i+1;
				e.n[6] = (k+1)*(nx*ny)+(j+1)*nx+i+1;
				e.n[7] = (k+1)*(nx*ny)+(j+1)*nx+i;
			}

	// create the surface cells
	ne = 0;
	m_Cell2D[0].resize(m_nx*m_ny);
	for (j=0; j<m_ny; ++j)
		for (i=0; i<m_nx; ++i)
		{
			CELL2D& e = m_Cell2D[0][ne++];
			e.ntag = 0;
			e.n[3] = j*nx + i;
			e.n[2] = j*nx + i+1;
			e.n[1] = (j+1)*nx + i+1;
			e.n[0] = (j+1)*nx + i;
		}

	ne = 0;
	m_Cell2D[1].resize(m_nx*m_nz);
	for (k=0; k<m_nz; ++k)
		for (i=0; i<m_nx; ++i)
		{
			CELL2D& e = m_Cell2D[1][ne++];
			e.ntag = 0;
			e.n[0] = k*nx*ny + i;
			e.n[1] = k*nx*ny + i+1;
			e.n[2] = (k+1)*nx*ny + i+1;
			e.n[3] = (k+1)*nx*ny + i;
		}

	ne = 0;
	m_Cell2D[2].resize(m_ny*m_nz);
	for (k=0; k<m_nz; ++k)
		for (j=0; j<m_ny; ++j)
		{
			CELL2D& e = m_Cell2D[2][ne++];
			e.ntag = 0;
			e.n[0] = k*nx*ny + j*nx + nx-1;
			e.n[1] = k*nx*ny + (j+1)*nx + nx-1;
			e.n[2] = (k+1)*nx*ny + (j+1)*nx + nx-1;
			e.n[3] = (k+1)*nx*ny + j*nx + nx-1;
		}

	ne = 0;
	m_Cell2D[3].resize(m_nx*m_nz);
	for (k=0; k<m_nz; ++k)
		for (i=0; i<m_nx; ++i)
		{
			CELL2D& e = m_Cell2D[3][ne++];
			e.ntag = 0;
			e.n[3] = k*nx*ny + (ny-1)*nx + i;
			e.n[2] = k*nx*ny + (ny-1)*nx + i+1;
			e.n[1] = (k+1)*nx*ny + (ny-1)*nx + i+1;
			e.n[0] = (k+1)*nx*ny + (ny-1)*nx + i;
		}

	ne = 0;
	m_Cell2D[4].resize(m_ny*m_nz);
	for (k=0; k<m_nz; ++k)
		for (j=0; j<m_ny; ++j)
		{
			CELL2D& e = m_Cell2D[4][ne++];
			e.ntag = 0;
			e.n[3] = k*nx*ny + j*nx;
			e.n[2] = k*nx*ny + (j+1)*nx;
			e.n[1] = (k+1)*nx*ny + (j+1)*nx;
			e.n[0] = (k+1)*nx*ny + j*nx;
		}

	ne = 0;
	m_Cell2D[5].resize(m_nx*m_ny);
	for (j=0; j<m_ny; ++j)
		for (i=0; i<m_nx; ++i)
		{
			CELL2D& e = m_Cell2D[5][ne++];
			e.ntag = 0;
			e.n[0] = (nz-1)*nx*ny + j*nx + i;
			e.n[1] = (nz-1)*nx*ny + j*nx + i+1;
			e.n[2] = (nz-1)*nx*ny + (j+1)*nx + i+1;
			e.n[3] = (nz-1)*nx*ny + (j+1)*nx + i;
		}
}

//-----------------------------------------------------------------------------
void FoamGen::EvalGrid()
{
	int i, j, k;

	// evaluate the grid
	int nn = m_Node.size();
	for (i=0; i<nn; ++i)
	{
		NODE& node = m_Node[i];
		node.v = 0;

		// find the closest seed

		int imin = 0;
		double dmin = (node.r - m_Seed[0].r).Length(), d;
		for (j=1; j<m_nseed; ++j)
		{
			d = (node.r - m_Seed[j].r).Length();
			if (d < dmin)
			{
				imin = j;
				dmin = d;
			}
		}

		// set the value
		node.v = dmin*m_Seed[imin].v;
	}

	// smooth the grid
	for (i=0; i<m_nsmooth; ++i) SmoothGrid();

	// normalize between 0 and 1
	double vmin = 1e35, vmax = -1e35;
	for (i=0; i<nn; ++i)
	{
		if (m_Node[i].v < vmin) vmin = m_Node[i].v;
		if (m_Node[i].v > vmax) vmax = m_Node[i].v;
	}
	if (vmin == vmax) vmax++;
	for (i=0; i<nn; ++i)
	{
		double v = m_Node[i].v;
		m_Node[i].v = (v - vmin)/(vmax - vmin);
	}

	// evaluate the cells
	m_eps = m_ref;
	int ne = m_Cell.size();
	for (i=0; i<ne; ++i)
	{
		CELL& e = m_Cell[i];
		e.ntag = 0;
		for (j=0; j<8; ++j)
		{
			if (m_Node[e.n[j]].v > m_eps) e.ntag |= (1 << j);
		}
		e.ntag = 255-e.ntag;
	}

	// evaluate surface cells
	for (k=0; k<6; ++k)
	{
		ne = m_Cell2D[k].size();
		for (i=0; i<ne; ++i)
		{
			CELL2D& e = m_Cell2D[k][i];
			e.ntag = 0;
			for (j=0; j<4; ++j)
			{
				if (m_Node[e.n[j]].v > m_eps) e.ntag |= (1 << j);
			}
			e.ntag = 15 - e.ntag;
		}
	}
}

//-----------------------------------------------------------------------------
void FoamGen::SmoothGrid()
{
	int i, j, k;
	int nx = m_nx+1;
	int ny = m_ny+1;
	int nz = m_nz+1;
	for (k=1; k<nz-1; ++k)
	{
		for (j=1; j<ny-1; ++j)
		{
			for (i=1; i<nx-1; ++i)
			{
				NODE& n0 = Node(i, j, k);

				double v1 = Node(i-1, j, k).v;
				double v2 = Node(i+1, j, k).v;
				double v3 = Node(i, j-1, k).v;
				double v4 = Node(i, j+1, k).v;
				double v5 = Node(i, j, k-1).v;
				double v6 = Node(i, j, k+1).v;

				n0.v = (v1+v2+v3+v4+v5+v6)/6.0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FoamGen::DistortGrid()
{
	int i, j, k, l;
	double v[7], w[6];
	double f = 0.7;
	double dx = f*m_w/m_nx;
	double dy = f*m_h/m_ny;
	double dz = f*m_d/m_nz;

	double lx, ly, lz;
	for (k=1; k<m_nx; ++k)
	{
		for (j=1; j<m_ny; ++j)
		{
			for (i=1; i<m_nx; ++i)
			{
				NODE& node = Node(i,j,k);

				v[0] = Node(i, j, k).v;
				v[1] = Node(i-1, j, k).v;
				v[2] = Node(i+1, j, k).v;
				v[3] = Node(i, j-1, k).v;
				v[4] = Node(i, j+1, k).v;
				v[5] = Node(i, j, k-1).v;
				v[6] = Node(i, j, k+1).v;

				for (l=0; l<6; ++l) 
				{
					if (v[l] == v[0]) w[l] = -1.0; else w[l] = (v[0] - m_eps)/(v[0] - v[l]);
				}

				lx = ly = lz = 0;
				if ((w[0]>=0.0) && (w[0] < 0.5)) lx += (0.5 - w[0])*dx;
				if ((w[1]>=0.0) && (w[1] < 0.5)) lx -= (0.5 - w[1])*dx;

				if ((w[2]>=0.0) && (w[2] < 0.5)) ly += (0.5 - w[2])*dy;
				if ((w[3]>=0.0) && (w[3] < 0.5)) ly -= (0.5 - w[3])*dy;

				if ((w[4]>=0.0) && (w[4] < 0.5)) lz += (0.5 - w[4])*dz;
				if ((w[5]>=0.0) && (w[5] < 0.5)) lz -= (0.5 - w[5])*dz;

				node.r -= vec3d(lx, ly, lz);
			}
		}
	}
}


//-----------------------------------------------------------------------------
void FoamGen::CalcGradient()
{
	int i, j;
	int NN = m_Node.size();
	for (i=0; i<NN; ++i) m_Node[i].g = vec3d(0,0,0);

	double g[8][3] = {
		{-1, -1, -1},
		{ 1, -1, -1},
		{ 1,  1, -1},
		{-1,  1, -1},
		{-1, -1,  1},
		{ 1, -1,  1},
		{ 1,  1,  1},
		{-1,  1,  1}
	};

	double Hr[8][8], Hs[8][8], Ht[8][8];
	for (i=0; i<8; ++i)
	{
		Hr[i][0] = -0.125*(1-g[i][1])*(1-g[i][2]); Hs[i][0] = -0.125*(1-g[i][0])*(1-g[i][2]); Ht[i][0] = -0.125*(1-g[i][0])*(1-g[i][1]);
		Hr[i][1] =  0.125*(1-g[i][1])*(1-g[i][2]); Hs[i][1] = -0.125*(1+g[i][0])*(1-g[i][2]); Ht[i][1] = -0.125*(1+g[i][0])*(1-g[i][1]);
		Hr[i][2] =  0.125*(1+g[i][1])*(1-g[i][2]); Hs[i][2] =  0.125*(1+g[i][0])*(1-g[i][2]); Ht[i][2] = -0.125*(1+g[i][0])*(1+g[i][1]);
		Hr[i][3] = -0.125*(1+g[i][1])*(1-g[i][2]); Hs[i][3] =  0.125*(1-g[i][0])*(1-g[i][2]); Ht[i][3] = -0.125*(1-g[i][0])*(1+g[i][1]);
		Hr[i][4] = -0.125*(1-g[i][1])*(1+g[i][2]); Hs[i][4] = -0.125*(1-g[i][0])*(1+g[i][2]); Ht[i][4] =  0.125*(1-g[i][0])*(1-g[i][1]);
		Hr[i][5] =  0.125*(1-g[i][1])*(1+g[i][2]); Hs[i][5] = -0.125*(1+g[i][0])*(1+g[i][2]); Ht[i][5] =  0.125*(1+g[i][0])*(1-g[i][1]);
		Hr[i][6] =  0.125*(1+g[i][1])*(1+g[i][2]); Hs[i][6] =  0.125*(1+g[i][0])*(1+g[i][2]); Ht[i][6] =  0.125*(1+g[i][0])*(1+g[i][1]);
		Hr[i][7] = -0.125*(1+g[i][1])*(1+g[i][2]); Hs[i][7] =  0.125*(1-g[i][0])*(1+g[i][2]); Ht[i][7] =  0.125*(1-g[i][0])*(1+g[i][1]);
	}

	int NE = m_Cell.size();
	for (i=0; i<NE; ++i)
	{
		CELL& e = m_Cell[i];

		double v[8];
		for (j=0; j<8; ++j) v[j] = m_Node[e.n[j]].v;

		for (j=0; j<8; ++j)
		{

		}
	}
}

//-----------------------------------------------------------------------------
FSMesh* FoamGen::CreateMesh()
{
	int i;
	int NE = m_Cell.size();

	// allocate some storage for edges and faces
	m_Edge.clear();
	m_Face.clear();
	m_Edge.reserve(NE*12);

	// setup node-edge table
	m_NET.resize(m_Node.size());
	for (i=0; i<(int) m_NET.size(); ++i) m_NET[i].ne = 0;

	// First we need to figure out how many nodes and faces we will be creating
	m_nface[0] = 0;
	m_nface[1] = 0;
	for (i=0; i<NE; ++i)
	{
		CELL& e = m_Cell[i];

		// loop over faces
		int* pf = LUT[e.ntag];
		for (int l=0; l<5; l++)
		{
			if (*pf == -1) break;

			FACE f;

			for (int k=0; k<3; k++)
			{
				int n1 = e.n[ET_HEX[pf[k]][0]];
				int n2 = e.n[ET_HEX[pf[k]][1]];
				if (n1 > n2) { n1 ^= n2; n2 ^= n1; n1 ^= n2; }

				int n = FindEdge(n1, n2);
				if (n == -1) 
				{
					vec3d r1 = m_Node[n1].r;
					vec3d r2 = m_Node[n2].r;
					double v1 = m_Node[n1].v;
					double v2 = m_Node[n2].v;
					if (v1 == v2) v2++;
					EDGE edge;
					edge.n[0] = n1;
					edge.n[1] = n2;
					edge.w = (m_eps - v1) / (v2 - v1);
					edge.r = r1*(1.0 - edge.w) + r2*edge.w;
					n = m_Edge.size();

					m_Edge.push_back(edge);
					m_NET[n1].e[ m_NET[n1].ne++ ] = n;
				}

				f.n[k] = n;
			}

			f.nid = 0;
			m_Face.push_back(f);
			m_nface[1] += 1;

			pf+=3;
		}
	}

	// build the surfaces
	for (int j=0; j<6; ++j)
	{
		m_nface[j+2] = m_nface[j+1];
		int ne = m_Cell2D[j].size();
		for (i=0; i<ne; ++i)
		{
			CELL2D& e = m_Cell2D[j][i];
			const int * pf = F2D[15 - e.ntag];
			for (int l=0; l<4; ++l)
			{
				if (*pf == -1) break;

				FACE f;
				for (int k=0; k<3; ++k)
				{
					int n1, n2;
					if (pf[k] < 4) 
					{
						n1 = n2 = e.n[pf[k]];
					}
					else
					{
						int m = pf[k]-4;
						n1 = e.n[ET2D[m][0]];
						n2 = e.n[ET2D[m][1]];
						if (n1 > n2) { n1 ^= n2; n2 ^= n1; n1 ^= n2; }
					}

					int n = FindEdge(n1, n2);
					if (n==-1)
					{
						vec3d r1 = m_Node[n1].r;
						vec3d r2 = m_Node[n2].r;
						EDGE edge;
						edge.n[0] = n1;
						edge.n[1] = n2;
						if (n1 != n2)
						{
							double v1 = m_Node[n1].v;
							double v2 = m_Node[n2].v;
							if (v1 == v2) v2++;
							edge.w = (m_eps - v1) / (v2 - v1);
							edge.r = r1*(1.0 - edge.w) + r2*edge.w;
						}
						else 
						{
							edge.w = 1;
							edge.r = r1;
						}

						n = m_Edge.size();

						m_Edge.push_back(edge);
						assert(m_NET[n1].ne < 6);
						m_NET[n1].e[ m_NET[n1].ne++ ] = n;
					}
					f.n[k] = n;
				}

				f.nid = j+1;
				m_Face.push_back(f);
				m_nface[j+2] += 1;
				pf += 3;
			}
		}
	}

	// Build the mesh
	int nn = m_Edge.size();
	int nf = m_Face.size();
	if ((nn==0) || (nf==0)) return 0;

	FSMesh* pm = new FSMesh;
	pm->Create(nn, nf, nf);

	for (i=0; i<nn; ++i)
	{
		EDGE& e = m_Edge[i];
		FSNode& node = pm->Node(i);
		node.r = e.r;
	}

	for (i=0; i<nf; ++i)
	{
		FSElement& e = pm->Element(i);
		FACE& f = m_Face[i];
		e.SetType(FE_TRI3);
		e.m_node[0] = f.n[0];
		e.m_node[1] = f.n[1];
		e.m_node[2] = f.n[2];
		e.m_gid = 0;

		FSFace& s = pm->Face(i);
		s.SetType(FE_FACE_TRI3);
		s.n[0] = f.n[0];
		s.n[1] = f.n[1];
		s.n[2] = f.n[2];
		s.n[3] = f.n[2];
		s.m_gid = f.nid;
		s.m_sid = f.nid;
	}

	pm->BuildMesh();
	
	return pm;
}

//-----------------------------------------------------------------------------
int FoamGen::FindEdge(int n1, int n2)
{
/*	if (m_Edge.empty()) return -1;
	int N = m_Edge.size();
	for (int i=0; i<N; ++i)
	{
		EDGE& e = m_Edge[i];
		if (((e.n[0]==n1)&&(e.n[1]==n2))||
			((e.n[1]==n1)&&(e.n[0]==n2))) return i;
	}
	return -1;
*/
	int ne = m_NET[n1].ne;
	assert(ne<=6);
	for (int i=0; i<ne; ++i)
	{
		EDGE& e = m_Edge[m_NET[n1].e[i]];
		if (((e.n[0]==n1)&&(e.n[1]==n2))||
			((e.n[1]==n1)&&(e.n[0]==n2))) return m_NET[n1].e[i];
	}
	return -1;
}
