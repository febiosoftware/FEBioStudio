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

#include "stdafx.h"
#include "ICPRegistration.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>

GICPRegistration::GICPRegistration()
{
}

Transform GICPRegistration::Register(GObject* ptrg, GObject* psrc, const double tol, const int maxIter)
{
	FEMesh& trgMesh = *ptrg->GetFEMesh();
	FEMesh& srcMesh = *psrc->GetFEMesh();

	int NX = trgMesh.Nodes();
	int NP = srcMesh.Nodes();

	// get the global coordinates
	vector<vec3d> X(NX);
	vector<vec3d> P(NP);
	for (int i=0; i<NX; ++i) X[i] = ptrg->GetTransform().LocalToGlobal(trgMesh.Node(i).r);
	for (int i=0; i<NP; ++i) P[i] = psrc->GetTransform().LocalToGlobal(srcMesh.Node(i).r);

	//find center of mass
	vec3d cp0 = CenterOfMass(P);
	vec3d cx0 = CenterOfMass(X);

	// store the initial coordinates
	vector<vec3d> P0 = P;

	// calculate the initial displacements to bring the points closer together
	vec3d t0 = cx0 - cp0;
	for (int i = 0; i<NP; i++) P[i] += t0;

	// estimate the size of the model so we can make the error dimensionless
	BOX box(P[0], P[0]);
	for (int i=1; i<NP; ++i) box += P[i];
	double R = box.Radius();

	// reserve space for the Y-vector
	// (stores the closest points in X to P)
	vector<vec3d> Y(NP);

	// loop over max iteration
	Transform Q;
	double prev_err = 0.0;
	for (int counter = 0; counter < maxIter; counter++)
	{
		// Compute the closest point set Y
		ClosestPointSet(X, P, Y);

		// compute the registration
		double err = 0;
		Q = Register(P0, Y, &err);

		// apply the registration
		ApplyTransform(P0, Q, P);

		// check convergence
		if (fabs((err - prev_err)/R) < tol) break;

		prev_err = err;
	}

	vec3d r_old = psrc->GetTransform().GetPosition();
	vec3d r_new = Q.GetRotation()*(r_old) + Q.GetPosition();

	Q.SetPosition(r_new - r_old);

	return Q;
}

void GICPRegistration::ClosestPointSet(const vector<vec3d>& X, const vector<vec3d>& P, vector<vec3d>& Y)
{
	// get the vector sizes
	int NX = (int) X.size();
	int NP = (int) P.size();

	// make sure Y is the right size
	// (must be same size as P)
	Y.resize(NP);

	// Find the closest node int X for each point in P
	// and store in Y
	for (int i = 0; i<NP; i++)
	{
		double min_dist = 1e99;
		const vec3d& Pi = P[i];
		for(int j = 0; j<NX; j++)
		{
			const vec3d& Xj = X[j];
			double dist = (Pi - Xj).Length();
			if (dist < min_dist)
			{
				min_dist = dist;
				Y[i] = Xj;
			}
		}
	}
}

vec3d GICPRegistration::CenterOfMass(const vector<vec3d>& S)
{
	vec3d c(0,0,0);
	int N = (int) S.size();
	for (int i=0; i<N; ++i) c += S[i];
	return (c/N);
}

