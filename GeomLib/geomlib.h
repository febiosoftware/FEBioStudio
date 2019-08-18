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
