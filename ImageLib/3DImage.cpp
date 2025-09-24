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
#include "3DImage.h"
#include <stdio.h>
#include <math.h>
#include <memory>
#include <cstring>
#include <string>
#include <algorithm>
#include <fstream>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C3DImage::C3DImage() : m_pb(nullptr), m_cx(0), m_cy(0), m_cz(0), m_bps(1),
    m_pixelType(CImage::UINT_8), m_box(0, 0, 0, 1, 1, 1), m_orientation(mat3d::identity())
{
	m_maxValue = 1;
	m_minValue = 0;
}

C3DImage::~C3DImage()
{
	CleanUp();
}

void C3DImage::CleanUp()
{
	if(m_pb) delete [] m_pb;
	m_pb = nullptr;
	m_cx = m_cy = m_cz = 0;
}

bool C3DImage::Create(int nx, int ny, int nz, uint8_t* data, int pixelType)
{
    // Check to make sure this does not allocate memory of size 0.
    if(nx*ny*nz == 0)
      return false;

	// reallocate data if necessary
	if ((nx*ny*nz != m_cx*m_cy*m_cz) || (m_pixelType != pixelType))
	{
	    CleanUp();

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
			m_pixelType = CImage::UINT_8;
		}

        if(data == nullptr)
        {
            uint64_t newSize = (uint64_t)nx * (uint64_t)ny * (uint64_t)nz * (uint64_t)m_bps;
            m_pb = new uint8_t[newSize];

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

bool C3DImage::IsRGB()
{
    return m_pixelType == CImage::INT_RGB8 || m_pixelType == CImage::UINT_RGB8 
        || m_pixelType == CImage::INT_RGB16 || m_pixelType == CImage::UINT_RGB16;
}

std::string C3DImage::PixelTypeString()
{
    switch (m_pixelType)
    {
    case CImage::UINT_8:
        return "8-bit Unsigned Integer";
    case CImage::INT_8:
        return "8-bit Signed Integer";
    case CImage::UINT_16:
        return "16-bit Unsigned Integer";
    case CImage::INT_16:
        return "16-bit Signed Integer";
    case CImage::UINT_32:
        return "32-bit Unsigned Integer";
    case CImage::INT_32:
        return "32-bit Signed Integer";
    case CImage::UINT_RGB8:
        return "8-bit Unsigned Integer RGB";
    case CImage::INT_RGB8:
        return "8-bit Signed Integer RGB";
    case CImage::UINT_RGB16:
        return "16-bit Unsigned Integer RGB";
    case CImage::INT_RGB16:
        return "16-bit Signed Integer RGB";
    case CImage::REAL_32:
        return "32-bit Real";
    case CImage::REAL_64:
        return "64-bit Real";
    default:
        assert(false);
    }
	return "(unknown)";
}

double C3DImage::Value(int i, int j, int k, int channel)
{
    double h;
    switch (m_pixelType)
    {
    case CImage::UINT_8:
    {
        h = m_pb[m_cx*(k*m_cy + j) + i];
        break;
    }
    case CImage::INT_8:
    {
        h = ((char*)m_pb)[m_cx*(k*m_cy + j) + i];
        break;
    }
    case CImage::UINT_16:
    {
        h = ((uint16_t*)m_pb)[m_cx*(k*m_cy + j) + i];
        break;
    }
    case CImage::INT_16:
    {
        h = ((int16_t*)m_pb)[m_cx*(k*m_cy + j) + i];
        break;
    }
    case CImage::UINT_32:
    {
        h = ((uint32_t*)m_pb)[m_cx*(k*m_cy + j) + i];
        break;
    }
    case CImage::INT_32:
    {
        h = ((int32_t*)m_pb)[m_cx*(k*m_cy + j) + i];
        break;
    }
    case CImage::UINT_RGB8:
    {
        h = m_pb[(m_cx*(k*m_cy + j) + i)*3 + channel];
        break;
    }
    case CImage::INT_RGB8:
    {
        h = ((char*)m_pb)[(m_cx*(k*m_cy + j) + i)*3 + channel];
        break;
    }
    case CImage::UINT_RGB16:
    {
        h = ((uint16_t*)m_pb)[(m_cx*(k*m_cy + j) + i)*3 + channel];
        break;
    }
    case CImage::INT_RGB16:
    {
        h = ((int16_t*)m_pb)[(m_cx*(k*m_cy + j) + i)*3 + channel];
        break;
    }
    case CImage::REAL_32:
    {
        h = ((float*)m_pb)[m_cx*(k*m_cy + j) + i];
        break;
    }
    case CImage::REAL_64:
    {
        h = ((double*)m_pb)[m_cx*(k*m_cy + j) + i];
        break;
    }
    }

    return h;
}

double C3DImage::Value(double fx, double fy, int nz, int channel)
{
	double r, s;

	int ix = (int) ((m_cx-1)*fx);
	int iy = (int) ((m_cy-1)*fy);

	if (ix == (m_cx - 1)) { ix--; r = 1; } else r = 2*(((m_cx-1)*fx) - ix)-1;
	if (iy == (m_cy - 1)) { iy--; s = 1; } else s = 2*(((m_cy-1)*fy) - iy)-1;

	double h;
    switch (m_pixelType)
    {
    case CImage::UINT_8:
    {
        uint8_t* pb = m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    case CImage::INT_8:
    {
        char* pb = (char*)m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    case CImage::UINT_16:
    {
        uint16_t* pb = (uint16_t*)m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    case CImage::INT_16:
    {
        int16_t* pb = (int16_t*)m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    case CImage::UINT_32:
    {
        uint32_t* pb = (uint32_t*)m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    case CImage::INT_32:
    {
        int32_t* pb = (int32_t*)m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    case CImage::UINT_RGB8:
    {
        uint8_t* pb = (uint8_t*)m_pb + nz*m_cx*m_cy*3 + channel;
        h  = (1-r)*(1-s)*pb[ix*3   +  iy*3*m_cx];
        h += (1+r)*(1-s)*pb[ix*3+3 +  iy*3*m_cx];
        h += (1+r)*(1+s)*pb[ix*3+3 + (iy*3+3)*m_cx*3];
        h += (1-r)*(1+s)*pb[ix*3   + (iy*3+3)*m_cx*3];
        break;
    }
    case CImage::INT_RGB8:
    {
        int8_t* pb = (int8_t*)m_pb + nz*m_cx*m_cy*3 + channel;
        h  = (1-r)*(1-s)*pb[ix*3   +  iy*3*m_cx];
        h += (1+r)*(1-s)*pb[ix*3+3 +  iy*3*m_cx];
        h += (1+r)*(1+s)*pb[ix*3+3 + (iy*3+3)*m_cx*3];
        h += (1-r)*(1+s)*pb[ix*3   + (iy*3+3)*m_cx*3];
        break;
    }
    case CImage::UINT_RGB16:
    {
        uint16_t* pb = (uint16_t*)m_pb + nz*m_cx*m_cy*3 + channel;
        h  = (1-r)*(1-s)*pb[ix*3   +  iy*3*m_cx];
        h += (1+r)*(1-s)*pb[ix*3+3 +  iy*3*m_cx];
        h += (1+r)*(1+s)*pb[ix*3+3 + (iy*3+3)*m_cx*3];
        h += (1-r)*(1+s)*pb[ix*3   + (iy*3+3)*m_cx*3];
        break;
    }
    case CImage::INT_RGB16:
    {
        int16_t* pb = (int16_t*)m_pb + nz*m_cx*m_cy*3 + channel;
        h  = (1-r)*(1-s)*pb[ix*3   +  iy*3*m_cx];
        h += (1+r)*(1-s)*pb[ix*3+3 +  iy*3*m_cx];
        h += (1+r)*(1+s)*pb[ix*3+3 + (iy*3+3)*m_cx*3];
        h += (1-r)*(1+s)*pb[ix*3   + (iy*3+3)*m_cx*3];
        break;
    }
    case CImage::REAL_32:
    {
        float* pb = (float*)m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    case CImage::REAL_64:
    {
        double* pb = (double*)m_pb + nz*m_cx*m_cy;
        h  = (1-r)*(1-s)*pb[ix   +  iy   *m_cx];
        h += (1+r)*(1-s)*pb[ix+1 +  iy   *m_cx];
        h += (1+r)*(1+s)*pb[ix+1 + (iy+1)*m_cx];
        h += (1-r)*(1+s)*pb[ix   + (iy+1)*m_cx];
        break;
    }
    }

	return 0.25*h;
}

double C3DImage::Peek(double r, double s, double t, int channel)
{
	int n1,n2,n3,n4,n5,n6,n7,n8;
	double h1,h2,h3,h4,h5,h6,h7,h8;

	if (r < 0) r = 0; if (r > 1) r = 1;
	if (s < 0) s = 0; if (s > 1) s = 1;
	if (t < 0) t = 0; if (t > 1) t = 1;

	int i = (int)(r*(m_cx-1)); if (i == (m_cx - 1)) i = m_cx - 2;
	int j = (int)(s*(m_cy-1)); if (j == (m_cy - 1)) j = m_cy - 2;
	int k = (int)(t*(m_cz-1)); if (k == (m_cz - 1)) k = m_cz - 2;

	r = 2.0*(r*(m_cx-1) - i) - 1.0;
	s = 2.0*(s*(m_cy-1) - j) - 1.0;
	t = 2.0*(t*(m_cz-1) - k) - 1.0;

    if(IsRGB())
    {
        n1 = (i + j*m_cx + k*m_cx*m_cy) + channel;
        n2 = n1 + 3;
        n3 = n2 + m_cx*3;
        n4 = n3 - 3;
        n5 = n1 + (m_cz > 1 ? m_cx*m_cy*3 : 0);
        n6 = n5 + 3;
        n7 = n6 + m_cx*3;
        n8 = n7 - 3;
    }
    else
    {
        n1 = i + j*m_cx + k*m_cx*m_cy;
        n2 = n1 + 1;
        n3 = n2 + m_cx;
        n4 = n3 - 1;
        n5 = n1 + (m_cz > 1 ? m_cx*m_cy : 0);
        n6 = n5 + 1;
        n7 = n6 + m_cx;
        n8 = n7 - 1;
    }
	

	h1 = (1-r)*(1-s)*(1-t);
	h2 = (1+r)*(1-s)*(1-t);
	h3 = (1+r)*(1+s)*(1-t);
	h4 = (1-r)*(1+s)*(1-t);
	h5 = (1-r)*(1-s)*(1+t);
	h6 = (1+r)*(1-s)*(1+t);
	h7 = (1+r)*(1+s)*(1+t);
	h8 = (1-r)*(1+s)*(1+t);
    
    double val;

    switch (m_pixelType)
    {
    case CImage::UINT_8:
    {
        uint8_t* pb = m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::INT_8:
    {
        char* pb = (char*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::UINT_16:
    {
        uint16_t* pb = (uint16_t*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::INT_16:
    {
        int16_t* pb = (int16_t*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::UINT_32:
    {
        uint32_t* pb = (uint32_t*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::INT_32:
    {
        int32_t* pb = (int32_t*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::UINT_RGB8:
    {
        uint8_t* pb = m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::INT_RGB8:
    {
        char* pb = (char*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::UINT_RGB16:
    {
        uint16_t* pb = (uint16_t*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::INT_RGB16:
    {
        int16_t* pb = (int16_t*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::REAL_32:
    {
        float* pb = (float*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    case CImage::REAL_64:
    {
        double* pb = (double*)m_pb;
        val = (h1*pb[n1]+h2*pb[n2]+h3*pb[n3]+h4*pb[n4]+h5*pb[n5]+h6*pb[n6]+h7*pb[n7]+h8*pb[n8])*0.125;
        break;
    }
    }

    return val;
}

double C3DImage::ValueAtGlobalPos(vec3d pos, int channel)
{
    vec3d locPos = m_orientation.transpose()*(pos - vec3d(m_box.x0, m_box.y0, m_box.z0));
    double relX = locPos.x/(m_box.x1 - m_box.x0);
    double relY = locPos.y/(m_box.y1 - m_box.y0);
    double relZ = locPos.z/(m_box.z1 - m_box.z0);

    if(relX < 0 || relX > 1 ||
        relY < 0 || relY > 1 ||
        relZ < 0 || relZ > 1 )
    {
        return 0;
    }

	if (Depth() == 1)
	{
		return Value(relX, relY, 0, channel);
	}
	else
	{
		return Peek(relX, relY, relZ, channel);
	}
}

template <class pType> 
void C3DImage::CopySliceX(pType* dest, int n, int channels)
{
    pType* ps;
    pType* orig = (pType*)m_pb;

    for (int z=0; z<m_cz; z++)
    {
        ps = orig + z*m_cx*m_cy*channels + n*channels;
        for (int y=0; y<m_cy; y++, ps += m_cx*channels)
        {
            for(int ch = 0; ch < channels; ch++) *dest++ = ps[ch];
        } 
    }
}

void C3DImage::GetSliceX(CImage& im, int n)
{
	// create image data
	if ((im.Width() != m_cy) || (im.Height() != m_cz) || im.PixelType() != m_pixelType) 
        im.Create(m_cy, m_cz, nullptr, m_pixelType);

    assert(im.PixelType() == m_pixelType);

    switch (m_pixelType)
    {
    case CImage::UINT_8:
        CopySliceX<uint8_t>(im.GetBytes(), n);
        break;
    case CImage::INT_8:
        CopySliceX<int8_t>((int8_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_16:
        CopySliceX<uint16_t>((uint16_t*)im.GetBytes(), n);
        break;
    case CImage::INT_16:
        CopySliceX<int16_t>((int16_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_32:
        CopySliceX<uint32_t>((uint32_t*)im.GetBytes(), n);
        break;
    case CImage::INT_32:
        CopySliceX<int32_t>((int32_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_RGB8:
        CopySliceX<uint8_t>((uint8_t*)im.GetBytes(), n, 3);
        break;
    case CImage::INT_RGB8:
        CopySliceX<int8_t>((int8_t*)im.GetBytes(), n, 3);
        break;
    case CImage::UINT_RGB16:
        CopySliceX<uint16_t>((uint16_t*)im.GetBytes(), n, 3);
        break;
    case CImage::INT_RGB16:
        CopySliceX<int16_t>((int16_t*)im.GetBytes(), n, 3);
        break;
    case CImage::REAL_32:
        CopySliceX<float>((float*)im.GetBytes(), n);
        break;
    case CImage::REAL_64:
        CopySliceX<double>((double*)im.GetBytes(), n);
        break;
    default:
        assert(false);
    }
}

template <class pType> void C3DImage::CopySliceY(pType* dest, int n, int channels)
{
    pType* ps;
    pType* orig = (pType*) m_pb;

    for (int z=0; z<m_cz; z++)
    {
        ps = orig + z*m_cx*m_cy*channels + n*m_cx*channels;
        for (int x=0; x<m_cx; x++, ps+=channels)
        {
            for(int ch = 0; ch < channels; ch++) *dest++ = ps[ch];
        } 
    }
}

void C3DImage::GetSliceY(CImage& im, int n)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cz) || im.PixelType() != m_pixelType) 
        im.Create(m_cx, m_cz, nullptr, m_pixelType);

    assert(im.PixelType() == m_pixelType);

	switch (m_pixelType)
    {
    case CImage::UINT_8:
        CopySliceY<uint8_t>(im.GetBytes(), n);
        break;
    case CImage::INT_8:
        CopySliceY<int8_t>((int8_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_16:
        CopySliceY<uint16_t>((uint16_t*)im.GetBytes(), n);
        break;
    case CImage::INT_16:
        CopySliceY<int16_t>((int16_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_32:
        CopySliceY<uint32_t>((uint32_t*)im.GetBytes(), n);
        break;
    case CImage::INT_32:
        CopySliceY<int32_t>((int32_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_RGB8:
        CopySliceY<uint8_t>((uint8_t*)im.GetBytes(), n, 3);
        break;
    case CImage::INT_RGB8:
        CopySliceY<int8_t>((int8_t*)im.GetBytes(), n, 3);
        break;
    case CImage::UINT_RGB16:
        CopySliceY<uint16_t>((uint16_t*)im.GetBytes(), n, 3);
        break;
    case CImage::INT_RGB16:
        CopySliceY<int16_t>((int16_t*)im.GetBytes(), n, 3);
        break;
    case CImage::REAL_32:
        CopySliceY<float>((float*)im.GetBytes(), n);
        break;
    case CImage::REAL_64:
        CopySliceY<double>((double*)im.GetBytes(), n);
        break;
    default:
        assert(false);
    }
	
}

template <class pType> void C3DImage::CopySliceZ(pType* dest, int n, int channels)
{
    pType* orig = (pType*) m_pb;
    pType* ps = orig + n*m_cx*m_cy*channels;

    for (int i=0; i<m_cx*m_cy*channels; i++, ps++) *dest++ = *ps;
}

void C3DImage::GetSliceZ(CImage& im, int n)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cy) || im.PixelType() != m_pixelType) 
        im.Create(m_cx, m_cy, nullptr, m_pixelType);

    assert(im.PixelType() == m_pixelType);

	switch (m_pixelType)
    {
    case CImage::UINT_8:
        CopySliceZ<uint8_t>(im.GetBytes(), n);
        break;
    case CImage::INT_8:
        CopySliceZ<int8_t>((int8_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_16:
        CopySliceZ<uint16_t>((uint16_t*)im.GetBytes(), n);
        break;
    case CImage::INT_16:
        CopySliceZ<int16_t>((int16_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_32:
        CopySliceZ<uint32_t>((uint32_t*)im.GetBytes(), n);
        break;
    case CImage::INT_32:
        CopySliceZ<int32_t>((int32_t*)im.GetBytes(), n);
        break;
    case CImage::UINT_RGB8:
        CopySliceZ<uint8_t>((uint8_t*)im.GetBytes(), n, 3);
        break;
    case CImage::INT_RGB8:
        CopySliceZ<int8_t>((int8_t*)im.GetBytes(), n, 3);
        break;
    case CImage::UINT_RGB16:
        CopySliceZ<uint16_t>((uint16_t*)im.GetBytes(), n, 3);
        break;
    case CImage::INT_RGB16:
        CopySliceZ<int16_t>((int16_t*)im.GetBytes(), n, 3);
        break;
    case CImage::REAL_32:
        CopySliceZ<float>((float*)im.GetBytes(), n);
        break;
    case CImage::REAL_64:
        CopySliceZ<double>((double*)im.GetBytes(), n);
        break;
    default:
        assert(false);
    }
}

template <class pType> void C3DImage::CopySampledSliceX(pType* dest, double f, int channels)
{  
    for (int z = 0; z<m_cz; z++)
    {
        double fz = z / (double) (m_cz - 1.0);
        for (int y = 0; y<m_cy; y++)
        {
            double fy = y / (double) (m_cy - 1.0);

            for(int ch = 0; ch < channels; ch++) *dest++ = (pType)Peek(f, fy, fz, ch);
        }
    }
}

void C3DImage::GetSampledSliceX(CImage& im, double f)
{
	// create image data
	if ((im.Width() != m_cy) || (im.Height() != m_cz) || im.PixelType() != m_pixelType) 
        im.Create(m_cy, m_cz, nullptr, m_pixelType);

    assert(im.PixelType() == m_pixelType);

	switch (m_pixelType)
    {
    case CImage::UINT_8:
        CopySampledSliceX<uint8_t>(im.GetBytes(), f);
        break;
    case CImage::INT_8:
        CopySampledSliceX<int8_t>((int8_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_16:
        CopySampledSliceX<uint16_t>((uint16_t*)im.GetBytes(), f);
        break;
    case CImage::INT_16:
        CopySampledSliceX<int16_t>((int16_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_32:
        CopySampledSliceX<uint32_t>((uint32_t*)im.GetBytes(), f);
        break;
    case CImage::INT_32:
        CopySampledSliceX<int32_t>((int32_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_RGB8:
        CopySampledSliceX<uint8_t>((uint8_t*)im.GetBytes(), f, 3);
        break;
    case CImage::INT_RGB8:
        CopySampledSliceX<int8_t>((int8_t*)im.GetBytes(), f, 3);
        break;
    case CImage::UINT_RGB16:
        CopySampledSliceX<uint16_t>((uint16_t*)im.GetBytes(), f, 3);
        break;
    case CImage::INT_RGB16:
        CopySampledSliceX<int16_t>((int16_t*)im.GetBytes(), f, 3);
        break;
    case CImage::REAL_32:
        CopySampledSliceX<float>((float*)im.GetBytes(), f);
        break;
    case CImage::REAL_64:
        CopySampledSliceX<double>((double*)im.GetBytes(), f);
        break;
    default:
        assert(false);
    }
}

template <class pType> void C3DImage::CopySampledSliceY(pType* dest, double f, int channels)
{  
    for (int z = 0; z<m_cz; z++)
    {
        double fz = z / (double)(m_cz - 1.0);
        for (int x = 0; x<m_cx; x++)
        {
            double fx = x / (double)(m_cx - 1.0);
            for(int ch = 0; ch < channels; ch++) *dest++ = (pType)Peek(fx, f, fz, ch);
            *dest++ = (pType)Peek(fx, f, fz, 1);
            *dest++ = (pType)Peek(fx, f, fz, 2);
        }
    }
}

void C3DImage::GetSampledSliceY(CImage& im, double f)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cz) || im.PixelType() != m_pixelType) 
        im.Create(m_cx, m_cz, nullptr, m_pixelType);

    assert(im.PixelType() == m_pixelType);

	switch (m_pixelType)
    {
    case CImage::UINT_8:
        CopySampledSliceY<uint8_t>(im.GetBytes(), f);
        break;
    case CImage::INT_8:
        CopySampledSliceY<int8_t>((int8_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_16:
        CopySampledSliceY<uint16_t>((uint16_t*)im.GetBytes(), f);
        break;
    case CImage::INT_16:
        CopySampledSliceY<int16_t>((int16_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_32:
        CopySampledSliceY<uint32_t>((uint32_t*)im.GetBytes(), f);
        break;
    case CImage::INT_32:
        CopySampledSliceY<int32_t>((int32_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_RGB8:
        CopySampledSliceY<uint8_t>((uint8_t*)im.GetBytes(), f, 3);
        break;
    case CImage::INT_RGB8:
        CopySampledSliceY<int8_t>((int8_t*)im.GetBytes(), f, 3);
        break;
    case CImage::UINT_RGB16:
        CopySampledSliceY<uint16_t>((uint16_t*)im.GetBytes(), f, 3);
        break;
    case CImage::INT_RGB16:
        CopySampledSliceY<int16_t>((int16_t*)im.GetBytes(), f, 3);
        break;
    case CImage::REAL_32:
        CopySampledSliceY<float>((float*)im.GetBytes(), f);
        break;
    case CImage::REAL_64:
        CopySampledSliceY<double>((double*)im.GetBytes(), f);
        break;
    default:
        assert(false);
    }
}

template <class pType> void C3DImage::CopySampledSliceZ(pType* dest, double f, int channels)
{
    if (Depth() == 1)
    {
        for (int y = 0; y < m_cy; y++)
        {
            double fy = y / (double)(m_cy - 1.0);
            for (int x = 0; x < m_cx; x++)
            {
                double fx = x / (double)(m_cx - 1.0);
                for(int ch = 0; ch < channels; ch++) *dest++ = (pType)Value(fx, fy, 0, ch);
            }
        }
    }
    else
    {
        for (int y = 0; y < m_cy; y++)
        {
            double fy = y / (double)(m_cy - 1.0);
            for (int x = 0; x < m_cx; x++)
            {
                double fx = x / (double)(m_cx - 1.0);
                for(int ch = 0; ch < channels; ch++) *dest++ = (pType)Peek(fx, fy, f, ch);
            }
        }
    }
}

void C3DImage::GetSampledSliceZ(CImage& im, double f)
{
	// create image data
	if ((im.Width() != m_cx) || (im.Height() != m_cy) || im.PixelType() != m_pixelType) 
        im.Create(m_cx, m_cy, nullptr, m_pixelType);

    assert(im.PixelType() == m_pixelType);

	switch (m_pixelType)
    {
    case CImage::UINT_8:
        CopySampledSliceZ<uint8_t>(im.GetBytes(), f);
        break;
    case CImage::INT_8:
        CopySampledSliceZ<int8_t>((int8_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_16:
        CopySampledSliceZ<uint16_t>((uint16_t*)im.GetBytes(), f);
        break;
    case CImage::INT_16:
        CopySampledSliceZ<int16_t>((int16_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_32:
        CopySampledSliceZ<uint32_t>((uint32_t*)im.GetBytes(), f);
        break;
    case CImage::INT_32:
        CopySampledSliceZ<int32_t>((int32_t*)im.GetBytes(), f);
        break;
    case CImage::UINT_RGB8:
        CopySampledSliceZ<uint8_t>((uint8_t*)im.GetBytes(), f, 3);
        break;
    case CImage::INT_RGB8:
        CopySampledSliceZ<int8_t>((int8_t*)im.GetBytes(), f, 3);
        break;
    case CImage::UINT_RGB16:
        CopySampledSliceZ<uint16_t>((uint16_t*)im.GetBytes(), f, 3);
        break;
    case CImage::INT_RGB16:
        CopySampledSliceZ<int16_t>((int16_t*)im.GetBytes(), f, 3);
        break;
    case CImage::REAL_32:
        CopySampledSliceZ<float>((float*)im.GetBytes(), f);
        break;
    case CImage::REAL_64:
        CopySampledSliceZ<double>((double*)im.GetBytes(), f);
        break;
    default:
        assert(false);
    }
}

template <class pType> void C3DImage::CalcMinMaxValue()
{
    size_t N  = m_cx*m_cy*m_cz;
    if(IsRGB())
    {
        N *= 3;
    }

    pType* data = (pType*)m_pb;

    double maxValue = data[0], minValue = data[0];
    #pragma omp parallel shared(maxValue, minValue) firstprivate(data)
	{
		double threadMax = data[0], threadMin = data[0];
        #pragma omp for
		for (int i = 0; i < N; ++i)
		{
			if (data[i] > threadMax) threadMax = data[i];
			if (data[i] < threadMin) threadMin = data[i];
		}
        
        #pragma omp critical
		{
			if (threadMax > maxValue) maxValue = threadMax;
			if (threadMin < minValue) minValue = threadMin;
		}
	}

	m_maxValue = maxValue;
	m_minValue = minValue;
}

void C3DImage::GetMinMax(double& min, double& max, bool recalc)
{
    if(recalc)
    {
        switch (m_pixelType)
        {
        case CImage::UINT_8:
            CalcMinMaxValue<uint8_t>();
            break;
        case CImage::INT_8:
            CalcMinMaxValue<int8_t>();
            break;
        case CImage::UINT_16:
            CalcMinMaxValue<uint16_t>();
            break;
        case CImage::INT_16:
            CalcMinMaxValue<int16_t>();
            break;
        case CImage::UINT_32:
            CalcMinMaxValue<uint32_t>();
            break;
        case CImage::INT_32:
            CalcMinMaxValue<int32_t>();
            break;
        case CImage::UINT_RGB8:
            CalcMinMaxValue<uint8_t>();
            break;
        case CImage::INT_RGB8:
            CalcMinMaxValue<int8_t>();
            break;
        case CImage::UINT_RGB16:
            CalcMinMaxValue<uint16_t>();
            break;
        case CImage::INT_RGB16:
            CalcMinMaxValue<int16_t>();
            break;
        case CImage::REAL_32:
            CalcMinMaxValue<float>();
            break;
        case CImage::REAL_64:
            CalcMinMaxValue<double>();
            break;
        default:
            assert(false);
        }
    }

    min = m_minValue;
    max = m_maxValue;
}

template <class pType> void C3DImage::ZeroTemplate(int channels)
{
    pType* data = (pType*)m_pb;
    int n = m_cx*m_cy*m_cz*channels;
    for (int i=0; i<n; i++) data[i] = 0;
}


void C3DImage::Zero()
{
    switch (m_pixelType)
    {
    case CImage::UINT_8:
        ZeroTemplate<uint8_t>();
        break;
    case CImage::INT_8:
        ZeroTemplate<int8_t>();
        break;
    case CImage::UINT_16:
        ZeroTemplate<uint16_t>();
        break;
    case CImage::INT_16:
        ZeroTemplate<int16_t>();
        break;
    case CImage::UINT_32:
        ZeroTemplate<uint32_t>();
        break;
    case CImage::INT_32:
        ZeroTemplate<int32_t>();
        break;
    case CImage::UINT_RGB8:
        ZeroTemplate<uint8_t>(3);
        break;
    case CImage::INT_RGB8:
        ZeroTemplate<int8_t>(3);
        break;
    case CImage::UINT_RGB16:
        ZeroTemplate<uint16_t>(3);
        break;
    case CImage::INT_RGB16:
        ZeroTemplate<int16_t>(3);
        break;
    case CImage::REAL_32:
        ZeroTemplate<float>();
        break;
    case CImage::REAL_64:
        ZeroTemplate<double>();
        break;
    default:
        assert(false);
    }
}

bool C3DImage::ExportRAW(const std::string& filename)
{
	if (m_pb == nullptr) return false;

	int nsize = m_cx * m_cy * m_cz * m_bps;
	if (nsize <= 0) return false;

	std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
	if (!file.is_open()) return false;
	file.write(reinterpret_cast<char*>(m_pb), nsize);
	file.close();

	return true;
}
