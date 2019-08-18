#include "math3d.h"
#include "lusolver.h"
#include <assert.h>

//-----------------------------------------------------------------------------
void mat3fs::Principals(float e[3]) const
{
	const static float ONETHIRD = 1.f / 3.f;

	// pressure
	float p = -( x + y + z )*ONETHIRD;

	DeviatoricPrincipals(e);

	e[0] -= p;
	e[1] -= p;
	e[2] -= p;
}

//-----------------------------------------------------------------------------
void mat3fs::DeviatoricPrincipals(float e[3]) const
{
	const static float ONETHIRD = 1.f / 3.f;

	// pressure
	float p = -( x + y + z )*ONETHIRD;

	// deviatoric stresses
	float dev[3];
	dev[0] = x + p;
	dev[1] = y + p;
	dev[2] = z + p;

	// invariants
	float I[3];
	I[0] = dev[0] + dev[1] + dev[2]; // = tr[s']

	I[1] = 0.5f*( dev[0]*dev[0] 
		        + dev[1]*dev[1]
				+ dev[2]*dev[2])
		   + xy*xy 
		   + yz*yz 
		   + xz*xz; // = s':s'

	I[2] = -dev[0]*dev[1]*dev[2]-2.0f*xy*yz*xz
		   + dev[0]*yz*yz
		   + dev[1]*xz*xz
		   + dev[2]*xy*xy; // = -det(s')

	// check to see if we can have a non-zero divisor, if no
	// set principal stress to 0
	if (I[1] != 0)
	{
		double a = -0.5*sqrt(27.0/I[1]) * I[2] / I[1];
		if (a < 0)
			a = MAX(a, -1.0);
		else
			a = MIN(a, 1.0);
		double w = acos(a)*ONETHIRD;
		double val = 2.0*sqrt(I[1]*ONETHIRD);
		e[0] = (float) (val*cos(w));
		w = w - 2.0*PI*ONETHIRD;
		e[1] = (float) (val*cos(w));
		w = w + 4.0*PI*ONETHIRD;
		e[2] = (float) (val*cos(w));
	}
	else
	{
		e[0] = e[1] = e[2] = 0.f;
	}
}

//-----------------------------------------------------------------------------
float mat3fs::MaxShear() const
{
	float e[3];
	Principals(e);

	float t1 = (float) fabs(0.5f*(e[1] - e[2]));
	float t2 = (float) fabs(0.5f*(e[2] - e[0]));
	float t3 = (float) fabs(0.5f*(e[0] - e[1]));

	// TODO: is this necessary? I think the values returned
	//       by Principals are already ordered.
	float tmax = t1;
	if (t2 > tmax) tmax = t2;
	if (t3 > tmax) tmax = t3;

	return tmax;
}

//-----------------------------------------------------------------------------
#define ROTATE(a, i, j, k, l) g=a[i][j]; h=a[k][l];a[i][j]=g-s*(h+g*tau); a[k][l] = h + s*(g - h*tau);
#define SWAPF(a, b) { float t = a; a = b; b = t; }
#define SWAPV(a, b) { vec3f t = a; a = b; b = t; }

