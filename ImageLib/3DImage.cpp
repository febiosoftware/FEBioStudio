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

#include "stdafx.h"
#include "3DImage.h"
#include <stdio.h>
#include <math.h>
#include <memory>

//-----------------------------------------------------------------------------
// find the power of 2 that is closest to n
int closest_pow2(int n)
{
	const int PMIN = 16;
	const int PMAX = 512;
	int p = (int)pow(2.0, (int)(0.5 + log((double)n) / log(2.0)));
	if (p < PMIN) p = PMIN;
	if (p > PMAX) p = PMAX;
	return p;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C3DImage::C3DImage()
{
	m_pb = 0;
	m_cx = m_cy = m_cz = 0;
}

C3DImage::~C3DImage()
{
	CleanUp();
}

void C3DImage::CleanUp()
{
	delete [] m_pb;
	m_pb = 0;
	m_cx = m_cy = m_cz = 0;
}

bool C3DImage::Create(int nx, int ny, int nz, Byte* data)
{
    // Check to make sure this does not allocate memory of size 0.
    if(nx*ny*nz == 0)
      return false;

	// reallocate data if necessary
	if (nx*ny*nz != m_cx*m_cy*m_cz)
	{
	  CleanUp();

      if(data == nullptr)
      {
        m_pb = new Byte[nx*ny*nz];
        if (m_pb == nullptr) return false;
      }
      else
        m_pb = data;
	}

	m_cx = nx;
	m_cy = ny;
	m_cz = nz;

	return true;
}

bool C3DImage::LoadFromFile(const char* szfile, int nbits)
{
	FILE* fp = fopen(szfile, "rb");
	if (fp == 0) return false;

	size_t nsize = m_cx*m_cy*m_cz;
	if (nbits == 16)
	{
		word* m_ptmp = new word[nsize];
		size_t nread = fread(m_ptmp, sizeof(word), nsize, fp);
		for(size_t i=0; i<nsize; i++)
			m_pb[i] = m_ptmp[i] >> 8;
		delete [] m_ptmp;
		if (nsize != nread) return false;
	}
	else
	{
		size_t nread = fread(m_pb, 1, nsize, fp);
		if (nsize != nread) return false;
	}

	// cleanup
	fclose(fp);

	return true;
}

// BitBlt assumes that the 3D and 2D images have the same resolution !
void C3DImage::BitBlt(CImage& im, int nslice)
{
	// go to the beginning of the slice
	Byte* ps = m_pb + nslice*m_cx*m_cy;
	Byte* pd = im.GetBytes();

	// copy image data
	int n = m_cx*m_cy;
	for (int i=0; i<n; i++) *pd++ = *ps++;
}

void C3DImage::StretchBlt(CImage& im, int nslice)
{
	Byte* pd = im.GetBytes();

	int nx = im.Width();
	int ny = im.Height();

	int i0 = 0;
	int j0 = 0;

	Byte* p0, *p1, *p2, *p3;
	int h0, h1, h2, h3;
	int w = 0, h = 0;

	int Hx = nx - 1;
	int Hy = ny - 1;
	int H = Hx*Hy;

	for (int y=0; y<ny; y++)
	{
		i0 = 0;
		w = 0;

		while (y*(m_cy-1)>(j0+1)*(ny-1)) { j0++; h -= (ny-1); }

		p0 = nslice*m_cx*m_cy + m_pb + (j0*m_cx);
		p1 = p0 + 1;
		p2 = p0 + m_cx;
		p3 = p2 + 1;

		for (int x=0; x<nx; x++)
		{
			while (x*(m_cx-1)>(i0+1)*(nx-1)) { i0++; w -= (nx-1); p0++; p1++; p2++; p3++; }

			h0 = (Hx - w)*(Hy - h);
			h1 = w*(Hy - h);
			h2 = (Hx - w)*h;
			h3 = w*h;

			*pd++ = (h0*p0[0] + h1*p1[0] + h2*p2[0] + h3*p3[0])/H;

			w += (m_cx-1);
		}

		h += (m_cy-1);
	}
}

void C3DImage::StretchBlt(C3DImage& im)
{
	int nx = im.Width();
	int ny = im.Height();
	int nz = im.Depth();
	Byte* pb = im.m_pb;
	for (int k=0; k<nz; ++k)
		for (int j=0; j<ny; ++j)
			for (int i=0; i<nx; ++i, ++pb)
			{
				double r = (double) i / (double) (nx - 1);
				double s = (double) j / (double) (ny - 1);
				double t = (double) k / (double) (nz - 1);
				*pb = Peek(r, s, t);
			}
}


Byte C3DImage::Value(double fx, double fy, int nz)
{
	double r, s;

	Byte* pb = m_pb + nz*m_cx*m_cy;

	int ix = (int) ((m_cx-1)*fx);
	int iy = (int) ((m_cy-1)*fy);

	if (ix == (m_cx - 1)) { ix--; r = 1; } else r = 2*(((m_cx-1)*fx) - ix)-1;
	if (iy == (m_cy - 1)) { iy--; s = 1; } else s = 2*(((m_cy-1)*fy) - iy)-1;

	double h;
	h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
	h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
	h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
	h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];

	return (Byte)(0.25*h);
}

