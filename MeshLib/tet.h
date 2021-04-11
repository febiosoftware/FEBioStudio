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

namespace TET4 {

// shape functions
inline void shape(double* H, double r, double s, double t)
{
    H[0] = 1.0 - r - s - t;
    H[1] = r;
    H[2] = s;
    H[3] = t;
}

// shape function derivatives
inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
{
    Hr[0] = -1; Hs[0] = -1; Ht[0] = -1;
    Hr[1] = 1;    Hs[1] = 0; Ht[1] = 0;
    Hr[2] = 0;    Hs[2] = 1; Ht[2] = 0;
    Hr[3] = 0;    Hs[3] = 0; Ht[3] = 1;
}

// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
inline void iso_coord(int n, double q[3])
{
    switch (n)
    {
        case -1: q[0] = 0.25; q[1] = 0.25; q[2] = 0.25; break;
        case 0: q[0] = 0; q[1] = 0; q[2] = 0; break;
        case 1: q[0] = 1; q[1] = 0; q[2] = 0; break;
        case 2: q[0] = 0; q[1] = 1; q[2] = 0; break;
        case 3: q[0] = 0; q[1] = 0; q[2] = 1; break;
    }
}

// gauss coordinates and gauss weights
inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
{
    // gaussian integration for tetrahedral elements
    const double a = 0.58541020;
    const double b = 0.13819660;
    const double w = 1.0 / 24.0;
    
    gr[0] = b; gs[0] = b; gt[0] = b; gw[0] = w;
    gr[1] = a; gs[1] = b; gt[1] = b; gw[1] = w;
    gr[2] = b; gs[2] = a; gt[2] = b; gw[2] = w;
    gr[3] = b; gs[3] = b; gt[3] = a; gw[3] = w;
}
}

namespace TET5 {

// shape functions
inline void shape(double* H, double r, double s, double t)
{
    H[0] = 1.0 - r - s - t;
    H[1] = r;
    H[2] = s;
    H[3] = t;
    H[4] = 256.0*H[0] * H[1] * H[2] * H[3];
    
    H[0] -= 0.25*H[4];
    H[1] -= 0.25*H[4];
    H[2] -= 0.25*H[4];
    H[3] -= 0.25*H[4];
}

// shape function derivatives
inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
{
    Hr[0] = -1; Hs[0] = -1; Ht[0] = -1;
    Hr[1] = 1;    Hs[1] = 0; Ht[1] = 0;
    Hr[2] = 0;    Hs[2] = 1; Ht[2] = 0;
    Hr[3] = 0;    Hs[3] = 0; Ht[3] = 1;
    
    Hr[4] = 256.0*(s*t*(1 - r - s - t) - r*s*t);
    Hs[4] = 256.0*(r*t*(1 - r - s - t) - r*s*t);
    Ht[4] = 256.0*(r*s*(1 - r - s - t) - r*s*t);
    
    Hr[0] -= 0.25*Hr[4]; Hr[1] -= 0.25*Hr[4]; Hr[2] -= 0.25*Hr[4]; Hr[3] -= 0.25*Hr[4];
    Hs[0] -= 0.25*Hs[4]; Hs[1] -= 0.25*Hs[4]; Hs[2] -= 0.25*Hs[4]; Hs[3] -= 0.25*Hs[4];
    Ht[0] -= 0.25*Ht[4]; Ht[1] -= 0.25*Ht[4]; Ht[2] -= 0.25*Ht[4]; Ht[3] -= 0.25*Ht[4];
}

// iso-parametric coordinates of nodes (for n = -1 return center coordinates)
inline void iso_coord(int n, double q[3])
{
    switch (n)
    {
        case -1: q[0] = 0.25; q[1] = 0.25; q[2] = 0.25; break;
        case 0: q[0] = 0; q[1] = 0; q[2] = 0; break;
        case 1: q[0] = 1; q[1] = 0; q[2] = 0; break;
        case 2: q[0] = 0; q[1] = 1; q[2] = 0; break;
        case 3: q[0] = 0; q[1] = 0; q[2] = 1; break;
        case 4: q[0] = 0.25; q[1] = 0.25; q[2] = 0.25; break;
    }
}

// gauss coordinates and gauss weights
inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
{
    // gaussian integration for tetrahedral elements
    const double a = 0.58541020;
    const double b = 0.13819660;
    const double w = 1.0 / 24.0;
    
    gr[0] = b; gs[0] = b; gt[0] = b; gw[0] = w;
    gr[1] = a; gs[1] = b; gt[1] = b; gw[1] = w;
    gr[2] = b; gs[2] = a; gt[2] = b; gw[2] = w;
    gr[3] = b; gs[3] = b; gt[3] = a; gw[3] = w;
}
}

