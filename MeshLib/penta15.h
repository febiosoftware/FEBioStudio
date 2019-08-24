#pragma once

namespace PENTA15
{
	// shape functions
	inline void shape(double* H, double r, double s, double t)
	{
		double u = 1 - r - s;

		H[0] = u*(2 * u - 1)*(t - 1)*t / 2;
		H[1] = r*(2 * r - 1)*(t - 1)*t / 2;
		H[2] = s*(2 * s - 1)*(t - 1)*t / 2;
		H[3] = u*(2 * u - 1)*(t + 1)*t / 2;
		H[4] = r*(2 * r - 1)*(t + 1)*t / 2;
		H[5] = s*(2 * s - 1)*(t + 1)*t / 2;
		H[6] = 2 * u*r*(t - 1)*t;
		H[7] = 2 * r*s*(t - 1)*t;
		H[8] = 2 * s*u*(t - 1)*t;
		H[9] = 2 * u*r*(t + 1)*t;
		H[10] = 2 * r*s*(t + 1)*t;
		H[11] = 2 * s*u*(t + 1)*t;
		H[12] = u*(1 + t)*(1 - t);
		H[13] = r*(1 + t)*(1 - t);
		H[14] = s*(1 + t)*(1 - t);
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
	{
        Hr[ 0] = ((-3 + 4*r + 4*s)*(-1 + t)*t)/2.;
        Hr[ 1] = ((-1 + 4*r)*(-1 + t)*t)/2.;
        Hr[ 2] = 0;
        Hr[ 3] = ((-3 + 4*r + 4*s)*t*(1 + t))/2.;
        Hr[ 4] = ((-1 + 4*r)*t*(1 + t))/2.;
        Hr[ 5] = 0;
        Hr[ 6] = -2*(-1 + 2*r + s)*(-1 + t)*t;
        Hr[ 7] = 2*s*(-1 + t)*t;
        Hr[ 8] = -2*s*(-1 + t)*t;
        Hr[ 9] = -2*(-1 + 2*r + s)*t*(1 + t);
        Hr[10] = 2*s*t*(1 + t);
        Hr[11] = -2*s*t*(1 + t);
        Hr[12] = -1 + t*t;
        Hr[13] = 1 - t*t;
        Hr[14] = 0;
            
        Hs[ 0] = ((-3 + 4*r + 4*s)*(-1 + t)*t)/2.;
        Hs[ 1] = 0;
        Hs[ 2] = ((-1 + 4*s)*(-1 + t)*t)/2.;
        Hs[ 3] = ((-3 + 4*r + 4*s)*t*(1 + t))/2.;
        Hs[ 4] = 0;
        Hs[ 5] = ((-1 + 4*s)*t*(1 + t))/2.;
        Hs[ 6] = -2*r*(-1 + t)*t;
        Hs[ 7] = 2*r*(-1 + t)*t;
        Hs[ 8] = -2*(-1 + r + 2*s)*(-1 + t)*t;
        Hs[ 9] = -2*r*t*(1 + t);
        Hs[10] = 2*r*t*(1 + t);
        Hs[11] = -2*(-1 + r + 2*s)*t*(1 + t);
        Hs[12] = -1 + t*t;
        Hs[13] = 0;
        Hs[14] = 1 - t*t;
            
        Ht[ 0] = ((-1 + r + s)*(-1 + 2*r + 2*s)*(-1 + 2*t))/2.;
        Ht[ 1] = (r*(-1 + 2*r)*(-1 + 2*t))/2.;
        Ht[ 2] = (s*(-1 + 2*s)*(-1 + 2*t))/2.;
        Ht[ 3] = ((-1 + r + s)*(-1 + 2*r + 2*s)*(1 + 2*t))/2.;
        Ht[ 4] = (r*(-1 + 2*r)*(1 + 2*t))/2.;
        Ht[ 5] = (s*(-1 + 2*s)*(1 + 2*t))/2.;
        Ht[ 6] = 2*r*(-1 + r + s)*(1 - 2*t);
        Ht[ 7] = 2*r*s*(-1 + 2*t);
        Ht[ 8] = 2*s*(-1 + r + s)*(1 - 2*t);
        Ht[ 9] = -2*r*(-1 + r + s)*(1 + 2*t);
        Ht[10] = 2*r*s*(1 + 2*t);
        Ht[11] = -2*s*(-1 + r + s)*(1 + 2*t);
        Ht[12] = 2*(-1 + r + s)*t;
        Ht[13] = -2*r*t;
        Ht[14] = -2*s*t;
	}

	// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
	void iso_coord(int n, double q[3])
	{
        const double t = 1.0/3.0;
        switch (n)
        {
            case -1: q[0] = t;  q[1] = t;   q[2] = 0; break;
            case  0: q[0] = 0;  q[1] = 0;   q[2] = -1; break;
            case  1: q[0] = 1;  q[1] = 0;   q[2] = -1; break;
            case  2: q[0] = 0;  q[1] = 1;   q[2] = -1; break;
            case  3: q[0] = 0;  q[1] = 0;   q[2] = 1; break;
            case  4: q[0] = 1;  q[1] = 0;   q[2] = 1; break;
            case  5: q[0] = 0;  q[1] = 1;   q[2] = 1; break;
            case  6: q[0] = 0.5;q[1] = 0;   q[2] = -1; break;
            case  7: q[0] = 0.5;q[1] = 0.5; q[2] = -1; break;
            case  8: q[0] = 0;  q[1] = 0.5; q[2] = -1; break;
            case  9: q[0] = 0.5;q[1] = 0;   q[2] = 1; break;
            case 10: q[0] = 0.5;q[1] = 0.5; q[2] = 1; break;
            case 11: q[0] = 0;  q[1] = 0.5; q[2] = 1; break;
            case 12: q[0] = 0;  q[1] = 0;   q[2] = 0; break;
            case 13: q[0] = 1;  q[1] = 0;   q[2] = 0; break;
            case 14: q[0] = 0;  q[1] = 1;   q[2] = 0; break;
        }
	}
}
