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

//! OpenGL color class with RGBA components stored as 8-bit unsigned integers
class GLColor
{
public:
	//! Alpha component (0-255)
	uint8_t	a, b, g, r;

public:
	//! Default constructor - creates black color with full opacity
	GLColor() : a(255), b(0), g(0), r(0) {}
	//! Constructor with RGB components and optional alpha
	GLColor(uint8_t ur, uint8_t ug, uint8_t ub, uint8_t ua = 255)
	{
		r = ur;	g = ug;	b = ub;	a = ua;
	}

	//! Scalar multiplication operator - scales RGB components by factor
	GLColor operator * (double f)
	{
		return GLColor((uint8_t)(r*f), (uint8_t)(g*f), (uint8_t)(b*f));
	}

	//! Color addition operator - adds RGB components
	GLColor operator + (GLColor& c)
	{
		return GLColor(r + c.r, g + c.g, b + c.b);
	}

	//! Constructor from packed 32-bit unsigned integer (RGBA format)
	GLColor(unsigned int c)
	{
		r = ((c >> 24) & 0xFF);
		g = ((c >> 16) & 0xFF);
		b = ((c >>  8) & 0xFF);
		a = ((c      ) & 0xFF);
	}

	//! Convert color to packed 32-bit unsigned integer
	unsigned int to_uint() { return (unsigned int)((r << 24) | (g << 16) | (b << 8) | a); }

	//! Convert color components to normalized float array (0.0-1.0)
	void toFloat(float f[4]) const
	{
		f[0] = (float)r / 255.f;
		f[1] = (float)g / 255.f;
		f[2] = (float)b / 255.f;
		f[3] = (float)a / 255.f;
	}

	//! Convert color components to normalized double array (0.0-1.0)
	void toDouble(double d[4]) const
	{
		d[0] = (double)r / 255.0;
		d[1] = (double)g / 255.0;
		d[2] = (double)b / 255.0;
		d[3] = (double)a / 255.0;
	}

	//! Create white color (255, 255, 255)
	static GLColor White() { return GLColor(255, 255, 255); }
	//! Create red color (255, 0, 0)
	static GLColor Red()   { return GLColor(255,   0,   0); }
	//! Create green color (0, 255, 0)
	static GLColor Green() { return GLColor(  0, 255,   0); }
	//! Create blue color (0, 0, 255)
	static GLColor Blue()  { return GLColor(  0,   0, 255); }
	//! Create yellow color (255, 255, 0)
	static GLColor Yellow(){ return GLColor(255, 255,   0); }
	//! Create black color (0, 0, 0)
	static GLColor Black() { return GLColor(  0,   0,   0); }
	//! Create color from normalized float RGB values (0.0-1.0)
	static GLColor FromRGBf(float r, float g, float b, float a = 1.f) { return GLColor((uint8_t)(r*255.f), (uint8_t)(g*255.f), (uint8_t)(b*255.f), (uint8_t)(a * 255.f)); }
};