namespace TET10
    {
    // shape functions
    inline void shape(double* H, double r, double s, double t)
    {
        double r1 = 1.0 - r - s - t;
        double r2 = r;
        double r3 = s;
        double r4 = t;
        
        H[0] = r1*(2.0*r1 - 1.0);
        H[1] = r2*(2.0*r2 - 1.0);
        H[2] = r3*(2.0*r3 - 1.0);
        H[3] = r4*(2.0*r4 - 1.0);
        H[4] = 4.0*r1*r2;
        H[5] = 4.0*r2*r3;
        H[6] = 4.0*r3*r1;
        H[7] = 4.0*r1*r4;
        H[8] = 4.0*r2*r4;
        H[9] = 4.0*r3*r4;
    }
    
    // shape function derivatives
    inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
    {
        Hr[0] = -3.0 + 4.0*r + 4.0*(s + t);
        Hr[1] =  4.0*r - 1.0;
        Hr[2] =  0.0;
        Hr[3] =  0.0;
        Hr[4] =  4.0 - 8.0*r - 4.0*(s + t);
        Hr[5] =  4.0*s;
        Hr[6] = -4.0*s;
        Hr[7] = -4.0*t;
        Hr[8] =  4.0*t;
        Hr[9] =  0.0;
        
        Hs[0] = -3.0 + 4.0*s + 4.0*(r + t);
        Hs[1] =  0.0;
        Hs[2] =  4.0*s - 1.0;
        Hs[3] =  0.0;
        Hs[4] = -4.0*r;
        Hs[5] =  4.0*r;
        Hs[6] =  4.0 - 8.0*s - 4.0*(r + t);
        Hs[7] = -4.0*t;
        Hs[8] =  0.0;
        Hs[9] =  4.0*t;
        
        Ht[0] = -3.0 + 4.0*t + 4.0*(r + s);
        Ht[1] =  0.0;
        Ht[2] =  0.0;
        Ht[3] =  4.0*t - 1.0;
        Ht[4] = -4.0*r;
        Ht[5] =  0.0;
        Ht[6] = -4.0*s;
        Ht[7] =  4.0 - 8.0*t - 4.0*(r + s);
        Ht[8] =  4.0*r;
        Ht[9] =  4.0*s;
    }
    
    // iso-parametric coordinates of nodes (for n = -1 return center coordinates)
    inline void iso_coord(int n, double q[3])
    {
        switch (n)
        {
            case -1: q[0] = 0.25; q[1] = 0.25; q[2] = 0.25; break;
            case 0: q[0] = 0; q[1] = 0; q[2] = 0; break;
            case 1: q[0] = 1; q[1] = 0; q[2] = 0; break;
            case 2: q[0] = 0; q[1] = 1; q[2] = 0; break;
            case 3: q[0] = 0; q[1] = 0; q[2] = 1; break;
            case 4: q[0] = 0.5; q[1] = 0.0; q[2] = 0.0; break;
            case 5: q[0] = 0.5; q[1] = 0.5; q[2] = 0.0; break;
            case 6: q[0] = 0.0; q[1] = 0.5; q[2] = 0.0; break;
            case 7: q[0] = 0.0; q[1] = 0.0; q[2] = 0.5; break;
            case 8: q[0] = 0.5; q[1] = 0.0; q[2] = 0.5; break;
            case 9: q[0] = 0.0; q[1] = 0.5; q[2] = 0.5; break;
        }
    }

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
        // gaussian integration for tetrahedral elements
        const double w = 1.0/6.0;
        gr[0] = 0.0158359099; gs[0] = 0.3280546970; gt[0] = 0.3280546970; gw[0] = 0.138527967*w;
        gr[1] = 0.3280546970; gs[1] = 0.0158359099; gt[1] = 0.3280546970; gw[1] = 0.138527967*w;
        gr[2] = 0.3280546970; gs[2] = 0.3280546970; gt[2] = 0.0158359099; gw[2] = 0.138527967*w;
        gr[3] = 0.3280546970; gs[3] = 0.3280546970; gt[3] = 0.3280546970; gw[3] = 0.138527967*w;
        gr[4] = 0.6791431780; gs[4] = 0.1069522740; gt[4] = 0.1069522740; gw[4] = 0.111472033*w;
        gr[5] = 0.1069522740; gs[5] = 0.6791431780; gt[5] = 0.1069522740; gw[5] = 0.111472033*w;
        gr[6] = 0.1069522740; gs[6] = 0.1069522740; gt[6] = 0.6791431780; gw[6] = 0.111472033*w;
        gr[7] = 0.1069522740; gs[7] = 0.1069522740; gt[7] = 0.1069522740; gw[7] = 0.111472033*w;
    }
    }

