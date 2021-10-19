/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "powell.h"

#define SQR(a) ((a)*(a))

//--------------------------------------------------------------------------------
// routine to find 1D minimum
// from Numerical Recipes in C, section 10.3, page 404-405
// modified for this application
//
#define SHFT(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);
#define SIGN(a,b) ((b)>=0.0 ? (a) : (-(a)))

double brent(double ax, double bx, double cx, double (*f)(double), double tol, double* xmin)
{
	const int ITMAX = 100;
	const double CGOLD = 0.3819660;
	const double ZEPS = 1.0e-10;
	int iter;
	double a,b,d=0,etemp,fu,fv,fw,fx,p,q,r,tol1,tol2,u,v,w,x,xm;
	double e=0.0;

	a = (ax < cx ? ax : cx);
	b = (ax > cx ? ax : cx);
	x=w=v=bx;
	fw=fv=fx=(*f)(x);
	for (iter=1;iter<=ITMAX;iter++)
	{
		xm = 0.5*(a+b);
		tol2=2.0*(tol1=tol*fabs(x)+ZEPS);
		if (fabs(x-xm) <= (tol2 - 0.5*(b-a)))
		{
			*xmin = x;
			return fx;
		}
		if (fabs(e) > tol1)
		{
			r = (x-w)*(fx-fv);
			q = (x-v)*(fx-fw);
			p = (x-v)*q - (x-w)*r;
			q = 2.0*(q-r);
			if (q > 0.0) p = -p;
			q = fabs(q);
			etemp = e;
			e=d;
			if (fabs(p)>=fabs(0.5*q*etemp)||p<=q*(a-x)||p>=q*(b-x))
				d = CGOLD*(e=(x >= xm ? a-x : b-x));
			else
			{
				d = p/q;
				u=x+d;
				if (u-a < tol2 || b-u < tol2) d=SIGN(tol1,xm-x);
			}
		}
		else d = CGOLD*(e=(x >= xm ? a-x : b-x));
		u = (fabs(d) >= tol1 ? x+d : x+SIGN(tol1,d));
		fu = (*f)(u);

		if (fu <= fx)
		{
			if (u >= x) a=x; else b=x;
			SHFT(v,w,x,u);
			SHFT(fv,fw,fx,fu);
		}
		else
		{
			if (u<x) a=u; else b=u;
			if (fu <= fw || w==x)
			{
				v=w;
				w=u;
				fv=fw;
				fw=fu;
			}
			else if (fu <= fv || v==x || v==w)
			{
				v=u;
				fv=fu;
			}
		}
	}

	fprintf(stderr, "ERROR : Too many iterations in brent routine\n");

	*xmin = x;
	return fx;
}

//--------------------------------------------------------------------------------
// routine for bracketing a minimum
// from Numerical Recipes in C, section 10.1, page 400-401
// modified for this application
//
#define FMAX(a,b) ((a)>(b) ? (a) : (b))

void mnbrak(double* ax, double* bx, double* cx, double* fa, double* fb, double* fc, double (*fnc)(double))
{
	double ulim, u, r, q, fu, dum;
	const double GOLD = 1.618034;
	const double GLIMIT = 100.0;
	const double TINY = 1.0e-20;

	*fa=(*fnc)(*ax);
	*fb=(*fnc)(*bx);
	if (*fb>*fa)
	{
		SHFT(dum,*ax,*bx,dum);
		SHFT(dum,*fb,*fa,dum);
	}
	*cx = (*bx)+GOLD*(*bx-*ax);
	*fc = (*fnc)(*cx);
	while (*fb > *fc)
	{
		r = (*bx-*ax)*(*fb-*fc);
		q = (*bx-*cx)*(*fb-*fa);
		u = (*bx) - ((*bx-*cx)*q-(*bx-*ax)*r)/(2.0*SIGN(FMAX(fabs(q-r),TINY),q-r));
		ulim = (*bx)+GLIMIT*(*cx-*bx);
		if ((*bx-u)*(u-*cx) > 0.0)
		{
			fu= (*fnc)(u);
			if (fu < *fc)
			{
				*ax = (*bx);
				*bx = u;
				*fa = (*fb);
				*fb = fu;
				return;
			}
			else if (fu > *fb)
			{
				*cx = u;
				*fc = fu;
				return;
			}
			u=(*cx)+GOLD*(*cx-*bx);
			fu = (*fnc)(u);
		}
		else if ((*cx-u)*(u-ulim) > 0.0)
		{
			fu=(*fnc)(u);
			if (fu < *fc)
			{
				SHFT(*bx,*cx,u,*cx+GOLD*(*cx-*bx));
				SHFT(*fb,*fc,fu,(*fnc)(u));
			}
		}
		else if ((u-ulim)*(ulim-*cx) >= 0.0)
		{
			u=ulim;
			fu=(*fnc)(u);
		}
		else
		{
			u=(*cx)+GOLD*(*cx-*bx);
			fu = (*fnc)(u);
		}
		SHFT(*ax,*bx,*cx,u);
		SHFT(*fa,*fb,*fc,fu);
	}
}
