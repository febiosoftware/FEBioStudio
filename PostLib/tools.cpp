#include "stdafx.h"
#include "tools.h"

//-----------------------------------------------------------------------------
// project point x onto the triangle defined by y
bool ProjectToTriangle(vec3f* y, vec3f& x, vec3f& q, double tol)
{
	// calculate base vectors 
	vec3f e1 = y[1] - y[0];
	vec3f e2 = y[2] - y[0];

	// calculate plane normal
	vec3f n = e1^e2; n.Normalize();

	// project x onto the plane
	q = x - n*((x - y[0])*n);

	// set up metric tensor
	double G[2][2];
	G[0][0] = e1*e1;
	G[0][1] = G[1][0] = e1*e2;
	G[1][1] = e2*e2;

	// invert metric tensor
	double D = G[0][0] * G[1][1] - G[0][1] * G[1][0];
	double Gi[2][2];
	Gi[0][0] = G[1][1] / D;
	Gi[1][1] = G[0][0] / D;
	Gi[0][1] = Gi[1][0] = -G[0][1] / D;

	// calculate dual base vectors
	vec3f E1 = e1*Gi[0][0] + e2*Gi[0][1];
	vec3f E2 = e1*Gi[1][0] + e2*Gi[1][1];

	// now we can calculate r and s
	vec3f t = q - y[0];
	double r = t*E1;
	double s = t*E2;

	return ((r >= -tol) && (s >= -tol) && (r + s <= 1.0 + tol));
}

//-----------------------------------------------------------------------------
// project onto a quadrilateral surface.
bool ProjectToQuad(vec3f* y, vec3f& x, vec3f& q, double tol)
{
	double R[2], u[2], D;
	double gr[4] = { -1, +1, +1, -1 };
	double gs[4] = { -1, -1, +1, +1 };
	double H[4], Hr[4], Hs[4], Hrs[4];

	int i, j;
	int NMAX = 50, n = 0;

	// evaulate scalar products
	double xy[4] = { x*y[0], x*y[1], x*y[2], x*y[3] };
	double yy[4][4];
	yy[0][0] = y[0] * y[0]; yy[1][1] = y[1] * y[1]; yy[2][2] = y[2] * y[2]; yy[3][3] = y[3] * y[3];
	yy[0][1] = yy[1][0] = y[0] * y[1];
	yy[0][2] = yy[2][0] = y[0] * y[2];
	yy[0][3] = yy[3][0] = y[0] * y[3];
	yy[1][2] = yy[2][1] = y[1] * y[2];
	yy[1][3] = yy[3][1] = y[1] * y[3];
	yy[2][3] = yy[3][2] = y[2] * y[3];

	double r = 0, s = 0;

	// loop until converged
	bool bconv = false;
	double normu;
	do
	{
		// evaluate shape functions and shape function derivatives.
		for (i = 0; i<4; ++i)
		{
			H[i] = 0.25*(1 + gr[i] * r)*(1 + gs[i] * s);

			Hr[i] = 0.25*gr[i] * (1 + gs[i] * s);
			Hs[i] = 0.25*gs[i] * (1 + gr[i] * r);

			Hrs[i] = 0.25*gr[i] * gs[i];
		}

		// set up the system of equations
		R[0] = R[1] = 0;
		double A[2][2] = { 0 };
		for (i = 0; i<4; ++i)
		{
			R[0] -= (xy[i])*Hr[i];
			R[1] -= (xy[i])*Hs[i];

			A[0][1] += (xy[i])*Hrs[i];
			A[1][0] += (xy[i])*Hrs[i];

			for (j = 0; j<4; ++j)
			{
				double yij = yy[i][j];
				R[0] -= -H[j] * Hr[i] * (yij);
				R[1] -= -H[j] * Hs[i] * (yij);

				A[0][0] -= (yij)*(Hr[i] * Hr[j]);
				A[1][1] -= (yij)*(Hs[i] * Hs[j]);

				A[0][1] -= (yij)*(Hr[i] * Hs[j] + Hrs[i] * H[j]);
				A[1][0] -= (yij)*(Hs[i] * Hr[j] + Hrs[i] * H[j]);
			}
		}

		// determinant of A
		D = A[0][0] * A[1][1] - A[0][1] * A[1][0];

		// solve for u = A^(-1)*R
		u[0] = (A[1][1] * R[0] - A[0][1] * R[1]) / D;
		u[1] = (A[0][0] * R[1] - A[1][0] * R[0]) / D;

		// calculate displacement norm
		normu = u[0] * u[0] + u[1] * u[1];

		// check for convergence
		bconv = ((normu < 1e-10));
		if (!bconv && (n <= NMAX))
		{
			// Don't update if converged otherwise the point q
			// does not correspond with the current values for (r,s)
			r += u[0];
			s += u[1];
			++n;
		}
		else break;
	} while (1);

	// evaluate q
	q = y[0] * H[0] + y[1] * H[1] + y[2] * H[2] + y[3] * H[3];

	return ((r >= -1.0 - tol) && (r <= 1.0 + tol) && (s >= -1.0 - tol) && (s <= 1.0 + tol));
}

