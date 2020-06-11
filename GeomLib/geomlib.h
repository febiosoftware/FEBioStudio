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
#include <assert.h>

//-----------------------------------------------------------------------------
// Structure for representing 2D POINT2Ds
struct POINT2D
{
	double x, y;
};

//-----------------------------------------------------------------------------
POINT2D operator - (POINT2D a, POINT2D b);
double operator * (POINT2D a, POINT2D b);

//-----------------------------------------------------------------------------
double Area2(POINT2D a, POINT2D b, POINT2D c);
int AreaSign(POINT2D a, POINT2D b, POINT2D c, const double eps);
bool Between(POINT2D a, POINT2D b, POINT2D c);
int ParallelInt(POINT2D a, POINT2D b, POINT2D c, POINT2D d, POINT2D& p, double eps);
int SegSegInt(POINT2D a, POINT2D b, POINT2D c, POINT2D d, POINT2D& p, double eps);

//-----------------------------------------------------------------------------
int ConvexIntersect(POINT2D* P, int n, POINT2D* Q, int m, POINT2D* R);

//-----------------------------------------------------------------------------
int ConvexIntersectSH(POINT2D* P, int n, POINT2D* Q, int m, POINT2D* R);

//-----------------------------------------------------------------------------
bool PointInConvexPoly(POINT2D p, POINT2D* P, int n);

//-----------------------------------------------------------------------------
bool ConvexPolyInConvexPoly(POINT2D* P, int n, POINT2D* Q, int m);
