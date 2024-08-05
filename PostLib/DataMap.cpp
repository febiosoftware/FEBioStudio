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
#include "DataMap.h"
#include "FEPostMesh.h"
#include <assert.h>
using namespace Post;

//-----------------------------------------------------------------------------
// this function calculates the gradient map of the current evaluated
// values at the nodes.
// the algorithm calculates the gradient at the center of each element
// and projects the vector to the nodes.
//
void VectorMap::Gradient(int ntime, std::vector<float> &v)
{
	assert(m_pmesh);
	if (m_pmesh == 0) return;
	std::vector<vec3f>& G = m_Data[ntime];
	FEPostMesh& mesh = *m_pmesh;

	int i, k;

	// we evaluate dH/dr at the center of the element : (r,s,t) = (0,0,0)
	static const double a = 0.125; // = 1/8
	static const double dHdr[3][8] = {
		{-a, a, a,-a,-a, a, a,-a },
		{-a,-a, a, a,-a,-a, a, a },
		{-a,-a,-a,-a, a, a, a, a }};


	double f[8];
	vec3f x[8];

	mat3d J;
	int node;

	float dfdx[3];
	double dfdr[3];

	vec3f* gn[8];

	assert(G.size() == mesh.Nodes());
	assert(v.size() == mesh.Nodes());

	for (i=0; i<mesh.Nodes(); ++i) G[i] = vec3f(0,0,0);

	for (i=0; i<mesh.Elements(); i++)
	{
		FEElement_& e = mesh.ElementRef(i);

		if (e.IsSolid())
		{
			// get the nodal positions and values
			int ne = e.Nodes();
			int NE[8];
			switch (e.Type())
			{
			case FE_HEX8:
			case FE_HEX20:
			case FE_HEX27:
				{
					for (k=0; k<8; ++k) NE[k] = k;
				}
				break;
			case FE_PENTA6:
            case FE_PENTA15:
                {
					NE[0] = 0; NE[1] = 1; NE[2] = NE[3] = 2;
					NE[4] = 3; NE[5] = 4; NE[6] = NE[7] = 5;
				}
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
			case FE_TET20:
                {
					NE[0] = 0; NE[1] = 1; NE[2] = 2; NE[3] = 2; 
					NE[4] = NE[5] = NE[6] = NE[7] = 3;
                }
				break;
            case FE_PYRA5:
            case FE_PYRA13:
                {
                    NE[0] = 0; NE[1] = 1; NE[2] = 2; NE[3] = 3;
                    NE[4] = NE[5] = NE[6] = NE[7] = 4;
                }
                    break;
			}

			for (k=0; k<8; k++) 
			{ 
				node = e.m_node[NE[k]];
				x[k] = to_vec3f(mesh.Node(node).r);
				f[k] = v[node];
				gn[k] = &G[node];
			}

			// evaluate jacobian
			J.zero();
			for (k=0; k<8; k++)
			{
				J[0][0] += x[k].x*dHdr[0][k]; J[0][1] += x[k].x*dHdr[1][k]; J[0][2] += x[k].x*dHdr[2][k];
				J[1][0] += x[k].y*dHdr[0][k]; J[1][1] += x[k].y*dHdr[1][k]; J[1][2] += x[k].y*dHdr[2][k];
				J[2][0] += x[k].z*dHdr[0][k]; J[2][1] += x[k].z*dHdr[1][k]; J[2][2] += x[k].z*dHdr[2][k];
			}

			// invert jacobian
			if (J.invert() == false) return;

			// evaluate df/dr = SUM( dH[k]/dr * f[k] )
			dfdr[0] = dfdr[1] = dfdr[2] = 0;
			for (k=0; k<8; k++)
			{
				dfdr[0] += dHdr[0][k]*f[k];
				dfdr[1] += dHdr[1][k]*f[k];
				dfdr[2] += dHdr[2][k]*f[k];
			}

			// evaluate df/dx = J^(-T)*df/dr
			dfdx[0] = (float)(J[0][0]*dfdr[0] + J[1][0]*dfdr[1] + J[2][0]*dfdr[2]);
			dfdx[1] = (float)(J[0][1]*dfdr[0] + J[1][1]*dfdr[1] + J[2][1]*dfdr[2]);
			dfdx[2] = (float)(J[0][2]*dfdr[0] + J[1][2]*dfdr[1] + J[2][2]*dfdr[2]);

			// project the vector to the nodes
			for (k=0; k<8; k++)
			{
				gn[k]->x += dfdx[0];
				gn[k]->y += dfdx[1];
				gn[k]->z += dfdx[2];
			}
		}
	}

	// "normalize" the gradients
	for (i=0; i<mesh.Nodes(); i++)
	{
		const std::vector<NodeElemRef>& nel = mesh.NodeElemList(i);
		if (!nel.empty()) G[i] /= (float) nel.size();
		G[i] *= -1;
	}
}