void mat3fs::eigen(vec3f e[3], float l[3]) const
{
	const int NMAX = 50;
	double sm, tresh, g, h, t, c, tau, s, th;
	int i, j, k;

	// copy the Matrix components since we will be overwriting them
	double a[3][3] = {
			{x , xy, xz},
			{xy, y , yz}, 
			{xz, yz, z }
	};

	// the v Matrix contains the eigen vectors
	// intialize to identity
	double v[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 }
	};

	// initialize b and d to the diagonal of a
	double b[3] = {a[0][0], a[1][1], a[2][2]};
	double d[3] = {a[0][0], a[1][1], a[2][2]};
	double z[3] = {0};

	const double eps = 0;//1.0e-15;

	// loop
	int n, nrot = 0;
	for (n=0; n<NMAX; ++n)
	{
		// sum off-diagonal elements
		sm = fabs(a[0][1]) + fabs(a[0][2]) + fabs(a[1][2]);
		if (sm <= eps) break;

		// set the treshold
		if (n < 3) tresh = 0.2*sm/9.0; else tresh = 0.0;

		// loop over off-diagonal elements
		for (i=0; i<2; ++i)
		{
			for (j=i+1; j<3; ++j)
			{
				g = 100.0*fabs(a[i][j]);

				// after four sweeps, skip the rotation if the off-diagonal element is small
				if ((n > 3) && ((fabs(d[i])+g) == fabs(d[i]))
							&& ((fabs(d[j])+g) == fabs(d[j])))
				{
					a[i][j] = 0.0;
				}
				else if (fabs(a[i][j]) > tresh)
				{
					h = d[j] - d[i];
					if ((fabs(h)+g) == fabs(h))
						t = a[i][j]/h;
					else
					{
						th = 0.5*h/a[i][j];
						t = 1.0/(fabs(th) + sqrt(1+th*th));
						if (th < 0.0) t = -t;
					}

					c = 1.0/sqrt(1.0 + t*t);
					s = t*c;
					tau = s/(1.0+c);
					h = t*a[i][j];
					z[i] -= h;
					z[j] += h;
					d[i] -= h;
					d[j] += h;
					a[i][j] = 0;

					for (k=  0; k<=i-1; ++k) { ROTATE(a, k, i, k, j) }
					for (k=i+1; k<=j-1; ++k) { ROTATE(a, i, k, k, j) }
					for (k=j+1; k<   3; ++k) { ROTATE(a, i, k, j, k) }
					for (k=  0; k<   3; ++k) { ROTATE(v, k, i, k, j) }
					++nrot;
				}
			}
		}

		for (i=0; i<3; ++i) 
		{
			b[i] += z[i];
			d[i] = b[i];
			z[i] = 0.0;
		}
	}

	// we sure we converged
	assert(n < NMAX);

	// copy eigenvalues
	l[0] = (float) d[0];
	l[1] = (float) d[1];
	l[2] = (float) d[2];

	// copy eigenvectors
	e[0].x = (float) v[0][0]; e[0].y = (float) v[1][0]; e[0].z = (float) v[2][0];
	e[1].x = (float) v[0][1]; e[1].y = (float) v[1][1]; e[1].z = (float) v[2][1];
	e[2].x = (float) v[0][2]; e[2].y = (float) v[1][2]; e[2].z = (float) v[2][2];

	// we still need to sort the eigenvalues
	if (l[1] > l[0]) { SWAPF(l[0], l[1]); SWAPV(e[0], e[1]); }
	if (l[2] > l[0]) { SWAPF(l[0], l[2]); SWAPV(e[0], e[2]); }
	if (l[2] > l[1]) { SWAPF(l[2], l[1]); SWAPV(e[2], e[1]); }
}

//-----------------------------------------------------------------------------
vec3f mat3fs::PrincDirection(int l)
{
	vec3f e[3];
	float lam[3];
	eigen(e, lam);
	return e[l-1]*lam[l-1];
}

//=============================================================================
Mat3d::Mat3d(double a00, double a01, double a02, double a10, double a11, double a12, double a20, double a21, double a22)
{
	m_data[0][0] = a00; m_data[0][1] = a01; m_data[0][2] = a02;
	m_data[1][0] = a10; m_data[1][1] = a11; m_data[1][2] = a12;
	m_data[2][0] = a20; m_data[2][1] = a21; m_data[2][2] = a22;
}

//-----------------------------------------------------------------------------
vec3f Mat3d::operator*(const vec3f& r)
{
	return vec3f(
		m_data[0][0]*r.x+m_data[0][1]*r.y+m_data[0][2]*r.z,
		m_data[1][0]*r.x+m_data[1][1]*r.y+m_data[1][2]*r.z,
		m_data[2][0]*r.x+m_data[2][1]*r.y+m_data[2][2]*r.z);
}

//-----------------------------------------------------------------------------
Mat3d Mat3d::transpose()
{
	return Mat3d(m_data[0][0], m_data[1][0], m_data[2][0], m_data[0][1], m_data[1][1], m_data[2][1], m_data[0][2], m_data[1][2], m_data[2][2]);
}

//=============================================================================
Matrix::Matrix(int r, int c) : m_nr(r), m_nc(c)
{
	m_ne = r*c;
	if (m_ne > 0) d = new double[m_ne];
	else d = 0;
}

