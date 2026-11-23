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
#include "FEMeshUntangler.h"
#include <array>
#include <deque>
#include <thread>
#include <cmath>
#include <algorithm>
using namespace std;

namespace {

	typedef FEMeshUntangler::Element Element;
	typedef FEMeshUntangler::NodeConstraint NodeConstraint;

	const double GP = 0.57735026918962576451;	// 1/sqrt(3)
	const double HR[8] = { -1, 1, 1, -1, -1, 1, 1, -1 };
	const double HS[8] = { -1, -1, 1, 1, -1, -1, 1, 1 };
	const double HT[8] = { -1, -1, -1, -1, 1, 1, 1, 1 };

	inline double comp(const vec3d& v, int k) { return (k == 0 ? v.x : (k == 1 ? v.y : v.z)); }

	int elemNodes(FEMeshUntangler::ElemType t)
	{
		switch (t)
		{
		case FEMeshUntangler::TET4: return 4;
		case FEMeshUntangler::PYRA5: return 5;
		case FEMeshUntangler::PENTA6: return 6;
		default: return 8;
		}
	}

	// shape function derivatives (dN/dr, dN/ds, dN/dt), same as FECore
	void shapeDeriv(FEMeshUntangler::ElemType type, double r, double s, double t, vec3d* dN)
	{
		switch (type)
		{
		case FEMeshUntangler::TET4:
			dN[0] = vec3d(-1, -1, -1); dN[1] = vec3d(1, 0, 0); dN[2] = vec3d(0, 1, 0); dN[3] = vec3d(0, 0, 1);
			break;
		case FEMeshUntangler::PYRA5:
			dN[0] = vec3d(-0.125 * (1 - s) * (1 - t), -0.125 * (1 - r) * (1 - t), -0.125 * (1 - r) * (1 - s));
			dN[1] = vec3d( 0.125 * (1 - s) * (1 - t), -0.125 * (1 + r) * (1 - t), -0.125 * (1 + r) * (1 - s));
			dN[2] = vec3d( 0.125 * (1 + s) * (1 - t),  0.125 * (1 + r) * (1 - t), -0.125 * (1 + r) * (1 + s));
			dN[3] = vec3d(-0.125 * (1 + s) * (1 - t),  0.125 * (1 - r) * (1 - t), -0.125 * (1 - r) * (1 + s));
			dN[4] = vec3d(0, 0, 0.5);
			break;
		case FEMeshUntangler::HEX8:
			for (int i = 0; i < 8; ++i)
				dN[i] = vec3d(0.125 * HR[i] * (1 + s * HS[i]) * (1 + t * HT[i]),
							  0.125 * HS[i] * (1 + r * HR[i]) * (1 + t * HT[i]),
							  0.125 * HT[i] * (1 + r * HR[i]) * (1 + s * HS[i]));
			break;
		case FEMeshUntangler::PENTA6:
			dN[0] = vec3d(-0.5 * (1 - t), -0.5 * (1 - t), -0.5 * (1 - r - s));
			dN[1] = vec3d( 0.5 * (1 - t),  0.0          , -0.5 * r);
			dN[2] = vec3d( 0.0          ,  0.5 * (1 - t), -0.5 * s);
			dN[3] = vec3d(-0.5 * (1 + t), -0.5 * (1 + t),  0.5 * (1 - r - s));
			dN[4] = vec3d( 0.5 * (1 + t),  0.0          ,  0.5 * r);
			dN[5] = vec3d( 0.0          ,  0.5 * (1 + t),  0.5 * s);
			break;
		}
	}

