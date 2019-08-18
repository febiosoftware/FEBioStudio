// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Image.h"
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// CImage
//////////////////////////////////////////////////////////////////////

CImage::CImage()
{
	m_pb = 0;
	m_cx = m_cy = 0;
	m_bdel = true;
}

CImage::CImage(int nx, int ny)
{
	m_pb = new byte[nx*ny];
	for (int i=0; i<nx*ny; i++) m_pb[i] = 0;

	m_cx = nx;
	m_cy = ny;

	m_bdel = true;
}

CImage::CImage(const CImage& im)
{
	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new byte[m_cx*m_cy];
	memcpy(m_pb, im.m_pb, m_cx*m_cy);

	m_bdel = true;
}

CImage& CImage::operator = (const CImage& im)
{
	if (m_bdel) delete [] m_pb;

	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new byte[m_cx*m_cy];
	memcpy(m_pb, im.m_pb, m_cx*m_cy);

	m_bdel = true;

	return (*this);
}

CImage& CImage::operator -= (const CImage& im)
{
	byte* pbS = im.m_pb;
	byte* pbD = m_pb;

	int nsize = m_cx*m_cy;

	for (int i=0; i<nsize; i++, pbS++, pbD++)
		*pbD = byte((((int) *pbD - (int) *pbS) + 255) >> 1);

	return (*this);
}

CImage::~CImage()
{
	if (m_bdel) delete [] m_pb;
}

void CImage::Create(int nx, int ny, byte* pb)
{
	if (pb)
	{
		m_pb = pb;
		m_bdel = false;
	}
	else
	{
		if (m_bdel) delete [] m_pb;

		m_pb = new byte[nx*ny];
		for (int i=0; i<nx*ny; i++) m_pb[i] = 0;
		m_bdel = true;
	}

	m_cx = nx;
	m_cy = ny;
}

void CImage::StretchBlt(CImage& im)
{
	byte* pd = im.m_pb;

	int nx = im.Width();
	int ny = im.Height();

	int i0 = 0;
	int j0 = 0;

	byte* p0, *p1, *p2, *p3;
	int h0, h1, h2, h3;
	int w = 0, h = 0;

	int Hx = nx - 1;
	int Hy = ny - 1;
	int H = Hx*Hy;

	for (int y=0; y<ny; y++)
	{
		i0 = 0;
		w = 0;

		while (y*(m_cy-1)>(j0+1)*(ny-1)) { j0++; h -= (ny-1); }

		p0 = m_pb + (j0*m_cx);
		p1 = p0 + 1;
		p2 = p0 + (m_cy != 1 ? m_cx : 0);
		p3 = p2 + 1;

		for (int x=0; x<nx; x++)
		{
			while (x*(m_cx-1)>(i0+1)*(nx-1)) { i0++; w -= (nx-1); p0++; p1++; p2++; p3++; }

			h0 = (Hx - w)*(Hy - h);
			h1 = w*(Hy - h);
			h2 = (Hx - w)*h;
			h3 = w*h;

			*pd++ = (h0*p0[0] + h1*p1[0] + h2*p2[0] + h3*p3[0])/H;

			w += (m_cx-1);
		}

		h += (m_cy-1);
	}
}

//////////////////////////////////////////////////////////////////////
// CRGBImage
//////////////////////////////////////////////////////////////////////
CRGBImage::CRGBImage()
{
	m_pb = 0;
	m_cx = m_cy = 0;
}

CRGBImage::~CRGBImage()
{
	delete [] m_pb;
}

CRGBImage::CRGBImage(int nx, int ny)
{
	m_pb = 0;
	m_cx = m_cy = 0;

	Create(nx, ny);
}

CRGBImage::CRGBImage(const CRGBImage& im)
{
	m_pb = 0;

	int nx = im.m_cx;
	int ny = im.m_cy;

	Create(nx, ny);

	memcpy(m_pb, im.m_pb, 3*nx*ny);
}

void CRGBImage::Create(int nx, int ny)
{
	if (m_pb) delete [] m_pb;

	m_cx = nx;
	m_cy = ny;

	m_pb = new byte[nx*ny][3];
}

CRGBImage& CRGBImage::operator =(const CRGBImage& im)
{
	int nx = im.m_cx;
	int ny = im.m_cy;

	Create(nx, ny);

	memcpy(m_pb, im.m_pb, 3*nx*ny);

	return (*this);
}

void CRGBImage::SwapRB()
{
	byte* pb = *m_pb;
	byte t;
	for (int i=0; i<m_cx*m_cy; i++, pb += 3)
	{
		t = pb[0];
		pb[0] = pb[2];
		pb[2] = t;
	}
}

void CRGBImage::FlipY()
{
	byte t;
	for (int i=0; i<m_cy/2; ++i)
	{
		int i0 = i;
		int i1 = m_cy-1-i;
		if (i0 != i1)
		{
			byte (*psrc)[3] = m_pb + i0*m_cx;
			byte (*pdst)[3] = m_pb + i1*m_cx;
			for (int j=0; j<m_cx; ++j)
			{
				t = psrc[j][0]; psrc[j][0] = pdst[j][0]; pdst[j][0] = t;
				t = psrc[j][1]; psrc[j][1] = pdst[j][1]; pdst[j][1] = t;
				t = psrc[j][2]; psrc[j][2] = pdst[j][2]; pdst[j][2] = t;
			}
		}
	}
}

