// Image.h: interface for the CImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGE_H__71581AFE_09FC_4016_8F60_46A02C1FEBA8__INCLUDED_)
#define AFX_IMAGE_H__71581AFE_09FC_4016_8F60_46A02C1FEBA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ColorMap.h"

typedef unsigned short word;
typedef unsigned int dword;


////////////////////////////////////////////////////////////////////////
// CImage - Implements a gray scale image

class CImage  
{
public:
	CImage();
	CImage(int nx, int ny);
	CImage(const CImage& im);
	virtual ~CImage();

	CImage& operator = (const CImage& im);
	CImage& operator -= (const CImage& im);

	void Create(int nx, int ny, byte* pb = 0);
	void StretchBlt(CImage& im);

	int Width () const { return m_cx; }
	int Height() const { return m_cy; }

	byte* GetBytes() const { return m_pb; }

	byte* GetPixel(int i, int j) { return m_pb + (j*m_cx + i); }

	byte value(int i, int j) { return m_pb[j*m_cx + i]; }

	void Zero() { for (int i=0; i<m_cx*m_cy; i++) m_pb[i] = 0; }

protected:
	byte* m_pb;	// rgb image data

	bool	m_bdel;	// delete image data at clean up ?
	
	int	m_cx;
	int	m_cy;
};

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

	int Size() { return m_cx*m_cy*3; }

	void SwapRB(); // swap R and B component
	void FlipY(); // flip y-dimension

	bool SaveBMP(const char* szfile);
	bool SaveTIF(const char* szfile);

protected:
	byte   (*m_pb)[3];
	int		m_cx;
	int		m_cy;
};


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

	byte* GetPixel(int i, int j) { return m_pb + ((j*m_cx + i)<<2); }

	int Width() { return m_cx; }
	int Height() { return m_cy; }

protected:
	byte*	m_pb;	// rgba image data
	int		m_cx;
	int		m_cy;
};

#endif // !defined(AFX_IMAGE_H__71581AFE_09FC_4016_8F60_46A02C1FEBA8__INCLUDED_)
