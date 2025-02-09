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
#include "ImageModel.h"
#include <XML/XMLReader.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <filesystem>
using std::string;

namespace fs = std::filesystem;

#ifndef  WORD
#define WORD	uint16_t
#define DWORD	uint32_t
#endif // ! WORD

#ifdef WIN32
	int __declspec(dllimport) _fseeki64(FILE*, __int64, int);
	#define fseek _fseeki64
#endif

enum TifCompression {
	TIF_COMPRESSION_NONE = 1,
	TIF_COMPRESSION_CCITTRLE = 2,
	TIF_COMPRESSION_CCITTFAX3 = 3, // = COMPRESSION_CCITT_T4
	TIF_COMPRESSION_CCITTFAX4 = 4, // = COMPRESSION_CCITT_T6
	TIF_COMPRESSION_LZW = 5,
	TIF_COMPRESSION_OJPEG = 6,
	TIF_COMPRESSION_JPEG = 7,
	TIF_COMPRESSION_NEXT = 32766,
	TIF_COMPRESSION_CCITTRLEW = 32771,
	TIF_COMPRESSION_PACKBITS = 32773,
	TIF_COMPRESSION_THUNDERSCAN = 32809,
	TIF_COMPRESSION_IT8CTPAD = 32895,
	TIF_COMPRESSION_IT8LW = 32896,
	TIF_COMPRESSION_IT8MP = 32897,
	TIF_COMPRESSION_IT8BL = 32898,
	TIF_COMPRESSION_PIXARFILM = 32908,
	TIF_COMPRESSION_PIXARLOG = 32909,
	TIF_COMPRESSION_DEFLATE = 32946,
	TIF_COMPRESSION_ADOBE_DEFLATE = 8,
	TIF_COMPRESSION_DCS = 32947,
	TIF_COMPRESSION_JBIG = 34661,
	TIF_COMPRESSION_SGILOG = 34676,
	TIF_COMPRESSION_SGILOG24 = 34677,
	TIF_COMPRESSION_JP2000 = 34712
};

enum TifPhotometric {
	PHOTOMETRIC_MINISWHITE = 0,
	PHOTOMETRIC_MINISBLACK = 1,
	PHOTOMETRIC_RGB = 2,
	PHOTOMETRIC_PALETTE = 3,
	PHOTOMETRIC_MASK = 4,
	PHOTOMETRIC_SEPARATED = 5,
	PHOTOMETRIC_YCBCR = 6,
	PHOTOMETRIC_CIELAB = 8,
	PHOTOMETRIC_ICCLAB = 9,
	PHOTOMETRIC_ITULAB = 10,
	PHOTOMETRIC_LOGL = 32844,
	PHOTOMETRIC_LOGLUV = 32845
};

namespace ome {
	enum DimensionOrder {
		Unknown,
		XYZTC,
		XYCZT,
	};
}

