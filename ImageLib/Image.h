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

#pragma once

#include <cstdint>

////////////////////////////////////////////////////////////////////////
// CImage - Implements a gray scale image

class CImage  
{
public:
    enum { UINT_8, INT_8, UINT_16, INT_16, UINT_32, INT_32, UINT_RGB8, INT_RGB8, UINT_RGB16, INT_RGB16, REAL_32, REAL_64 };

public:
	CImage();
	CImage(int nx, int ny, int pixelType = UINT_8);
	CImage(const CImage& im);
	virtual ~CImage();

    int PixelType() { return m_pixelType; }
	int BPS() const { return m_bps; }
    bool IsRGB();

	CImage& operator = (const CImage& im);
	CImage& operator -= (const CImage& im);

	void Create(int nx, int ny, uint8_t* pb = 0, int pixelType = UINT_8);
    void Clear();

	int Width () const { return m_cx; }
	int Height() const { return m_cy; }

	uint8_t* GetBytes() const { return m_pb; }

	uint8_t* GetPixel(int i, int j) { return m_pb + (j*m_cx + i); }

    double Value(int i, int j, int channel = 0);

	void Zero() { for (int i=0; i<m_cx*m_cy; i++) m_pb[i] = 0; }

protected:
	uint8_t*   m_pb;	// image data
    int     m_pixelType; // pixel representation
	int		m_bps;	// bytes per sample

	bool	m_bdel;	// delete image data at clean up ?
	
	int	m_cx;
	int	m_cy;
};
