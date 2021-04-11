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
    inline void iso_coord(int n, double q[3])
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

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
        const double a = 1.0 / sqrt(3.0);
        gr[0] = -a; gs[0] = -a; gt[0] = -a; gw[0] = 1;
        gr[1] = a; gs[1] = -a; gt[1] = -a; gw[1] = 1;
        gr[2] = a; gs[2] = a; gt[2] = -a; gw[2] = 1;
        gr[3] = -a; gs[3] = a; gt[3] = -a; gw[3] = 1;
        gr[4] = -a; gs[4] = -a; gt[4] = a; gw[4] = 1;
        gr[5] = a; gs[5] = -a; gt[5] = a; gw[5] = 1;
        gr[6] = a; gs[6] = a; gt[6] = a; gw[6] = 1;
        gr[7] = -a; gs[7] = a; gt[7] = a; gw[7] = 1;
    }
    }


namespace PYRA13
    {
    // shape functions
    inline void shape(double* H, double r, double s, double t)
    {
        H[5] = 0.25*(1 - r*r)*(1 - s)*(1 - t);
        H[6] = 0.25*(1 - s*s)*(1 + r)*(1 - t);
        H[7] = 0.25*(1 - r*r)*(1 + s)*(1 - t);
        H[8] = 0.25*(1 - s*s)*(1 - r)*(1 - t);
        H[9] = 0.25*(1 - t*t)*(1 - r)*(1 - s);
        H[10] = 0.25*(1 - t*t)*(1 + r)*(1 - s);
        H[11] = 0.25*(1 - t*t)*(1 + r)*(1 + s);
        H[12] = 0.25*(1 - t*t)*(1 - r)*(1 + s);
        
        H[0] = 0.125*(1 - r)*(1 - s)*(1 - t) - 0.5*(H[5] + H[8] + H[9]);
        H[1] = 0.125*(1 + r)*(1 - s)*(1 - t) - 0.5*(H[5] + H[6] + H[10]);
        H[2] = 0.125*(1 + r)*(1 + s)*(1 - t) - 0.5*(H[6] + H[7] + H[11]);
        H[3] = 0.125*(1 - r)*(1 + s)*(1 - t) - 0.5*(H[7] + H[8] + H[12]);
        H[4] = 0.5*t*(1 + t);
    }
    
    // shape function derivatives
    inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
    {
        Hr[ 0] = 0.125 + r*(0.25 + s*(-0.25 + 0.25*t) - 0.25*t) + s*s*(-0.125 + 0.125*t) +
        s*(-0.125 + 0.125*t)*t - 0.125*t*t;
        Hr[ 1] = -0.125 + r*(0.25 + s*(-0.25 + 0.25*t) - 0.25*t) + s*s*(0.125 - 0.125*t) +
        s*(0.125 - 0.125*t)*t + 0.125*t*t;
        Hr[ 2] = -0.125 + r*(0.25 + s*(0.25 - 0.25*t) - 0.25*t) + s*s*(0.125 - 0.125*t) +
        s*(-0.125 + 0.125*t)*t + 0.125*t*t;
        Hr[ 3] = 0.125 + r*(0.25 + s*(0.25 - 0.25*t) - 0.25*t) + s*s*(-0.125 + 0.125*t) +
        s*(0.125 - 0.125*t)*t - 0.125*t*t;
        Hr[ 4] = 0;
        Hr[ 5] = -0.5*r*(-1. + s)*(-1. + t);
        Hr[ 6] = 0.25*(-1. + s*s)*(-1. + t);
        Hr[ 7] = 0.5*r*(1. + s)*(-1. + t);
        Hr[ 8] = -0.25*(-1. + s*s)*(-1. + t);
        Hr[ 9] = -0.25*(-1. + s)*(-1. + t*t);
        Hr[10] = 0.25*(-1. + s)*(-1. + t*t);
        Hr[11] = -0.25*(1. + s)*(-1. + t*t);
        Hr[12] = 0.25*(1. + s)*(-1. + t*t);
        
        Hs[ 0] = 0.125 + s*(0.25 - 0.25*t) + r*r*(-0.125 + 0.125*t) - 0.125*t*t +
        r*(s*(-0.25 + 0.25*t) + (-0.125 + 0.125*t)*t);
        Hs[ 1] = 0.125 + s*(0.25 - 0.25*t) + r*r*(-0.125 + 0.125*t) - 0.125*t*t +
        r*(s*(0.25 - 0.25*t) + (0.125 - 0.125*t)*t);
        Hs[ 2] = -0.125 + s*(0.25 - 0.25*t) + r*r*(0.125 - 0.125*t) + 0.125*t*t +
        r*(s*(0.25 - 0.25*t) + (-0.125 + 0.125*t)*t);
        Hs[ 3] = -0.125 + s*(0.25 - 0.25*t) + r*r*(0.125 - 0.125*t) + 0.125*t*t +
        r*(s*(-0.25 + 0.25*t) + (0.125 - 0.125*t)*t);
        Hs[ 4] = 0;
        Hs[ 5] = -0.25*(-1. + r*r)*(-1. + t);
        Hs[ 6] = 0.5*(1. + r)*s*(-1. + t);
        Hs[ 7] = 0.25*(-1. + r*r)*(-1. + t);
        Hs[ 8] = -0.5*(-1. + r)*s*(-1. + t);
        Hs[ 9] = -0.25*(-1. + r)*(-1. + t*t);
        Hs[10] = 0.25*(1. + r)*(-1. + t*t);
        Hs[11] = -0.25*(1. + r)*(-1. + t*t);
        Hs[12] = 0.25*(-1. + r)*(-1. + t*t);
        
        Ht[ 0] = -0.125*(-1. + r)*(-1. + s) + 0.125*(-1. + r*r)*(-1. + s) +
        0.125*(-1. + r)*(-1. + s*s) + 0.25*(-1. + r)*(-1. + s)*t;
        Ht[ 1] = 0.125*(1. + r)*(-1. + s) + 0.125*(-1. + r*r)*(-1. + s) -
        0.125*(1. + r)*(-1. + s*s) - 0.25*(1. + r)*(-1. + s)*t;
        Ht[ 2] = -0.125*(1. + r)*(1. + s) - 0.125*(-1. + r*r)*(1. + s) -
        0.125*(1. + r)*(-1. + s*s) + 0.25*(1. + r)*(1. + s)*t;
        Ht[ 3] = 0.125*(-1. + r)*(1. + s) - 0.125*(-1. + r*r)*(1. + s) +
        0.125*(-1. + r)*(-1. + s*s) - 0.25*(-1. + r)*(1. + s)*t;
        Ht[ 4] = 0.5 + 1.*t;
        Ht[ 5] = -0.25*(-1. + r*r)*(-1. + s);
        Ht[ 6] = 0.25*(1. + r)*(-1. + s*s);
        Ht[ 7] = 0.25*(-1. + r*r)*(1. + s);
        Ht[ 8] = -0.25*(-1. + r)*(-1. + s*s);
        Ht[ 9] = -0.5*(-1. + r)*(-1. + s)*t;
        Ht[10] = 0.5*(1. + r)*(-1. + s)*t;
        Ht[11] = -0.5*(1. + r)*(1. + s)*t;
        Ht[12] = 0.5*(-1. + r)*(1. + s)*t;
    }
    
    // iso-parametric coordinates of nodes (for n = -1 return center coordinates)
    inline void iso_coord(int n, double q[3])
    {
        switch (n)
        {
            case -1: q[0] = 0.0; q[1] = 0.0; q[2] = 0.0; break;
            case 0: q[0] = -1.0; q[1] = -1.0; q[2] = -1.0; break;
            case 1: q[0] = 1.0; q[1] = -1.0; q[2] = -1.0; break;
            case 2: q[0] = 1.0; q[1] = 1.0; q[2] = -1.0; break;
            case 3: q[0] = -1.0; q[1] = 1.0; q[2] = -1.0; break;
            case 4: q[0] = 0.0; q[1] = 0.0; q[2] = 1.0; break;
            case 5: q[0] = 0.0; q[1] = -1.0; q[2] = -1.0; break;
            case 6: q[0] = 1.0; q[1] = 0.0; q[2] = -1.0; break;
            case 7: q[0] = 0.0; q[1] = 1.0; q[2] = -1.0; break;
            case 8: q[0] = -1.0; q[1] = 0.0; q[2] = -1.0; break;
            case 9: q[0] = -1.0; q[1] = -1.0; q[2] = 0.0; break;
            case 10: q[0] = 1.0; q[1] = -1.0; q[2] = 0.0; break;
            case 11: q[0] = 1.0; q[1] = 1.0; q[2] = 0.0; break;
            case 12: q[0] = -1.0; q[1] = 1.0; q[2] = 0.0; break;
        }
    }

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
        const double a = 1.0 / sqrt(3.0);
        gr[0] = -a; gs[0] = -a; gt[0] = -a; gw[0] = 1;
        gr[1] = a; gs[1] = -a; gt[1] = -a; gw[1] = 1;
        gr[2] = a; gs[2] = a; gt[2] = -a; gw[2] = 1;
        gr[3] = -a; gs[3] = a; gt[3] = -a; gw[3] = 1;
        gr[4] = -a; gs[4] = -a; gt[4] = a; gw[4] = 1;
        gr[5] = a; gs[5] = -a; gt[5] = a; gw[5] = 1;
        gr[6] = a; gs[6] = a; gt[6] = a; gw[6] = 1;
        gr[7] = -a; gs[7] = a; gt[7] = a; gw[7] = 1;
    }
    }
