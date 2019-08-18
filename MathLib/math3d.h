#pragma once
#include <math.h>
#include <assert.h>
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
// useful constants for trig
#ifndef PI
#define PI 3.1415926
#endif

#ifndef RAD2DEG
#define RAD2DEG (180.0/PI)
#endif

#ifndef DEG2RAD
#define DEG2RAD (PI/180.0)
#endif

//-----------------------------------------------------------------------------
// The following classes are defined in this file
class mat3d;     // general 3D matrix of doubles
class mat3ds;    // symmetric 3D matrix of doubles
class mat3da;    // anti-symmetric 3D matrix of doubles
class mat3dd;    // diagonal matrix of doubles

//-----------------------------------------------------------------------------
// class vec2d defines a 2D vector
class vec2d
{
public:
	vec2d() { x = y = 0.0; }
	vec2d(double X, double Y) { x = X; y = Y; }

	vec2d operator - () { return vec2d(-x, -y); }

	vec2d operator - (const vec2d& r) const { return vec2d(x - r.x, y - r.y); }
	vec2d operator + (const vec2d& r) const { return vec2d(x + r.x, y + r.y); }
	vec2d operator * (double g) const { return vec2d(x*g, y*g); }
    vec2d operator / (double a) { return vec2d(x/a, y/a); }
    
    vec2d& operator += (const vec2d& r) { x += r.x; y += r.y; return (*this); }
    vec2d& operator -= (const vec2d& r) { x -= r.x; y -= r.y; return (*this); }
    
    vec2d& operator *= (double a) { x*=a; y*=a; return (*this); }
    vec2d& operator /= (double a) { x/=a; y/=a; return (*this); }

	double operator * (const vec2d& r) const { return (x*r.x + y*r.y); }

	double norm() const { return sqrt(x*x + y*y); }
	double unit() { double R = sqrt(x*x + y*y); if (R != 0) {x /= R; y/= R; }; return R; }

	bool operator == (const vec2d& r) const { return (x==r.x)&&(y==r.y); }

public:
	double	x, y;
};

//-----------------------------------------------------------------------------
// class vec3d defines a 3D vector
//
class vec3d
{
public:
	vec3d() { x = y = z = 0; }
	vec3d(double rx, double ry, double rz) { x = rx; y = ry; z = rz; }
	vec3d(const vec2d& r) { x = r.x; y = r.y; z = 0; }

	vec3d operator + (const vec3d& v) const { return vec3d( x + v.x, y + v.y, z + v.z); }
	vec3d operator - (const vec3d& v) const { return vec3d( x - v.x, y - v.y, z - v.z); }
	vec3d operator ^ (const vec3d& v) const
	{ 
		return vec3d( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); 
	}

	double operator * (const vec3d& v) const { return (x*v.x + y*v.y + z*v.z); }

	vec3d operator * (const double g) const { return vec3d(x*g, y*g, z*g); }
	vec3d operator / (const double g) const { return vec3d(x/g, y/g, z/g); }

	const vec3d& operator += (const vec3d& v) { x += v.x; y += v.y; z += v.z; return (*this); }
	const vec3d& operator -= (const vec3d& v) { x -= v.x; y -= v.y; z -= v.z; return (*this); }
	const vec3d& operator /= (const double f) { x /= f; y /= f; z /= f; return (*this); }
	const vec3d& operator /= (const int n) { x /= n; y /= n; z /= n; return (*this); }
	const vec3d& operator *= (const double f) { x*=f; y*=f; z*=f; return (*this); }
    
    double& operator() (int i)
    {
        switch(i)
        {
            case 0: {return x; break;}
            case 1: {return y; break;}
            case 2: {return z; break;}
            default: {return x; break;}
        }
    }

    double operator() (int i) const
    {
        switch(i)
        {
            case 0: {return x; break;}
            case 1: {return y; break;}
            case 2: {return z; break;}
            default: {return x; break;}
        }
    }
    
	vec3d operator - () const { return vec3d(-x, -y, -z); }

	double Length() const { return (double) sqrt(x*x + y*y + z*z); }
	double SqrLength() const { return x*x + y*y + z*z; }
    

	vec3d& Normalize()
	{
		double L = Length();
		if (L != 0) { x /= L; y /= L; z /= L; }

		return (*this);
	}

	vec3d Normalized() const
	{
		double L = Length();
		if (L == 0) L = 1.0;
		return vec3d(x/L, y/L, z/L);
	}