	// ideal reference elements of unit size: regular tet (unit edges), pyramid with a
	// unit square base and height 1/2 (one sixth of a cube), unit cube
	void refNodes(FEMeshUntangler::ElemType type, vec3d* X)
	{
		switch (type)
		{
		case FEMeshUntangler::TET4:
			X[0] = vec3d(0, 0, 0); X[1] = vec3d(1, 0, 0);
			X[2] = vec3d(0.5, sqrt(3.0) / 2.0, 0); X[3] = vec3d(0.5, sqrt(3.0) / 6.0, sqrt(2.0 / 3.0));
			break;
		case FEMeshUntangler::PYRA5:
			X[0] = vec3d(-0.5, -0.5, 0); X[1] = vec3d(0.5, -0.5, 0);
			X[2] = vec3d(0.5, 0.5, 0); X[3] = vec3d(-0.5, 0.5, 0); X[4] = vec3d(0, 0, 0.5);
			break;
		case FEMeshUntangler::HEX8:
			for (int i = 0; i < 8; ++i) X[i] = vec3d(0.5 * HR[i], 0.5 * HS[i], 0.5 * HT[i]);
			break;
		case FEMeshUntangler::PENTA6:
			X[0] = vec3d(0, 0, 0); X[1] = vec3d(1, 0, 0); X[2] = vec3d(0.5, sqrt(3.0) / 2.0, 0);
			X[3] = vec3d(0, 0, 1); X[4] = vec3d(1, 0, 1); X[5] = vec3d(0.5, sqrt(3.0) / 2.0, 1);
			break;
		}
	}