typedef struct _TiffHeader
{
	WORD  Identifier;  /* byte-order Identifier */
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

typedef struct _TifStrip
{
	DWORD	offset;
	DWORD	byteCount;
} TIFSTRIP;

size_t lzw_decompress(uint8_t* dst, uint8_t* src, size_t max_dst_size);

typedef struct _TiffImage
{
	DWORD	nx;
	DWORD	ny;
	WORD	photometric;
	WORD	bps;
	float	xres;
	float	yres;
	uint8_t* description;
	uint8_t* pd;
} TIFIMAGE;

class CTiffImageSource::Impl
{
public:
	Impl() { m_fp = nullptr; }
	~Impl() { clear(); }

	void clear()
	{
		if (m_fp) {
			fclose(m_fp); m_fp = nullptr;
		}
		if (m_img.empty() == false)
		{
			for (int i = 0; i < m_img.size(); ++i)
			{
				_TiffImage& im = m_img[i];
				if (im.description) delete[] im.description;
				delete[] im.pd;
			}
			m_img.clear();
		}

		for (int i = 0; i < m_ifd.size(); ++i)
		{
			// cleanup
			_TifIfd& ifd = m_ifd[i];
			delete[] ifd.TagList;
		}
		m_ifd.clear();
	}

	bool Open();
	bool ReadIFDs();
	bool readIFD();
	bool readImage(_TifIfd& ifd);

public:
	std::string filename;
	bool	m_bigE = false;
	FILE* m_fp = nullptr;
	std::vector<_TiffImage>	m_img;
	std::vector<_TifIfd>	m_ifd;
};

CTiffImageSource::CTiffImageSource(CImageModel* imgModel, const std::string& filename) : CImageSource(TIFF, imgModel), m(new CTiffImageSource::Impl)
{
	m->filename = filename;
}

CTiffImageSource::CTiffImageSource(CImageModel* imgModel) : CImageSource(RAW, imgModel), m(new CTiffImageSource::Impl)
{
}

CTiffImageSource::~CTiffImageSource()
{
	delete m;
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
	if (m->Open() == false) return error("failed opending file.");

	// read all IFDs
	setCurrentTask("Reading IFDs ...");
	if (m->ReadIFDs() == false) return error("failed to read IFDs");

	// read the images
	try {
		char buf[256] = { 0 };
		int n = (int)m->m_ifd.size();
		for (int i = 0; i < n; ++i)
		{
			sprintf(buf, "reading image [%d/%d]", i + 1, n);
			setCurrentTask(buf);
			setProgress((100.0 * i) / n);
			if (m->readImage(m->m_ifd[i]) == false) break;

			if (IsCanceled()) return false;
		}
		setCurrentTask("finishing...");
	}
	catch (std::exception e)
	{
		return error(e.what());
	}
	catch (...)
	{
		return error("unknown exception");
	}
	setProgress(100.0);
	fclose(m->m_fp);
	m->m_fp = nullptr;

	// see if we read any image data
	if (m->m_img.size() == 0) return error("no image data read.");
	int nc = 1;	// nr of channels
	int nx = m->m_img[0].nx;
	int ny = m->m_img[0].ny;
	int nbps = m->m_img[0].bps;
	int dimOrder = ome::DimensionOrder::Unknown;

	float zspacing = 1.f;

	char* szdescription = (char*)m->m_img[0].description;
	if (szdescription && GetImageModel())
	{
		CImageModel* mdl = GetImageModel();
		mdl->SetInfo(szdescription);

		// see if this is an xml formatted text
		if (strncmp(szdescription, "<?xml", 5) == 0)
		{
			string xmlString(szdescription);
			XMLReader xml;
			if (xml.OpenString(xmlString))
			{
				XMLTag tag;
				if (xml.FindTag("OME", tag))
				{
					++tag;
					do
					{
						if (tag == "Image")
						{
							++tag;
							do
							{
								if (tag == "Pixels")
								{
									int sizeC = tag.AttributeValue<int>("SizeC", 1);
									nc = sizeC;

									const char* szdimOrder = tag.AttributeValue("DimensionOrder", true);
									if (szdimOrder)
									{
										if (strcmp(szdimOrder, "XYZTC") == 0) dimOrder = ome::DimensionOrder::XYZTC;
										if (strcmp(szdimOrder, "XYCZT") == 0) dimOrder = ome::DimensionOrder::XYCZT;
									}
								}
								else xml.SkipTag(tag);
								++tag;
							} while (!tag.isend());
						}
						else xml.SkipTag(tag);
						++tag;
					} while (!tag.isend());
				}
			}
		}
		else if (strncmp(szdescription, "ImageJ", 6) == 0)
		{
			string s(szdescription);
			std::istringstream ss(s);
			std::vector<string> strings;
			while (std::getline(ss, s, '\n')) strings.push_back(s);

			for (string& s : strings)
			{
				size_t n = s.find('=');
				if (n != std::string::npos)
				{
					string sl = s.substr(0, n);
					string sr = s.substr(n + 1, std::string::npos);
					if (sl == "channels")
					{
						nc = atoi(sr.c_str());
					}
					else if (sl == "spacing")
					{
						zspacing = atof(sr.c_str());
					}
				}
			}
		}
	}

	// figure out number of z-slices (= images / channels)
	int images = m->m_img.size();
	int nz = images / nc; assert((images % nc) == 0);

	// build the 3D image
	C3DImage* im = new C3DImage;
	if (nc == 1)
	{
		if (nbps == 8)
		{
			im->Create(nx, ny, nz);
			uint8_t* buf = im->GetBytes();
			DWORD imSize = nx * ny;
			for (int i = 0; i < nz; ++i)
			{
				_TiffImage& im = m->m_img[i];
				memcpy(buf, im.pd, imSize);

				if (im.photometric == PHOTOMETRIC_MINISWHITE)
				{
					for (int n = 0; n < imSize; ++n)
					{
						uint8_t b = buf[n];
						buf[n] = 255 - b;
					}
				}
				buf += imSize;
			}
		}
		else if (nbps == 16)
		{
			im->Create(nx, ny, nz, nullptr, CImage::UINT_16);
			WORD* buf = (WORD*)im->GetBytes();
			DWORD imSize = nx * ny;
			for (int i = 0; i < nz; ++i)
			{
				_TiffImage& im = m->m_img[i];
				WORD* b = (WORD*)im.pd;
				if (m->m_bigE)
				{
					for (int j = 0; j < imSize; ++j)
					{
						buf[j] = *b++;
						byteswap(buf[j]);
					}
				}
				else
				{
					for (int j = 0; j < imSize; ++j) buf[j] = *b++;
				}
				buf += imSize;
			}
		}
	}
	else if (nc == 3)
	{
		if (nbps == 8)
		{
			// This will be mapped to a RGB image
			im->Create(nx, ny, nz, nullptr, CImage::UINT_RGB8);
			DWORD imSize = nx * ny;
			for (int k = 0; k < images; ++k)
			{
				_TiffImage& tif = m->m_img[k];
				int slice = k / 3;
				int channel = k % 3;
				uint8_t* buf = im->GetBytes() + slice * imSize * 3;
				if (tif.bps == 8)
				{
					for (int i = 0; i < imSize; ++i)
					{
						buf[3 * i + channel] = tif.pd[i];
					}
				}
			}
		}
		else if (nbps == 16)
		{
			// This will be mapped to a RGB image
			im->Create(nx, ny, nz, nullptr, CImage::UINT_RGB16);
			DWORD imSize = nx * ny;
			assert(dimOrder != ome::DimensionOrder::Unknown);
			if (dimOrder == ome::DimensionOrder::XYCZT)
			{
				for (int k = 0; k < images; ++k)
				{
					_TiffImage& tif = m->m_img[k];
					int slice = k / 3;
					int channel = k % 3;
					WORD* buf = (WORD*)im->GetBytes() + slice * imSize * 3;
					if (tif.bps == 16)
					{
						WORD* b = (WORD*)tif.pd;
						for (int i = 0; i < imSize; ++i)
						{
							buf[3 * i + channel] = b[i];
							if (m->m_bigE) byteswap(buf[3 * i + channel]);
						}
					}
				}
			}
			else if (dimOrder == ome::DimensionOrder::XYZTC)
			{
				for (int k = 0; k < images; ++k)
				{
					_TiffImage& tif = m->m_img[k];
					int slice = k % nz;
					int channel = k / nz;
					WORD* buf = (WORD*)im->GetBytes() + slice * imSize * 3;
					if (tif.bps == 16)
					{
						WORD* b = (WORD*)tif.pd;
						for (int i = 0; i < imSize; ++i)
						{
							buf[3 * i + channel] = b[i];
							if (m->m_bigE) byteswap(buf[3 * i + channel]);
						}
					}
				}
			}
		}
	}

	float fx = (float) nx / m->m_img[0].xres;
	float fy = (float) ny / m->m_img[0].yres;
	float fz = (zspacing != 0 ? nz * zspacing : nz);

	BOX box(0, 0, 0, fx, fy, fz);
	im->SetBoundingBox(box);
	AssignImage(im);

	// clean up
	m->clear();

	return true;
}

bool CTiffImageSource::Impl::Open()
{
	if (filename.empty()) return false;
	const char* szfile = filename.c_str();
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

	return true;
}

bool CTiffImageSource::Impl::ReadIFDs()
{
	// read the IFDs
	try {
		bool bdone = false;
		while (bdone == false) bdone = readIFD();
	}
	catch (...)
	{
		return false;
	}

	// see if anything was read in
	if (m_ifd.empty()) return false;

	return true;
}

bool CTiffImageSource::Impl::readIFD()
{
	// read the IFD
	TIFIFD ifd;
	fread(&ifd.NumDirEntries, sizeof(WORD), 1, m_fp);
	if (m_bigE) byteswap(ifd.NumDirEntries);
	if ((ifd.NumDirEntries <= 0) || (ifd.NumDirEntries >= 65536))
	{
		throw std::domain_error("Invalid number of entries in IFD.");
	}

	// read the tags
	ifd.TagList = new _TifTag[ifd.NumDirEntries];
	int nread = fread(ifd.TagList, sizeof(_TifTag), ifd.NumDirEntries, m_fp);
	if (nread != ifd.NumDirEntries)  throw std::domain_error("Fatal error reading IFD.");

	// swap if necessary
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
			else if (t.DataType == 5) byteswap(t.DataOffset);
		}
	}

	// store the IFD
	m_ifd.push_back(ifd);

	// read the next IDF offset
	DWORD nextIFD = 0;
	fread(&nextIFD, sizeof(DWORD), 1, m_fp);
	if (m_bigE) byteswap(nextIFD);

	// jump to the next IFD
	if (nextIFD != 0)
	{
		if (nextIFD > 0x7FFFFFFF)
		{
			fseek(m_fp, 0x7FFFFFFF, SEEK_SET);
			DWORD offset = nextIFD - 0x7FFFFFFF;
			fseek(m_fp, offset, SEEK_CUR);
		}
		else fseek(m_fp, nextIFD, SEEK_SET);
	}
	else return true;

	return false;
}

