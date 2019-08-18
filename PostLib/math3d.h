#ifndef _MATH3D_H_
#define _MATH3D_H_

#include <math.h>
#include <vector>
using namespace std;

#ifndef PI
#define PI 3.1415926f
#endif

#ifndef DEG2RAD
#define DEG2RAD (PI/180.f)
#endif

#ifndef RAD2DEG
#define RAD2DEG (180.f/PI)
#endif

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

//-----------------------------------------------------------------------------
class vec2f
{
public:
	vec2f() { x = y = 0; }
	vec2f(float rx, float ry) { x = rx; y = ry; }

public:
	float	x, y;
};

//-----------------------------------------------------------------------------
class vec3f
{
public:
	vec3f() { x = y = z = 0; }
	vec3f(float rx, float ry, float rz) { x = rx; y = ry; z = rz; }

	vec3f operator + (const vec3f& v) const { return vec3f( x + v.x, y + v.y, z + v.z); }
	vec3f operator - (const vec3f& v) const { return vec3f( x - v.x, y - v.y, z - v.z); }
	vec3f operator ^ (const vec3f& v) const
	{ 
		return vec3f( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); 
	}

	float operator * (const vec3f& v) const { return (x*v.x + y*v.y + z*v.z); }

	vec3f operator * (const float g) const { return vec3f(x*g, y*g, z*g); }
	vec3f operator / (const float g) const { return vec3f(x/g, y/g, z/g); }

	const vec3f& operator += (const vec3f& v) { x += v.x; y += v.y; z += v.z; return (*this); }
	const vec3f& operator -= (const vec3f& v) { x -= v.x; y -= v.y; z -= v.z; return (*this); }
	const vec3f& operator /= (const float& f) { x /= f; y /= f; z /= f; return (*this); }
	const vec3f& operator /= (const int& n) { x /= n; y /= n; z /= n; return (*this); }
	const vec3f& operator *= (const float& f) { x*=f; y*=f; z*=f; return (*this); }

	vec3f operator - () { return vec3f(-x, -y, -z); }

	float Length() const { return (float) sqrt(x*x + y*y + z*z); }

	float SqrLength() const { return (float) (x*x + y*y + z*z); }

	vec3f& Normalize()
	{
		float L = Length();
		if (L != 0) { x /= L; y /= L; z /= L; }

		return (*this);
	}

public:
	float x, y, z;
};

//-----------------------------------------------------------------------------
// This class stores a 2nd order diagonal tensor
class mat3fd
{
public:
	mat3fd() { x = y = z = 0.f; }
	mat3fd(float X, float Y, float Z) { x = X; y = Y; z = Z; }

public:
	float x, y, z;
};

//-----------------------------------------------------------------------------
// mat3fs stores a 2nd order symmetric tensor
//
class mat3fs
{
public:
	// constructors
	mat3fs() { x = y = z = xy = yz = xz = 0; }
	mat3fs(float fx, float fy, float fz, float fxy, float fyz, float fxz)
	{
		x = fx; y = fy; z = fz;
		xy = fxy; yz = fyz; xz = fxz;
	}

	// operators
	mat3fs& operator += (const mat3fs& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		xy += v.xy;
		yz += v.yz;
		xz += v.xz;

		return (*this);
	}

	// operators
	mat3fs& operator -= (const mat3fs& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		xy -= v.xy;
		yz -= v.yz;
		xz -= v.xz;

		return (*this);
	}

	mat3fs& operator *= (float g)
	{
		x *= g;
		y *= g;
		z *= g;
		xy *= g;
		yz *= g;
		xz *= g;

		return (*this);
	}

	mat3fs& operator /= (float g)
	{
		x /= g;
		y /= g;
		z /= g;
		xy /= g;
		yz /= g;
		xz /= g;

		return (*this);
	}

	mat3fs operator + (const mat3fs& a) { return mat3fs(x + a.x, y + a.y, z + a.z, xy + a.xy, yz + a.yz, xz + a.xz); }
	mat3fs operator - (const mat3fs& a) { return mat3fs(x - a.x, y - a.y, z - a.z, xy - a.xy, yz - a.yz, xz - a.xz); }

	mat3fs operator * (float a)
	{
		return mat3fs(x*a, y*a, z*a, xy*a, yz*a, xz*a);
	}

