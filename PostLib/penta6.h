#pragma once

namespace PENTA6
{
	// shape functions
	inline void shape(double* H, double r, double s, double t)
	{ 
		H[0] = 0.5*(1 - t)*(1 - r - s);
		H[1] = 0.5*(1 - t)*r;
		H[2] = 0.5*(1 - t)*s;
		H[3] = 0.5*(1 + t)*(1 - r - s);
		H[4] = 0.5*(1 + t)*r;
		H[5] = 0.5*(1 + t)*s;
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
	{
		Hr[0] = -0.5*(1 - t); Hs[0] = -0.5*(1 - t); Ht[0] = -0.5*(1 - r - s);
		Hr[1] =  0.5*(1 - t); Hs[1] =  0.0        ;	Ht[1] = -0.5*r;
		Hr[2] =  0.0        ; Hs[2] =  0.5*(1 - t);	Ht[2] = -0.5*s;
		Hr[3] = -0.5*(1 + t); Hs[3] = -0.5*(1 + t); Ht[3] =  0.5*(1 - r - s);
		Hr[4] =  0.5*(1 + t); Hs[4] =  0.0        ; Ht[4] =  0.5*r;
		Hr[5] =  0.0        ; Hs[5] =  0.5*(1 + t);	Ht[5] =  0.5*s;
	}

	// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
	void iso_coord(int n, double q[3])
	{
		const double t = 1.0 / 3.0;
		switch (n)
		{
		case -1: q[0] = t; q[1] = t; q[2] = 0; break;
		case 0: q[0] = 0; q[1] = 0; q[2] = -1; break;
		case 1: q[0] = 1; q[1] = 0; q[2] = -1; break;
		case 2: q[0] = 0; q[1] = 1; q[2] = -1; break;
		case 3: q[0] = 0; q[1] = 0; q[2] = 1; break;
		case 4: q[0] = 1; q[1] = 0; q[2] = 1; break;
		case 5: q[0] = 0; q[1] = 1; q[2] = 1; break;
		}
	}
}
