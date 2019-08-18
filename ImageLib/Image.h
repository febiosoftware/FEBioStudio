#pragma once

typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned char byte;

//---------------------------------------------------------------------------------------
// Class to represent an 8-bit gray-scale image
class Image  
{
public:
	Image();
	Image(int nx, int ny);
	Image(const Image& im);
	virtual ~Image();

	Image& operator = (const Image& im);
	Image& operator -= (const Image& im);

	void Create(int nx, int ny, byte* pb = 0);
	void StretchBlt(Image& im);

	int Width () const { return m_cx; }
	int Height() const { return m_cy; }

	byte* GetBytes() const { return m_pb; }

	byte* GetPixel(int i, int j) { return m_pb + (j*m_cx + i); }

	void Zero() { for (int i=0; i<m_cx*m_cy; i++) m_pb[i] = 0; }

	byte Peek(double x, double y);

protected:
	byte* m_pb;	// image data

	bool	m_bdel;	// delete image data at clean up ?
	
	int	m_cx;
	int	m_cy;
};

//---------------------------------------------------------------------------------------
// Implements a RGB image
class RGBImage
{
public:
	RGBImage();
	RGBImage(int nx, int ny);
	RGBImage(const RGBImage& im);
	virtual ~RGBImage();

	RGBImage& operator = (const RGBImage& im);

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


//---------------------------------------------------------------------------------------
// Implements a RGBA image
class RGBAImage
{
public:
	RGBAImage();
	RGBAImage(int nx, int ny);
	RGBAImage(const RGBAImage& im);
	virtual ~RGBAImage();

	RGBAImage& operator = (const RGBAImage& im);

	void StretchBlt(Image& im);
	void StretchBlt(RGBAImage& im);

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
