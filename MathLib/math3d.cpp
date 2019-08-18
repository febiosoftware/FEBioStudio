#include "math3d.h"

//-----------------------------------------------------------------------------
// These functions are defined in colsol.cpp
void lubksb(double**a, int n, int *indx, double b[]);
void ludcmp(double**a, int n, int* indx);

//-----------------------------------------------------------------------------
quatd quatd::slerp(quatd &q1, quatd &q2, double t) 
{
	quatd q3;
	double dot = quatd::dot(q1, q2);

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
		return quatd::lerp(q1,q3,t);
}

//-----------------------------------------------------------------------------
void quatd::SetEuler(double X, double Y, double Z)
{
	// convert to radians
	X *= DEG2RAD;
	Y *= DEG2RAD;
	Z *= DEG2RAD;

	// calculate cos and sin of angles
	double cz = cos(Z*0.5);
	double sz = sin(Z* 0.5);
	double cx = cos(X* 0.5);
	double sx = sin(X* 0.5);
	double cy = cos(Y* 0.5);
	double sy = sin(Y* 0.5);

	// define quaternion
	w = cz * cx * cy + sz * sx * sy;
	x = cz * sx * cy - sz * cx * sy;
	y = cz * cx * sy + sz * sx * cy;
	z = sz * cx * cy - cz * sx * sy;
}

//-----------------------------------------------------------------------------
void quatd::GetEuler(double& X, double& Y, double& Z) const
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

	// convert to degrees
	X *= DEG2RAD;
	Y *= DEG2RAD;
	Z *= DEG2RAD;
}

