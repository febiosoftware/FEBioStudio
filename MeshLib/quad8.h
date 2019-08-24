#pragma once
#include <MathLib/math3d.h>

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

