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

// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Image.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// CImage
//////////////////////////////////////////////////////////////////////

CImage::CImage()
{
	m_pb = 0;
    m_pixelType = UINT_8;
    m_bps = 1;
	m_cx = m_cy = 0;
	m_bdel = true;
}

CImage::CImage(int nx, int ny, int pixelType)
{
    m_pixelType = pixelType;

    switch (pixelType)
    {
    case CImage::INT_8     : m_bps = 1; break;
    case CImage::UINT_8    : m_bps = 1; break;
    case CImage::INT_16    :
    case CImage::UINT_16   : m_bps = 2; break;
    case CImage::INT_32    :
    case CImage::UINT_32   : m_bps = 4; break;
    case CImage::INT_RGB8  :
    case CImage::UINT_RGB8 : m_bps = 3; break;
    case CImage::INT_RGB16 :
    case CImage::UINT_RGB16: m_bps = 6; break;
    case CImage::REAL_32   : m_bps = 4; break;
    case CImage::REAL_64   : m_bps = 8; break;
    default:
        assert(false);
    }

	m_pb = new uint8_t[nx*ny*m_bps] {0};

	m_cx = nx;
	m_cy = ny;

	m_bdel = true;
}

CImage::CImage(const CImage& im)
{
	m_cx = im.m_cx;
	m_cy = im.m_cy;
    m_pixelType = im.m_pixelType;
    m_bps = im.m_bps;

	m_pb = new uint8_t[m_cx*m_cy*m_bps];
	memcpy(m_pb, im.m_pb, m_cx*m_cy*m_bps);

	m_bdel = true;
}

bool CImage::IsRGB()
{
    return m_pixelType == INT_RGB8 || m_pixelType == UINT_RGB8 
        || m_pixelType == INT_RGB16 || m_pixelType == UINT_RGB16;
}

CImage& CImage::operator = (const CImage& im)
{
	if (m_bdel) delete [] m_pb;

	m_cx = im.m_cx;
	m_cy = im.m_cy;
    m_pixelType = im.m_pixelType;
    m_bps = im.m_bps;

	m_pb = new uint8_t[m_cx*m_cy*m_bps];
	memcpy(m_pb, im.m_pb, m_cx*m_cy*m_bps);

	m_bdel = true;

	return (*this);
}

CImage& CImage::operator -= (const CImage& im)
{
	uint8_t* pbS = im.m_pb;
	uint8_t* pbD = m_pb;

	int nsize = m_cx*m_cy*m_bps;

	for (int i=0; i<nsize; i++, pbS++, pbD++)
		*pbD = uint8_t((((int) *pbD - (int) *pbS) + 255) >> 1);

	return (*this);
}

CImage::~CImage()
{
	if (m_bdel) delete [] m_pb;
}

void CImage::Create(int nx, int ny, uint8_t* pb, int pixelType)
{
    m_cx = nx;
	m_cy = ny;
    
    m_pixelType = pixelType;

    switch (pixelType)
    {
    case CImage::INT_8     : m_bps = 1; break;
    case CImage::UINT_8    : m_bps = 1; break;
    case CImage::INT_16    :
    case CImage::UINT_16   : m_bps = 2; break;
    case CImage::INT_32    :
    case CImage::UINT_32   : m_bps = 4; break;
    case CImage::INT_RGB8  :
    case CImage::UINT_RGB8 : m_bps = 3; break;
    case CImage::INT_RGB16 :
    case CImage::UINT_RGB16: m_bps = 6; break;
    case CImage::REAL_32   : m_bps = 4; break;
    case CImage::REAL_64   : m_bps = 8; break;
    default:
        assert(false);
    }

	if (pb)
	{
		m_pb = pb;
		m_bdel = false;
	}
	else
	{
		if (m_bdel) delete [] m_pb;

		m_pb = new uint8_t[nx*ny*m_bps] {0};
		m_bdel = true;
	}
}

void CImage::Clear()
{
    m_cx = 0;
	m_cy = 0;

    if(m_pb)
    {
        if (m_bdel) delete [] m_pb;

        m_pb = nullptr;
    }
}

double CImage::Value(int i, int j, int channel)
{
    double h;
    switch (m_pixelType)
    {
    case CImage::UINT_8:
    {
        h = m_pb[m_cx*j + i];
        break;
    }
    case CImage::INT_8:
    {
        h = ((char*)m_pb)[m_cx*j + i];
        break;
    }
    case CImage::UINT_16:
    {
        h = ((uint16_t*)m_pb)[m_cx*j + i];
        break;
    }
    case CImage::INT_16:
    {
        h = ((int16_t*)m_pb)[m_cx*j + i];
        break;
    }
    case CImage::UINT_32:
    {
        h = ((uint32_t*)m_pb)[m_cx*j + i];
        break;
    }
    case CImage::INT_32:
    {
        h = ((int32_t*)m_pb)[m_cx*j + i];
        break;
    }
    case CImage::UINT_RGB8:
    {
        h = m_pb[(m_cx*j + i)*3 + channel];
        break;
    }
    case CImage::INT_RGB8:
    {
        h = ((char*)m_pb)[(m_cx*j + i)*3 + channel];
        break;
    }
    case CImage::UINT_RGB16:
    {
        h = ((uint16_t*)m_pb)[(m_cx*j + i)*3 + channel];
        break;
    }
    case CImage::INT_RGB16:
    {
        h = ((int16_t*)m_pb)[(m_cx*j + i)*3 + channel];
        break;
    }
    case CImage::REAL_32:
    {
        h = ((float*)m_pb)[m_cx*j + i];
        break;
    }
    case CImage::REAL_64:
    {
        h = ((double*)m_pb)[m_cx*j + i];
        break;
    }
    }

    return h;

}