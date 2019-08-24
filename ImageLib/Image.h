#pragma once
typedef unsigned char byte;
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