namespace TET15
    {
    // shape functions
    inline void shape(double* H, double r, double s, double t)
    {
        double r1 = 1.0 - r - s - t;
        double r2 = r;
        double r3 = s;
        double r4 = t;
        
        H[14] = 256 * r1*r2*r3*r4;
        
        H[10] = 27.0*r1*r2*r3;
        H[11] = 27.0*r1*r2*r4;
        H[12] = 27.0*r2*r3*r4;
        H[13] = 27.0*r3*r1*r4;
        
        H[0] = r1*(2.0*r1 - 1.0) + (H[10] + H[11] + H[13]) / 9.0 - H[14] / 64.0;
        H[1] = r2*(2.0*r2 - 1.0) + (H[10] + H[11] + H[12]) / 9.0 - H[14] / 64.0;
        H[2] = r3*(2.0*r3 - 1.0) + (H[10] + H[12] + H[13]) / 9.0 - H[14] / 64.0;
        H[3] = r4*(2.0*r4 - 1.0) + (H[11] + H[12] + H[13]) / 9.0 - H[14] / 64.0;
        
        H[4] = 4.0*r1*r2 - 4.0*(H[10] + H[11]) / 9.0 + H[14] / 8.0;
        H[5] = 4.0*r2*r3 - 4.0*(H[10] + H[12]) / 9.0 + H[14] / 8.0;
        H[6] = 4.0*r3*r1 - 4.0*(H[10] + H[13]) / 9.0 + H[14] / 8.0;
        H[7] = 4.0*r1*r4 - 4.0*(H[11] + H[13]) / 9.0 + H[14] / 8.0;
        H[8] = 4.0*r2*r4 - 4.0*(H[11] + H[12]) / 9.0 + H[14] / 8.0;
        H[9] = 4.0*r3*r4 - 4.0*(H[12] + H[13]) / 9.0 + H[14] / 8.0;
        
        H[10] -= 27.0*H[14] / 64.0;
        H[11] -= 27.0*H[14] / 64.0;
        H[12] -= 27.0*H[14] / 64.0;
        H[13] -= 27.0*H[14] / 64.0;
    }
    
    // shape function derivatives
    inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
    {
        Hr[14] = 256.0*s*t*(1.0 - 2.0*r - s - t);
        Hs[14] = 256.0*r*t*(1.0 - r - 2.0*s - t);
        Ht[14] = 256.0*r*s*(1.0 - r - s - 2.0*t);
        
        Hr[10] =  27.0*s*(1.0 - 2.0*r - s - t);
        Hr[11] =  27.0*t*(1.0 - 2.0*r - s - t);
        Hr[12] =  27.0*s*t;
        Hr[13] = -27.0*s*t;
        
        Hs[10] =  27.0*r*(1.0 - r - 2.0*s - t);
        Hs[11] = -27.0*r*t;
        Hs[12] =  27.0*r*t;
        Hs[13] =  27.0*t*(1.0 - r - 2.0*s - t);
        
        Ht[10] = -27.0*r*s;
        Ht[11] =  27.0*r*(1.0 - r - s - 2.0*t);
        Ht[12] =  27.0*r*s;
        Ht[13] =  27.0*s*(1.0 - r - s - 2.0*t);
        
        Hr[0] = -3.0 + 4.0*r + 4.0*(s + t) + (Hr[10] + Hr[11] + Hr[13])/9.0 - Hr[14]/64.0;
        Hr[1] =  4.0*r - 1.0               + (Hr[10] + Hr[11] + Hr[12])/9.0 - Hr[14]/64.0;
        Hr[2] =  0.0                       + (Hr[10] + Hr[12] + Hr[13])/9.0 - Hr[14]/64.0;
        Hr[3] =  0.0                       + (Hr[11] + Hr[12] + Hr[13])/9.0 - Hr[14]/64.0;
        Hr[4] =  4.0 - 8.0*r - 4.0*(s + t) - 4.0*(Hr[10] + Hr[11])/9.0 + Hr[14]/8.0;
        Hr[5] =  4.0*s                       - 4.0*(Hr[10] + Hr[12])/9.0 + Hr[14]/8.0;
        Hr[6] = -4.0*s                       - 4.0*(Hr[10] + Hr[13])/9.0 + Hr[14]/8.0;
        Hr[7] = -4.0*t                       - 4.0*(Hr[11] + Hr[13])/9.0 + Hr[14]/8.0;
        Hr[8] =  4.0*t                       - 4.0*(Hr[11] + Hr[12])/9.0 + Hr[14]/8.0;
        Hr[9] =  0.0                       - 4.0*(Hr[12] + Hr[13])/9.0 + Hr[14]/8.0;
        
        Hs[0] = -3.0 + 4.0*s + 4.0*(r + t) + (Hs[10] + Hs[11] + Hs[13])/9.0 - Hs[14]/64.0;
        Hs[1] =  0.0                       + (Hs[10] + Hs[11] + Hs[12])/9.0 - Hs[14]/64.0;
        Hs[2] =  4.0*s - 1.0               + (Hs[10] + Hs[12] + Hs[13])/9.0 - Hs[14]/64.0;
        Hs[3] =  0.0                       + (Hs[11] + Hs[12] + Hs[13])/9.0 - Hs[14]/64.0;
        Hs[4] = -4.0*r                       - 4.0*(Hs[10] + Hs[11])/9.0 + Hs[14]/8.0;
        Hs[5] =  4.0*r                       - 4.0*(Hs[10] + Hs[12])/9.0 + Hs[14]/8.0;
        Hs[6] =  4.0 - 8.0*s - 4.0*(r + t) - 4.0*(Hs[10] + Hs[13])/9.0 + Hs[14]/8.0;
        Hs[7] = -4.0*t                       - 4.0*(Hs[11] + Hs[13])/9.0 + Hs[14]/8.0;
        Hs[8] =  0.0                       - 4.0*(Hs[11] + Hs[12])/9.0 + Hs[14]/8.0;
        Hs[9] =  4.0*t                       - 4.0*(Hs[12] + Hs[13])/9.0 + Hs[14]/8.0;
        
        Ht[0] = -3.0 + 4.0*t + 4.0*(r + s) + (Ht[10] + Ht[11] + Ht[13])/9.0 - Ht[14]/64.0;
        Ht[1] =  0.0                       + (Ht[10] + Ht[11] + Ht[12])/9.0 - Ht[14]/64.0;
        Ht[2] =  0.0                       + (Ht[10] + Ht[12] + Ht[13])/9.0 - Ht[14]/64.0;
        Ht[3] =  4.0*t - 1.0               + (Ht[11] + Ht[12] + Ht[13])/9.0 - Ht[14]/64.0;
        Ht[4] = -4.0*r                       - 4.0*(Ht[10] + Ht[11])/9.0 + Ht[14]/8.0;
        Ht[5] =  0.0                       - 4.0*(Ht[10] + Ht[12])/9.0 + Ht[14]/8.0;
        Ht[6] = -4.0*s                       - 4.0*(Ht[10] + Ht[13])/9.0 + Ht[14]/8.0;
        Ht[7] =  4.0 - 8.0*t - 4.0*(r + s) - 4.0*(Ht[11] + Ht[13])/9.0 + Ht[14]/8.0;
        Ht[8] =  4.0*r                       - 4.0*(Ht[11] + Ht[12])/9.0 + Ht[14]/8.0;
        Ht[9] =  4.0*s                       - 4.0*(Ht[12] + Ht[13])/9.0 + Ht[14]/8.0;
        
        Hr[10] -= 27.0*Hr[14]/64.0;
        Hr[11] -= 27.0*Hr[14]/64.0;
        Hr[12] -= 27.0*Hr[14]/64.0;
        Hr[13] -= 27.0*Hr[14]/64.0;
        
        Hs[10] -= 27.0*Hs[14]/64.0;
        Hs[11] -= 27.0*Hs[14]/64.0;
        Hs[12] -= 27.0*Hs[14]/64.0;
        Hs[13] -= 27.0*Hs[14]/64.0;
        
        Ht[10] -= 27.0*Ht[14]/64.0;
        Ht[11] -= 27.0*Ht[14]/64.0;
        Ht[12] -= 27.0*Ht[14]/64.0;
        Ht[13] -= 27.0*Ht[14]/64.0;
    }
    
    // iso-parametric coordinates of nodes (for n = -1 return center coordinates)
    inline void iso_coord(int n, double q[3])
    {
        const double t = 1.0 / 3.0;
        switch (n)
        {
            case -1: q[0] = 0.25; q[1] = 0.25; q[2] = 0.25; break;
            case 0: q[0] = 0; q[1] = 0; q[2] = 0; break;
            case 1: q[0] = 1; q[1] = 0; q[2] = 0; break;
            case 2: q[0] = 0; q[1] = 1; q[2] = 0; break;
            case 3: q[0] = 0; q[1] = 0; q[2] = 1; break;
            case 4: q[0] = 0.5; q[1] = 0.0; q[2] = 0.0; break;
            case 5: q[0] = 0.5; q[1] = 0.5; q[2] = 0.0; break;
            case 6: q[0] = 0.0; q[1] = 0.5; q[2] = 0.0; break;
            case 7: q[0] = 0.0; q[1] = 0.0; q[2] = 0.5; break;
            case 8: q[0] = 0.5; q[1] = 0.0; q[2] = 0.5; break;
            case 9: q[0] = 0.0; q[1] = 0.5; q[2] = 0.5; break;
            case 10: q[0] = t; q[1] = t; q[2] = 0; break;
            case 11: q[0] = t; q[1] = 0; q[2] = t; break;
            case 12: q[0] = t; q[1] = t; q[2] = t; break;
            case 13: q[0] = 0; q[1] = t; q[2] = t; break;
            case 14: q[0] = 0.25; q[1] = 0.25; q[2] = 0.25; break;
        }
    }

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
        // gaussian integration for tetrahedral elements
        const double w = 1.0/6.0;
        gr[0] = 0.0158359099; gs[0] = 0.3280546970; gt[0] = 0.3280546970; gw[0] = 0.138527967*w;
        gr[1] = 0.3280546970; gs[1] = 0.0158359099; gt[1] = 0.3280546970; gw[1] = 0.138527967*w;
        gr[2] = 0.3280546970; gs[2] = 0.3280546970; gt[2] = 0.0158359099; gw[2] = 0.138527967*w;
        gr[3] = 0.3280546970; gs[3] = 0.3280546970; gt[3] = 0.3280546970; gw[3] = 0.138527967*w;
        gr[4] = 0.6791431780; gs[4] = 0.1069522740; gt[4] = 0.1069522740; gw[4] = 0.111472033*w;
        gr[5] = 0.1069522740; gs[5] = 0.6791431780; gt[5] = 0.1069522740; gw[5] = 0.111472033*w;
        gr[6] = 0.1069522740; gs[6] = 0.1069522740; gt[6] = 0.6791431780; gw[6] = 0.111472033*w;
        gr[7] = 0.1069522740; gs[7] = 0.1069522740; gt[7] = 0.1069522740; gw[7] = 0.111472033*w;
    }
    }

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
        double L1 = 1.0 - r - s - t;
        double L2 = r;
        double L3 = s;
        double L4 = t;
        
        Hr[0] = -3. / 2.*(3 * L1 - 2)*L1 - 3. / 2.*(3 * L1 - 1)*L1 - 0.5*(3 * L1 - 1)*(3 * L1 - 2);
        Hr[1] = 3. / 2.*(3 * L2 - 2)*L2 + 3. / 2.*(3 * L2 - 1)*L2 + 0.5*(3 * L2 - 1)*(3 * L2 - 2);
        Hr[2] = 0.0;
        Hr[3] = 0.0;
        Hr[4] = -27. / 2.*L1*L2 - 9.0 / 2.0*(3 * L1 - 1)*L2 + 9.0 / 2.0*(3 * L1 - 1)*L1;
        Hr[5] = 27. / 2.*L1*L2 - 9.0 / 2.0*(3 * L2 - 1)*L2 + 9.0 / 2.0*(3 * L2 - 1)*L1;
        Hr[6] = 27. / 2.*L2*L3 + 9.0 / 2.0*(3 * L2 - 1)*L3;
        Hr[7] = 9.0 / 2.0*(3 * L3 - 1)*L3;
        Hr[8] = -27. / 2.*L1*L3 - 9.0 / 2.0*(3 * L1 - 1)*L3;
        Hr[9] = -9.0 / 2.0*(3 * L3 - 1)*L3;
        Hr[10] = -27. / 2.*L1*L4 - 9.0 / 2.0*(3 * L1 - 1)*L4;
        Hr[11] = -9. / 2.*(3 * L4 - 1)*L4;
        Hr[12] = 27. / 2.*L2*L4 + 9. / 2.*(3 * L2 - 1)*L4;
        Hr[13] = 9. / 2.*(3 * L4 - 1)*L4;
        Hr[14] = 0.0;
        Hr[15] = 0.0;
        Hr[16] = -27 * L2*L4 + 27 * L1*L4;
        Hr[17] = 27 * L3*L4;
        Hr[18] = -27 * L3*L4;
        Hr[19] = -27 * L2*L3 + 27 * L1*L3;
        
        Hs[0] = -3. / 2.*(3 * L1 - 2)*L1 - 3. / 2.*(3 * L1 - 1)*L1 - 0.5*(3 * L1 - 1)*(3 * L1 - 2);
        Hs[1] = 0.0;
        Hs[2] = 3. / 2.*(3 * L3 - 2)*L3 + 3. / 2.*(3 * L3 - 1)*L3 + 0.5*(3 * L3 - 1)*(3 * L3 - 2);
        Hs[3] = 0.0;
        Hs[4] = -27. / 2.*L1*L2 - 9. / 2.*(3 * L1 - 1)*L2;
        Hs[5] = -9. / 2.*(3 * L2 - 1)*L2;
        Hs[6] = 9. / 2.*(3 * L2 - 1)*L2;
        Hs[7] = 27. / 2.*L2*L3 + 9. / 2.*(3 * L3 - 1)*L2;
        Hs[8] = -27. / 2.*L1*L3 - 9. / 2.*(3 * L1 - 1)*L3 + 9. / 2.*(3 * L1 - 1)*L1;
        Hs[9] = 27. / 2.*L1*L3 - 9. / 2.*(3 * L3 - 1)*L3 + 9. / 2.*(3 * L3 - 1)*L1;
        Hs[10] = -27. / 2.*L1*L4 - 9. / 2.*(3 * L1 - 1)*L4;
        Hs[11] = -9. / 2.*(3 * L4 - 1)*L4;
        Hs[12] = 0.0;
        Hs[13] = 0.0;
        Hs[14] = 27. / 2.*L3*L4 + 9. / 2.*(3 * L3 - 1)*L4;
        Hs[15] = 9. / 2.*(3 * L4 - 1)*L4;
        Hs[16] = -27 * L2*L4;
        Hs[17] = 27 * L2*L4;
        Hs[18] = -27 * L3*L4 + 27 * L1*L4;
        Hs[19] = -27 * L2*L3 + 27 * L1*L2;
        
        Ht[0] = -3. / 2.*(3 * L1 - 2)*L1 - 3. / 2.*(3 * L1 - 1)*L1 - 0.5*(3 * L1 - 1)*(3 * L1 - 2);
        Ht[1] = 0.0;
        Ht[2] = 0.0;
        Ht[3] = 3. / 2.*(3 * L4 - 2)*L4 + 3. / 2.*(3 * L4 - 1)*L4 + 0.5*(3 * L4 - 1)*(3 * L4 - 2);
        Ht[4] = -27. / 2.*L1*L2 - 9. / 2.*(3 * L1 - 1)*L2;
        Ht[5] = -9. / 2.*(3 * L2 - 1)*L2;
        Ht[6] = 0.0;
        Ht[7] = 0.0;
        Ht[8] = -27. / 2.*L1*L3 - 9. / 2.*(3 * L1 - 1)*L3;
        Ht[9] = -9. / 2.*(3 * L3 - 1)*L3;
        Ht[10] = -27. / 2.*L1*L4 - 9. / 2.*(3 * L1 - 1)*L4 + 9. / 2.*(3 * L1 - 1)*L1;
        Ht[11] = 27. / 2.*L1*L4 - 9. / 2.*(3 * L4 - 1)*L4 + 9. / 2.*(3 * L4 - 1)*L1;
        Ht[12] = 9. / 2.*(3 * L2 - 1)*L2;
        Ht[13] = 27. / 2.*L2*L4 + 9. / 2.*(3 * L4 - 1)*L2;
        Ht[14] = 9. / 2.*(3 * L3 - 1)*L3;
        Ht[15] = 27. / 2.*L3*L4 + 9. / 2.*(3 * L4 - 1)*L3;
        Ht[16] = -27 * L2*L4 + 27 * L1*L2;
        Ht[17] = 27 * L2*L3;
        Ht[18] = -27 * L3*L4 + 27 * L1*L3;
        Ht[19] = -27 * L2*L3;
    }
    
    // iso-parametric coordinates of nodes (for n = -1 return center coordinates)
    inline void iso_coord(int n, double q[3])
    {
        // TODO: Implement this
    }

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
        // gaussian integration for tetrahedral elements
        gr[0] = 0.25; gs[0] = 0.25; gt[0] = 0.25; gw[0] = 0.030283678097089;
        
        gr[1] = 0.333333333333333; gs[1] = 0.333333333333333; gt[1] = 0.333333333333333; gw[1] = 0.006026785714286;
        gr[2] = 0.000000000000000; gs[2] = 0.333333333333333; gt[2] = 0.333333333333333; gw[2] = 0.006026785714286;
        gr[3] = 0.333333333333333; gs[3] = 0.000000000000000; gt[3] = 0.333333333333333; gw[3] = 0.006026785714286;
        gr[4] = 0.333333333333333; gs[4] = 0.333333333333333; gt[4] = 0.000000000000000; gw[4] = 0.006026785714286;
        
        gr[5] = 0.090909090909091; gs[5] = 0.090909090909091; gt[5] = 0.090909090909091; gw[5] = 0.011645249086029;
        gr[6] = 0.727272727272727; gs[6] = 0.090909090909091; gt[6] = 0.090909090909091; gw[6] = 0.011645249086029;
        gr[7] = 0.090909090909091; gs[7] = 0.727272727272727; gt[7] = 0.090909090909091; gw[7] = 0.011645249086029;
        gr[8] = 0.090909090909091; gs[8] = 0.090909090909091; gt[8] = 0.727272727272727; gw[8] = 0.011645249086029;
        
        gr[9] = 0.433449846426336; gs[9] = 0.066550153573664; gt[9] = 0.066550153573664; gw[9] = 0.010949141561386;
        gr[10] = 0.066550153573664; gs[10] = 0.433449846426336; gt[10] = 0.066550153573664; gw[10] = 0.010949141561386;
        gr[11] = 0.066550153573664; gs[11] = 0.066550153573664; gt[11] = 0.433449846426336; gw[11] = 0.010949141561386;
        gr[12] = 0.066550153573664; gs[12] = 0.433449846426336; gt[12] = 0.433449846426336; gw[12] = 0.010949141561386;
        gr[13] = 0.433449846426336; gs[13] = 0.066550153573664; gt[13] = 0.433449846426336; gw[13] = 0.010949141561386;
        gr[14] = 0.433449846426336; gs[14] = 0.433449846426336; gt[14] = 0.066550153573664; gw[14] = 0.010949141561386;
    }
    }