Transform GICPRegistration::Register(const vector<vec3d>& P, const vector<vec3d>& Y, double* perr)
{
	//Find center of mass
	vec3d cp = CenterOfMass(P);
	vec3d cy = CenterOfMass(Y);

	//Find cross product of com_y and com_p
	mat3d cpy;
	cpy[0][0] = cp.x * cy.x; cpy[0][1] = cp.x * cy.y; cpy[0][2] = cp.x * cy.z;
	cpy[1][0] = cp.y * cy.x; cpy[1][1] = cp.y * cy.y; cpy[1][2] = cp.y * cy.z;
	cpy[2][0] = cp.z * cy.x; cpy[2][1] = cp.z * cy.y; cpy[2][2] = cp.z * cy.z;

	//Find the cross covariance matrix (Sigma)
	int NP = (int) P.size();
	mat3d S; S.zero();
	for (int i = 0; i<NP; i++)
	{
		const vec3d& pi = P[i];
		const vec3d& yi = Y[i];
		
		// temp = p_i * y_i
		mat3d temp;
		temp[0][0] = pi.x * yi.x; temp[0][1] = pi.x * yi.y; temp[0][2] = pi.x * yi.z;
		temp[1][0] = pi.y * yi.x; temp[1][1] = pi.y * yi.y; temp[1][2] = pi.y * yi.z;
		temp[2][0] = pi.z * yi.x; temp[2][1] = pi.z * yi.y; temp[2][2] = pi.z * yi.z;
		S += temp;
	}
	S /= NP;
	S -= cpy;

	// trace of sigma
	double trS = S.trace();

	// column vector Delta
	double D[3] = {S[1][2] - S[2][1], S[2][0] - S[0][2], S[0][1] - S[1][0]};

	// setup the Q vector
	matrix Q(4, 4);
	Q[0][0] = trS; 
	Q[0][1] = D[0]; Q[0][2] = D[1]; Q[0][3] = D[2];
	Q[1][0] = Q[0][1]; Q[2][0] = Q[0][2]; Q[3][0] = Q[0][3];
	Q[1][1] = 2.0*S[0][0] - Q[0][0]; 
	Q[2][2] = 2.0*S[1][1] - Q[0][0];
	Q[3][3] = 2.0*S[2][2] - Q[0][0];
	Q[1][2] = S[0][1] + S[1][0]; Q[1][3] = S[0][2] + S[2][0];
	Q[2][1] = S[1][0] + S[0][1]; Q[2][3] = S[1][2] + S[2][1];
	Q[3][1] = S[2][0] + S[0][2]; Q[3][2] = S[2][1] + S[1][2];

	//Unit eigen vector for the matrix Q 
	matrix Eigen(4, 4);
	vector<double> eigen_values;
	eigen_values.reserve(4);
	Q.eigen_vectors(Eigen, eigen_values);

	//Find maximum eigen value
	double max_eigen = eigen_values[0];
	int max_eigen_index = 0;
	for (int i = 1; i<4; i++)
	{
		if (max_eigen < eigen_values[i])
		{
			max_eigen = eigen_values[i];
			max_eigen_index = i;
		}
	}

	double q_r[4]; //unit eigen vector corresponding to maximum eigen value of Q
	q_r[0] = Eigen[0][max_eigen_index]; q_r[1] = Eigen[1][max_eigen_index];
	q_r[2] = Eigen[2][max_eigen_index]; q_r[3] = Eigen[3][max_eigen_index];

	// optimal eigenvector
	quatd qR(q_r[1], q_r[2], q_r[3], q_r[0]);

	// optimal translation
	vec3d qT = cy - qR*cp;

	Transform T;
	T.SetPosition(qT);
	T.SetRotation(qR);

	if (perr)
	{
		double& err = *perr;

		const vec3d& t = T.GetPosition();
		const quatd& q = T.GetRotation();

		int N = (int)P.size();
		for (int i = 0; i<N; ++i)
		{
			const vec3d& yi = Y[i];
			const vec3d& p0 = P[i];
			vec3d p1 = q*p0 + t;
			err += (yi - p1)*(yi - p1);
		}
		err = sqrt(err/N);
	}

	return T;
}

void GICPRegistration::ApplyTransform(const vector<vec3d>& P0, const Transform& Q, vector<vec3d>& P)
{
	const vec3d& t = Q.GetPosition();
	const quatd& q = Q.GetRotation();

	int N = (int) P0.size();
	P.resize(N);
	for (int i=0; i<N; ++i)
	{
		vec3d p = P0[i];
		P[i] = q*p + t;
	}
}