bool CRGBImage::SaveBMP(const char* szfile)
{
	SwapRB();

	// create the file
	FILE* fp = fopen(szfile, "wb");
	if (fp == 0) return false;

	// save the fileheader
	word	bfType = 0x4d42;			// file type, must be "BM"
	dword	bfSize = 54+3*m_cx*m_cy;	// size in bytes of file
	word	bfReserved1 = 0;			// reserved, must be zero
	word	bfReserved2 = 0;			// reserved, must be zero
	dword	bfOffBits = 54;				// offset in bytes from beginning of BMPHEADER to bitmap bits.

	// info header
	dword	biSize = 40;		// size of structure (should be sizeof(BMPINFOHEADER)=40)
	dword	biWidth = m_cx;		// width of bitmap in pixels
	dword	biHeight= m_cy;		// height of bitmap in pixels
	word	biPlanes = 1;		// number of planes (must be 1)
	word	biBitCount = 24;	// number of bits per pixel (here 24)
	dword	biCompression = 0;	// type of compression (here 0, no compression)
	dword	biSizeImage = 0;	// size of image in bytes (here can be 0)
	dword	biXPelsPerMeter = 0;// horizontal resolution (here not used)
	dword	biYPelsPerMeter = 0;// vertical resolution (here not used)
	dword	biClrUsed = 0;		// number of color indices (here 0)
	dword	biClrImportant = 0;	// number of colors required to display image (here 0)

	fwrite(&bfType         , sizeof(bfType         ), 1, fp);
	fwrite(&bfSize         , sizeof(bfSize         ), 1, fp);
	fwrite(&bfReserved1    , sizeof(bfReserved1    ), 1, fp);
	fwrite(&bfReserved2    , sizeof(bfReserved2    ), 1, fp);
	fwrite(&bfOffBits      , sizeof(bfOffBits      ), 1, fp);
	fwrite(&biSize         , sizeof(biSize         ), 1, fp);
	fwrite(&biWidth        , sizeof(biWidth        ), 1, fp);
	fwrite(&biHeight       , sizeof(biHeight       ), 1, fp);
	fwrite(&biPlanes       , sizeof(biPlanes       ), 1, fp);
	fwrite(&biBitCount     , sizeof(biBitCount     ), 1, fp);
	fwrite(&biCompression  , sizeof(biCompression  ), 1, fp);
	fwrite(&biSizeImage    , sizeof(biSizeImage    ), 1, fp);
	fwrite(&biXPelsPerMeter, sizeof(biXPelsPerMeter), 1, fp);
	fwrite(&biYPelsPerMeter, sizeof(biYPelsPerMeter), 1, fp);
	fwrite(&biClrUsed      , sizeof(biClrUsed      ), 1, fp);
	fwrite(&biClrImportant , sizeof(biClrImportant ), 1, fp);

	

	// save the pixels
	// we need to make sure that the image is 32-bit aligned
	int wb = 4*((3*m_cx-1)/4+1);
	byte* pb = new byte[wb];
	for (int i=0; i<m_cy; i++)
	{
		memcpy(pb, m_pb+i*m_cx, 3*m_cx);
		fwrite(pb, sizeof(byte), wb, fp);
	}
	delete [] pb;

	fclose(fp);

	SwapRB();

	return true;
}

union FIELD_VALUE {
	word	sval;
	dword	lval;
	byte	bval[4];
};

