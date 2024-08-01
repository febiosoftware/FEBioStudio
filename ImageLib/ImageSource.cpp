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

#include "ImageSource.h"
#include "ImageModel.h"
#include <ImageLib/3DImage.h>
#include <ImageLib/ImageSITK.h>
#include <FSCore/FSDir.h>

using namespace Post;

CImageSource::CImageSource(int type, CImageModel* imgModel)
    : m_type(type), m_imgModel(imgModel), m_img(nullptr), m_originalImage(nullptr)
{
}

CImageModel* CImageSource::GetImageModel()
{
	return m_imgModel;
}

void CImageSource::SetImageModel(CImageModel* imgModel) 
{
	m_imgModel = imgModel;
}

CImageSource::~CImageSource()
{
    if(m_img != m_originalImage)
    {
        delete m_img;
    }
	
    delete m_originalImage;
}

void CImageSource::ClearFilters()
{
    if( m_img != m_originalImage)
    {
        delete m_img;
        m_img = m_originalImage;
    }
}

C3DImage* CImageSource::GetImageToFilter()
{
    if(m_img == m_originalImage)
    {
#ifdef HAS_ITK
        m_img = new CImageSITK();
#else
        m_img = new C3DImage();
#endif
        
        // Set old bounding box and orientaion on new image as default
        BOX originalBox = m_originalImage->GetBoundingBox();
        mat3d originalOrientation = m_originalImage->GetOrientation();
        m_img->SetBoundingBox(originalBox);
        m_img->SetOrientation(originalOrientation);
    }

    return m_img;
}

void CImageSource::AssignImage(C3DImage* im)
{
    delete m_originalImage;
    if(m_img != m_originalImage)
    {
        delete m_img;
    }

    m_originalImage = im;
    m_img = im;
}

//========================================================================

CRawImageSource::CRawImageSource(CImageModel* imgModel, const std::string& filename, int imgType, int nx, int ny, int nz, BOX box, bool byteSwap)
    : CImageSource(CImageSource::RAW, imgModel), m_filename(filename), m_type(imgType), m_nx(nx), 
        m_ny(ny), m_nz(nz), m_box(box), m_byteSwap(byteSwap)
{
    SetName(FSDir::fileName(filename));
}

CRawImageSource::CRawImageSource(CImageModel* imgModel)
    : CImageSource(CImageSource::RAW, imgModel)
{
	m_nx = m_ny = m_nz = 0;
	m_type = -1;
}

void CRawImageSource::SetFileName(const std::string& filename)
{
	m_filename = filename;
}

bool CRawImageSource::Load()
{
    C3DImage* im = new C3DImage;
    if (im->Create(m_nx, m_ny, m_nz, nullptr, m_type) == false)
    {
        delete im;
        return false;
    }

    if (LoadFromFile(m_filename.c_str(), im) == false)
    {
        delete im;
        return false;
    }

    im->SetBoundingBox(m_box);

    AssignImage(im);

    return true;
}

void byteswap16(uint16_t& v)
{
	unsigned char* b = (unsigned char*)(&v);
	b[0] ^= b[1];
	b[1] ^= b[0];
	b[0] ^= b[1];
}

void byteswap32(uint32_t& v)
{
	unsigned char* b = (unsigned char*)(&v);
	b[0] ^= b[3]; b[3] ^= b[0]; b[0] ^= b[3];
	b[1] ^= b[2]; b[2] ^= b[1]; b[1] ^= b[2];
}

void byteswap64(uint64_t& v)
{
	unsigned char* b = (unsigned char*)(&v);
	b[0] ^= b[7]; b[7] ^= b[0]; b[0] ^= b[7];
	b[1] ^= b[6]; b[6] ^= b[1]; b[1] ^= b[6];
    b[2] ^= b[5]; b[5] ^= b[2]; b[2] ^= b[5];
	b[3] ^= b[4]; b[4] ^= b[3]; b[3] ^= b[4];
}

bool CRawImageSource::LoadFromFile(const char* szfile, C3DImage* im)
{
	FILE* fp = fopen(szfile, "rb");
	if (fp == 0) return false;

	size_t nsize = m_nx * m_ny * m_nz;
	if (nsize == 0) return false;

	int bps = im->BPS();

	uint8_t* buf = im->GetBytes();
	size_t dataSize = bps * nsize;
	size_t nread = fread(buf, 1, dataSize, fp);
	
	// cleanup
	fclose(fp);

    // enum { UINT_8, INT_8, UINT_16, INT_16, UINT_RGB8, INT_RGB8, UINT_RGB16, INT_RGB16, REAL_32, REAL_64 };

    if(m_byteSwap)
    {
        switch (im->PixelType())
        {
        case CImage::UINT_8:
        case CImage::INT_8:
        case CImage::UINT_RGB8:
        case CImage::INT_RGB8:
            break;
        case CImage::UINT_16:
        case CImage::INT_16:
        case CImage::UINT_RGB16:
        case CImage::INT_RGB16:
        {
            uint16_t* data = (uint16_t*)buf;
            for(int i = 0; i < nsize; i++) byteswap16(data[i]);
            break;
        }
        case CImage::UINT_32:
        case CImage::INT_32:
        case CImage::REAL_32:
        {
            uint32_t* data = (uint32_t*)buf;
            for(int i = 0; i < nsize; i++) byteswap32(data[i]);
            break;
        }
        case CImage::REAL_64:
        {
            uint64_t* data = (uint64_t*)buf;
            for(int i = 0; i < nsize; i++) byteswap64(data[i]);
            break;
        }
        default:
            assert(false);
        }
    }

	return (dataSize == nread);
}

void CRawImageSource::Save(OArchive& ar)
{
    ar.WriteChunk(0, m_filename);
    
    ar.WriteChunk(1, m_nx);
    ar.WriteChunk(2, m_ny);
    ar.WriteChunk(3, m_nz);

    ar.WriteChunk(4, m_box.x0);
    ar.WriteChunk(5, m_box.y0);
    ar.WriteChunk(6, m_box.z0);
    ar.WriteChunk(7, m_box.x1);
    ar.WriteChunk(8, m_box.y1);
    ar.WriteChunk(9, m_box.z1);

    ar.WriteChunk(10, m_byteSwap);

	ar.WriteChunk(11, m_type);
}

void CRawImageSource::Load(IArchive& ar)
{
    BOX tempBox;
    bool foundBox = false;

    while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case  0: ar.read(m_filename); break;
		case  1: ar.read(m_nx); break;
		case  2: ar.read(m_ny); break;
		case  3: ar.read(m_nz); break;
		case  4: ar.read(m_box.x0); break;
		case  5: ar.read(m_box.y0); break;
		case  6: ar.read(m_box.z0); break;
		case  7: ar.read(m_box.x1); break;
		case  8: ar.read(m_box.y1); break;
		case  9: ar.read(m_box.z1); break;
		case 10: ar.read(m_byteSwap); break;
		case 11: ar.read(m_type); break;

		// TODO: The data below does not appear to be saved in the Save function. Does this do anything then?
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

    // Read in image data
    Load();

    // Set location of image if it was saved
    if(foundBox)
    {
        m_img->SetBoundingBox(tempBox);
    }

}