	bool invert3(const double A[3][3], double B[3][3])
	{
		double det = A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
				   - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0])
				   + A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
		if (det == 0) return false;
		double id = 1.0 / det;
		B[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]) * id;
		B[0][1] = (A[0][2] * A[2][1] - A[0][1] * A[2][2]) * id;
		B[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) * id;
		B[1][0] = (A[1][2] * A[2][0] - A[1][0] * A[2][2]) * id;
		B[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]) * id;
		B[1][2] = (A[0][2] * A[1][0] - A[0][0] * A[1][2]) * id;
		B[2][0] = (A[1][0] * A[2][1] - A[1][1] * A[2][0]) * id;
		B[2][1] = (A[0][1] * A[2][0] - A[0][0] * A[2][1]) * id;
		B[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]) * id;
		return true;
	}

	// Integration point data of an element type. With w[g][i] = M_g^T dN_i(g), where M_g is
	// the inverse Jacobian of the reference element, the normalized Jacobian of an element of
	// size a is J = (1/a) sum_i x_i (outer) w[g][i].
	struct ElemRef
	{
		int	nodes = 0;
		int	points = 0;		// integration points: points [0, points)
		int	corners = 0;	// corners: points [points, points + corners)
		vector<array<vec3d, 8> > dN;
		vector<array<vec3d, 8> > w;
	};

	ElemRef buildRef(FEMeshUntangler::ElemType type)
	{
		ElemRef R;
		R.nodes = elemNodes(type);
		vector<vec3d> pts;
		if (type == FEMeshUntangler::TET4) pts.push_back(vec3d(0.25, 0.25, 0.25));
		else if (type == FEMeshUntangler::PENTA6)
		{
			// FEBio's 6-point rule
			const double a = 1.0 / 6.0, b = 2.0 / 3.0;
			for (double t : { -GP, GP }) { pts.push_back(vec3d(a, a, t)); pts.push_back(vec3d(b, a, t)); pts.push_back(vec3d(a, b, t)); }
		}
		else for (int g = 0; g < 8; ++g) pts.push_back(vec3d(GP * HR[g], GP * HS[g], GP * HT[g]));
		R.points = (int)pts.size();

		// corners (the Jacobian there is that of the corner's three edges); not the pyramid's
		// apex, where the Jacobian is zero
		switch (type)
		{
		case FEMeshUntangler::HEX8:
			for (int i = 0; i < 8; ++i) pts.push_back(vec3d(HR[i], HS[i], HT[i]));
			break;
		case FEMeshUntangler::PYRA5:
			for (int i = 0; i < 4; ++i) pts.push_back(vec3d(HR[i], HS[i], -1.0));
			break;
		case FEMeshUntangler::PENTA6:
			for (double t : { -1.0, 1.0 }) { pts.push_back(vec3d(0, 0, t)); pts.push_back(vec3d(1, 0, t)); pts.push_back(vec3d(0, 1, t)); }
			break;
		default:
			break;
		}
		R.corners = (int)pts.size() - R.points;
		R.dN.resize(pts.size());
		R.w.resize(pts.size());

		vec3d X[8];
		refNodes(type, X);
		for (int g = 0; g < (int)pts.size(); ++g)
		{
			vec3d* dN = R.dN[g].data();
			shapeDeriv(type, pts[g].x, pts[g].y, pts[g].z, dN);

			double G[3][3] = { {0,0,0},{0,0,0},{0,0,0} }, M[3][3];
			for (int i = 0; i < R.nodes; ++i)
				for (int a = 0; a < 3; ++a)
					for (int k = 0; k < 3; ++k) G[a][k] += comp(X[i], a) * comp(dN[i], k);
			invert3(G, M);

			for (int i = 0; i < R.nodes; ++i)
			{
				double w[3];
				for (int l = 0; l < 3; ++l)
				{
					w[l] = 0;
					for (int k = 0; k < 3; ++k) w[l] += M[k][l] * comp(dN[i], k);
				}
				R.w[g][i] = vec3d(w[0], w[1], w[2]);
			}
		}
		return R;
	}

	const ElemRef& elemRef(FEMeshUntangler::ElemType type)
	{
		static const ElemRef ref[4] = { buildRef(FEMeshUntangler::TET4), buildRef(FEMeshUntangler::PYRA5), buildRef(FEMeshUntangler::HEX8), buildRef(FEMeshUntangler::PENTA6) };
		return ref[type];
	}

	inline double chiFnc(double D, double eps)
	{
		double s = sqrt(eps * eps + D * D);
		return (D >= 0 ? 0.5 * (D + s) : 0.5 * eps * eps / (s - D));
	}

	// Regularized distortion of a Jacobian with columns a, b, c:
	//   f = (1-theta) tr(J^T J)/(3 chi^(2/3)) + theta (det^2 + 1)/(2 chi),  chi = chi(det J, eps).
	// P receives df/dJ (columns); D receives det J.
	inline double distortion(const vec3d& a, const vec3d& b, const vec3d& c, double eps, double theta, vec3d* P, double& D)
	{
		vec3d ca = b ^ c, cb = c ^ a, cc = a ^ b;	// derivatives of det J with respect to the columns
		D = a * ca;
		double T = a * a + b * b + c * c;
		double s = sqrt(eps * eps + D * D);
		double chi = (D >= 0 ? 0.5 * (D + s) : 0.5 * eps * eps / (s - D));
		double chi23 = cbrt(chi * chi);
		double fs = T / (3.0 * chi23);
		double fv = (D * D + 1.0) / (2.0 * chi);
		if (P)
		{
			double dchi = chi / s;
			double dT = (1.0 - theta) / (3.0 * chi23);
			double dD = -(1.0 - theta) * (2.0 / 3.0) * fs / chi * dchi + theta * (D / chi - fv / chi * dchi);
			P[0] = a * (2.0 * dT) + ca * dD;
			P[1] = b * (2.0 * dT) + cb * dD;
			P[2] = c * (2.0 * dT) + cc * dD;
		}
		return (1.0 - theta) * fs + theta * fv;
	}

	// Energy of one element: the average over its integration points, plus the average over its
	// corners. The corners use eps_c = max(eps, CORNER_EPS), so they penalize badly shaped or
	// folded corners strongly but are never a barrier (FEBio only checks the integration points).
	// Adds dE/dx_i to g[i] if g != nullptr. dmin receives the smallest det at the integration points.
	const double CORNER_EPS = 0.05;
	const double CORNER_WEIGHT = 1.0;
	double elementEnergy(const Element& e, double invA, const vector<vec3d>& x, double eps, double theta, vec3d* g, double& dmin)
	{
		const ElemRef& R = elemRef(e.type);
		double f = 0;
		int np = R.points + R.corners;
		for (int p = 0; p < np; ++p)
		{
			bool corner = (p >= R.points);
			double wgt = (corner ? CORNER_WEIGHT / R.corners : 1.0 / R.points);
			double ep = (corner ? max(eps, CORNER_EPS) : eps);

			vec3d J[3] = { vec3d(0,0,0), vec3d(0,0,0), vec3d(0,0,0) };
			for (int i = 0; i < R.nodes; ++i)
			{
				const vec3d& xi = x[e.n[i]];
				const vec3d& w = R.w[p][i];
				J[0] += xi * w.x; J[1] += xi * w.y; J[2] += xi * w.z;
			}
			J[0] *= invA; J[1] *= invA; J[2] *= invA;

			vec3d P[3];
			double D;
			f += wgt * distortion(J[0], J[1], J[2], ep, theta, (g ? P : nullptr), D);
			if (!corner) dmin = min(dmin, D);
			if (g)
			{
				double s = wgt * invA;
				for (int i = 0; i < R.nodes; ++i)
				{
					const vec3d& w = R.w[p][i];
					g[i] += (P[0] * w.x + P[1] * w.y + P[2] * w.z) * s;
				}
			}
		}
		return f;
	}

	// size of the ideal element with the same mean edge length (edge of the regular tet or
	// cube, base edge of the pyramid)
	double elementSize(const vector<vec3d>& r, const Element& e)
	{
		const int* n = e.n;
		double L = 0; int m = 0;
		auto edge = [&](int p, int q) { L += (r[n[p]] - r[n[q]]).Length(); m++; };
		switch (e.type)
		{
		case FEMeshUntangler::TET4: edge(0, 1); edge(1, 2); edge(2, 0); edge(0, 3); edge(1, 3); edge(2, 3); break;
		case FEMeshUntangler::PYRA5:
			for (int k = 0; k < 4; ++k) { edge(k, (k + 1) % 4); edge(k, 4); }
			break;
		case FEMeshUntangler::HEX8:
			for (int k = 0; k < 4; ++k) { edge(k, (k + 1) % 4); edge(k + 4, (k + 1) % 4 + 4); edge(k, k + 4); }
			break;
		case FEMeshUntangler::PENTA6:
			for (int k = 0; k < 3; ++k) { edge(k, (k + 1) % 3); edge(k + 3, (k + 1) % 3 + 3); edge(k, k + 3); }
			break;
		}
		double a = L / m;
		// the mean edge length of the reference pyramid is (1 + sqrt(3)/2)/2 of its base edge
		if (e.type == FEMeshUntangler::PYRA5) a /= 0.5 * (1.0 + 0.5 * sqrt(3.0));
		return a;
	}

	typedef std::function<double(const vector<double>&, vector<double>&)> ObjFnc;

	double dot(const vector<double>& a, const vector<double>& b)
	{
		double s = 0;
		for (size_t i = 0; i < a.size(); ++i) s += a[i] * b[i];
		return s;
	}

	// L-BFGS with a backtracking (Armijo) line search. step0 is the largest variable change
	// of the first (steepest descent) step. Returns the number of iterations.
	int lbfgs(vector<double>& z, const ObjFnc& eval, int maxIter, double step0)
	{
		const int M = 8;
		size_t n = z.size();
		if (n == 0) return 0;

		vector<double> g(n), zn(n), gn(n), d(n), q(n);
		deque<vector<double> > S, Y;
		deque<double> rho;
		double f = eval(z, g);

		int it = 0;
		for (; it < maxIter; ++it)
		{
			// two-loop recursion: d = -H g
			int m = (int)S.size();
			vector<double> alpha(m);
			q = g;
			for (int k = m - 1; k >= 0; --k)
			{
				alpha[k] = rho[k] * dot(S[k], q);
				for (size_t i = 0; i < n; ++i) q[i] -= alpha[k] * Y[k][i];
			}
			double gamma = (m > 0 ? dot(S[m - 1], Y[m - 1]) / dot(Y[m - 1], Y[m - 1]) : 1.0);
			for (size_t i = 0; i < n; ++i) q[i] *= gamma;
			for (int k = 0; k < m; ++k)
			{
				double beta = rho[k] * dot(Y[k], q);
				for (size_t i = 0; i < n; ++i) q[i] += S[k][i] * (alpha[k] - beta);
			}
			for (size_t i = 0; i < n; ++i) d[i] = -q[i];

			double gd = dot(g, d);
			if (!(gd < 0))
			{
				// not a descent direction: restart with steepest descent
				S.clear(); Y.clear(); rho.clear(); m = 0;
				for (size_t i = 0; i < n; ++i) d[i] = -g[i];
				gd = -dot(g, g);
			}
			if (gd == 0) break;

			double t = 1.0;
			if (m == 0)
			{
				double dmax = 0;
				for (size_t i = 0; i < n; ++i) dmax = max(dmax, fabs(d[i]));
				if (dmax > 0) t = step0 / dmax;
			}

			// backtracking line search
			bool accepted = false;
			double fn = f;
			for (int ls = 0; ls < 40; ++ls)
			{
				for (size_t i = 0; i < n; ++i) zn[i] = z[i] + t * d[i];
				fn = eval(zn, gn);
				if (std::isfinite(fn) && (fn <= f + 1e-4 * t * gd)) { accepted = true; break; }
				t *= 0.5;
			}
			if (!accepted) break;

			// update the history
			vector<double> s(n), y(n);
			for (size_t i = 0; i < n; ++i) { s[i] = zn[i] - z[i]; y[i] = gn[i] - g[i]; }
			double sy = dot(s, y);
			if (sy > 1e-12 * sqrt(dot(s, s) * dot(y, y)))
			{
				S.push_back(s); Y.push_back(y); rho.push_back(1.0 / sy);
				if ((int)S.size() > M) { S.pop_front(); Y.pop_front(); rho.pop_front(); }
			}

			double df = f - fn;
			z.swap(zn); g.swap(gn); f = fn;
			if (df <= 1e-10 * max(1.0, fabs(f))) { ++it; break; }
		}
		return it;
	}

	//-------------------------------------------------------------------------
	class Optimizer
	{
	public:
		Optimizer(vector<vec3d>& r, const vector<Element>& elems, int threads) : m_x(r), m_el(elems)
		{
			m_threads = threads;
			if (m_threads <= 0) m_threads = max(1, (int)std::thread::hardware_concurrency());

			// element sizes from the initial configuration
			int NE = (int)elems.size();
			m_invA.assign(NE, 1.0);
			vector<double> a(NE, 0.0);
			double asum = 0; int na = 0;
			for (int i = 0; i < NE; ++i)
			{
				a[i] = elementSize(r, elems[i]);
				if (a[i] > 0) { asum += a[i]; na++; }
			}
			m_avgSize = (na > 0 ? asum / na : 1.0);
			for (int i = 0; i < NE; ++i) m_invA[i] = 1.0 / (a[i] > 0 ? a[i] : m_avgSize);
		}

		double AverageSize() const { return m_avgSize; }

		// define the variables: free nodes have 3, nodes in a plane 2, nodes on a line 1
		void SetVariables(const vector<NodeConstraint>& c, bool fixSliding)
		{
			int NN = (int)m_x.size();
			m_off.assign(NN, -1);
			m_ndof.assign(NN, 0);
			m_basis.assign(NN, array<vec3d, 3>());
			m_nvar = 0;
			for (int i = 0; i < NN; ++i)
			{
				int nd = 0;
				switch (c[i].type)
				{
				case FEMeshUntangler::FREE:
					nd = 3;
					m_basis[i][0] = vec3d(1, 0, 0); m_basis[i][1] = vec3d(0, 1, 0); m_basis[i][2] = vec3d(0, 0, 1);
					break;
				case FEMeshUntangler::PLANE:
					if (!fixSliding)
					{
						vec3d n = c[i].dir; n.unit();
						vec3d a = (fabs(n.x) < 0.9 ? vec3d(1, 0, 0) : vec3d(0, 1, 0));
						vec3d t1 = n ^ a; t1.unit();
						vec3d t2 = n ^ t1; t2.unit();
						m_basis[i][0] = t1; m_basis[i][1] = t2;
						nd = 2;
					}
					break;
				case FEMeshUntangler::LINE:
					if (!fixSliding)
					{
						vec3d t = c[i].dir; t.unit();
						m_basis[i][0] = t;
						nd = 1;
					}
					break;
				default:
					break;
				}
				if (nd > 0) { m_off[i] = m_nvar; m_ndof[i] = nd; m_nvar += nd; }
			}

			// only elements with a movable node contribute to the optimization
			m_active.clear();
			for (int i = 0; i < (int)m_el.size(); ++i)
			{
				const Element& e = m_el[i];
				int nn = elemNodes(e.type);
				for (int j = 0; j < nn; ++j)
					if (m_ndof[e.n[j]] > 0) { m_active.push_back(i); break; }
			}
		}

		int Variables() const { return m_nvar; }

		// energy of the active elements and its gradient with respect to the nodes (if G != nullptr)
		double Energy(const vector<vec3d>& X, vector<vec3d>* G, double& dmin) const
		{
			int NA = (int)m_active.size();
			int nt = ((m_threads > 1) && (NA > 4000) ? m_threads : 1);
			vector<double> fsum(nt, 0.0), dm(nt, 1e300);
			vector<vector<vec3d> > gt((G && (nt > 1)) ? nt : 0);
			if (G) G->assign(X.size(), vec3d(0, 0, 0));

			auto work = [&](int t) {
				int i0 = (int)((long long)NA * t / nt);
				int i1 = (int)((long long)NA * (t + 1) / nt);
				vector<vec3d>* gb = nullptr;
				if (G)
				{
					if (nt == 1) gb = G;
					else { gt[t].assign(X.size(), vec3d(0, 0, 0)); gb = &gt[t]; }
				}
				double f = 0, d = 1e300;
				vec3d ge[8];
				for (int k = i0; k < i1; ++k)
				{
					int ei = m_active[k];
					const Element& e = m_el[ei];
					int nn = elemNodes(e.type);
					for (int j = 0; j < nn; ++j) ge[j] = vec3d(0, 0, 0);
					f += elementEnergy(e, m_invA[ei], X, m_eps, m_theta, (gb ? ge : nullptr), d);
					if (gb) for (int j = 0; j < nn; ++j) (*gb)[e.n[j]] += ge[j];
				}
				fsum[t] = f; dm[t] = d;
			};

			if (nt == 1) work(0);
			else
			{
				vector<std::thread> th;
				for (int t = 0; t < nt; ++t) th.emplace_back(work, t);
				for (auto& h : th) h.join();
				if (G) for (int t = 0; t < nt; ++t) for (size_t i = 0; i < X.size(); ++i) (*G)[i] += gt[t][i];
			}

			double f = 0;
			dmin = 1e300;
			for (int t = 0; t < nt; ++t) { f += fsum[t]; dmin = min(dmin, dm[t]); }
			return f;
		}

		// positions for the variables z (relative to the start positions m_x0)
		void Positions(const vector<double>& z, vector<vec3d>& X) const
		{
			X = m_x0;
			for (size_t i = 0; i < X.size(); ++i)
				for (int k = 0; k < m_ndof[i]; ++k) X[i] += m_basis[i][k] * z[m_off[i] + k];
		}

		// Outer iterations of Garanzha et al.: minimize for fixed eps, then decrease eps.
		// Returns the number of outer iterations.
		int Run(vector<NodeConstraint>& c, bool fixSliding, FEMeshUntangler::ProjectFnc project,
			FEMeshUntangler::ProgressFnc progress, int maxOuter, int maxInner, int iter0, bool& canceled)
		{
			SetVariables(c, fixSliding);
			if (m_nvar == 0) return 0;

			double dmin;
			m_eps = 1.0;
			Energy(m_x, nullptr, dmin);
			m_eps = (dmin > 0 ? 1e-9 : min(m_epsMax, max(1e-3, 0.2 * fabs(dmin))));
			double Eprev = Energy(m_x, nullptr, dmin);

			int outer = 0;
			vector<vec3d> X;
			for (; outer < maxOuter; ++outer)
			{
				SetVariables(c, fixSliding);	// the sliding directions may have changed
				m_x0 = m_x;
				vector<double> z(m_nvar, 0.0);
				bool projected = (project && !fixSliding);
				vector<NodeConstraint> ctmp;
				ObjFnc eval = [&](const vector<double>& zz, vector<double>& gz) {
					// Sliding nodes move in their tangent planes/lines and are then projected onto
					// their surface/curve, so the energy (a barrier for eps -> 0) sees the actual
					// positions. The gradient uses the tangent directions.
					Positions(zz, X);
					if (projected) { ctmp = c; project(X, ctmp); }
					vector<vec3d> gx;
					double d;
					double f = Energy(X, &gx, d);
					gz.assign(m_nvar, 0.0);
					for (size_t i = 0; i < gx.size(); ++i)
						for (int k = 0; k < m_ndof[i]; ++k) gz[m_off[i] + k] = gx[i] * m_basis[i][k];
					return f;
				};
				lbfgs(z, eval, maxInner, 0.1 * m_avgSize);
				Positions(z, m_x);

				if (project && !fixSliding) project(m_x, c);

				double epsUsed = m_eps;
				double E = Energy(m_x, nullptr, dmin);
				if (progress && !progress(iter0 + outer, dmin)) { canceled = true; ++outer; break; }

				bool converged = (dmin > 0) && (epsUsed <= 1e-6) && (fabs(Eprev - E) <= 1e-3 * fabs(E));

				double sigma = max(1.0 - E / Eprev, 0.1);
				double mu = (1.0 - sigma) * chiFnc(dmin, m_eps);
				if (dmin < mu) m_eps = min(m_epsMax, 2.0 * sqrt(mu * (mu - dmin)));
				else m_eps = 1e-9;
				Eprev = E;

				if (converged) { ++outer; break; }
			}
			return outer;
		}

	private:
		vector<vec3d>&			m_x;
		const vector<Element>&	m_el;
		vector<double>	m_invA;
		double			m_avgSize = 1.0;
		int				m_threads = 1;
		double			m_theta = 0.05;
		// Upper bound for eps: a flattened element has an energy of at least theta/eps, so
		// for eps < theta collapsing elements never lowers the energy. (Without the bound,
		// a badly tangled start can make the whole mesh collapse while nodes slide.)
		double			m_epsMax = 0.0125;
		double			m_eps = 1.0;

		vector<int>		m_active;
		vector<int>		m_off;
		vector<int>		m_ndof;
		vector<array<vec3d, 3> >	m_basis;
		int				m_nvar = 0;
		vector<vec3d>	m_x0;
	};
}