bool CRGBImage::SaveTIF(const char* szfile)
{
	// determine the endianess
	word wrd = 0x4D49;
	byte bt  = ((byte*) &wrd)[0];

	word order = ((word)bt << 8) | bt;

	// create the file
	FILE* fp = fopen(szfile, "wb");
	if (fp == 0) return false;

	// write the order
	fwrite(&order, sizeof(word), 1, fp);

	// write the type
	word type = 0x2A;
	fwrite(&type, sizeof(word), 1, fp);

	// write the offset to the IFD
	dword offset = 8;
	fwrite(&offset, sizeof(dword), 1, fp);

	// write the IFD size
	word ifd_size = 7;
	fwrite(&ifd_size, sizeof(word), 1, fp);

	// write the fields
	word  nTag;
	word  nType;
	dword nLength;
	FIELD_VALUE val;

	// FIELD : SubfileType
	nTag = 255;
	nType = 3; // short
	nLength = 1;
	val.sval = 1;
	fwrite(&nTag    , sizeof(nTag    ), 1, fp);
	fwrite(&nType   , sizeof(nType   ), 1, fp);
	fwrite(&nLength , sizeof(nLength ), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : ImageWidth
	nTag = 256;
	nType = 3; // short
	nLength = 1;
	val.sval = m_cx;
	fwrite(&nTag    , sizeof(nTag    ), 1, fp);
	fwrite(&nType   , sizeof(nType   ), 1, fp);
	fwrite(&nLength , sizeof(nLength ), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : ImageHeight
	nTag = 257;
	nType = 3; // short
	nLength = 1;
	val.sval = m_cy;
	fwrite(&nTag    , sizeof(nTag    ), 1, fp);
	fwrite(&nType   , sizeof(nType   ), 1, fp);
	fwrite(&nLength , sizeof(nLength ), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : BitsPerSample
	nTag = 258;
	nType = 3; // short
	nLength = 1;
	val.sval = 8;
	fwrite(&nTag    , sizeof(nTag    ), 1, fp);
	fwrite(&nType   , sizeof(nType   ), 1, fp);
	fwrite(&nLength , sizeof(nLength ), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : PhotometricInterpretation
	nTag = 262;
	nType = 3; // short
	nLength = 1;
	val.sval = 2;
	fwrite(&nTag    , sizeof(nTag    ), 1, fp);
	fwrite(&nType   , sizeof(nType   ), 1, fp);
	fwrite(&nLength , sizeof(nLength ), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : StripOffset
	nTag = 273;
	nType = 4; // short
	nLength = 1;
	val.lval = 8 + 2 + ifd_size*12 + 4;
	fwrite(&nTag    , sizeof(nTag    ), 1, fp);
	fwrite(&nType   , sizeof(nType   ), 1, fp);
	fwrite(&nLength , sizeof(nLength ), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : SamplesPerPixel
	nTag = 277;
	nType = 3; // short
	nLength = 1;
	val.sval = 3;
	fwrite(&nTag    , sizeof(nTag    ), 1, fp);
	fwrite(&nType   , sizeof(nType   ), 1, fp);
	fwrite(&nLength , sizeof(nLength ), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// next IFD
	dword next = 0;
	fwrite(&next, sizeof(next), 1, fp);

	// write the image data
	for (int y=m_cy-1; y>=0; y--)
		fwrite(m_pb + y*m_cx, sizeof(byte)*3, m_cx, fp);

	// and we're done !
	fclose(fp);

	return true;
}


//////////////////////////////////////////////////////////////////////
// CRGBAImage
//////////////////////////////////////////////////////////////////////

CRGBAImage::CRGBAImage()
{
	m_pb = 0;
	m_cx = m_cy = 0;
}

CRGBAImage::CRGBAImage(int nx, int ny)
{
	m_pb = new byte[nx*ny*4];
	for (int i=0; i<nx*ny*4; i++) m_pb[i] = 0;

	m_cx = nx;
	m_cy = ny;
}

CRGBAImage::CRGBAImage(const CRGBAImage& im)
{
	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new byte[m_cx*m_cy*4];
	memcpy(m_pb, im.m_pb, m_cx*m_cy*4);
}

CRGBAImage& CRGBAImage::operator = (const CRGBAImage& im)
{
	delete [] m_pb;

	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new byte[m_cx*m_cy*4];
	memcpy(m_pb, im.m_pb, m_cx*m_cy*4);

	return (*this);
}

CRGBAImage::~CRGBAImage()
{
	delete [] m_pb;
}

void CRGBAImage::Create(int nx, int ny)
{
	if (m_pb) delete [] m_pb;

	m_pb = new byte[nx*ny*4];
	for (int i=0; i<nx*ny*4; i++) m_pb[i] = 0;

	m_cx = nx;
	m_cy = ny;
}

void CRGBAImage::StretchBlt(CRGBAImage& im)
{
	byte* pd = im.m_pb;

	int nx = im.Width();
	int ny = im.Height();

	int i0 = 0;
	int j0 = 0;

	byte* p0, *p1, *p2, *p3;
	int h0, h1, h2, h3;
	int w = 0, h = 0;

	int Hx = nx - 1;
	int Hy = ny - 1;
	int H = Hx*Hy;

	for (int y=0; y<ny; y++)
	{
		i0 = 0;
		w = 0;

		while (y*(m_cy-1)>(j0+1)*(ny-1)) { j0++; h -= (ny-1); }

		p0 = m_pb + 4*(j0*m_cx);
		p1 = p0 + 4;
		p2 = p0 + 4*m_cx;
		p3 = p2 + 4;

		for (int x=0; x<nx; x++)
		{
			while (x*(m_cx-1)>(i0+1)*(nx-1)) { i0++; w -= (nx-1); p0 += 4; p1 += 4; p2 += 4; p3 += 4; }

			h0 = (Hx - w)*(Hy - h);
			h1 = w*(Hy - h);
			h2 = (Hx - w)*h;
			h3 = w*h;

			pd[0] = (h0*p0[0] + h1*p1[0] + h2*p2[0] + h3*p3[0])/H;
			pd[1] = (h0*p0[1] + h1*p1[1] + h2*p2[1] + h3*p3[1])/H;
			pd[2] = (h0*p0[2] + h1*p1[2] + h2*p2[2] + h3*p3[2])/H;
			pd[3] = (h0*p0[3] + h1*p1[3] + h2*p2[3] + h3*p3[3])/H;

			pd += 4;

			w += (m_cx-1);
		}

		h += (m_cy-1);
	}
}