//-----------------------------------------------------------------------------
bool matrix::solve(vector<double>& x, vector<double>& b)
{
	// check sizes
	if (m_nr != m_nc) return false;
	if (((int)x.size() != m_nr) || ((int)b.size() != m_nr)) return false;

	// step 1. Factorization

	const double TINY = 1.0e-20;
	int i, imax, j, k;
	double big, dum, sum, temp;

	matrix& a = *this;

	int n = a.Rows();
	
	// create index vector
	vector<int> indx(n);

	vector<double> vv(n);
	for (i=0; i<n; ++i)
	{
		big = 0;
		for (j=0; j<n; ++j)
			if ((temp=fabs(a(i,j))) > big) big = temp;
		if (big == 0) return false; // singular matrix
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
void matrix::mult_transpose_self(matrix& AAt)
{
	matrix& A = *this;
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
matrix::matrix(int r, int c) : m_nr(r), m_nc(c)
{
	m_ne = r*c;
	if (m_ne > 0) d = new double[m_ne];
	else d = 0;
}

//-----------------------------------------------------------------------------
matrix::matrix(const matrix& m)
{
	m_nr = m.m_nr;
	m_nc = m.m_nc;
	m_ne = m_nr*m_nc;
	if (m_ne > 0)
	{
		d = new double[m_ne];
		for (int i=0; i<m_ne; ++i) d[i] = m.d[i];
	}
	else d = 0;
}

//-----------------------------------------------------------------------------
void matrix::operator = (const matrix& m)
{
	m_nr = m.m_nr;
	m_nc = m.m_nc;
	m_ne = m_nr*m_nc;
	if (d) delete [] d;
	if (m_ne > 0)
	{
		d = new double[m_ne];
		for (int i = 0; i<m_ne; ++i) d[i] = m.d[i];
	}
	else d = 0;
}

//-----------------------------------------------------------------------------
void matrix::zero()
{
	for (int i=0; i<m_ne; ++i) d[i] = 0.0;
}

//-----------------------------------------------------------------------------
void matrix::mult(vector<double>& x, vector<double>& y)
{
	for (int i = 0; i<m_nr; ++i)
	{
		double* di = d + i*m_nc;
		y[i] = 0.0;
		for (int j = 0; j<m_nc; ++j) y[i] += di[j] * x[j];
	}
}

//-----------------------------------------------------------------------------
matrix matrix::operator * (const matrix& m)
{
	assert(m_nc == m.m_nr);
	matrix a(m_nr, m.m_nc);
	matrix& T = *this;

	for (int i = 0; i<m_nr; ++i)
	{
		for (int j = 0; j<m.m_nc; ++j)
		{
			a(i, j) = 0;
			for (int k = 0; k<m_nc; ++k) a(i, j) += T(i,k) * m(k, j);
		}
	}

	return a;
}

//-----------------------------------------------------------------------------
matrix& matrix::operator += (const matrix& m)
{
    assert((m_nr == m.m_nr ) && (m_nc == m.m_nc));
    for (int i=0; i<m_nr*m_nc; ++i) d[i] += m.d[i];
    return (*this);
}

//-----------------------------------------------------------------------------
void matrix::mult_transpose(vector<double>& x, vector<double>& y)
{
	for (int i=0; i<m_nc; ++i) y[i] = 0.0;

	for (int i=0; i<m_nr; ++i)
	{
		double* di = d + i*m_nc;
		for (int j=0; j<m_nc; ++j) y[j] += di[j]*x[i];
	}
}

//-----------------------------------------------------------------------------
bool matrix::lsq_solve(vector<double>& x, vector<double>& b)
{
	if ((int) x.size() != m_nc) return false;
	if ((int) b.size() != m_nr) return false;

	vector<double> y(m_nc);
	mult_transpose(b, y);

	matrix AA(m_nc, m_nc);
	mult_transpose_self(AA);

	AA.solve(x, y);

	return true;
}

#define ROTATE(a, i, j, k, l) g=a[i][j]; h=a[k][l];a[i][j]=g-s*(h+g*tau); a[k][l] = h + s*(g - h*tau);

bool matrix::eigen_vectors(matrix& Eigen, vector<double>& eigen_values){
	matrix& A = *this;
	int N = m_nc;
	int R = m_nr;
	const int NMAX = 50;
	double sm, tresh, g, h, t, c, tau, s, th;
	const double eps = 0;//1.0e-15;
	int k ;

	//initialize Eigen to identity
	for(int i = 0; i< R ; i++)
	{
		for(int j = 0;j<N;j++) Eigen[i][j] = 0;
		Eigen[i][i] = 1;
	}
	vector<double> b;
	b.reserve(R);
	vector<double> z;
	z.reserve(R);

	// initialize b and eigen_values to the diagonal of A
	for(int i = 0; i<R;i++)
	{
		b.push_back(A[i][i]);
		eigen_values.push_back(A[i][i]);
		z.push_back(0);
	}
	// loop
	int n, nrot = 0;
	for (n=0; n<NMAX; ++n)
	{
		// sum off-diagonal elements
		sm = 0;
		for(int i = 0; i<N-1;i++){
			for(int j = i+1; j<N; j++)
				sm += fabs(A[i][j]);
		}
		if (sm <= eps) {
			break;
		}
		// set the treshold
		if (n < 3) tresh = 0.2*sm/(R*R); else tresh = 0.0;

		// loop over off-diagonal elements
		for(int i = 0; i<N-1;i++){
			for(int j = i+1; j<N; j++){

				g = 100.0*fabs(A[i][j]);
				// after four sweeps, skip the rotation if the off-diagonal element is small
				if ((n > 3) && ((fabs(eigen_values[i])+g) == fabs(eigen_values[i])) && ((fabs(eigen_values[j])+g) == fabs(eigen_values[j])))
				{
					A[i][j] = 0.0;
				}
				else if (fabs(A[i][j]) > tresh){
					h = eigen_values[j] - eigen_values[i];
					if ((fabs(h)+g) == fabs(h))
						t = A[i][j]/h;
					else
					{
						th = 0.5*h/A[i][j];
						t = 1.0/(fabs(th) + sqrt(1+th*th));
						if (th < 0.0) t = -t;
					}
					c = 1.0/sqrt(1.0 + t*t);
					s = t*c;
					tau = s/(1.0+c);
					h = t*A[i][j];
					z[i] -= h;
					z[j] += h;
					eigen_values[i] -= h;
					eigen_values[j] += h;
					A[i][j] = 0;
					for (k=  0; k<=i-1; ++k) { ROTATE(A, k, i, k, j) }
					for (k=i+1; k<=j-1; ++k) { ROTATE(A, i, k, k, j) }
					for (k=j+1; k<N; ++k) { ROTATE(A, i, k, j, k) }
					for (k=  0; k<N; ++k) { ROTATE(Eigen, k, i, k, j) }
					++nrot;
				}
			}
		}//end of for loop

		//Update eigen_values with the sum. Reinitialize z.
		for (int i=0; i<R; ++i) 
		{
			b[i] += z[i];
			eigen_values[i] = b[i];
			z[i] = 0.0;
		}
	}

	// we sure we converged
	assert(n < NMAX);
	return true;
}

//-----------------------------------------------------------------------------
matrix matrix::inverse()
{
	// make sure this is a square matrix
	assert(m_nr == m_nc);

	// make a copy of this matrix
	// since we don't want to change it
	matrix tmp(*this);
	double** a = new double*[m_nr];
	for (int i = 0; i<m_nr; ++i) a[i] = tmp.d + i*m_nr;

	// do a LU decomposition
	int n = m_nr;
	vector<int> indx(n);
	ludcmp(a, n, &indx[0]);

	// allocate the inverse matrix
	matrix ai(n, n);

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

	delete [] a;

	return ai;
}