bool CTiffImageSource::Impl::readImage(_TifIfd& ifd)
{
	// process tags
	DWORD imWidth = 0, imLength = 0;
	DWORD rowsPerStrip = 0, stripOffsets = 0, stripByteCounts = 0, bitsPerSample = 0, compression = TIF_COMPRESSION_NONE;
	int numberOfStrips = 1;
	int photometric = PHOTOMETRIC_MINISBLACK;
	int descrCount = 0, descrOffset = 0;
	int xres_off = -1, yres_off = -1;
	for (int i = 0; i < ifd.NumDirEntries; ++i)
	{
		TIFTAG& t = ifd.TagList[i];
		switch (t.TagId)
		{
		case 256: imWidth = t.DataOffset; break;
		case 257: imLength = t.DataOffset; break;
		case 258: bitsPerSample = t.DataOffset; break;
		case 259: compression = t.DataOffset; break;
		case 262: photometric = t.DataOffset; break;
		case 270: { descrCount = t.DataCount; descrOffset = t.DataOffset; } break;
		case 278: rowsPerStrip = t.DataOffset; break;
		case 273: { stripOffsets = t.DataOffset; numberOfStrips = t.DataCount; break; }
		case 279: stripByteCounts = t.DataOffset; break;
		case 282: xres_off = t.DataOffset; break;
		case 283: yres_off = t.DataOffset; break;
		}
	}

	if ((bitsPerSample != 8) && (bitsPerSample != 16))
	{
		throw std::domain_error("Only 8 and 16 bit tif supported.");
	}

	if ((compression != TIF_COMPRESSION_NONE) && (compression != TIF_COMPRESSION_LZW))
	{
		throw std::domain_error("Only uncompressed and LZW compressed tiff are supported.");
	}

	if (stripByteCounts == 0)
	{
		if (compression == TIF_COMPRESSION_NONE) stripByteCounts = imWidth * imLength * (bitsPerSample == 16 ? 2 : 1);
		else throw std::domain_error("Invalid stripbyte count.");
	}

	// get the x-resolution
	float xres = 1.f;
	if (xres_off != -1)
	{
		fseek(m_fp, xres_off, SEEK_SET);
		unsigned int nom, den;
		fread(&nom, sizeof(unsigned int), 1, m_fp);
		fread(&den, sizeof(unsigned int), 1, m_fp);
		if (m_bigE) { byteswap(nom); byteswap(den); }

		xres = (float)nom / (float)den;
	}

	// get the x-resolution
	float yres = 1.f;
	if (yres_off != -1)
	{
		fseek(m_fp, yres_off, SEEK_SET);
		unsigned int nom, den;
		fread(&nom, sizeof(unsigned int), 1, m_fp);
		fread(&den, sizeof(unsigned int), 1, m_fp);
		if (m_bigE) { byteswap(nom); byteswap(den); }

		yres = (float)nom / (float)den;
	}

	// read the description if present
	uint8_t* description = nullptr;
	if ((descrCount > 0) && (descrOffset > 0))
	{
		description = new uint8_t[descrCount + 1];
		fseek(m_fp, descrOffset, SEEK_SET);
		int nread = fread(description, 1, descrCount, m_fp); assert(nread == descrCount);
		description[nread] = 0; // don't think this is necessary, but let's just to be safe
	}

	// find the strips
	if (numberOfStrips == 0) throw std::invalid_argument("no strips");
	std::vector<TIFSTRIP> strips(numberOfStrips);
	if (numberOfStrips == 1)
	{
		strips[0].offset = stripOffsets;
		strips[0].byteCount = stripByteCounts;
	}
	else
	{
		fseek(m_fp, stripOffsets, SEEK_SET);
		DWORD* tmp = new DWORD[numberOfStrips];
		fread(tmp, sizeof(DWORD), numberOfStrips, m_fp);
		for (int i = 0; i < numberOfStrips; ++i)
		{
			if (m_bigE) byteswap(tmp[i]);
			strips[i].offset = tmp[i];
		}

		fseek(m_fp, stripByteCounts, SEEK_SET);
		fread(tmp, sizeof(DWORD), numberOfStrips, m_fp);
		for (int i = 0; i < numberOfStrips; ++i)
		{
			if (m_bigE) byteswap(tmp[i]);
			strips[i].byteCount = tmp[i];
		}
		delete[] tmp;
	}

	// allocate buffer for image
	DWORD imSize = imWidth * imLength * (bitsPerSample == 16 ? 2 : 1);
	uint8_t* buf = new uint8_t[imSize];
	uint8_t* tmp = buf;
	_TiffImage im;
	im.nx = imWidth;
	im.ny = imLength;
	im.bps = bitsPerSample;
	im.pd = buf;
	im.photometric = photometric;
	im.description = description;
	im.xres = (xres != 0.f ? xres : 1.f);
	im.yres = (yres != 0.f ? yres : 1.f);
	m_img.push_back(im);

	// This assumes only one strip per image!!
	DWORD bytesRead = 0;
	int nstrip = 0;
	while (bytesRead < imSize)
	{
		TIFSTRIP& strip = strips[nstrip++];
		fseek(m_fp, strip.offset, SEEK_SET);

		if (compression == TIF_COMPRESSION_NONE)
		{
			fread(tmp, strip.byteCount, 1, m_fp);
			bytesRead += strip.byteCount;
			tmp += strip.byteCount;
		}
		else if (compression == TIF_COMPRESSION_LZW)
		{
			// read the compressed stream
			uint8_t* stream = new uint8_t[strip.byteCount];
			fread(stream, strip.byteCount, 1, m_fp);

			// decompress the zream
			int decompressedSize = lzw_decompress(tmp, stream, imSize);
			bytesRead += decompressedSize;
			tmp += decompressedSize;
		}
	}

	return true;
}

