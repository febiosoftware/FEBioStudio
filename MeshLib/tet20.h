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

namespace TET20
{
	// shape functions
	inline void shape(double* H, double r, double s, double t)
	{
		double L1 = 1.0 - r - s - t;
		double L2 = r;
		double L3 = s;
		double L4 = t;

		H[0] = 0.5*(3*L1 - 1)*(3*L1 - 2)*L1;
		H[1] = 0.5*(3*L2 - 1)*(3*L2 - 2)*L2;
		H[2] = 0.5*(3*L3 - 1)*(3*L3 - 2)*L3;
		H[3] = 0.5*(3*L4 - 1)*(3*L4 - 2)*L4;
		H[4] = 9.0/2.0*(3*L1 - 1)*L1*L2;
		H[5] = 9.0/2.0*(3*L2 - 1)*L1*L2;
		H[6] = 9.0/2.0*(3*L2 - 1)*L2*L3;
		H[7] = 9.0/2.0*(3*L3 - 1)*L2*L3;
		H[8] = 9.0/2.0*(3*L1 - 1)*L1*L3;
		H[9] = 9.0/2.0*(3*L3 - 1)*L1*L3;
		H[10] = 9.0/2.0*(3*L1 - 1)*L1*L4;
		H[11] = 9.0/2.0*(3*L4 - 1)*L1*L4;
		H[12] = 9.0/2.0*(3*L2 - 1)*L2*L4;
		H[13] = 9.0/2.0*(3*L4 - 1)*L2*L4;
		H[14] = 9.0/2.0*(3*L3 - 1)*L3*L4;
		H[15] = 9.0/2.0*(3*L4 - 1)*L3*L4;
		H[16] = 27.0*L1*L2*L4;
		H[17] = 27.0*L2*L3*L4;
		H[18] = 27.0*L1*L3*L4;
		H[19] = 27.0*L1*L2*L3;
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
	{
		// TODO: Implement this
	}

	// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
	inline void iso_coord(int n, double q[3])
	{
		// TODO: Implement this
	}
}
