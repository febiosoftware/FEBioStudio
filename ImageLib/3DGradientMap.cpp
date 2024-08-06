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
#include "3DGradientMap.h"

//=============================================================================
// C3DGradientMap
//=============================================================================

C3DGradientMap::C3DGradientMap(C3DImage& im, BOX box) : m_im(im), m_box(box)
{

}

C3DGradientMap::~C3DGradientMap()
{
}

template<class pType> vec3f C3DGradientMap::ValueTemplate(int i, int j, int k)
{
    pType* data = (pType*)m_im.GetBytes();

    // get the image dimensions
	int nx = m_im.Width();
	int ny = m_im.Height();
	int nz = m_im.Depth();

	float dxi = (nx - 1.f) / (float)m_box.Width();
	float dyi = (ny - 1.f) / (float)m_box.Height();
	float dzi = (nz - 1.f) / (float)m_box.Depth();

    // calculate the gradient
	vec3f r;

    // x-component
	if (i == 0) r.x = ((float)data[nx*(k*ny + j) + i + 1] - (float)data[nx*(k*ny + j) + i]) * dxi;
	else if (i == nx - 1) r.x = ((float)data[nx*(k*ny + j) + i] - (float)data[nx*(k*ny + j) + i - 1]) * dxi;
	else r.x = ((float)data[nx*(k*ny + j) + i + 1] - (float)data[nx*(k*ny + j) + i - 1]) * (0.5f*dxi);

	// y-component
	if (j == 0) r.y = ((float)data[nx*(k*ny + j + 1) + i] - (float)data[nx*(k*ny + j) + i]) * dyi;
	else if (j == ny - 1) r.y = ((float)data[nx*(k*ny + j) + i] - (float)data[nx*(k*ny + j - 1) + i]) * dyi;
	else r.y = ((float)data[nx*(k*ny + j + 1) + i] - (float)data[nx*(k*ny + j - 1) + i]) * (0.5f*dyi);

	// z-component
	if (k == 0) r.z = ((float)data[nx*((k + 1)*ny + j) + i] - (float)data[nx*(k*ny + j) + i]) * dzi;
	else if (k == nz - 1) r.z = ((float)data[nx*(k*ny + j) + i] - (float)data[nx*((k - 1)*ny + j) + i]) * dzi;
	else r.z = ((float)data[nx*((k + 1)*ny + j) + i] - (float)data[nx*((k - 1)*ny + j) + i]) * (0.5f*dzi);

    return r;
}

vec3f C3DGradientMap::Value(int i, int j, int k)
{
	switch (m_im.PixelType())
    {
    case CImage::UINT_8:
        return ValueTemplate<uint8_t>(i, j, k);
    case CImage::INT_8:
        return ValueTemplate<int8_t>(i, j, k);
    case CImage::UINT_16:
        return ValueTemplate<uint16_t>(i, j, k);
    case CImage::INT_16:
        return ValueTemplate<int16_t>(i, j, k);
    case CImage::UINT_32:
        return ValueTemplate<uint16_t>(i, j, k);
    case CImage::INT_32:
        return ValueTemplate<int16_t>(i, j, k);
    case CImage::UINT_RGB8:
        return ValueTemplate<uint8_t>(i, j, k);
    case CImage::INT_RGB8:
        return ValueTemplate<int8_t>(i, j, k);
    case CImage::UINT_RGB16:
        return ValueTemplate<uint16_t>(i, j, k);
    case CImage::INT_RGB16:
        return ValueTemplate<int16_t>(i, j, k);
    case CImage::REAL_32:
        return ValueTemplate<float>(i, j, k);
    case CImage::REAL_64:
        return ValueTemplate<double>(i, j, k);
    default:
        assert(false);
    }

	// we shouldn't get here, but added it to avoid compiler warning
	return vec3f(0.f, 0.f, 0.f);
}