void CTiffImageSource::Save(OArchive& ar)
{
    // save image path relative to model file
    fs::path image = m->filename;
    fs::path mdl = ar.GetFilename();

    string relFilename = fs::relative(image, mdl.parent_path()).string();

    ar.WriteChunk(0, relFilename);
    ar.WriteChunk(1, (int)m_type);

	if (m_originalImage)
	{
		BOX box = m_originalImage->GetBoundingBox();
		ar.WriteChunk(100, box.x0);
		ar.WriteChunk(101, box.y0);
		ar.WriteChunk(102, box.z0);
		ar.WriteChunk(103, box.x1);
		ar.WriteChunk(104, box.y1);
		ar.WriteChunk(105, box.z1);
	}
}

void CTiffImageSource::Load(IArchive& ar)
{
    BOX tempBox;
    bool foundBox = false;

    while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0:
			ar.read(m->filename);
			break;
		case 1:
        {
            int type;
            ar.read(type);
            m_type = type;
            break;
        }
			

        case 100:
			ar.read(tempBox.x0);
            foundBox = true;
            break;
        case 101:
			ar.read(tempBox.y0);
            break;
        case 102:
			ar.read(tempBox.z0);
            break;
        case 103:
			ar.read(tempBox.x1);
            break;
        case 104:
			ar.read(tempBox.y1);
            break;
        case 105:
			ar.read(tempBox.z1);
            break;
		}
		ar.CloseChunk();
	}

    // Convert relative file path back to absolute file path
    fs::path image = m->filename;
    fs::path mdl = ar.GetFilename();

    // Old files may have saved absolute paths
    if(image.is_relative())
    {
        m->filename = fs::absolute(mdl.parent_path() / image).string();
    }

    // Read in image data
    Load();

    // Set location of image if it was saved
    if(m_img && foundBox)
    {
        m_img->SetBoundingBox(tempBox);
    }
}

