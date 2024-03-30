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
        Hr[1] =  0.5*(1 - t); Hs[1] =  0.0        ;    Ht[1] = -0.5*r;
        Hr[2] =  0.0        ; Hs[2] =  0.5*(1 - t);    Ht[2] = -0.5*s;
        Hr[3] = -0.5*(1 + t); Hs[3] = -0.5*(1 + t); Ht[3] =  0.5*(1 - r - s);
        Hr[4] =  0.5*(1 + t); Hs[4] =  0.0        ; Ht[4] =  0.5*r;
        Hr[5] =  0.0        ; Hs[5] =  0.5*(1 + t);    Ht[5] =  0.5*s;
    }
    
    // iso-parametric coordinates of nodes (for n = -1 return center coordinates)
    inline void iso_coord(int n, double q[3])
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

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
		const double a = 1.0 / 6.0;
		const double b = 2.0 / 3.0;
		const double c = 1.0 / sqrt(3.0);
		const double w = 1.0 / 6.0;

		gr[0] = a; gs[0] = a; gt[0] = -c; gw[0] = w;
		gr[1] = b; gs[1] = a; gt[1] = -c; gw[1] = w;
		gr[2] = a; gs[2] = b; gt[2] = -c; gw[2] = w;
		gr[3] = a; gs[3] = a; gt[3] = c; gw[3] = w;
		gr[4] = b; gs[4] = a; gt[4] = c; gw[4] = w;
		gr[5] = a; gs[5] = b; gt[5] = c; gw[5] = w;
    }
    }

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
    inline void iso_coord(int n, double q[3])
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

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
		const double a = 1.0 / 3.0;
		const double b = 1.0 / 5.0;
		const double c = 3.0 / 5.0;
		const double d = sqrt(a);
		gr[0] = a; gs[0] = a; gt[0] = -d; gw[0] = -27.0 / 96.0;
		gr[1] = c; gs[1] = b; gt[1] = -d; gw[1] = 25.0 / 96.0;
		gr[2] = b; gs[2] = b; gt[2] = -d; gw[2] = 25.0 / 96.0;
		gr[3] = b; gs[3] = c; gt[3] = -d; gw[3] = 25.0 / 96.0;
		gr[4] = a; gs[4] = a; gt[4] = d; gw[4] = -27.0 / 96.0;
		gr[5] = c; gs[5] = b; gt[5] = d; gw[5] = 25.0 / 96.0;
		gr[6] = b; gs[6] = b; gt[6] = d; gw[6] = 25.0 / 96.0;
		gr[7] = b; gs[7] = c; gt[7] = d; gw[7] = 25.0 / 96.0;
    }
    }