//-----------------------------------------------------------------------------
Matrix::Matrix(const Matrix& m)
{
	m_nr = m.m_nr;
	m_nc = m.m_nc;
	m_ne = m.m_ne;
	d = 0;
	if (m_ne > 0)
	{
		d = new double[m_ne];
		for (int i = 0; i<m_ne; ++i) d[i] = m.d[i];
	}
}

//-----------------------------------------------------------------------------
void Matrix::operator = (const Matrix& m)
{
	if (d) delete [] d;

	m_nr = m.m_nr;
	m_nc = m.m_nc;
	m_ne = m.m_ne;
	d = 0;
	if (m_ne > 0)
	{
		d = new double[m_ne];
		for (int i = 0; i<m_ne; ++i) d[i] = m.d[i];
	}
}

//-----------------------------------------------------------------------------
void Matrix::zero()
{
	for (int i=0; i<m_ne; ++i) d[i] = 0.0;
}

//-----------------------------------------------------------------------------
void Matrix::mult_transpose(vector<double>& x, vector<double>& y)
{
	for (int i=0; i<m_nc; ++i) y[i] = 0.0;

	for (int i=0; i<m_nr; ++i)
	{
		double* di = d + i*m_nc;
		for (int j=0; j<m_nc; ++j) y[j] += di[j]*x[i];
	}
}

//-----------------------------------------------------------------------------
bool Matrix::lsq_solve(vector<double>& x, vector<double>& b)
{
	if ((int) x.size() != m_nc) return false;
	if ((int) b.size() != m_nr) return false;

	vector<double> y(m_nc);
	mult_transpose(b, y);

	Matrix AA(m_nc, m_nc);
	mult_transpose_self(AA);

	AA.solve(x, y);

	return true;
}

//-----------------------------------------------------------------------------
void Matrix::mult(vector<double>& x, vector<double>& y)
{
	for (int i = 0; i<m_nr; ++i)
	{
		double* di = d + i*m_nc;
		y[i] = 0.0;
		for (int j = 0; j<m_nc; ++j) y[i] += di[j] * x[j];
	}
}

//-----------------------------------------------------------------------------
void Matrix::mult_transpose_self(Matrix& AAt)
{
	Matrix& A = *this;
	int N = m_nc;
	int R = m_nr;
	for (int i=0; i<N; ++i)
		for (int j=0; j<N; ++j)
		{
			double& aij = AAt[i][j];
			aij = 0.0;
			for (int k=0; k<R; ++k) aij += A[k][i]*A[k][j];
		}
}

//-----------------------------------------------------------------------------
bool Matrix::solve(vector<double>& x, vector<double>& b)
{
	// check sizes
	if (m_nr != m_nc) return false;
	if (((int)x.size() != m_nr) || ((int)b.size() != m_nr)) return false;

	// step 1. Factorization

	const double TINY = 1.0e-20;
	int i, imax, j, k;
	double big, dum, sum, temp;

	Matrix& a = *this;

	int n = a.Rows();
	
	// create index vector
	vector<int> indx(n);

	vector<double> vv(n);
	for (i=0; i<n; ++i)
	{
		big = 0;
		for (j=0; j<n; ++j)
			if ((temp=fabs(a(i,j))) > big) big = temp;
		if (big == 0) return false; // singular Matrix
		vv[i] = 1.0 / big;
	}

	for (j=0; j<n; ++j)
	{
		for (i=0; i<j; ++i)
		{
			sum = a(i,j);
			for (k=0; k<i; ++k) sum -= a(i,k)*a(k,j);
			a(i,j) = sum;
		}
		big = 0;
		imax = j;
		for (i=j;i<n;++i)
		{
			sum = a(i,j);
			for (k=0; k<j; ++k) sum -= a(i,k)*a(k,j);
			a(i,j) = sum;
			if ((dum=vv[i]*fabs(sum))>=big)
			{
				big = dum;
				imax = i;
			}
		}

		if (j != imax)
		{
			for (k=0; k<n; ++k)
			{
				dum = a(imax,k);
				a(imax,k) = a(j,k);
				a(j,k) = dum;
			}
			vv[imax] = vv[j];
		}

		indx[j] = imax;
		if (a(j,j) == 0) a(j,j) = TINY;
		if (j != n-1)
		{
			dum = 1.0/a(j,j);
			for (i=j+1;i<n; ++i) a(i,j) *= dum;
		}
	}

	// step 2. back-solve
	x = b;

	int ii=0, ip;

	n = a.Rows();
	for (i=0; i<n; ++i)
	{
		ip = indx[i];
		sum = x[ip];
		x[ip] = x[i];
		if (ii != 0)
			for (j=ii-1;j<i;++j) sum -= a(i,j)*x[j];
		else if (sum != 0)
			ii = i+1;
		x[i] = sum;
	}

	for (i=n-1; i>=0; --i)
	{
		sum = x[i];
		for (j=i+1; j<n; ++j) sum -= a(i,j)*x[j];
		x[i] = sum/a(i,i);
	}

	return true;
}

