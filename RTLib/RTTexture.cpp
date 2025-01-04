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
#include "RTTexture.h"

#define CLAMP(v, l, r) ((v) < (l) ? (l) : ((v) > (r) ? (r) : (v)))

rt::Texture1D::Texture1D()
{
	data = nullptr;
	size = 0;
}

rt::Texture1D::~Texture1D()
{
	delete data;
}

void rt::Texture1D::setImageData(size_t n, unsigned char* bytes)
{
	size = n;
	if (data) delete data;
	if (n == 0) {
		data = nullptr; 
		return;
	}
	else {
		data = new float[3 * n];
		unsigned char* b = bytes;
		float* d = data;
		for (size_t i = 0; i < n; ++i, b += 3, d += 3) {
			d[0] = (float)b[0] / 255.f;
			d[1] = (float)b[1] / 255.f;
			d[2] = (float)b[2] / 255.f;
		}
	}
}

rt::Color rt::Texture1D::sample(float r)
{
	r = CLAMP(r, 0, 1);
	int n = (int)(r * (size - 1));
	float* v = data + 3 * n;
	return rt::Color(v[0], v[1], v[2]);
}
