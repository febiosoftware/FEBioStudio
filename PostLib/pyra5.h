#pragma once

namespace PYRA5 
{
	// shape functions
	inline void shape(double* H, double r, double s, double t)
	{
		H[0] = 0.125*(1.0 - r)*(1.0 - s)*(1.0 - t);
		H[1] = 0.125*(1.0 + r)*(1.0 - s)*(1.0 - t);
		H[2] = 0.125*(1.0 + r)*(1.0 + s)*(1.0 - t);
		H[3] = 0.125*(1.0 - r)*(1.0 + s)*(1.0 - t);
		H[4] = 0.5*(1.0 + t);
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
	{
		Hr[0] = -0.125*(1.0 - s)*(1.0 - t);
		Hr[1] =  0.125*(1.0 - s)*(1.0 - t);
		Hr[2] =  0.125*(1.0 + s)*(1.0 - t);
		Hr[3] = -0.125*(1.0 + s)*(1.0 - t);
		Hr[4] =  0.0;

		Hs[0] = -0.125*(1.0 - r)*(1.0 - t);
		Hs[1] = -0.125*(1.0 + r)*(1.0 - t);
		Hs[2] =  0.125*(1.0 + r)*(1.0 - t);
		Hs[3] =  0.125*(1.0 - r)*(1.0 - t);
		Hs[4] =  0.0;

		Ht[0] = -0.125*(1.0 - r)*(1.0 - s);
		Ht[1] = -0.125*(1.0 + r)*(1.0 - s);
		Ht[2] = -0.125*(1.0 + r)*(1.0 + s);
		Ht[3] = -0.125*(1.0 - r)*(1.0 + s);
		Ht[4] =  0.5;
	}

	// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
	void iso_coord(int n, double q[3])
	{
		switch (n)
		{
		case -1: q[0] = 0.0; q[1] = 0.0; q[2] = 0.0; break;
		case 0: q[0] = -1.0; q[1] = -1.0; q[2] = -1.0; break;
		case 1: q[0] = 1.0; q[1] = -1.0; q[2] = -1.0; break;
		case 2: q[0] = 1.0; q[1] = 1.0; q[2] = -1.0; break;
		case 3: q[0] = -1.0; q[1] = 1.0; q[2] = -1.0; break;
		case 4: q[0] = 0.0; q[1] = 0.0; q[2] = 1.0; break;
		}
	}
}