void CTiffImageSource::SetFileName(const std::string& filename)
{
    m->filename = filename;
}

class LZWDecompress
{
	enum {
		CLEAR_CODE = 256,
		EOI_CODE = 257
	};

	typedef std::pair<uint8_t*, int>	Entry;

public:
	LZWDecompress(uint8_t* src) : m_src(src) 
	{
		m_max_size = 0;
		m_s = m_src; m_startBit = 0; m_bps = 9; 
		buildMask(m_bps);
		m_pageSize = 0;
	}

	~LZWDecompress()
	{
		clearDictionary();
	}

	void writeString(const Entry& entry)
	{
		uint8_t* b = entry.first;
		int size = entry.second;
		for (int i = 0; i < size; ++i) { (*m_d++) = *b++; m_dsize++; }
		int a = 0;
	}

	size_t decompress(uint8_t* dst, size_t max_buf_size)
	{
		m_d = dst;
		m_dsize = 0;
		m_max_size = max_buf_size;
		DWORD code = 0, oldcode = 0;
		while ((code = nextCode()) != EOI_CODE)
		{
			if (code == CLEAR_CODE)
			{
				initDictionary();
				code = nextCode();
				if (code == EOI_CODE) break;
				writeString(m_dic[code]);
				oldcode = code;
			}
			else
			{
				if (code < m_dic.size())
				{
					writeString(m_dic[code]);
					Entry sc = appendEntry(m_dic[oldcode], m_dic[code].first[0]);
					addToDictionary(sc);
					oldcode = code;
				}
				else
				{
					Entry OutString = appendEntry(m_dic[oldcode], m_dic[oldcode].first[0]);
					writeString(OutString);
					addToDictionary(OutString);
					oldcode = code;
				}
			}
			assert(m_dsize <= max_buf_size);
		}
		return m_dsize;
	}

