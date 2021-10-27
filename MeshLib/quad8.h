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
#include <FECore/vec3d.h>

namespace QUAD8
{
	// shape functions
	inline void shape(double* H, double r, double s)
	{
		H[4] = 0.5*(1 - r*r)*(1 - s);
		H[5] = 0.5*(1 - s*s)*(1 + r);
		H[6] = 0.5*(1 - r*r)*(1 + s);
		H[7] = 0.5*(1 - s*s)*(1 - r);

		H[0] = 0.25*(1 - r)*(1 - s) - 0.5*(H[4] + H[7]);
		H[1] = 0.25*(1 + r)*(1 - s) - 0.5*(H[4] + H[5]);
		H[2] = 0.25*(1 + r)*(1 + s) - 0.5*(H[5] + H[6]);
		H[3] = 0.25*(1 - r)*(1 + s) - 0.5*(H[6] + H[7]);
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double r, double s)
	{
		Hr[4] = -r*(1 - s);
		Hr[5] = 0.5*(1 - s*s);
		Hr[6] = -r*(1 + s);
		Hr[7] = -0.5*(1 - s*s);

		Hr[0] = -0.25*(1 - s) - 0.5*(Hr[4] + Hr[7]);
		Hr[1] = 0.25*(1 - s) - 0.5*(Hr[4] + Hr[5]);
		Hr[2] = 0.25*(1 + s) - 0.5*(Hr[5] + Hr[6]);
		Hr[3] = -0.25*(1 + s) - 0.5*(Hr[6] + Hr[7]);

		Hs[4] = -0.5*(1 - r*r);
		Hs[5] = -s*(1 + r);
		Hs[6] = 0.5*(1 - r*r);
		Hs[7] = -s*(1 - r);

		Hs[0] = -0.25*(1 - r) - 0.5*(Hs[4] + Hs[7]);
		Hs[1] = -0.25*(1 + r) - 0.5*(Hs[4] + Hs[5]);
		Hs[2] = 0.25*(1 + r) - 0.5*(Hs[5] + Hs[6]);
		Hs[3] = 0.25*(1 - r) - 0.5*(Hs[6] + Hs[7]);
	}

	inline vec3d eval(vec3d* x, double r, double s)
	{
		double H[8];
		shape(H, r, s);
		return vec3d(x[0]*H[0]+x[1]*H[1]+x[2]*H[2]+x[3]*H[3]+x[4]*H[4]+x[5]*H[5]+x[6]*H[6]+x[7]*H[7]);
	}
}

