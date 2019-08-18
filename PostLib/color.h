#pragma once

typedef unsigned char byte;

struct GLColor
{
	byte r, g, b, a;

public:
	GLColor() { r = g = b = 0; a = 255; }
	GLColor(byte br, byte bg, byte bb, byte ba = 255)
	{
		r = br; g = bg; b = bb; a = ba; 
	}
	GLColor(unsigned int c)
	{
		r = ((c >> 24) & 0xFF);
		g = ((c >> 16) & 0xFF);
		b = ((c >>  8) & 0xFF);
		a = 255;
	}

	operator unsigned int () { return (int)(((((r << 8) | g) << 8) | b) << 8); }
};
