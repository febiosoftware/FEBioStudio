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

#include "stdafx.h"
#include "RGBImage.h"
#include <stdio.h>

#ifdef WIN32
#include <memory>
#else
#include <memory.h>
#endif

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
	delete[] m_pb;
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

	memcpy(m_pb, im.m_pb, 3 * nx*ny);
}

void CRGBImage::Create(int nx, int ny)
{
	if (m_pb) delete[] m_pb;

	m_cx = nx;
	m_cy = ny;

	m_pb = new uint8_t[nx*ny][3];
}

CRGBImage& CRGBImage::operator =(const CRGBImage& im)
{
	int nx = im.m_cx;
	int ny = im.m_cy;

	Create(nx, ny);

	memcpy(m_pb, im.m_pb, 3 * nx*ny);

	return (*this);
}

void CRGBImage::SwapRB()
{
	uint8_t* pb = *m_pb;
	uint8_t t;
	for (int i = 0; i<m_cx*m_cy; i++, pb += 3)
	{
		t = pb[0];
		pb[0] = pb[2];
		pb[2] = t;
	}
}

void CRGBImage::FlipY()
{
	uint8_t t;
	for (int i = 0; i<m_cy / 2; ++i)
	{
		int i0 = i;
		int i1 = m_cy - 1 - i;
		if (i0 != i1)
		{
			uint8_t(*psrc)[3] = m_pb + i0*m_cx;
			uint8_t(*pdst)[3] = m_pb + i1*m_cx;
			for (int j = 0; j<m_cx; ++j)
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
	uint16_t	bfType = 0x4d42;			// file type, must be "BM"
	uint32_t	bfSize = 54 + 3 * m_cx*m_cy;	// size in bytes of file
	uint16_t	bfReserved1 = 0;			// reserved, must be zero
	uint16_t	bfReserved2 = 0;			// reserved, must be zero
	uint32_t	bfOffBits = 54;				// offset in bytes from beginning of BMPHEADER to bitmap bits.

										// info header
	uint32_t	biSize = 40;		// size of structure (should be sizeof(BMPINFOHEADER)=40)
	uint32_t	biWidth = m_cx;		// width of bitmap in pixels
	uint32_t	biHeight = m_cy;		// height of bitmap in pixels
	uint16_t	biPlanes = 1;		// number of planes (must be 1)
	uint16_t	biBitCount = 24;	// number of bits per pixel (here 24)
	uint32_t	biCompression = 0;	// type of compression (here 0, no compression)
	uint32_t	biSizeImage = 0;	// size of image in bytes (here can be 0)
	uint32_t	biXPelsPerMeter = 0;// horizontal resolution (here not used)
	uint32_t	biYPelsPerMeter = 0;// vertical resolution (here not used)
	uint32_t	biClrUsed = 0;		// number of color indices (here 0)
	uint32_t	biClrImportant = 0;	// number of colors required to display image (here 0)

	fwrite(&bfType, sizeof(bfType), 1, fp);
	fwrite(&bfSize, sizeof(bfSize), 1, fp);
	fwrite(&bfReserved1, sizeof(bfReserved1), 1, fp);
	fwrite(&bfReserved2, sizeof(bfReserved2), 1, fp);
	fwrite(&bfOffBits, sizeof(bfOffBits), 1, fp);
	fwrite(&biSize, sizeof(biSize), 1, fp);
	fwrite(&biWidth, sizeof(biWidth), 1, fp);
	fwrite(&biHeight, sizeof(biHeight), 1, fp);
	fwrite(&biPlanes, sizeof(biPlanes), 1, fp);
	fwrite(&biBitCount, sizeof(biBitCount), 1, fp);
	fwrite(&biCompression, sizeof(biCompression), 1, fp);
	fwrite(&biSizeImage, sizeof(biSizeImage), 1, fp);
	fwrite(&biXPelsPerMeter, sizeof(biXPelsPerMeter), 1, fp);
	fwrite(&biYPelsPerMeter, sizeof(biYPelsPerMeter), 1, fp);
	fwrite(&biClrUsed, sizeof(biClrUsed), 1, fp);
	fwrite(&biClrImportant, sizeof(biClrImportant), 1, fp);



	// save the pixels
	// we need to make sure that the image is 32-bit aligned
	int wb = 4 * ((3 * m_cx - 1) / 4 + 1);
	uint8_t* pb = new uint8_t[wb];
	for (int i = 0; i<m_cy; i++)
	{
		memcpy(pb, m_pb + i*m_cx, 3 * m_cx);
		fwrite(pb, sizeof(uint8_t), wb, fp);
	}
	delete[] pb;

	fclose(fp);

	SwapRB();

	return true;
}

union FIELD_VALUE {
	uint16_t	sval;
	uint32_t	lval;
	uint8_t	bval[4];
};

bool CRGBImage::SaveTIF(const char* szfile)
{
	// determine the endianess
	uint16_t wrd = 0x4D49;
	uint8_t bt = ((uint8_t*)&wrd)[0];

	uint16_t order = ((uint16_t)bt << 8) | bt;

	// create the file
	FILE* fp = fopen(szfile, "wb");
	if (fp == 0) return false;

	// write the order
	fwrite(&order, sizeof(uint16_t), 1, fp);

	// write the type
	uint16_t type = 0x2A;
	fwrite(&type, sizeof(uint16_t), 1, fp);

	// write the offset to the IFD
	uint32_t offset = 8;
	fwrite(&offset, sizeof(uint32_t), 1, fp);

	// write the IFD size
	uint16_t ifd_size = 7;
	fwrite(&ifd_size, sizeof(uint16_t), 1, fp);

	// write the fields
	uint16_t  nTag;
	uint16_t  nType;
	uint32_t nLength;
	FIELD_VALUE val;

	// FIELD : SubfileType
	nTag = 255;
	nType = 3; // short
	nLength = 1;
	val.sval = 1;
	fwrite(&nTag, sizeof(nTag), 1, fp);
	fwrite(&nType, sizeof(nType), 1, fp);
	fwrite(&nLength, sizeof(nLength), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : ImageWidth
	nTag = 256;
	nType = 3; // short
	nLength = 1;
	val.sval = m_cx;
	fwrite(&nTag, sizeof(nTag), 1, fp);
	fwrite(&nType, sizeof(nType), 1, fp);
	fwrite(&nLength, sizeof(nLength), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : ImageHeight
	nTag = 257;
	nType = 3; // short
	nLength = 1;
	val.sval = m_cy;
	fwrite(&nTag, sizeof(nTag), 1, fp);
	fwrite(&nType, sizeof(nType), 1, fp);
	fwrite(&nLength, sizeof(nLength), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : BitsPerSample
	nTag = 258;
	nType = 3; // short
	nLength = 1;
	val.sval = 8;
	fwrite(&nTag, sizeof(nTag), 1, fp);
	fwrite(&nType, sizeof(nType), 1, fp);
	fwrite(&nLength, sizeof(nLength), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : PhotometricInterpretation
	nTag = 262;
	nType = 3; // short
	nLength = 1;
	val.sval = 2;
	fwrite(&nTag, sizeof(nTag), 1, fp);
	fwrite(&nType, sizeof(nType), 1, fp);
	fwrite(&nLength, sizeof(nLength), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : StripOffset
	nTag = 273;
	nType = 4; // short
	nLength = 1;
	val.lval = 8 + 2 + ifd_size * 12 + 4;
	fwrite(&nTag, sizeof(nTag), 1, fp);
	fwrite(&nType, sizeof(nType), 1, fp);
	fwrite(&nLength, sizeof(nLength), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// FIELD : SamplesPerPixel
	nTag = 277;
	nType = 3; // short
	nLength = 1;
	val.sval = 3;
	fwrite(&nTag, sizeof(nTag), 1, fp);
	fwrite(&nType, sizeof(nType), 1, fp);
	fwrite(&nLength, sizeof(nLength), 1, fp);
	fwrite(&val.lval, sizeof(val.lval), 1, fp);

	// next IFD
	uint32_t next = 0;
	fwrite(&next, sizeof(next), 1, fp);

	// write the image data
	for (int y = m_cy - 1; y >= 0; y--)
		fwrite(m_pb + y*m_cx, sizeof(uint8_t) * 3, m_cx, fp);

	// and we're done !
	fclose(fp);

	return true;
}