	mat3fs operator / (float g)
	{
		return mat3fs(x/g, y/g, z/g, xy/g, yz/g, xz/g);
	}

	vec3f operator * (vec3f& r)
	{
		return vec3f(
			x *r.x + xy*r.y + xz*r.z, 
			xy*r.x + y *r.y + yz*r.z,
			xz*r.x + yz*r.y + z *r.z);
	}

	// Effective or von-mises value
	float von_mises() const
	{
		float vm;
		vm  = x*x + y*y + z*z;
		vm -= x*y + y*z + x*z;
		vm += 3*(xy*xy + yz*yz + xz*xz);
		vm = (float)sqrt(vm >= 0.0 ? vm : 0.0);
		return vm;
	}

	// principle values
	void Principals(float e[3]) const;

	// principle directions
	vec3f PrincDirection(int l);

	// deviatroric principle values
	void DeviatoricPrincipals(float e[3]) const;

	// max-shear value
	float MaxShear() const;

	// eigen-vectors and values
	void eigen(vec3f e[3], float l[3]) const;

	// trace
	float tr() { return x+y+z; }

	// determinant
	float det() { return (x*y*z + xy*yz*xz + xz*xy*yz - y*xz*xz - x*yz*yz - z*xy*xy); }

public:
	float x, y, z;
	float xy, yz, xz;
};

///////////////////////////////////////////////////////////////////
// quat4f

class quat4f
{
public:
	// constructors
	quat4f () { x = y = z = w = 0.f; }

	quat4f( const float angle, vec3f v)
	{
		w = (float) cos(angle * 0.5);

		float sina = (float) sin(angle * 0.5);

		v.Normalize();
		
		x = v.x*sina;
		y = v.y*sina;
		z = v.z*sina;
	}

	quat4f (vec3f v1, vec3f v2)
	{
		vec3f n = v1^v2;
		n.Normalize();

		float d = v1*v2;

		float sina = (float) sqrt((1.0-d)*0.5);
		float cosa = (float) sqrt((1.0+d)*0.5);

		w = cosa;

		x = n.x*sina;
		y = n.y*sina;
		z = n.z*sina;

	}

	quat4f(const float qx, const float qy, const float qz, const float qw = 0.0)
	{
		w = qw;
		x = qx;
		y = qy;
		z = qz;
	}

	bool operator != (const quat4f& q) { return ((x!=q.x) || (y!=q.y) || (z!=q.z) || (w!=q.w)); }

	quat4f operator - () { return quat4f(-x, -y, -z, -w); }

	// addition and substraction

