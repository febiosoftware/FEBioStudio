#pragma once

namespace HEX27
{
	// shape functions
	inline void shape(double* H, double r, double s, double t)
	{
		double R[3] = { 0.5*r*(r - 1.0), 0.5*r*(r + 1.0), 1.0 - r*r };
		double S[3] = { 0.5*s*(s - 1.0), 0.5*s*(s + 1.0), 1.0 - s*s };
		double T[3] = { 0.5*t*(t - 1.0), 0.5*t*(t + 1.0), 1.0 - t*t };

		H[0] = R[0] * S[0] * T[0];
		H[1] = R[1] * S[0] * T[0];
		H[2] = R[1] * S[1] * T[0];
		H[3] = R[0] * S[1] * T[0];
		H[4] = R[0] * S[0] * T[1];
		H[5] = R[1] * S[0] * T[1];
		H[6] = R[1] * S[1] * T[1];
		H[7] = R[0] * S[1] * T[1];
		H[8] = R[2] * S[0] * T[0];
		H[9] = R[1] * S[2] * T[0];
		H[10] = R[2] * S[1] * T[0];
		H[11] = R[0] * S[2] * T[0];
		H[12] = R[2] * S[0] * T[1];
		H[13] = R[1] * S[2] * T[1];
		H[14] = R[2] * S[1] * T[1];
		H[15] = R[0] * S[2] * T[1];
		H[16] = R[0] * S[0] * T[2];
		H[17] = R[1] * S[0] * T[2];
		H[18] = R[1] * S[1] * T[2];
		H[19] = R[0] * S[1] * T[2];
		H[20] = R[2] * S[0] * T[2];
		H[21] = R[1] * S[2] * T[2];
		H[22] = R[2] * S[1] * T[2];
		H[23] = R[0] * S[2] * T[2];
		H[24] = R[2] * S[2] * T[0];
		H[25] = R[2] * S[2] * T[1];
		H[26] = R[2] * S[2] * T[2];
	}

	// shape function derivatives
	inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
	{
		double R[3] = {0.5*r*(r-1.0), 0.5*r*(r+1.0), 1.0 - r*r};
		double S[3] = {0.5*s*(s-1.0), 0.5*s*(s+1.0), 1.0 - s*s};
		double T[3] = {0.5*t*(t-1.0), 0.5*t*(t+1.0), 1.0 - t*t};

		double DR[3] = {r - 0.5, r  + 0.5, -2.0*r};
		double DS[3] = {s - 0.5, s  + 0.5, -2.0*s};
		double DT[3] = {t - 0.5, t  + 0.5, -2.0*t};

		Hr[ 0] = DR[0]*S[0]*T[0];
		Hr[ 1] = DR[1]*S[0]*T[0];
		Hr[ 2] = DR[1]*S[1]*T[0];
		Hr[ 3] = DR[0]*S[1]*T[0];
		Hr[ 4] = DR[0]*S[0]*T[1];
		Hr[ 5] = DR[1]*S[0]*T[1];
		Hr[ 6] = DR[1]*S[1]*T[1];
		Hr[ 7] = DR[0]*S[1]*T[1];
		Hr[ 8] = DR[2]*S[0]*T[0];
		Hr[ 9] = DR[1]*S[2]*T[0];
		Hr[10] = DR[2]*S[1]*T[0];
		Hr[11] = DR[0]*S[2]*T[0];
		Hr[12] = DR[2]*S[0]*T[1];
		Hr[13] = DR[1]*S[2]*T[1];
		Hr[14] = DR[2]*S[1]*T[1];
		Hr[15] = DR[0]*S[2]*T[1];
		Hr[16] = DR[0]*S[0]*T[2];
		Hr[17] = DR[1]*S[0]*T[2];
		Hr[18] = DR[1]*S[1]*T[2];
		Hr[19] = DR[0]*S[1]*T[2];
		Hr[20] = DR[2]*S[0]*T[2];
		Hr[21] = DR[1]*S[2]*T[2];
		Hr[22] = DR[2]*S[1]*T[2];
		Hr[23] = DR[0]*S[2]*T[2];
		Hr[24] = DR[2]*S[2]*T[0];
		Hr[25] = DR[2]*S[2]*T[1];
		Hr[26] = DR[2]*S[2]*T[2];

		Hs[ 0] = R[0]*DS[0]*T[0];
		Hs[ 1] = R[1]*DS[0]*T[0];
		Hs[ 2] = R[1]*DS[1]*T[0];
		Hs[ 3] = R[0]*DS[1]*T[0];
		Hs[ 4] = R[0]*DS[0]*T[1];
		Hs[ 5] = R[1]*DS[0]*T[1];
		Hs[ 6] = R[1]*DS[1]*T[1];
		Hs[ 7] = R[0]*DS[1]*T[1];
		Hs[ 8] = R[2]*DS[0]*T[0];
		Hs[ 9] = R[1]*DS[2]*T[0];
		Hs[10] = R[2]*DS[1]*T[0];
		Hs[11] = R[0]*DS[2]*T[0];
		Hs[12] = R[2]*DS[0]*T[1];
		Hs[13] = R[1]*DS[2]*T[1];
		Hs[14] = R[2]*DS[1]*T[1];
		Hs[15] = R[0]*DS[2]*T[1];
		Hs[16] = R[0]*DS[0]*T[2];
		Hs[17] = R[1]*DS[0]*T[2];
		Hs[18] = R[1]*DS[1]*T[2];
		Hs[19] = R[0]*DS[1]*T[2];
		Hs[20] = R[2]*DS[0]*T[2];
		Hs[21] = R[1]*DS[2]*T[2];
		Hs[22] = R[2]*DS[1]*T[2];
		Hs[23] = R[0]*DS[2]*T[2];
		Hs[24] = R[2]*DS[2]*T[0];
		Hs[25] = R[2]*DS[2]*T[1];
		Hs[26] = R[2]*DS[2]*T[2];

		Ht[ 0] = R[0]*S[0]*DT[0];
		Ht[ 1] = R[1]*S[0]*DT[0];
		Ht[ 2] = R[1]*S[1]*DT[0];
		Ht[ 3] = R[0]*S[1]*DT[0];
		Ht[ 4] = R[0]*S[0]*DT[1];
		Ht[ 5] = R[1]*S[0]*DT[1];
		Ht[ 6] = R[1]*S[1]*DT[1];
		Ht[ 7] = R[0]*S[1]*DT[1];
		Ht[ 8] = R[2]*S[0]*DT[0];
		Ht[ 9] = R[1]*S[2]*DT[0];
		Ht[10] = R[2]*S[1]*DT[0];
		Ht[11] = R[0]*S[2]*DT[0];
		Ht[12] = R[2]*S[0]*DT[1];
		Ht[13] = R[1]*S[2]*DT[1];
		Ht[14] = R[2]*S[1]*DT[1];
		Ht[15] = R[0]*S[2]*DT[1];
		Ht[16] = R[0]*S[0]*DT[2];
		Ht[17] = R[1]*S[0]*DT[2];
		Ht[18] = R[1]*S[1]*DT[2];
		Ht[19] = R[0]*S[1]*DT[2];
		Ht[20] = R[2]*S[0]*DT[2];
		Ht[21] = R[1]*S[2]*DT[2];
		Ht[22] = R[2]*S[1]*DT[2];
		Ht[23] = R[0]*S[2]*DT[2];
		Ht[24] = R[2]*S[2]*DT[0];
		Ht[25] = R[2]*S[2]*DT[1];
		Ht[26] = R[2]*S[2]*DT[2];
	}

	// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
	void iso_coord(int n, double q[3])
	{
		switch (n)
		{
        case -1: q[0] = 0; q[1] = 0; q[2] = 0; break;
		case  0: q[0] = -1; q[1] = -1; q[2] = -1; break;
		case  1: q[0] =  1; q[1] = -1; q[2] = -1; break;
		case  2: q[0] =  1; q[1] =  1; q[2] = -1; break;
		case  3: q[0] = -1; q[1] =  1; q[2] = -1; break;
		case  4: q[0] = -1; q[1] = -1; q[2] =  1; break;
		case  5: q[0] =  1; q[1] = -1; q[2] =  1; break;
		case  6: q[0] =  1; q[1] =  1; q[2] =  1; break;
		case  7: q[0] = -1; q[1] =  1; q[2] =  1; break;
		case  8: q[0] =  0; q[1] = -1; q[2] = -1; break;
		case  9: q[0] =  1; q[1] =  0; q[2] = -1; break;
		case 10: q[0] =  0; q[1] =  1; q[2] = -1; break;
		case 11: q[0] = -1; q[1] =  0; q[2] = -1; break;
		case 12: q[0] =  0; q[1] = -1; q[2] =  1; break;
		case 13: q[0] =  1; q[1] =  0; q[2] =  1; break;
		case 14: q[0] =  0; q[1] =  1; q[2] =  1; break;
		case 15: q[0] = -1; q[1] =  0; q[2] =  1; break;
		case 16: q[0] = -1; q[1] = -1; q[2] =  0; break;
		case 17: q[0] =  1; q[1] = -1; q[2] =  0; break;
		case 18: q[0] =  1; q[1] =  1; q[2] =  0; break;
		case 19: q[0] = -1; q[1] =  1; q[2] =  0; break;
		case 20: q[0] =  0; q[1] = -1; q[2] =  0; break;
		case 21: q[0] =  1; q[1] =  0; q[2] =  0; break;
		case 22: q[0] =  0; q[1] =  1; q[2] =  0; break;
		case 23: q[0] = -1; q[1] =  0; q[2] =  0; break;
		case 24: q[0] =  0; q[1] =  0; q[2] = -1; break;
		case 25: q[0] =  0; q[1] =  0; q[2] =  1; break;
		case 26: q[0] =  0; q[1] =  0; q[2] =  0; break;
		}
	}
}
