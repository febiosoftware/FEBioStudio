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

typedef unsigned char Byte;

class GLColor
{
public:
	Byte	a, b, g, r;

public:
	GLColor() : a(255), b(0), g(0), r(0) {}
	GLColor(Byte ur, Byte ug, Byte ub, Byte ua = 255)
	{
		r = ur;	g = ug;	b = ub;	a = ua;
	}

	GLColor operator * (double f)
	{
		return GLColor((Byte)(r*f), (Byte)(g*f), (Byte)(b*f));
	}

	GLColor operator + (GLColor& c)
	{
		return GLColor(r + c.r, g + c.g, b + c.b);
	}

	GLColor(unsigned int c)
	{
		r = ((c >> 24) & 0xFF);
		g = ((c >> 16) & 0xFF);
		b = ((c >> 8) & 0xFF);
		a = 255;
	}

	unsigned int to_uint() { return (int)(((((r << 8) | g) << 8) | b) << 8); }

	static GLColor White() { return GLColor(255, 255, 255); }
	static GLColor FromRGBf(float r, float g, float b) { return GLColor((Byte)(r*255.f), (Byte)(g*255.f), (Byte)(b*255.f)); }
};
