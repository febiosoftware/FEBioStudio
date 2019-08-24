#pragma once
#include "Image.h"

/////////////////////////////////////////////////////////////////////////
// CRGBAImage - Implements a RGBA image

class CRGBAImage
{
public:
	CRGBAImage();
	CRGBAImage(int nx, int ny);
	CRGBAImage(const CRGBAImage& im);
	virtual ~CRGBAImage();

	CRGBAImage& operator = (const CRGBAImage& im);

	void StretchBlt(CRGBAImage& im);

	void Create(int nx, int ny);

	byte* GetBytes() { return m_pb; }

	byte* GetPixel(int i, int j) { return m_pb + ((j*m_cx + i) << 2); }

	int Width() { return m_cx; }
	int Height() { return m_cy; }

protected:
	byte*	m_pb;	// rgba image data
	int		m_cx;
	int		m_cy;
};

