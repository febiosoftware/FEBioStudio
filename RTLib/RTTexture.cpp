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

gl::Color rt::Texture1D::sample(float r)
{
	r = CLAMP(r, 0, 1);
	int n = (int)(r * (size - 1));
	float* v = data + 3 * n;
	return gl::Color(v[0], v[1], v[2]);
}

rt::Texture2D::Texture2D()
{
}

rt::Texture2D::~Texture2D()
{
}

void rt::Texture2D::setImageData(const CRGBAImage& image)
{
	img = image;
}

gl::Color rt::Texture2D::sample(float u, float v)
{
	int w = img.Width();
	int h = img.Height();
	u = CLAMP(u, 0, 1);
	v = CLAMP(v, 0, 1);

	double x = u * (w - 1);
	double y = v * (h - 1);

	int i0 = (int)floor(x);
	int j0 = (int)floor(y);

	double r = x - (double)i0;
	double s = y - (double)j0;

	int i1 = i0 + 1;
	int j1 = j0 + 1;

	if (i0 >= w - 1) { i0 = w - 2; i1 = w - 1; r = 1.0; }
	if (j0 >= h - 1) { j0 = h - 2; j1 = h - 1; s = 1.0; }

	double N[4] = {
		(1.0 - r) * (1.0 - s),
		r * (1.0 - s),
		r * s,
		(1.0 -r)*s
	};

	uint8_t* p0 = img.GetPixel(i0, j0);
	uint8_t* p1 = img.GetPixel(i1, j0);
	uint8_t* p2 = img.GetPixel(i1, j1);
	uint8_t* p3 = img.GetPixel(i0, j1);
	GLColor c0(p0[0], p0[1], p0[2], p0[3]);
	GLColor c1(p1[0], p1[1], p1[2], p1[3]);
	GLColor c2(p2[0], p2[1], p2[2], p2[3]);
	GLColor c3(p3[0], p3[1], p3[2], p3[3]);

	double f0[4], f1[4], f2[4], f3[4];
	c0.toDouble(f0);
	c1.toDouble(f1);
	c2.toDouble(f2);
	c3.toDouble(f3);

	double c[4];
	c[0] = N[0] * f0[0] + N[1] * f1[0] + N[2] * f2[0] + N[3] * f3[0];
	c[1] = N[0] * f0[1] + N[1] * f1[1] + N[2] * f2[1] + N[3] * f3[1];
	c[2] = N[0] * f0[2] + N[1] * f1[2] + N[2] * f2[2] + N[3] * f3[2];
	c[3] = N[0] * f0[3] + N[1] * f1[3] + N[2] * f2[3] + N[3] * f3[3];

	return gl::Color(c);
}

rt::Texture3D::Texture3D()
{
	tex = nullptr;
}

rt::Texture3D::~Texture3D()
{
	tex = nullptr;
}

void rt::Texture3D::setImageData(GLTexture3D* tex3d)
{
	tex = tex3d;
}

gl::Color rt::Texture3D::sample(float r, float s, float t)
{
	if (tex == nullptr) return gl::Color(0, 0, 0);

	C3DImage* img = tex->Get3DImage();

	double v = img->Peek(r, s, t);
	double vmin, vmax;
	img->GetMinMax(vmin, vmax, false);
	if (vmax == vmin) vmax++;
	double f = (v - vmin) / (vmax - vmin);
	return gl::Color(f, f, f, f);
}