	bool operator == (const vec3d& a) const
	{
		return ((x == a.x) && (y == a.y) && (z == a.z));
	}

public:
	double x, y, z;
};

///////////////////////////////////////////////////////////////////
// vec6d

class vec6d
{
public:
	vec6d() { x = y = z = xy = yz = xz = 0; }

public:
	double x, y, z;
	double xy, yz, xz;
};

///////////////////////////////////////////////////////////////////
// matrix

class matrix
{
public:
	matrix(int r, int c);
	matrix(const matrix& m);
	~matrix() { delete[] d; }

	void operator = (const matrix& m);

	void zero();

	double* operator [] (int i) { return d + i*m_nc; }
	double& operator () (int i, int j) { return d[i*m_nc + j]; }
	double operator () (int i, int j) const { return d[i*m_nc + j]; }
    matrix& operator += (const matrix& m);
    

	bool solve(vector<double>& x, vector<double>& b);

	bool lsq_solve(vector<double>& x, vector<double>& b);
	bool eigen_vectors(matrix& Eigen,vector<double>& eigen_values);
	int Rows() { return m_nr; }

	void mult(vector<double>& x, vector<double>& y);
	void mult_transpose(vector<double>& x, vector<double>& y);

	void mult_transpose_self(matrix& AAt);

	//! matrix inversion
	matrix inverse();

	//! multiplication
	matrix operator * (const matrix& m);

private:
	double*	d;
	int		m_nr, m_nc;
	int		m_ne;
};


///////////////////////////////////////////////////////////////////
// quatd

class quatd
{
public:
	// constructors
	quatd () { x = y = z = 0; w = 1; }

	quatd( const double angle, vec3d v)
	{
		w = (double) cos(angle * 0.5);

		double sina = (double) sin(angle * 0.5);

		v.Normalize();
		
		x = v.x*sina;
		y = v.y*sina;
		z = v.z*sina;
	}

	quatd (vec3d v1, vec3d v2)
	{
		vec3d n = v1^v2;
		n.Normalize();

		double d = v1*v2;

		double sina = (double) sqrt((1.0-d)*0.5);
		double cosa = (double) sqrt((1.0+d)*0.5);

		w = cosa;

		x = n.x*sina;
		y = n.y*sina;
		z = n.z*sina;

	}

	quatd(const double qx, const double qy, const double qz, const double qw = 1.0)
	{
		w = qw;
		x = qx;
		y = qy;
		z = qz;
	}

	bool operator != (const quatd& q) { return ((x!=q.x) || (y!=q.y) || (z!=q.z) || (w!=q.w)); }

	quatd operator - () { return quatd(-x, -y, -z, -w); }

	// addition and substraction