//-----------------------------------------------------------------------------
quat4f quat4f::slerp(quat4f &q1, quat4f &q2, double t) 
{
	quat4f q3;
	double dot = quat4f::dot(q1, q2);

	/*	dot = cos(theta)
		if (dot < 0), q1 and q2 are more than 90 degrees apart,
		so we can invert one to reduce spinning	*/
	if (dot < 0)
	{
		dot = -dot;
		q3 = -q2;
	} else q3 = q2;
		
	if (dot < 0.95f)
	{
		double angle = acos(dot);
		return (q1*sin(angle*(1-t)) + q3*sin(angle*t))/sin(angle);
	} else // if the angle is small, use linear interpolation								
		return quat4f::lerp(q1,q3,t);
}


//-----------------------------------------------------------------------------
Matrix Matrix::inverse()
{
	// make sure this is a square Matrix
	assert(m_nr == m_nc);

	// make a copy of this Matrix
	// since we don't want to change it
	Matrix tmp(*this);
	double** a = new double*[m_nr];
	for (int i = 0; i<m_nr; ++i) a[i] = tmp.d + i*m_nr;

	// do a LU decomposition
	int n = m_nr;
	vector<int> indx(n);
	ludcmp(a, n, &indx[0]);

	// allocate the inverse Matrix
	Matrix ai(n, n);

	// do a backsubstituation on the columns of a
	vector<double> b; b.assign(n, 0);
	for (int j = 0; j<n; ++j)
	{
		b[j] = 1;
		lubksb(a, n, &indx[0], &b[0]);

		for (int i = 0; i<n; ++i)
		{
			ai[i][j] = b[i];
			b[i] = 0;
		}
	}

	delete[] a;

	return ai;
}

//-----------------------------------------------------------------------------
Matrix Matrix::operator * (const Matrix& m)
{
	assert(m_nc == m.m_nr);
	Matrix a(m_nr, m.m_nc);
	Matrix& T = *this;

	for (int i = 0; i<m_nr; ++i)
	{
		for (int j = 0; j<m.m_nc; ++j)
		{
			a(i, j) = 0;
			for (int k = 0; k<m_nc; ++k) a(i, j) += T(i, k) * m(k, j);
		}
	}

	return a;
}


//-----------------------------------------------------------------------------
void quat4f::SetEuler(double X, double Y, double Z)
{
	// calculate cos and sin of angles
	double cz = cos(Z*0.5);
	double sz = sin(Z*0.5);
	double cx = cos(X*0.5);
	double sx = sin(X*0.5);
	double cy = cos(Y*0.5);
	double sy = sin(Y*0.5);

	// define quaternion
	w = cz * cx * cy + sz * sx * sy;
	x = cz * sx * cy - sz * cx * sy;
	y = cz * cx * sy + sz * sx * cy;
	z = sz * cx * cy - cz * sx * sy;
}

//-----------------------------------------------------------------------------
void quat4f::GetEuler(double& X, double& Y, double& Z) const
{
	// roll (x-axis rotation)
	double t0 = +2.0 * (w * x + y * z);
	double t1 = +1.0 - 2.0 * (x*x + y*y);
	X = atan2(t0, t1);

	// pitch (y-axis rotation)
	double t2 = +2.0 * (w*y - z*x);
	t2 = t2 > 1.0 ? 1.0 : t2;
	t2 = t2 < -1.0 ? -1.0 : t2;
	Y = asin(t2);

	// yaw (z-axis rotation)
	double t3 = +2.0 * (w * z + x * y);
	double t4 = +1.0 - 2.0 * (y*y + z*z);
	Z = atan2(t3, t4);
}