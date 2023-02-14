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
#include "TiffReader.h"
#include <ImageLib/3DImage.h>

#ifndef  WORD
#define WORD	uint16_t
#define DWORD	uint32_t
#endif // ! WORD


typedef struct _TiffHeader
{
	WORD  Identifier;  /* Byte-order Identifier */
	WORD  Version;     /* TIFF version number (always 2Ah) */
	DWORD IFDOffset;   /* Offset of the first Image File Directory*/
} TIFHEAD;

typedef struct _TifTag
{
	WORD   TagId;       /* The tag identifier  */
	WORD   DataType;    /* The scalar type of the data items  */
	DWORD  DataCount;   /* The number of items in the tag data  */
	DWORD  DataOffset;  /* The byte offset to the data items  */
} TIFTAG;

typedef struct _TifIfd
{
	WORD    NumDirEntries;    /* Number of Tags in IFD  */
	TIFTAG* TagList;			/* Array of Tags  */
	DWORD   NextIFDOffset;    /* Offset to next IFD  */
} TIFIFD;

CTiffImageSource::CTiffImageSource(Post::CImageModel* imgModel, const std::string& filename) : CImageSource(RAW, imgModel)
{
	m_filename = filename;
}

CTiffImageSource::CTiffImageSource(Post::CImageModel* imgModel) : CImageSource(RAW, imgModel)
{

}

CTiffImageSource::~CTiffImageSource()
{
}

void byteswap(WORD& v)
{
	unsigned char* b = (unsigned char*)(&v);
	b[0] ^= b[1];
	b[1] ^= b[0];
	b[0] ^= b[1];
}

void byteswap(DWORD& v)
{
	unsigned char* b = (unsigned char*)(&v);
	b[0] ^= b[3]; b[3] ^= b[0]; b[0] ^= b[3];
	b[1] ^= b[2]; b[2] ^= b[1]; b[1] ^= b[2];
}

bool CTiffImageSource::Load()
{
	if (m_filename.empty()) return false;
	const char* szfile = m_filename.c_str();
	m_fp = fopen(szfile, "rb");
	if (m_fp == 0) return false;

	// read the header
	TIFHEAD hdr;
	fread(&hdr, sizeof(TIFHEAD), 1, m_fp);

	// see if this is a tiff (and determine endianess)
	m_bigE = false;
	if (hdr.Identifier == 0x4D4D) m_bigE = true;
	else if (hdr.Identifier == 0x4949) m_bigE = false;

	if (m_bigE) byteswap(hdr.Version);
	if (hdr.Version != 0x2A) return false;

	// jump to the first IFD
	if (m_bigE) byteswap(hdr.IFDOffset);
	fseek(m_fp, hdr.IFDOffset, SEEK_SET);

	// read the image
	bool bdone = false;
	while (bdone == false)
	{
		bdone = (readImage() != true);
	}
	fclose(m_fp);

	// see if we read any image data
	if (m_pd.size() == 0) return false;
	int nz = m_pd.size();

	// build the 3D image
	C3DImage* im = new C3DImage;
	im->Create(m_cx, m_cy, nz);
	Byte* buf = im->GetBytes();
	DWORD imSize = m_cx * m_cy;
	for (int i = 0; i < nz; ++i)
	{
		memcpy(buf, m_pd[i], imSize);
		buf += imSize;
	}

	// clean up
	for (int i = 0; i < m_pd.size(); ++i) delete[] m_pd[i];
	m_pd.clear();

	BOX box(0, 0, 0, m_cx, m_cy, nz);
	im->SetBoundingBox(box);
	AssignImage(im);

	return true;
}

bool CTiffImageSource::readImage()
{
	// read the IFD
	TIFIFD ifd;
	fread(&ifd.NumDirEntries, sizeof(WORD), 1, m_fp);
	if (m_bigE) byteswap(ifd.NumDirEntries);
	if ((ifd.NumDirEntries <= 0) || (ifd.NumDirEntries >= 65536)) return false;

	// read the tags
	ifd.TagList = new _TifTag[ifd.NumDirEntries];
	int nread = fread(ifd.TagList, sizeof(_TifTag), ifd.NumDirEntries, m_fp);
	if (nread != ifd.NumDirEntries) { delete[] ifd.TagList; return false; }
	if (m_bigE)
	{
		for (int i = 0; i < ifd.NumDirEntries; ++i)
		{
			TIFTAG& t = ifd.TagList[i];
			byteswap(t.TagId);
			byteswap(t.DataType);
			byteswap(t.DataCount);

			if (t.DataType == 3)
			{
				WORD tmp = t.DataOffset;
				byteswap(tmp);
				t.DataOffset = tmp;
			}
			else if (t.DataType == 1) byteswap(t.DataOffset);
			else if (t.DataType == 2) byteswap(t.DataOffset);
			else if (t.DataType == 4) byteswap(t.DataOffset);
		}
	}

	// read the next IDF offset
	DWORD nextIFD = 0;
	fread(&nextIFD, sizeof(DWORD), 1, m_fp);
	if (m_bigE) byteswap(nextIFD);

	// process tags
	DWORD imWidth = 0, imLength = 0;
	DWORD rowsPerStrip = 0, stripOffsets = 0, stripByteCounts;
	for (int i = 0; i < ifd.NumDirEntries; ++i)
	{
		TIFTAG& t = ifd.TagList[i];
		switch (t.TagId)
		{
		case 256: imWidth = t.DataOffset; break;
		case 257: imLength = t.DataOffset; break;
		case 278: rowsPerStrip = t.DataOffset; break;
		case 273: stripOffsets = t.DataOffset; break;
		case 279: stripByteCounts = t.DataOffset; break;
		}
	}
	m_cx = imWidth;
	m_cy = imLength;

	// allocate buffer for image
	DWORD imSize = imWidth * imLength;
	Byte* buf = new Byte[imSize];
	m_pd.push_back(buf);

	DWORD bytesRead = 0;
	int strip = 0;
	while (bytesRead < imSize)
	{
		fseek(m_fp, stripOffsets, SEEK_SET);
		fread(buf, stripByteCounts, 1, m_fp);
		bytesRead += stripByteCounts;
	}

	// cleanup
	delete[] ifd.TagList;

	// jump to the next IFD
	if (nextIFD != 0) fseek(m_fp, nextIFD, SEEK_SET);
	else return false;

	return true;
}

void CTiffImageSource::Save(OArchive& ar)
{

}

void CTiffImageSource::Load(IArchive& ar)
{

}
