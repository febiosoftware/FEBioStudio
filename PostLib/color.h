#pragma once

typedef unsigned char byte;

class GLColor
{
public:
	byte	a, b, g, r;

public:
	GLColor() : a(255), b(0), g(0), r(0) {}
	GLColor(byte ur, byte ug, byte ub, byte ua = 255)
	{
		r = ur;	g = ug;	b = ub;	a = ua;
	}

	GLColor operator * (double f)
	{
		return GLColor((byte)(r*f), (byte)(g*f), (byte)(b*f));
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

	operator unsigned int() { return (int)(((((r << 8) | g) << 8) | b) << 8); }


	static GLColor White() { return GLColor(255, 255, 255); }
	static GLColor FromRGBf(float r, float g, float b) { return GLColor((byte)(r*255.f), (byte)(g*255.f), (byte)(b*255.f)); }
};
