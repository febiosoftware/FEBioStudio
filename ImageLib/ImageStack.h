#pragma once
#include "Image.h"

//-----------------------------------------------------------------------------
// A class for representing 3D image stacks
class ImageStack
{
public:
	ImageStack();
	virtual ~ImageStack();
	void CleanUp();

	bool Create(int nx, int ny, int nz);

	bool LoadFromFile(const char* szfile, int nbits);

	void BitBlt(Image& im, int nslice);
	void StretchBlt(Image& im, int nslice);
	void StretchBlt(ImageStack& im);

	int Width () { return m_cx; }
	int Height() { return m_cy; }
	int Depth () { return m_cz; }

	byte& value(int i, int j, int k) { return m_pb[m_cx*(k*m_cy + j) + i]; }
	byte Value(double fx, double fy, int nz);
	byte Peek(double fx, double fy, double fz);

	void Histogram(int* pdf);

	void GetSliceX(Image& im, int n);
	void GetSliceY(Image& im, int n);
	void GetSliceZ(Image& im, int n);

	void GetSampledSliceX(Image& im, double f);
	void GetSampledSliceY(Image& im, double f);
	void GetSampledSliceZ(Image& im, double f);

	void Invert();

	byte* GetBytes() { return m_pb; }

	void Zero();

protected:
	byte*	m_pb;	// image data
	int		m_cx;
	int		m_cy;
	int		m_cz;
};