	void buildMask(int n)
	{
		DWORD m = 0;
		for (int i = 0; i < n; ++i) m |= (1 << (15-i));
		m_mask = m;
	}

	DWORD nextCode()
	{
		DWORD code = 0;
		int bitsread = 0;
		WORD buf = m_s[0];
		while (bitsread < m_bps)
		{
			code |= ((buf >> (7 - m_startBit)) & 0x01) << (m_bps - bitsread - 1);
			bitsread++;
			m_startBit++;
			if (m_startBit == 8)
			{
				m_s++;
				buf = m_s[0];
				m_startBit = 0;
			}
		}
		return code;
	}

	Entry appendEntry(const Entry& entry, uint8_t b)
	{
		int newSize = entry.second + 1;
		Entry tmp = newEntry(newSize);
		memcpy(tmp.first, entry.first, entry.second);
		tmp.first[entry.second] = b;
		return tmp;
	}

	void addToDictionary(const Entry& s)
	{
		m_dic.push_back(s);
		if (m_dsize == m_max_size) return;

		if (m_dic.size() == 511)
		{
			m_bps = 10;
			buildMask(m_bps);
		}
		else if (m_dic.size() == 1023)
		{
			m_bps = 11;
			buildMask(m_bps);
		}
		else if (m_dic.size() == 2047)
		{
			m_bps = 12;
			buildMask(m_bps);
		}
		assert(m_dic.size() < 4096);
	}

