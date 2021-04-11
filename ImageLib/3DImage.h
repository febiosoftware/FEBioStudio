/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#pragma once
#include "Image.h"
#include <teem/nrrd.h>

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

	Byte& value(int i, int j, int k) { return m_pb[m_cx*(k*m_cy + j) + i]; }
	Byte Value(double fx, double fy, int nz);
	Byte Peek(double fx, double fy, double fz);

	void Histogram(int* pdf);

	void GetSliceX(CImage& im, int n);
	void GetSliceY(CImage& im, int n);
	void GetSliceZ(CImage& im, int n);

	void GetSampledSliceX(CImage& im, double f);
	void GetSampledSliceY(CImage& im, double f);
	void GetSampledSliceZ(CImage& im, double f);

	void Invert();

	Byte* GetBytes() { return m_pb; }

	void Zero();

	void FlipZ();

protected:
	Byte*	m_pb;	// image data
	int		m_cx;
	int		m_cy;
	int		m_cz;
};

//-----------------------------------------------------------------------------
// helper functions
int closest_pow2(int n);
