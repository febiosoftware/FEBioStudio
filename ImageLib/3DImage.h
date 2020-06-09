#pragma once
#include "Image.h"

//-----------------------------------------------------------------------------
// A class for representing 3D image stacks
class C3DImage
{
public:
	C3DImage();
	virtual ~C3DImage();
	void CleanUp();

	bool Create(int nx, int ny, int nz);

	bool LoadFromFile(const char* szfile, int nbits);

	void BitBlt(CImage& im, int nslice);
	void StretchBlt(CImage& im, int nslice);
	void StretchBlt(C3DImage& im);

	int Width () { return m_cx; }
	int Height() { return m_cy; }
	int Depth () { return m_cz; }

	byte& value(int i, int j, int k) { return m_pb[m_cx*(k*m_cy + j) + i]; }
	byte Value(double fx, double fy, int nz);
	byte Peek(double fx, double fy, double fz);

	void Histogram(int* pdf);

	void GetSliceX(CImage& im, int n);
	void GetSliceY(CImage& im, int n);
	void GetSliceZ(CImage& im, int n);

	void GetSampledSliceX(CImage& im, double f);
	void GetSampledSliceY(CImage& im, double f);
	void GetSampledSliceZ(CImage& im, double f);

	void Invert();

	byte* GetBytes() { return m_pb; }

	void Zero();

	void FlipZ();

protected:
	byte*	m_pb;	// image data
	int		m_cx;
	int		m_cy;
	int		m_cz;
};

//-----------------------------------------------------------------------------
// helper functions
int closest_pow2(int n);