	void clearDictionary()
	{
		for (int i = 0; i < m_pages.size(); ++i) delete[] m_pages[i];
		m_pages.clear();
		m_dic.clear();
		m_pageSize = 0;
	}

	void addDictionary(uint8_t b)
	{
		Entry entry = newEntry(1);
		entry.first[0] = b;
		m_dic.push_back(entry);
	}

	void initDictionary()
	{
		clearDictionary();
		for (int i = 0; i < 256; ++i) addDictionary(i);
		addDictionary(0);
		addDictionary(0);
		m_bps = 9;
	}

	Entry newEntry(int nsize)
	{
		if (nsize > m_pageSize)
		{
			// start a new page
			m_pageSize = DEFAULT_PAGE_SIZE;
			if (nsize > m_pageSize) m_pageSize = nsize;

			m_currentPage = new uint8_t[m_pageSize];
			m_pages.push_back(m_currentPage);
		}

		Entry entry{ m_currentPage, nsize };
		m_pageSize -= nsize;
		m_currentPage += nsize;

		return entry;
	}

	const Entry& operator [] (size_t n) { return m_dic[n]; }

private:
	enum { DEFAULT_PAGE_SIZE = 16384 }; // 16K

private:
	uint8_t* m_src;
	uint8_t* m_s;
	uint8_t* m_d;
	size_t	m_dsize;
	DWORD m_max_size;
	int	  m_startBit;
	int	  m_bps;
	DWORD	m_mask;

	std::vector<Entry>	m_dic;

	std::vector<uint8_t*>	m_pages;
	uint8_t* m_currentPage;
	size_t m_pageSize;
};

// this function decompresses a LZW compressed strip
// the src is the compressed data, and dst is used to store the decoded strip
size_t lzw_decompress(uint8_t* dst, uint8_t* src, size_t max_dst_size)
{
	LZWDecompress lzw(src);
	size_t n = lzw.decompress(dst, max_dst_size);
	return n;
}