//-----------------------------------------------------------------------------
// Normal projection of x, along direction t, onto triangle defined by y
bool ProjectToTriangle(vec3f* y, vec3f& x, vec3f& t, vec3f& q, double tol)
{
	// calculate base vectors 
	vec3f e1 = y[1] - y[0];
	vec3f e2 = y[2] - y[0];

	// calculate plane normal
	vec3f n = e1^e2; n.Normalize();

	// project x onto the plane
	double denom = n*t;
	if (denom == 0.0) return false;
	double l = n*(y[0] - x) / denom;
	q = x + t*l;

	// set up metric tensor
	double G[2][2];
	G[0][0] = e1*e1;
	G[0][1] = G[1][0] = e1*e2;
	G[1][1] = e2*e2;

	// invert metric tensor
	double D = G[0][0] * G[1][1] - G[0][1] * G[1][0];
	double Gi[2][2];
	Gi[0][0] = G[1][1] / D;
	Gi[1][1] = G[0][0] / D;
	Gi[0][1] = Gi[1][0] = -G[0][1] / D;

	// calculate dual base vectors
	vec3f E1 = e1*Gi[0][0] + e2*Gi[0][1];
	vec3f E2 = e1*Gi[1][0] + e2*Gi[1][1];

	// now we can calculate r and s
	vec3f p = q - y[0];
	double r = p*E1;
	double s = p*E2;

	return ((r >= -tol) && (s >= -tol) && (r + s <= 1.0 + tol));
}

//-----------------------------------------------------------------------------
// Normal projection of x, along direction t, onto quad defined by y
bool ProjectToQuad(vec3f* y, vec3f& x, vec3f& t, vec3f& q, double tol)
{
	// as an initial guess we project the point on the triangle 0,1,2
	// calculate base vectors 
	vec3f e1 = y[1] - y[0];
	vec3f e2 = y[2] - y[0];

	// calculate plane normal
	vec3f n = e1^e2; n.Normalize();

	// project x onto the plane
	double denom = n*t;
	if (denom == 0.0) return false;
	double l = n*(y[0] - x) / denom;
	q = x + t*l;

	// refine the solution
	double r = 0, s = 0;
	double err = 0;
	const int MAX_ITER = 10;
	int niter = 0;
	do
	{
		double H[4], Hr[4], Hs[4];
		H[0] = 0.25*(1.0 - r)*(1.0 - s);
		H[1] = 0.25*(1.0 + r)*(1.0 - s);
		H[2] = 0.25*(1.0 + r)*(1.0 + s);
		H[3] = 0.25*(1.0 - r)*(1.0 + s);

		Hr[0] = -0.25*(1.0 - s);
		Hr[1] =  0.25*(1.0 - s);
		Hr[2] =  0.25*(1.0 + s);
		Hr[3] = -0.25*(1.0 + s);

		Hs[0] = -0.25*(1.0 - r);
		Hs[1] = -0.25*(1.0 + r);
		Hs[2] =  0.25*(1.0 + r);
		Hs[3] =  0.25*(1.0 - r);

		// setup residual
		vec3f R;
		R = x + t*l - (y[0]*H[0] + y[1]*H[1] + y[2]*H[2] + y[3]*H[3]);

		vec3f R1 = t;
		vec3f R2 = -(y[0] * Hr[0] + y[1] * Hr[1] + y[2] * Hr[2] + y[3] * Hr[3]);
		vec3f R3 = -(y[0] * Hs[0] + y[1] * Hs[1] + y[2] * Hs[2] + y[3] * Hs[3]);

		// setup linear system
		Mat3d A;
		A[0][0] = R1.x; A[0][1] = R2.x; A[0][2] = R3.x;
		A[1][0] = R1.y; A[1][1] = R2.y; A[1][2] = R3.y;
		A[2][0] = R1.z; A[2][1] = R2.z; A[2][2] = R3.z;

		// solve linear system
		A.Invert();
		vec3f Du = A*R;

		// update solution
		l += -Du.x;
		r += -Du.y;
		s += -Du.z;

		// calculate error
		err = Du.Length();

		niter++;
	}
	while ((err > 1e-5) && (niter < MAX_ITER));

	// calculate solution
	q = x + t*l;

	if (niter >= MAX_ITER) return false;

	return ((r >= -1.0 - tol) && (r <= 1.0 + tol) && (s >= -1.0 - tol) && (s <= 1.0 + tol));
}