Byte C3DImage::Peek(double r, double s, double t)
{
	static int n1,n2,n3,n4,n5,n6,n7,n8;
	static double h1,h2,h3,h4,h5,h6,h7,h8;

	if (r < 0) r = 0; if (r > 1) r = 1;
	if (s < 0) s = 0; if (s > 1) s = 1;
	if (t < 0) t = 0; if (t > 1) t = 1;

	int i = (int)(r*(m_cx-1)); if (i == (m_cx - 1)) i = m_cx - 2;
	int j = (int)(s*(m_cy-1)); if (j == (m_cy - 1)) j = m_cy - 2;
	int k = (int)(t*(m_cz-1)); if (k == (m_cz - 1)) k = m_cz - 2;

	r = 2.0*(r*(m_cx-1) - i) - 1.0;
	s = 2.0*(s*(m_cy-1) - j) - 1.0;
	t = 2.0*(t*(m_cz-1) - k) - 1.0;

	n1 = i + j*m_cx + k*m_cx*m_cy;
	n2 = n1 + 1;
	n3 = n2 + m_cx;
	n4 = n3 - 1;
	n5 = n1 + (m_cz > 1 ? m_cx*m_cy : 0);
	n6 = n5 + 1;
	n7 = n6 + m_cx;
	n8 = n7 - 1;

	h1 = (1-r)*(1-s)*(1-t);
	h2 = (1+r)*(1-s)*(1-t);
	h3 = (1+r)*(1+s)*(1-t);
	h4 = (1-r)*(1+s)*(1-t);
	h5 = (1-r)*(1-s)*(1+t);
	h6 = (1+r)*(1-s)*(1+t);
	h7 = (1+r)*(1+s)*(1+t);
	h8 = (1-r)*(1+s)*(1+t);
	
	Byte* pb = m_pb;

	return (Byte)((h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125);
}


void C3DImage::Histogram(int* pdf)
{
	int i;
	for (i=0; i<256; i++) pdf[i] = 0;

	Byte* pb = m_pb;
	int nsize = m_cx*m_cy*m_cz;

	for (i=0; i<nsize; i++) pdf[ *pb++ ]++;
}

void C3DImage::GetSliceX(CImage& im, int n)
{
	// create image data
	if ((im.Width() != m_cy) || (im.Height() != m_cz)) im.Create(m_cy, m_cz);

	Byte* ps;
	Byte* pd = im.GetBytes();

	// copy image data
	for (int z=0; z<m_cz; z++)
	{
		ps = m_pb + z*m_cx*m_cy + n;
		for (int y=0; y<m_cy; y++, ps += m_cx)	*pd++ = *ps;
	}
}

void C3DImage::GetSliceY(CImage& im, int n)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cz)) im.Create(m_cx, m_cz);

	Byte* ps;
	Byte* pd = im.GetBytes();

	// copy image data
	for (int z=0; z<m_cz; z++)
	{
		ps = m_pb + z*m_cx*m_cy + n*m_cx;
		for (int x=0; x<m_cx; x++, ps++) *pd++ = *ps;
	}
}

void C3DImage::GetSliceZ(CImage& im, int n)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cy)) im.Create(m_cx, m_cy);

	// copy image data
	Byte* pd = im.GetBytes();
	Byte* ps = m_pb + n*m_cx*m_cy;

	for (int i=0; i<m_cx*m_cy; i++, ps++) *pd++ = *ps;
}

void C3DImage::GetSampledSliceX(CImage& im, double f)
{
	// create image data
	if ((im.Width() != m_cy) || (im.Height() != m_cz)) im.Create(m_cy, m_cz);

	Byte* pd = im.GetBytes();

	// copy image data
	for (int z = 0; z<m_cz; z++)
	{
		double fz = z / (double) (m_cz - 1.0);
		for (int y = 0; y<m_cy; y++)
		{
			double fy = y / (double) (m_cy - 1.0);
			*pd++ = Peek(f, fy, fz);
		}
	}
}

void C3DImage::GetSampledSliceY(CImage& im, double f)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cz)) im.Create(m_cx, m_cz);

	Byte* pd = im.GetBytes();

	// copy image data
	for (int z = 0; z<m_cz; z++)
	{
		double fz = z / (double)(m_cz - 1.0);
		for (int x = 0; x<m_cx; x++)
		{
			double fx = x / (double)(m_cx - 1.0);
			*pd++ = Peek(fx, f, fz);
		}
	}
}

void C3DImage::GetSampledSliceZ(CImage& im, double f)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cy)) im.Create(m_cx, m_cy);

	// copy image data
	Byte* pd = im.GetBytes();

	for (int y = 0; y<m_cy; y++)
	{
		double fy = y / (double)(m_cy - 1.0);
		for (int x = 0; x<m_cx; x++)
		{
			double fx = x / (double)(m_cx - 1.0);
			*pd++ = Peek(fx, fy, f);
		}
	}
}

void C3DImage::Invert()
{
	int n = m_cx*m_cy*m_cz;
	for (int i=0; i<n; i++) m_pb[i] = 255 - m_pb[i];
}

void C3DImage::Zero()
{
	int n = m_cx*m_cy*m_cz;
	for (int i=0; i<n; i++) m_pb[i] = 0;
}

void C3DImage::FlipZ()
{
	int nsize = m_cx*m_cy;
	Byte* buf = new Byte[nsize];
	for (int i = 0; i < m_cz / 2; ++i)
	{
		Byte* slice0 = m_pb + i*nsize;
		Byte* slice1 = m_pb + (m_cz - i - 1)*nsize;
		memcpy(buf, slice0, nsize);
		memcpy(slice0, slice1, nsize);
		memcpy(slice1, buf, nsize);
	}
    delete[] buf;
}

