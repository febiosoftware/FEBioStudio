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
#include "RayTraceSurface.h"
#include <memory>
#include <cstring>

RayTraceSurface::RayTraceSurface()
{
	w = 0;
	h = 0;
	d = nullptr;
}

RayTraceSurface::~RayTraceSurface()
{
	delete d;
}

void RayTraceSurface::create(size_t W, size_t H)
{
	if (d) delete d;
	w = W;
	h = H;
	d = new float[w * h * 4];
	memset(d, 0, w * h * 4);
}

float* RayTraceSurface::value(size_t x, size_t y)
{
	return d + ((y * w + x) << 2);
}

GLColor RayTraceSurface::colorValue(size_t x, size_t y)
{
	float* v = value(x, y);
	return GLColor::FromRGBf(v[0], v[1], v[2], v[3]);
}
