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
#include "RGBAImage.h"

#ifdef WIN32
#include <memory>
#else
#include <memory.h>
#endif

CRGBAImage::CRGBAImage()
{
	m_pb = 0;
	m_cx = m_cy = 0;
}

CRGBAImage::CRGBAImage(int nx, int ny, const void* imgdata)
{
	m_pb = new uint8_t[nx*ny * 4];

	if (imgdata)
		memcpy(m_pb, imgdata, 4 * nx * ny);
	else
		for (int i = 0; i<nx*ny * 4; i++) m_pb[i] = 0;

	m_cx = nx;
	m_cy = ny;
}

CRGBAImage::CRGBAImage(const CRGBAImage& im)
{
	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new uint8_t[m_cx*m_cy * 4];
	memcpy(m_pb, im.m_pb, m_cx*m_cy * 4);
}

CRGBAImage& CRGBAImage::operator = (const CRGBAImage& im)
{
	delete[] m_pb;

	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new uint8_t[m_cx*m_cy * 4];
	memcpy(m_pb, im.m_pb, m_cx*m_cy * 4);

	return (*this);
}

CRGBAImage::~CRGBAImage()
{
	delete[] m_pb;
}

void CRGBAImage::Create(int nx, int ny)
{
	if (m_pb) delete[] m_pb;

	m_pb = new uint8_t[nx*ny * 4];
	for (int i = 0; i<nx*ny * 4; i++) m_pb[i] = 0;

	m_cx = nx;
	m_cy = ny;
}

void CRGBAImage::StretchBlt(CRGBAImage& im)
{
	uint8_t* pd = im.m_pb;

	int nx = im.Width();
	int ny = im.Height();

	int i0 = 0;
	int j0 = 0;

	uint8_t* p0, *p1, *p2, *p3;
	int h0, h1, h2, h3;
	int w = 0, h = 0;

	int Hx = nx - 1;
	int Hy = ny - 1;
	int H = Hx*Hy;

	for (int y = 0; y<ny; y++)
	{
		i0 = 0;
		w = 0;

		while (y*(m_cy - 1)>(j0 + 1)*(ny - 1)) { j0++; h -= (ny - 1); }

		p0 = m_pb + 4 * (j0*m_cx);
		p1 = p0 + 4;
		p2 = p0 + 4 * m_cx;
		p3 = p2 + 4;

		for (int x = 0; x<nx; x++)
		{
			while (x*(m_cx - 1)>(i0 + 1)*(nx - 1)) { i0++; w -= (nx - 1); p0 += 4; p1 += 4; p2 += 4; p3 += 4; }

			h0 = (Hx - w)*(Hy - h);
			h1 = w*(Hy - h);
			h2 = (Hx - w)*h;
			h3 = w*h;

			pd[0] = (h0*p0[0] + h1*p1[0] + h2*p2[0] + h3*p3[0]) / H;
			pd[1] = (h0*p0[1] + h1*p1[1] + h2*p2[1] + h3*p3[1]) / H;
			pd[2] = (h0*p0[2] + h1*p1[2] + h2*p2[2] + h3*p3[2]) / H;
			pd[3] = (h0*p0[3] + h1*p1[3] + h2*p2[3] + h3*p3[3]) / H;

			pd += 4;

			w += (m_cx - 1);
		}

		h += (m_cy - 1);
	}
}