	quat4f operator + (const quat4f& q) const
	{
		return quat4f(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	quat4f operator - (const quat4f& q) const
	{
		return quat4f(x - q.x, y - q.y, z - q.z, w - q.w);
	}

	quat4f& operator += (const quat4f& q)
	{
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;

		return *this;
	}

	quat4f& operator -= (const quat4f& q)
	{
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;

		return *this;
	}


	// multiplication

	quat4f operator * (const quat4f& q) const
	{
		float qw = w*q.w - x*q.x - y*q.y - z*q.z;
		float qx = w*q.x + x*q.w + y*q.z - z*q.y;
		float qy = w*q.y + y*q.w + z*q.x - x*q.z;
		float qz = w*q.z + z*q.w + x*q.y - y*q.x;

		return quat4f(qx, qy, qz, qw);
	}

	quat4f& operator *= (const quat4f& q)
	{
		float qw = w*q.w - x*q.x - y*q.y - z*q.z;
		float qx = w*q.x + x*q.w + y*q.z - z*q.y;
		float qy = w*q.y + y*q.w + z*q.x - x*q.z;
		float qz = w*q.z + z*q.w + x*q.y - y*q.x;

		x = qx;
		y = qy;
		z = qz;
		w = qw;

		return *this;
	}

	quat4f operator*(const float a) const
	{
		return quat4f(x*a, y*a, z*a, w*a);
	}

	// division

	quat4f operator / (const float a) const
	{
		return quat4f(x/a, y/a, z/a, w/a);
	}

	quat4f& operator /= (const float a)
	{
		x /= a;
		y /= a;
		z /= a;
		w /= a;

		return *this;
	}

	// Special ops

	quat4f Conjugate() const { return quat4f(-x, -y, -z, w); }

	float Norm() const { return w*w + x*x + y*y + z*z; } 

	void MakeUnit() 
	{
		float N = (float) sqrt(w*w + x*x + y*y + z*z);

		if (N != 0)
		{
			x /= N;
			y /= N;
			z /= N;
			w /= N;
		}
		else w = 1.f;
	}

	quat4f Inverse() const
	{
		float N = w*w + x*x + y*y + z*z;
		if (N == 0.f) N = 1.f;

		return quat4f(-x/N, -y/N, -z/N, w/N);
	}

	float DotProduct(const quat4f& q) const
	{
		return w*q.w + x*q.x + y*q.y + z*q.z;
	}

	vec3f GetVector() const
	{
		return vec3f(x, y, z).Normalize();
	}

	float GetAngle() const
	{
		return (float)(acos(w)*2.0);
	}

/*	quat4f& MultiplyAngle(float fa)
	{
		float angle = fa*acos(w)*2.0;

		w = cos(angle * 0.5);

		float sina = sin(angle * 0.5);

		x *= sina;
		y *= sina;
		z *= sina;
	}
*/
	vec3f operator * (const vec3f& r)
	{
		vec3f p = r;
		RotateVector(p);
		return p;
	}

	// use only when *this is unit vector
	void RotateVector(vec3f& v) const
	{
		if ((w == 0) || ((x==0) && (y==0) && (z==0))) return;

		// v*q^-1
		float qw = v.x*x + v.y*y + v.z*z;
		float qx = v.x*w - v.y*z + v.z*y;
		float qy = v.y*w - v.z*x + v.x*z;
		float qz = v.z*w - v.x*y + v.y*x;

		// q* (v* q^-1)
		v.x = (float) (w*qx + x*qw + y*qz - z*qy);
		v.y = (float) (w*qy + y*qw + z*qx - x*qz);
		v.z = (float) (w*qz + z*qw + x*qy - y*qx);
	}

	void RotateVectorP(float* v, float* r) const
	{
		static float fx, fy, fz, fw;
		static float qw, qx, qy, qz;
		
		fx = (float) x;
		fy = (float) y;
		fz = (float) z;
		fw = (float) w;

		qw = v[0]*fx + v[1]*fy + v[2]*fz;
		qx = v[0]*fw - v[1]*fz + v[2]*fy;
		qy = v[1]*fw - v[2]*fx + v[0]*fz;
		qz = v[2]*fw - v[0]*fy + v[1]*fx;

		r[0] = (float) (fw*qx + fx*qw + fy*qz - fz*qy);
		r[1] = (float) (fw*qy + fy*qw + fz*qx - fx*qz);
		r[2] = (float) (fw*qz + fz*qw + fx*qy - fy*qx);
	}

	static double dot(quat4f &q1, quat4f &q2) 
	{ return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w; }

	static quat4f lerp(quat4f &q1, quat4f &q2, float t) 
	{ quat4f q = (q1*(1.f-t) + q2*t); q.MakeUnit(); return q; }

	static quat4f slerp(quat4f &q1, quat4f &q2, double t);

	// set a quaternion defined via the XYZ Euler angles (in radians)
	// Convention is first rotate about z, then x, and then y
	// This conforms to the Tait-Bryan angles (roll, pitch, yaw)
	void SetEuler(double x, double y, double z);
	void GetEuler(double& x, double& y, double& z) const;

public:
	float w;
	float x, y, z;
};

inline quat4f operator * (const float a, const quat4f& q)
{
	return q*a;
}

///////////////////////////////////////////////////////////////////
// mat3f

class mat3f
{
public:
	mat3f() { zero(); }

	mat3f(float a00, float a01, float a02, float a10, float a11, float a12, float a20, float a21, float a22)
	{
		m_data[0][0] = a00; m_data[0][1] = a01; m_data[0][2] = a02;
		m_data[1][0] = a10; m_data[1][1] = a11; m_data[1][2] = a12;
		m_data[2][0] = a20; m_data[2][1] = a21; m_data[2][2] = a22;
	}

	mat3f(const mat3fs& a)
	{
		m_data[0][0] = a.x ; m_data[0][1] = a.xy; m_data[0][2] = a.xz;
		m_data[1][0] = a.xy; m_data[1][1] = a.y ; m_data[1][2] = a.yz;
		m_data[2][0] = a.xz; m_data[2][1] = a.yz; m_data[2][2] = a.z ;
	}

	float* operator [] (int i) { return m_data[i]; }
	float& operator () (int i, int j) { return m_data[i][j]; }
	float operator () (int i, int j) const { return m_data[i][j]; }

	mat3f operator * (mat3f& m)
	{
		mat3f a;

		int k;
		for (k=0; k<3; k++)
		{
			a[0][0] += m_data[0][k]*m[k][0]; a[0][1] += m_data[0][k]*m[k][1]; a[0][2] += m_data[0][k]*m[k][2];
			a[1][0] += m_data[1][k]*m[k][0]; a[1][1] += m_data[1][k]*m[k][1]; a[1][2] += m_data[1][k]*m[k][2];
			a[2][0] += m_data[2][k]*m[k][0]; a[2][1] += m_data[2][k]*m[k][1]; a[2][2] += m_data[2][k]*m[k][2];
		}

		return a;
	}

	mat3f& operator *= (float g)
	{
		m_data[0][0] *= g;	m_data[0][1] *= g; m_data[0][2] *= g;
		m_data[1][0] *= g;	m_data[1][1] *= g; m_data[1][2] *= g;
		m_data[2][0] *= g;	m_data[2][1] *= g; m_data[2][2] *= g;
		return (*this);
	}

	mat3f& operator /= (float g)
	{
		m_data[0][0] /= g;	m_data[0][1] /= g; m_data[0][2] /= g;
		m_data[1][0] /= g;	m_data[1][1] /= g; m_data[1][2] /= g;
		m_data[2][0] /= g;	m_data[2][1] /= g; m_data[2][2] /= g;
		return (*this);
	}

	mat3f operator += (const mat3f& a)
	{
		m_data[0][0] += a.m_data[0][0]; m_data[0][1] += a.m_data[0][1]; m_data[0][2] += a.m_data[0][2];
		m_data[1][0] += a.m_data[1][0]; m_data[1][1] += a.m_data[1][1]; m_data[1][2] += a.m_data[1][2];
		m_data[2][0] += a.m_data[2][0]; m_data[2][1] += a.m_data[2][1]; m_data[2][2] += a.m_data[2][2];
		return (*this);
	}

	mat3f operator -= (const mat3f& a)
	{
		m_data[0][0] -= a.m_data[0][0]; m_data[0][1] -= a.m_data[0][1]; m_data[0][2] -= a.m_data[0][2];
		m_data[1][0] -= a.m_data[1][0]; m_data[1][1] -= a.m_data[1][1]; m_data[1][2] -= a.m_data[1][2];
		m_data[2][0] -= a.m_data[2][0]; m_data[2][1] -= a.m_data[2][1]; m_data[2][2] -= a.m_data[2][2];
		return (*this);
	}

	mat3fs sym() const
	{
		return mat3fs(m_data[0][0], m_data[1][1], m_data[2][2], 0.5f*(m_data[0][1] + m_data[1][0]), 0.5f*(m_data[1][2] + m_data[2][1]), 0.5f*(m_data[0][2] + m_data[2][0]));
	}

	void zero()
	{
		m_data[0][0] = m_data[0][1] = m_data[0][2] = 0.f;
		m_data[1][0] = m_data[1][1] = m_data[1][2] = 0.f;
		m_data[2][0] = m_data[2][1] = m_data[2][2] = 0.f;
	}

	vec3f col(int i) const
	{
		vec3f r;
		switch (i)
		{
		case 0: r.x = m_data[0][0]; r.y = m_data[1][0]; r.z = m_data[2][0]; break;
		case 1: r.x = m_data[0][1]; r.y = m_data[1][1]; r.z = m_data[2][1]; break;
		case 2: r.x = m_data[0][2]; r.y = m_data[1][2]; r.z = m_data[2][2]; break;
		}
		return r;
	}

	vec3f row(int i) const
	{
		vec3f r;
		switch (i)
		{
		case 0: r.x = m_data[0][0]; r.y = m_data[0][1]; r.z = m_data[0][2]; break;
		case 1: r.x = m_data[1][0]; r.y = m_data[1][1]; r.z = m_data[1][2]; break;
		case 2: r.x = m_data[2][0]; r.y = m_data[2][1]; r.z = m_data[2][2]; break;
		}
		return r;
	}

public:
	float m_data[3][3];
};

///////////////////////////////////////////////////////////////////
// Mat3d

class Mat3d
{
public:
	Mat3d() 
	{
		zero();
	}

	Mat3d(double a00, double a01, double a02, double a10, double a11, double a12, double a20, double a21, double a22);

	double* operator [] (int i) { return m_data[i]; }
	double& operator () (int i, int j) { return m_data[i][j]; }
	double operator () (int i, int j) const { return m_data[i][j]; }

	Mat3d operator * (Mat3d& m)
	{
		Mat3d a;

		int k;
		for (k=0; k<3; k++)
		{
			a[0][0] += m_data[0][k]*m[k][0]; a[0][1] += m_data[0][k]*m[k][1]; a[0][2] += m_data[0][k]*m[k][2];
			a[1][0] += m_data[1][k]*m[k][0]; a[1][1] += m_data[1][k]*m[k][1]; a[1][2] += m_data[1][k]*m[k][2];
			a[2][0] += m_data[2][k]*m[k][0]; a[2][1] += m_data[2][k]*m[k][1]; a[2][2] += m_data[2][k]*m[k][2];
		}

		return a;
	}

	Mat3d& operator *= (double g)
	{
		m_data[0][0] *= g;	m_data[0][1] *= g; m_data[0][2] *= g;
		m_data[1][0] *= g;	m_data[1][1] *= g; m_data[1][2] *= g;
		m_data[2][0] *= g;	m_data[2][1] *= g; m_data[2][2] *= g;
		return (*this);
	}

	vec3f operator * (const vec3f& v);

	double Invert()
	{
		// calculate determinant
		double det = 0.0;
		det += m_data[0][0]*m_data[1][1]*m_data[2][2];
		det += m_data[0][1]*m_data[1][2]*m_data[2][0];
		det += m_data[0][2]*m_data[1][0]*m_data[2][1];
		det -= m_data[0][2]*m_data[1][1]*m_data[2][0];
		det -= m_data[0][1]*m_data[1][0]*m_data[2][2];
		det -= m_data[0][0]*m_data[1][2]*m_data[2][1];

		if (det == 0.0) return det;
		double deti = 1.0 / det;

		// calculate conjugate Matrix
		double mi[3][3];

		mi[0][0] =  (m_data[1][1]*m_data[2][2] - m_data[1][2]*m_data[2][1]);
		mi[0][1] = -(m_data[1][0]*m_data[2][2] - m_data[1][2]*m_data[2][0]);
		mi[0][2] =  (m_data[1][0]*m_data[2][1] - m_data[1][1]*m_data[2][0]);

		mi[1][0] = -(m_data[0][1]*m_data[2][2] - m_data[0][2]*m_data[2][1]);
		mi[1][1] =  (m_data[0][0]*m_data[2][2] - m_data[0][2]*m_data[2][0]);
		mi[1][2] = -(m_data[0][0]*m_data[2][1] - m_data[0][1]*m_data[2][0]);

		mi[2][0] =  (m_data[0][1]*m_data[1][2] - m_data[0][2]*m_data[1][1]);
		mi[2][1] = -(m_data[0][0]*m_data[1][2] - m_data[0][2]*m_data[1][0]);
		mi[2][2] =  (m_data[0][0]*m_data[1][1] - m_data[0][1]*m_data[1][0]);

		// divide by det and transpose
		m_data[0][0] = mi[0][0]*deti; m_data[1][0] = mi[0][1]*deti; m_data[2][0] = mi[0][2]*deti;
		m_data[0][1] = mi[1][0]*deti; m_data[1][1] = mi[1][1]*deti; m_data[2][1] = mi[1][2]*deti;
		m_data[0][2] = mi[2][0]*deti; m_data[1][2] = mi[2][1]*deti; m_data[2][2] = mi[2][2]*deti;

		// return determinant
		return det;
	}

	void zero()
	{
		m_data[0][0] = m_data[0][1] = m_data[0][2] = 0.0;
		m_data[1][0] = m_data[1][1] = m_data[1][2] = 0.0;
		m_data[2][0] = m_data[2][1] = m_data[2][2] = 0.0;
	}

	Mat3d transpose();

protected:
	double	m_data[3][3];
};

//=============================================================================
class Matrix
{
public:
	Matrix(int r, int c);
	Matrix(const Matrix& m);
	~Matrix() { delete [] d; }
	void operator = (const Matrix& m);

	void zero();

	double* operator [] (int i) { return d + i*m_nc; }
	double& operator () (int i, int j) { return d[i*m_nc + j]; }
	double operator () (int i, int j) const { return d[i*m_nc + j]; }

	bool solve(vector<double>& x, vector<double>& b);

	bool lsq_solve(vector<double>& x, vector<double>& b);

	int Rows() { return m_nr; }

	void mult(vector<double>& x, vector<double>& y);
	void mult_transpose(vector<double>& x, vector<double>& y);

	void mult_transpose_self(Matrix& AAt);

	//! Matrix inversion
	Matrix inverse();

	//! multiplication
	Matrix operator * (const Matrix& m);

private:
	double*		d;
	int		m_nr, m_nc;
	int		m_ne;
};

//-----------------------------------------------------------------------------
//! class for 4th order tensors with major and minor symmetries.

// Due to the major symmetry we can store this tensor as a 6x6 Matrix.
// The tensor is stored in column major order:
//
//     | 0   1   3   6   10   15  |
//     |     2   4   7   11   16  |
//     |         5   8   12   17  |
// A = |             9   13   18  |
//     |                 14   19  |
//     |                      20  |
//
// Note that due to the minor symmetry we only store the upper Matrix of this tensor
//

class tens4fs
{
public:
	enum { NNZ = 21 };
    
	// default constructor
	tens4fs(){}
	tens4fs(const float g)
	{
		d[ 0] = g;
		d[ 1] = g; d[ 2] = g;
		d[ 3] = g; d[ 4] = g; d[ 5] = g;
		d[ 6] = g; d[ 7] = g; d[ 8] = g; d[ 9] = g;
		d[10] = g; d[11] = g; d[12] = g; d[13] = g; d[14] = g;
		d[15] = g; d[16] = g; d[17] = g; d[18] = g; d[19] = g; d[20] = g;
	}
    
	tens4fs(float m[6][6])
	{
		d[ 0] = m[0][0];
		d[ 1] = m[0][1]; d[ 2] = m[1][1];
		d[ 3] = m[0][2]; d[ 4] = m[1][2]; d[ 5] = m[2][2];
		d[ 6] = m[0][3]; d[ 7] = m[1][3]; d[ 8] = m[2][3]; d[ 9] = m[3][3];
		d[10] = m[0][4]; d[11] = m[1][4]; d[12] = m[2][4]; d[13] = m[3][4]; d[14] = m[4][4];
		d[15] = m[0][5]; d[16] = m[1][5]; d[17] = m[2][5]; d[18] = m[3][5]; d[19] = m[4][5]; d[20] = m[5][5];
	}
    
	tens4fs(float D[21])
	{
		d[ 0] = D[ 0];
		d[ 1] = D[ 1]; d[ 2] = D[ 2];
		d[ 3] = D[ 3]; d[ 4] = D[ 4]; d[ 5] = D[ 5];
		d[ 6] = D[ 6]; d[ 7] = D[ 7]; d[ 8] = D[ 8]; d[ 9] = D[ 9];
		d[10] = D[10]; d[11] = D[11]; d[12] = D[12]; d[13] = D[13]; d[14] = D[14];
		d[15] = D[15]; d[16] = D[16]; d[17] = D[17]; d[18] = D[18]; d[19] = D[19]; d[20] = D[20];
	}
    
	float& operator () (int i, int j, int k, int l)
	{
		const int m[3][3] = {{0,3,5},{3,1,4},{5,4,2}};
		tens4fs& T = (*this);
		return T(m[i][j], m[k][l]);
	}
    
	float operator () (int i, int j, int k, int l) const
	{
		const int m[3][3] = {{0,3,5},{3,1,4},{5,4,2}};
		const tens4fs& T = (*this);
		return T(m[i][j], m[k][l]);
	}
    
	float& operator () (int i, int j)
	{
		const int m[6] = {0, 1, 3, 6, 10, 15};
		if (i<=j) return d[m[j]+i]; else return d[m[i]+j];
	}
    
	float operator () (int i, int j) const
	{
		const int m[6] = {0, 1, 3, 6, 10, 15};
		if (i<=j) return d[m[j]+i]; else return d[m[i]+j];
	}
    
	// arithmetic operators
	tens4fs operator + (const tens4fs& t) const {
        tens4fs s;
        for (int i=0; i<NNZ; i++) s.d[i] = d[i] + t.d[i];
        return s;
    }
	tens4fs operator - (const tens4fs& t) const {
        tens4fs s;
        for (int i=0; i<NNZ; i++) s.d[i] = d[i] - t.d[i];
        return s;
    }
	tens4fs operator * (float g) const {
        tens4fs s;
        for (int i=0; i<NNZ; i++) s.d[i] = g*d[i];
        return s;
    }
	tens4fs operator / (float g) const {
        tens4fs s;
        for (int i=0; i<NNZ; i++) s.d[i] = d[i]/g;
        return s;
    }
    
	// arithmetic assignment operators
	tens4fs& operator += (const tens4fs& t) {
        for (int i=0; i<NNZ; i++) d[i] += t.d[i];
        return (*this);
    }
	tens4fs& operator -= (const tens4fs& t) {
        for (int i=0; i<NNZ; i++) d[i] -= t.d[i];
        return (*this);
    }
	tens4fs& operator *= (float g) {
        for (int i=0; i<NNZ; i++) d[i] *= g;
        return (*this);
    }
	tens4fs& operator /= (float g) {
        for (int i=0; i<NNZ; i++) d[i] /= g;
        return (*this);
    }
    
	// unary operators
	tens4fs operator - () const {
        tens4fs s;
        for (int i=0; i<NNZ; i++) s.d[i] = -d[i];
        return s;
    }
	
	// double dot product with tensor
	mat3fs dot(const mat3fs& m) const {
        mat3fs a;
        a.x = d[ 0]*m.x + d[ 1]*m.y + d[ 3]*m.z + 2*d[ 6]*m.xy + 2*d[10]*m.yz + 2*d[15]*m.xz;
        a.y = d[ 1]*m.x + d[ 2]*m.y + d[ 4]*m.z + 2*d[ 7]*m.xy + 2*d[11]*m.yz + 2*d[16]*m.xz;
        a.z = d[ 3]*m.x + d[ 4]*m.y + d[ 5]*m.z + 2*d[ 8]*m.xy + 2*d[12]*m.yz + 2*d[17]*m.xz;
        a.xy = d[ 6]*m.x + d[ 7]*m.y + d[ 8]*m.z + 2*d[ 9]*m.xy + 2*d[13]*m.yz + 2*d[18]*m.xz;
        a.yz = d[10]*m.x + d[11]*m.y + d[12]*m.z + 2*d[13]*m.xy + 2*d[14]*m.yz + 2*d[19]*m.xz;
        a.xz = d[15]*m.x + d[16]*m.y + d[17]*m.z + 2*d[18]*m.xy + 2*d[19]*m.yz + 2*d[20]*m.xz;
        return a;
    }
    
	// trace
	float tr() const {
        return (d[0]+d[2]+d[5]+2*(d[1]+d[3]+d[4]));
    }
	
	// initialize to zero
	void zero() {
        d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = d[7] = d[8] = d[9] =
        d[10] = d[11] = d[12] = d[13] = d[14] = d[15] = d[16] = d[17] = d[18] = d[19] = d[20] = 0;
    }
    
	// extract 6x6 Matrix
	void extract(float D[6][6]) {
        D[0][0] = d[0];  D[0][1] = d[1];  D[0][2] = d[3];  D[0][3] = d[6];  D[0][4] = d[10]; D[0][5] = d[15];
        D[1][0] = d[1];  D[1][1] = d[2];  D[1][2] = d[4];  D[1][3] = d[7];  D[1][4] = d[11]; D[1][5] = d[16];
        D[2][0] = d[3];  D[2][1] = d[4];  D[2][2] = d[5];  D[2][3] = d[8];  D[2][4] = d[12]; D[2][5] = d[17];
        D[3][0] = d[6];  D[3][1] = d[7];  D[3][2] = d[8];  D[3][3] = d[9];  D[3][4] = d[13]; D[3][5] = d[18];
        D[4][0] = d[10]; D[4][1] = d[11]; D[4][2] = d[12]; D[4][3] = d[13]; D[4][4] = d[14]; D[4][5] = d[19];
        D[5][0] = d[15]; D[5][1] = d[16]; D[5][2] = d[17]; D[5][3] = d[18]; D[5][4] = d[19]; D[5][5] = d[20];
    }
    
	// calculates the inverse
//	tens4fs inverse() const;
    
public:
	float d[NNZ];	// stored in column major order
};

#endif // _MATH3D_H_
