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
typedef unsigned char Byte;
typedef unsigned short word;
typedef unsigned int dword;

////////////////////////////////////////////////////////////////////////
// CImage - Implements a gray scale image

class CImage  
{
public:
	CImage();
	CImage(int nx, int ny);
	CImage(const CImage& im);
	virtual ~CImage();

	CImage& operator = (const CImage& im);
	CImage& operator -= (const CImage& im);

	void Create(int nx, int ny, Byte* pb = 0);
	void StretchBlt(CImage& im);

	int Width () const { return m_cx; }
	int Height() const { return m_cy; }

	Byte* GetBytes() const { return m_pb; }

	Byte* GetPixel(int i, int j) { return m_pb + (j*m_cx + i); }

	Byte value(int i, int j) { return m_pb[j*m_cx + i]; }

	void Zero() { for (int i=0; i<m_cx*m_cy; i++) m_pb[i] = 0; }

protected:
	Byte* m_pb;	// rgb image data

	bool	m_bdel;	// delete image data at clean up ?
	
	int	m_cx;
	int	m_cy;
};
