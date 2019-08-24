#pragma once
#include "Image.h"

/////////////////////////////////////////////////////////////////////////
// CRGBImage - Implements a RGB image

class CRGBImage
{
public:
	CRGBImage();
	CRGBImage(int nx, int ny);
	CRGBImage(const CRGBImage& im);
	virtual ~CRGBImage();

	CRGBImage& operator = (const CRGBImage& im);

	void Create(int nx, int ny);

	byte* GetBytes() { return (*m_pb); }

	int Width() { return m_cx; }
	int Height() { return m_cy; }

	int Size() { return m_cx*m_cy * 3; }

	void SwapRB(); // swap R and B component
	void FlipY(); // flip y-dimension

	bool SaveBMP(const char* szfile);
	bool SaveTIF(const char* szfile);

protected:
	byte(*m_pb)[3];
	int		m_cx;
	int		m_cy;
};
