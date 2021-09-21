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

namespace TRI3 {

	// shape functions
	inline void shape(double* H, double r, double s)
	{
		H[0] = 1.0 - r - s;
		H[1] = r;
		H[2] = s;
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double r, double s)
	{
		Hr[0] = -1; Hs[0] = -1;
		Hr[1] =  1; Hs[1] =  0;
		Hr[2] =  0; Hs[2] =  1;
	}

	// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
	inline void iso_coord(int n, double q[2])
	{
		const double THIRD = 1.0 / 3.0;
		switch (n)
		{
		case -1: q[0] = THIRD; q[1] = THIRD; break;
		case 0: q[0] = 0; q[1] = 0; break;
		case 1: q[0] = 1; q[1] = 0; break;
		case 2: q[0] = 0; q[1] = 1; break;
		}
	}

	inline void gauss_data(double* gr, double* gs, double* gt, double* gw)
	{
		//gauss intergration points
		const double a = 1.0 / 6.0;
		const double b = 2.0 / 3.0;
		const double c = 1.0 / sqrt(3.0);

		gr[0] = a; gs[0] = a; gt[0] = -c; gw[0] = a;
		gr[1] = b; gs[1] = a; gt[1] = -c; gw[1] = a;
		gr[2] = a; gs[2] = b; gt[2] = -c; gw[2] = a;
		gr[3] = a; gs[3] = a; gt[3] =  c; gw[3] = a;
		gr[4] = b; gs[4] = a; gt[4] =  c; gw[4] = a;
		gr[5] = a; gs[5] = b; gt[5] =  c; gw[5] = a;
	}
}
