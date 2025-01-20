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
#include "LaplaceSolver.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSNodeNodeList.h>
#include <MeshLib/FSNodeElementList.h>
#include <MeshLib/MeshMetrics.h>
using namespace std;

LaplaceSolver::LaplaceSolver()
{
	m_maxIters = 1000;
	m_tol = 1e-4;
	m_w = 1.0;

	m_niters = 0;
}

void LaplaceSolver::SetMaxIterations(int n)
{
	m_maxIters = n;
}

void LaplaceSolver::SetTolerance(double a)
{
	m_tol = a;
}

void LaplaceSolver::SetRelaxation(double w)
{
	m_w = w;
}

int LaplaceSolver::GetIterationCount() const
{
	return m_niters;
}

double LaplaceSolver::GetRelativeNorm() const
{
	return m_relNorm;
}

// Solves the Laplace equation on the mesh.
// Input: val = initial values for all nodes
//        bn  = boundary flags: 0 = free, 1 = fixed
// Output: val = solution
bool LaplaceSolver::Solve(FSMesh* pm, vector<double>& val, vector<int>& bn, int elemTag)
{
	m_niters = 0;

	// make sure the value and flag arrays are of the correct size
	int NN = pm->Nodes();
	if ((int) val.size() != NN) return false;
	if ((int) bn.size() != NN) return false;

	// find the value range
	double vmin = 0.0, vmax = 0.0;
	bool binit = false;
	for (int i=0; i<NN; ++i)
	{
		if (bn[i])
		{
			if (binit)
			{ 
				if (val[i] < vmin) vmin = val[i];
				if (val[i] > vmax) vmax = val[i];
			}
			else
			{
				vmin = vmax = val[i]; 
				binit = true; 
			}
		}
	}

	// if min and max are the same, then the solution is trivial
	if (vmin == vmax)
	{
		for (int i=0; i<NN; ++i) val[i] = vmin;
		return true;
	}

	// calculate the average value
	// we'll use this to initialize the free nodes
	double vavg = 0.5*(vmin + vmax);
	for (int i=0; i<NN; ++i)
		if (bn[i] == 0) val[i] = vavg;

	// get the element list from the tags
	int NE = pm->Elements();
	vector<int> elist; elist.reserve(NE);
	for (int i = 0; i < NE; ++i)
	{
		if (pm->Element(i).m_ntag == elemTag) elist.push_back(i);
	}

	// calculate the element volumes
	vector<double> Ve(NE, 0.0);
	for (int i = 0; i < elist.size(); ++i)
	{
		int eid = elist[i];
		FSElement& el = pm->Element(eid);
		if (el.IsSolid())
			Ve[eid] = FEMeshMetrics::ElementVolume(*pm, el);
		else
			Ve[eid] = FEMeshMetrics::ShellArea(*pm, el);
	}

	// build the node list
	vector<int> nodeList; nodeList.reserve(NN);
	pm->TagAllNodes(-1);
	for (int i = 0; i < elist.size(); ++i)
	{
		int eid = elist[i];
		FSElement& el = pm->Element(eid);
		int nn = el.Nodes();
		for (int j = 0; j < nn; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
	}
	int nc = 0;
	for (int i = 0; i < NN; ++i)
	{
		if (pm->Node(i).m_ntag != -1)
		{
			nodeList.push_back(i);
			pm->Node(i).m_ntag = nc++;
		}
		else
		{
			bn[i] = 2;
			val[i] = 0.0;
		}
	}
	assert(nc == nodeList.size());

	// create Node-Node list
	FSNodeNodeList NNL(pm);

	// create node-element list
	FSNodeElementList NEL;
	NEL.Build(pm);

	// we'll assign values to the edges and the nodes
	NNL.InitValues(0.0);
	vector<double> D(NN, 0.0);

	// build the diagonal terms
	for (int i=0; i<NN; ++i)
	{
		if (bn[i] == 0)
		{
			int eval = NEL.Valence(i);
			for (int j=0; j<eval; ++j)
			{
				int iel = NEL.ElementIndex(i, j);
				FSElement_& ej = *NEL.Element(i, j);

				if (ej.m_ntag == elemTag)
				{
					int na = ej.FindNodeIndex(i);
					assert(na != -1);

					double Vj = Ve[iel];
					int nj = ej.Nodes();
					double dot = 0.0;
					for (int k = 0; k < nj; ++k)
					{
						vec3d Ga = FEMeshMetrics::ShapeGradient(*pm, ej, na, k);
						dot += Ga * Ga;
					}
					dot *= Vj / nj;

					D[i] += dot;
				}
			}
		}
		else D[i] = 1.0;
	}

	// build the edge weights
	for (int i=0; i<NN; ++i)
	{
		int nval = NNL.Valence(i);
		for (int j=0; j<nval; ++j)
		{
			int ni = i;
			int nj = NNL.Node(i, j);

			// we must loop over the union of Sa and Sb
			int nei = NEL.Valence(ni);

			double Kij = 0.0;

			for (int k=0; k<nei; ++k)
			{
				int kel = NEL.ElementIndex(ni, k);
				if (NEL.HasElement(nj, kel))
				{
					FSElement_& ek = *NEL.Element(ni, k);

					if (ek.m_ntag == elemTag)
					{
						int na = ek.FindNodeIndex(ni); assert(na != -1);
						int nb = ek.FindNodeIndex(nj); assert(nb != -1);

						double Vk = Ve[kel];
						int nk = ek.Nodes();

						double dot = 0.0;
						for (int k = 0; k < nk; ++k)
						{
							vec3d Ga = FEMeshMetrics::ShapeGradient(*pm, ek, na, k);
							vec3d Gb = FEMeshMetrics::ShapeGradient(*pm, ek, nb, k);
							dot += Ga * Gb;
						}
						dot *= Vk / nk;

						Kij += dot;
					}
				}
			}

			NNL.Value(i, j) = Kij;
		}
	}

	// inverted diagonal values
	vector<double> Dinv(NN);
	for (int i=0; i<NN; ++i) Dinv[i] = 1.0 / D[i];

	// start the iterations
	double norm0 = 0, norm, normp;
	m_relNorm = 1.0;
	do
	{
		norm = 0;
		for (int i=0; i<NN; ++i)
		{
			if (bn[i] == 0)
			{
				double sum = 0;
				int nval = NNL.Valence(i);
				for (int k=0; k<nval; ++k) sum -= val[NNL.Node(i, k)]*NNL.Value(i,k);

				double newVal = (1.0 - m_w)*val[i] + sum * m_w * Dinv[i];

				double dv = (val[i] - newVal);
				norm += dv * dv;

				val[i] = newVal;
			}
		}
		norm = sqrt(norm);
		if (m_niters == 0) { norm0 = normp = norm; }
		m_relNorm = norm / norm0;
		double mu = norm / normp;
		normp = norm;

		printf("%d: %lg, %lg, %lg\n", m_niters, norm, m_relNorm, mu);
		m_niters++;
	}
	while ((m_niters < m_maxIters)&&(m_relNorm > m_tol));

	return (m_relNorm < m_tol);
}