//-----------------------------------------------------------------------------
FEMeshUntangler::FEMeshUntangler()
{
	m_threads = 0;
	m_maxOuter = 40;
	m_maxInner = 100;
	m_iterations = 0;
	m_inverted = 0;
	m_minJ = 0;
	m_minCornerJ = 0;
}

//-----------------------------------------------------------------------------
double FEMeshUntangler::MinJacobian(const vector<vec3d>& r, const Element& e)
{
	const ElemRef& R = elemRef(e.type);
	double jmin = 1e300;
	for (int p = 0; p < R.points; ++p)
	{
		vec3d G[3] = { vec3d(0,0,0), vec3d(0,0,0), vec3d(0,0,0) };
		for (int i = 0; i < R.nodes; ++i)
		{
			const vec3d& xi = r[e.n[i]];
			const vec3d& dN = R.dN[p][i];
			G[0] += xi * dN.x; G[1] += xi * dN.y; G[2] += xi * dN.z;
		}
		jmin = min(jmin, (G[0] ^ G[1]) * G[2]);
	}
	return jmin;
}

//-----------------------------------------------------------------------------
double FEMeshUntangler::ScaledJacobian(const vector<vec3d>& r, const Element& e)
{
	double a = elementSize(r, e);
	if (a <= 0) return 0;
	double d = 1e300;
	elementEnergy(e, 1.0 / a, r, 1e-9, 0.0, nullptr, d);
	return d;
}

