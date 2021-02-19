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

namespace HEX8
    {
    // shape functions
    inline void shape(double* H, double r, double s, double t)
    {
        H[0] = 0.125*(1 - r)*(1 - s)*(1 - t);
        H[1] = 0.125*(1 + r)*(1 - s)*(1 - t);
        H[2] = 0.125*(1 + r)*(1 + s)*(1 - t);
        H[3] = 0.125*(1 - r)*(1 + s)*(1 - t);
        H[4] = 0.125*(1 - r)*(1 - s)*(1 + t);
        H[5] = 0.125*(1 + r)*(1 - s)*(1 + t);
        H[6] = 0.125*(1 + r)*(1 + s)*(1 + t);
        H[7] = 0.125*(1 - r)*(1 + s)*(1 + t);
    }
    
    // shape function derivatives
    inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
    {
        Hr[0] = -0.125*(1 - s)*(1 - t); Hs[0] = -0.125*(1 - r)*(1 - t); Ht[0] = -0.125*(1 - r)*(1 - s);
        Hr[1] =  0.125*(1 - s)*(1 - t);    Hs[1] = -0.125*(1 + r)*(1 - t);    Ht[1] = -0.125*(1 + r)*(1 - s);
        Hr[2] =  0.125*(1 + s)*(1 - t);    Hs[2] =  0.125*(1 + r)*(1 - t);    Ht[2] = -0.125*(1 + r)*(1 + s);
        Hr[3] = -0.125*(1 + s)*(1 - t);    Hs[3] =  0.125*(1 - r)*(1 - t);    Ht[3] = -0.125*(1 - r)*(1 + s);
        Hr[4] = -0.125*(1 - s)*(1 + t);    Hs[4] = -0.125*(1 - r)*(1 + t);    Ht[4] =  0.125*(1 - r)*(1 - s);
        Hr[5] =  0.125*(1 - s)*(1 + t);    Hs[5] = -0.125*(1 + r)*(1 + t);    Ht[5] =  0.125*(1 + r)*(1 - s);
        Hr[6] =  0.125*(1 + s)*(1 + t);    Hs[6] =  0.125*(1 + r)*(1 + t);    Ht[6] =  0.125*(1 + r)*(1 + s);
        Hr[7] = -0.125*(1 + s)*(1 + t);    Hs[7] =  0.125*(1 - r)*(1 + t);    Ht[7] =  0.125*(1 - r)*(1 + s);
    }
    
    // iso-parametric coordinates of nodes (for n = -1 return center coordinates)
    inline void iso_coord(int n, double q[3])
    {
        switch (n)
        {
            case -1: q[0] = 0; q[1] = 0; q[2] = 0; break;
            case 0: q[0] = -1; q[1] = -1; q[2] = -1; break;
            case 1: q[0] = 1; q[1] = -1; q[2] = -1; break;
            case 2: q[0] = 1; q[1] = 1; q[2] = -1; break;
            case 3: q[0] = -1; q[1] = 1; q[2] = -1; break;
            case 4: q[0] = -1; q[1] = -1; q[2] = 1; break;
            case 5: q[0] = 1; q[1] = -1; q[2] = 1; break;
            case 6: q[0] = 1; q[1] = 1; q[2] = 1; break;
            case 7: q[0] = -1; q[1] = 1; q[2] = 1; break;
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

namespace HEX20
    {
    // shape functions
    inline void shape(double* H, double r, double s, double t)
    {
        H[8] = 0.25*(1 - r*r)*(1 - s)*(1 - t);
        H[9] = 0.25*(1 - s*s)*(1 + r)*(1 - t);
        H[10] = 0.25*(1 - r*r)*(1 + s)*(1 - t);
        H[11] = 0.25*(1 - s*s)*(1 - r)*(1 - t);
        H[12] = 0.25*(1 - r*r)*(1 - s)*(1 + t);
        H[13] = 0.25*(1 - s*s)*(1 + r)*(1 + t);
        H[14] = 0.25*(1 - r*r)*(1 + s)*(1 + t);
        H[15] = 0.25*(1 - s*s)*(1 - r)*(1 + t);
        H[16] = 0.25*(1 - t*t)*(1 - r)*(1 - s);
        H[17] = 0.25*(1 - t*t)*(1 + r)*(1 - s);
        H[18] = 0.25*(1 - t*t)*(1 + r)*(1 + s);
        H[19] = 0.25*(1 - t*t)*(1 - r)*(1 + s);
        
        H[0] = 0.125*(1 - r)*(1 - s)*(1 - t) - 0.5*(H[8] + H[11] + H[16]);
        H[1] = 0.125*(1 + r)*(1 - s)*(1 - t) - 0.5*(H[8] + H[9] + H[17]);
        H[2] = 0.125*(1 + r)*(1 + s)*(1 - t) - 0.5*(H[9] + H[10] + H[18]);
        H[3] = 0.125*(1 - r)*(1 + s)*(1 - t) - 0.5*(H[10] + H[11] + H[19]);
        H[4] = 0.125*(1 - r)*(1 - s)*(1 + t) - 0.5*(H[12] + H[15] + H[16]);
        H[5] = 0.125*(1 + r)*(1 - s)*(1 + t) - 0.5*(H[12] + H[13] + H[17]);
        H[6] = 0.125*(1 + r)*(1 + s)*(1 + t) - 0.5*(H[13] + H[14] + H[18]);
        H[7] = 0.125*(1 - r)*(1 + s)*(1 + t) - 0.5*(H[14] + H[15] + H[19]);
    }
    
    // shape function derivatives
    inline void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
    {
        Hr[ 8] = -0.5*r*(1 - s)*(1 - t);
        Hr[ 9] =  0.25*(1 - s*s)*(1 - t);
        Hr[10] = -0.5*r*(1 + s)*(1 - t);
        Hr[11] = -0.25*(1 - s*s)*(1 - t);
        Hr[12] = -0.5*r*(1 - s)*(1 + t);
        Hr[13] =  0.25*(1 - s*s)*(1 + t);
        Hr[14] = -0.5*r*(1 + s)*(1 + t);
        Hr[15] = -0.25*(1 - s*s)*(1 + t);
        Hr[16] = -0.25*(1 - t*t)*(1 - s);
        Hr[17] =  0.25*(1 - t*t)*(1 - s);
        Hr[18] =  0.25*(1 - t*t)*(1 + s);
        Hr[19] = -0.25*(1 - t*t)*(1 + s);
        
        Hr[0] = -0.125*(1 - s)*(1 - t) - 0.5*(Hr[ 8] + Hr[11] + Hr[16]);
        Hr[1] =  0.125*(1 - s)*(1 - t) - 0.5*(Hr[ 8] + Hr[ 9] + Hr[17]);
        Hr[2] =  0.125*(1 + s)*(1 - t) - 0.5*(Hr[ 9] + Hr[10] + Hr[18]);
        Hr[3] = -0.125*(1 + s)*(1 - t) - 0.5*(Hr[10] + Hr[11] + Hr[19]);
        Hr[4] = -0.125*(1 - s)*(1 + t) - 0.5*(Hr[12] + Hr[15] + Hr[16]);
        Hr[5] =  0.125*(1 - s)*(1 + t) - 0.5*(Hr[12] + Hr[13] + Hr[17]);
        Hr[6] =  0.125*(1 + s)*(1 + t) - 0.5*(Hr[13] + Hr[14] + Hr[18]);
        Hr[7] = -0.125*(1 + s)*(1 + t) - 0.5*(Hr[14] + Hr[15] + Hr[19]);
        
        Hs[ 8] = -0.25*(1 - r*r)*(1 - t);
        Hs[ 9] = -0.5*s*(1 + r)*(1 - t);
        Hs[10] = 0.25*(1 - r*r)*(1 - t);
        Hs[11] = -0.5*s*(1 - r)*(1 - t);
        Hs[12] = -0.25*(1 - r*r)*(1 + t);
        Hs[13] = -0.5*s*(1 + r)*(1 + t);
        Hs[14] = 0.25*(1 - r*r)*(1 + t);
        Hs[15] = -0.5*s*(1 - r)*(1 + t);
        Hs[16] = -0.25*(1 - t*t)*(1 - r);
        Hs[17] = -0.25*(1 - t*t)*(1 + r);
        Hs[18] =  0.25*(1 - t*t)*(1 + r);
        Hs[19] =  0.25*(1 - t*t)*(1 - r);
        
        Hs[0] = -0.125*(1 - r)*(1 - t) - 0.5*(Hs[ 8] + Hs[11] + Hs[16]);
        Hs[1] = -0.125*(1 + r)*(1 - t) - 0.5*(Hs[ 8] + Hs[ 9] + Hs[17]);
        Hs[2] =  0.125*(1 + r)*(1 - t) - 0.5*(Hs[ 9] + Hs[10] + Hs[18]);
        Hs[3] =  0.125*(1 - r)*(1 - t) - 0.5*(Hs[10] + Hs[11] + Hs[19]);
        Hs[4] = -0.125*(1 - r)*(1 + t) - 0.5*(Hs[12] + Hs[15] + Hs[16]);
        Hs[5] = -0.125*(1 + r)*(1 + t) - 0.5*(Hs[12] + Hs[13] + Hs[17]);
        Hs[6] =  0.125*(1 + r)*(1 + t) - 0.5*(Hs[13] + Hs[14] + Hs[18]);
        Hs[7] =  0.125*(1 - r)*(1 + t) - 0.5*(Hs[14] + Hs[15] + Hs[19]);
        
        Ht[ 8] = -0.25*(1 - r*r)*(1 - s);
        Ht[ 9] = -0.25*(1 - s*s)*(1 + r);
        Ht[10] = -0.25*(1 - r*r)*(1 + s);
        Ht[11] = -0.25*(1 - s*s)*(1 - r);
        Ht[12] =  0.25*(1 - r*r)*(1 - s);
        Ht[13] =  0.25*(1 - s*s)*(1 + r);
        Ht[14] =  0.25*(1 - r*r)*(1 + s);
        Ht[15] =  0.25*(1 - s*s)*(1 - r);
        Ht[16] = -0.5*t*(1 - r)*(1 - s);
        Ht[17] = -0.5*t*(1 + r)*(1 - s);
        Ht[18] = -0.5*t*(1 + r)*(1 + s);
        Ht[19] = -0.5*t*(1 - r)*(1 + s);
        
        Ht[0] = -0.125*(1 - r)*(1 - s) - 0.5*(Ht[ 8] + Ht[11] + Ht[16]);
        Ht[1] = -0.125*(1 + r)*(1 - s) - 0.5*(Ht[ 8] + Ht[ 9] + Ht[17]);
        Ht[2] = -0.125*(1 + r)*(1 + s) - 0.5*(Ht[ 9] + Ht[10] + Ht[18]);
        Ht[3] = -0.125*(1 - r)*(1 + s) - 0.5*(Ht[10] + Ht[11] + Ht[19]);
        Ht[4] =  0.125*(1 - r)*(1 - s) - 0.5*(Ht[12] + Ht[15] + Ht[16]);
        Ht[5] =  0.125*(1 + r)*(1 - s) - 0.5*(Ht[12] + Ht[13] + Ht[17]);
        Ht[6] =  0.125*(1 + r)*(1 + s) - 0.5*(Ht[13] + Ht[14] + Ht[18]);
        Ht[7] =  0.125*(1 - r)*(1 + s) - 0.5*(Ht[14] + Ht[15] + Ht[19]);
    }
    
    // iso-parametric coordinates of nodes (for n = -1 return center coordinates)
    inline void iso_coord(int n, double q[3])
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
    inline void iso_coord(int n, double q[3])
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

    // gauss coordinates and gauss weights
    inline void gauss_data(double *gr, double *gs, double *gt, double *gw)
    {
        // integration point coordinates
        const double a = 0.774596669241483;
        const double w1 = 5.0 / 9.0;
        const double w2 = 8.0 / 9.0;
        gr[ 0] = -a; gs[ 0] = -a; gt[ 0] = -a; gw[ 0] = w1*w1*w1;
        gr[ 1] =  0; gs[ 1] = -a; gt[ 1] = -a; gw[ 1] = w2*w1*w1;
        gr[ 2] =  a; gs[ 2] = -a; gt[ 2] = -a; gw[ 2] = w1*w1*w1;
        gr[ 3] = -a; gs[ 3] =  0; gt[ 3] = -a; gw[ 3] = w1*w2*w1;
        gr[ 4] =  0; gs[ 4] =  0; gt[ 4] = -a; gw[ 4] = w2*w2*w1;
        gr[ 5] =  a; gs[ 5] =  0; gt[ 5] = -a; gw[ 5] = w1*w2*w1;
        gr[ 6] = -a; gs[ 6] =  a; gt[ 6] = -a; gw[ 6] = w1*w1*w1;
        gr[ 7] =  0; gs[ 7] =  a; gt[ 7] = -a; gw[ 7] = w2*w1*w1;
        gr[ 8] =  a; gs[ 8] =  a; gt[ 8] = -a; gw[ 8] = w1*w1*w1;
        gr[ 9] = -a; gs[ 9] = -a; gt[ 9] =  0; gw[ 9] = w1*w1*w2;
        gr[10] =  0; gs[10] = -a; gt[10] =  0; gw[10] = w2*w1*w2;
        gr[11] =  a; gs[11] = -a; gt[11] =  0; gw[11] = w1*w1*w2;
        gr[12] = -a; gs[12] =  0; gt[12] =  0; gw[12] = w1*w2*w2;
        gr[13] =  0; gs[13] =  0; gt[13] =  0; gw[13] = w2*w2*w2;
        gr[14] =  a; gs[14] =  0; gt[14] =  0; gw[14] = w1*w2*w2;
        gr[15] = -a; gs[15] =  a; gt[15] =  0; gw[15] = w1*w1*w2;
        gr[16] =  0; gs[16] =  a; gt[16] =  0; gw[16] = w2*w1*w2;
        gr[17] =  a; gs[17] =  a; gt[17] =  0; gw[17] = w1*w1*w2;
        gr[18] = -a; gs[18] = -a; gt[18] =  a; gw[18] = w1*w1*w1;
        gr[19] =  0; gs[19] = -a; gt[19] =  a; gw[19] = w2*w1*w1;
        gr[20] =  a; gs[20] = -a; gt[20] =  a; gw[20] = w1*w1*w1;
        gr[21] = -a; gs[21] =  0; gt[21] =  a; gw[21] = w1*w2*w1;
        gr[22] =  0; gs[22] =  0; gt[22] =  a; gw[22] = w2*w2*w1;
        gr[23] =  a; gs[23] =  0; gt[23] =  a; gw[23] = w1*w2*w1;
        gr[24] = -a; gs[24] =  a; gt[24] =  a; gw[24] = w1*w1*w1;
        gr[25] =  0; gs[25] =  a; gt[25] =  a; gw[25] = w2*w1*w1;
        gr[26] =  a; gs[26] =  a; gt[26] =  a; gw[26] = w1*w1*w1;
        
    }
    }