	quatd operator + (const quatd& q) const
	{
		return quatd(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	quatd operator - (const quatd& q) const
	{
		return quatd(x - q.x, y - q.y, z - q.z, w - q.w);
	}

	quatd& operator += (const quatd& q)
	{
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;

		return *this;
	}

	quatd& operator -= (const quatd& q)
	{
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;

		return *this;
	}


	// multiplication

	quatd operator * (const quatd& q) const
	{
		double qw = w*q.w - x*q.x - y*q.y - z*q.z;
		double qx = w*q.x + x*q.w + y*q.z - z*q.y;
		double qy = w*q.y + y*q.w + z*q.x - x*q.z;
		double qz = w*q.z + z*q.w + x*q.y - y*q.x;

		return quatd(qx, qy, qz, qw);
	}

	quatd& operator *= (const quatd& q)
	{
		double qw = w*q.w - x*q.x - y*q.y - z*q.z;
		double qx = w*q.x + x*q.w + y*q.z - z*q.y;
		double qy = w*q.y + y*q.w + z*q.x - x*q.z;
		double qz = w*q.z + z*q.w + x*q.y - y*q.x;

		x = qx;
		y = qy;
		z = qz;
		w = qw;

		return *this;
	}

	quatd operator*(const double a) const
	{
		return quatd(x*a, y*a, z*a, w*a);
	}

	// division

	quatd operator / (const double a) const
	{
		return quatd(x/a, y/a, z/a, w/a);
	}

	quatd& operator /= (const double a)
	{
		x /= a;
		y /= a;
		z /= a;
		w /= a;

		return *this;
	}

	// Special ops

	quatd Conjugate() const { return quatd(-x, -y, -z, w); }

	double Norm() const { return w*w + x*x + y*y + z*z; } 

	void MakeUnit() 
	{
		double N = (double) sqrt(w*w + x*x + y*y + z*z);

		if (N != 0)
		{
			x /= N;
			y /= N;
			z /= N;
			w /= N;
		}
		else w = 1.f;
	}

	quatd Inverse() const
	{
		double N = w*w + x*x + y*y + z*z;

		return quatd(-x/N, -y/N, -z/N, w/N);
	}

	double DotProduct(const quatd& q) const
	{
		return w*q.w + x*q.x + y*q.y + z*q.z;
	}

	vec3d GetVector() const
	{
		return vec3d(x, y, z).Normalize();
	}

	double GetAngle() const
	{
		return (double)(acos(w)*2.0);
	}

/*	quatd& MultiplyAngle(double fa)
	{
		double angle = fa*acos(w)*2.0;

		w = cos(angle * 0.5);

		double sina = sin(angle * 0.5);

		x *= sina;
		y *= sina;
		z *= sina;
	}
*/


	// use only when *this is unit vector
	void RotateVector(vec3d& v) const
	{
		if ((w == 0) || ((x==0) && (y==0) && (z==0))) return;

		// v*q^-1
		double qw = v.x*x + v.y*y + v.z*z;
		double qx = v.x*w - v.y*z + v.z*y;
		double qy = v.y*w - v.z*x + v.x*z;
		double qz = v.z*w - v.x*y + v.y*x;

		// q* (v* q^-1)
		v.x = (double) (w*qx + x*qw + y*qz - z*qy);
		v.y = (double) (w*qy + y*qw + z*qx - x*qz);
		v.z = (double) (w*qz + z*qw + x*qy - y*qx);
	}

	// use only when *this is unit vector
	vec3d operator * (const vec3d& r) const
	{
		vec3d n = r;

		// v*q^-1
		double qw = n.x*x + n.y*y + n.z*z;
		double qx = n.x*w - n.y*z + n.z*y;
		double qy = n.y*w - n.z*x + n.x*z;
		double qz = n.z*w - n.x*y + n.y*x;

		// q* (v* q^-1)
		n.x = (w*qx + x*qw + y*qz - z*qy);
		n.y = (w*qy + y*qw + z*qx - x*qz);
		n.z = (w*qz + z*qw + x*qy - y*qx);

		return n;
	}
/*
	mat3d operator * (mat3d m) const
	{
		mat3d a;
		double qw, qx, qy, qz;
		for (int i=0; i<3; ++i)
		{
			// v*q^-1
			qw = m[0][i]*x + m[1][i]*y + m[2][i]*z;
			qx = m[0][i]*w - m[1][i]*z + m[2][i]*y;
			qy = m[1][i]*w - m[2][i]*x + m[0][i]*z;
			qz = m[2][i]*w - m[0][i]*y + m[1][i]*x;

			// q* (v* q^-1)
			a[0][i] = (w*qx + x*qw + y*qz - z*qy);
			a[1][i] = (w*qy + y*qw + z*qx - x*qz);
			a[2][i] = (w*qz + z*qw + x*qy - y*qx);
		}

		return a;
	}
*/
	void RotateVectorP(double* v, double* r) const
	{
		static double fx, fy, fz, fw;
		static double qw, qx, qy, qz;
		
		fx = (double) x;
		fy = (double) y;
		fz = (double) z;
		fw = (double) w;

		qw = v[0]*fx + v[1]*fy + v[2]*fz;
		qx = v[0]*fw - v[1]*fz + v[2]*fy;
		qy = v[1]*fw - v[2]*fx + v[0]*fz;
		qz = v[2]*fw - v[0]*fy + v[1]*fx;

		r[0] = (double) (fw*qx + fx*qw + fy*qz - fz*qy);
		r[1] = (double) (fw*qy + fy*qw + fz*qx - fx*qz);
		r[2] = (double) (fw*qz + fz*qw + fx*qy - fy*qx);
	}

	static double dot(quatd &q1, quatd &q2) 
	{ return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w; }

	static quatd lerp(quatd &q1, quatd &q2, double t) 
	{ quatd q = (q1*(1-t) + q2*t); q.MakeUnit(); return q; }

	static quatd slerp(quatd &q1, quatd &q2, double t) ;

	// set a quaternion defined via the XYZ Euler angles (in degrees)
	// Convention is first rotate about z, then x, and then y
	// This conforms to the Tait-Bryan angles (roll, pitch, yaw)
	void SetEuler(double x, double y, double z);
	void GetEuler(double& x, double& y, double& z) const;

public:
	double x, y, z;
	double w;
};

inline quatd operator * (const double a, const quatd& q)
{
	return q*a;
}
