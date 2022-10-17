/*This file is part of the FEBio Studio source code and is licensed under the MIT license
 listed below.

 See Copyright-FEBio-Studio.txt for details.

 Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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

namespace QUAD4 {

	// shape functions
	inline void shape(double* H, double r, double s)
	{
		H[0] = 0.25*(1 - r)*(1 - s);
		H[1] = 0.25*(1 + r)*(1 - s);
		H[2] = 0.25*(1 + r)*(1 + s);
		H[3] = 0.25*(1 - r)*(1 + s);
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double r, double s)
	{
		Hr[0] = -0.25*(1-s);
		Hr[1] =  0.25*(1-s);
		Hr[2] =  0.25*(1+s);
		Hr[3] = -0.25*(1+s);
    
		Hs[0] = -0.25*(1-r);
		Hs[1] = -0.25*(1+r);
		Hs[2] =  0.25*(1+r);
		Hs[3] =  0.25*(1-r);
	}

	// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
	inline void iso_coord(int n, double q[2])
	{
		switch (n)
		{
		case -1: q[0] = 0; q[1] =  0; break;
		case 0: q[0] = -1; q[1] = -1; break;
		case 1: q[0] =  1; q[1] = -1; break;
		case 2: q[0] =  1; q[1] =  1; break;
		case 3: q[0] = -1; q[1] =  1; break;
		}
	}

	inline void gauss_data(double* gr, double* gs, double* gt, double* gw)
	{
		const double a = 1.0 / sqrt(3.0);
		const double w = 1.0;

		gr[0] = -a; gs[0] = -a; gt[0] = -a; gw[0] = w;
		gr[1] =  a; gs[1] = -a; gt[1] = -a; gw[1] = w;
		gr[2] =  a; gs[2] =  a; gt[2] = -a; gw[2] = w;
		gr[3] = -a; gs[3] =  a; gt[3] = -a; gw[3] = w;

		gr[4] = -a; gs[4] = -a; gt[4] = a; gw[4] = w;
		gr[5] =  a; gs[5] = -a; gt[5] = a; gw[5] = w;
		gr[6] =  a; gs[6] =  a; gt[6] = a; gw[6] = w;
		gr[7] = -a; gs[7] =  a; gt[7] = a; gw[7] = w;
	}
}

