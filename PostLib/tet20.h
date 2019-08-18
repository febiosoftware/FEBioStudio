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
	void iso_coord(int n, double q[3])
	{
		// TODO: Implement this
	}
}