//-----------------------------------------------------------------------------
double FEMeshUntangler::CornerScaledJacobian(const vector<vec3d>& r, const Element& e)
{
	const ElemRef& R = elemRef(e.type);
	if (R.corners == 0) return ScaledJacobian(r, e);	// (a tet4 has the same Jacobian everywhere)
	double a = elementSize(r, e);
	if (a <= 0) return 0;
	double dmin = 1e300;
	for (int p = R.points; p < R.points + R.corners; ++p)
	{
		vec3d J[3] = { vec3d(0,0,0), vec3d(0,0,0), vec3d(0,0,0) };
		for (int i = 0; i < R.nodes; ++i)
		{
			const vec3d& xi = r[e.n[i]];
			const vec3d& w = R.w[p][i];
			J[0] += xi * w.x; J[1] += xi * w.y; J[2] += xi * w.z;
		}
		dmin = min(dmin, ((J[0] ^ J[1]) * J[2]) / (a * a * a));
	}
	return dmin;
}

//-----------------------------------------------------------------------------
bool FEMeshUntangler::Apply(vector<vec3d>& r, const vector<Element>& elems, vector<NodeConstraint>& c,
	ProjectFnc project, ProgressFnc progress)
{
	m_iterations = 0;
	m_inverted = 0;
	m_minJ = 0;
	m_minCornerJ = 0;
	if (elems.empty()) return true;

	Optimizer opt(r, elems, m_threads);
	bool canceled = false;

	auto countInverted = [&]() {
		int n = 0;
		for (const Element& e : elems) if (MinJacobian(r, e) <= 0) n++;
		return n;
	};
	bool sliding = false;
	for (const NodeConstraint& ci : c) if ((ci.type == PLANE) || (ci.type == LINE)) { sliding = true; break; }

	// phase 1: untangle and optimize with the sliding nodes fixed
	m_iterations = opt.Run(c, true, nullptr, progress, m_maxOuter, m_maxInner, 0, canceled);

	// phase 2: the sliding nodes move in their planes/along their lines and are projected
	// back onto their surface/curve after each outer iteration
	if (sliding && !canceled)
		m_iterations += opt.Run(c, false, project, progress, m_maxOuter, m_maxInner, m_iterations, canceled);
	m_inverted = countInverted();

	// phase 3: if the last projection tangled any elements, untangle with the sliding nodes fixed
	if (sliding && (m_inverted > 0) && !canceled)
	{
		m_iterations += opt.Run(c, true, nullptr, progress, m_maxOuter, m_maxInner, m_iterations, canceled);
		m_inverted = countInverted();
	}

	m_minJ = m_minCornerJ = 1e300;
	for (const Element& e : elems)
	{
		m_minJ = min(m_minJ, ScaledJacobian(r, e));
		m_minCornerJ = min(m_minCornerJ, CornerScaledJacobian(r, e));
	}

	return !canceled;
}
