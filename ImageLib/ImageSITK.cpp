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


#include "ImageSITK.h"
#include <FSCore/FSDir.h>

#ifdef HAS_ITK
#include <sitkImportImageFilter.h>
#include <sitkImageFileWriter.h>



namespace sitk = itk::simple;

itk::simple::Image CImageSITK::SITKImageFrom3DImage(C3DImage* img)
{
    BOX box = img->GetBoundingBox();
    unsigned int nx = img->Width();
    unsigned int ny = img->Height();
    unsigned int nz = img->Depth();
    mat3d orientation = img->GetOrientation();

    sitk::ImportImageFilter filter;    
    filter.SetSize({nx, ny, nz});
    filter.SetOrigin({box.x0, box.y0, box.z0});
    filter.SetSpacing({(box.x1 - box.x0)/nx, (box.y1 - box.y0)/ny, (box.z1 - box.z0)/nz});

    std::vector<double> orient {*orientation[0], *orientation[1], *orientation[2], *orientation[3], 
        *orientation[4], *orientation[5], *orientation[6], *orientation[7], *orientation[8]};
    filter.SetDirection(orient);

    switch (img->PixelType())
    {
    case CImage::UINT_8:
        filter.SetBufferAsUInt8((uint8_t*)img->GetBytes());
        break;
    case CImage::INT_8:
        filter.SetBufferAsInt8((int8_t*)img->GetBytes());
        break;
    case CImage::UINT_16:
        filter.SetBufferAsUInt16((uint16_t*)img->GetBytes());
        break;
    case CImage::INT_16:
        filter.SetBufferAsInt16((int16_t*)img->GetBytes());
        break;
    case CImage::UINT_32:
        filter.SetBufferAsUInt32((uint32_t*)img->GetBytes());
        break;
    case CImage::INT_32:
        filter.SetBufferAsInt32((int32_t*)img->GetBytes());
        break;
    case CImage::UINT_RGB8:
        filter.SetBufferAsUInt8((uint8_t*)img->GetBytes(), 3);
        break;
    case CImage::INT_RGB8:
        filter.SetBufferAsInt8((int8_t*)img->GetBytes(), 3);
        break;
    case CImage::UINT_RGB16:
        filter.SetBufferAsUInt16((uint16_t*)img->GetBytes(), 3);
        break;
    case CImage::INT_RGB16:
        filter.SetBufferAsInt16((int16_t*)img->GetBytes(), 3);
        break;
    case CImage::REAL_32:
        filter.SetBufferAsFloat((float*)img->GetBytes());
        break;
    case CImage::REAL_64:
        filter.SetBufferAsDouble((double*)img->GetBytes());
        break;
    default:
        assert(false);
    }

    return filter.Execute();
}

bool CImageSITK::WriteSITKImage(C3DImage* img, const std::string& filename)
{
    itk::simple::Image itkImage;

    if(!dynamic_cast<CImageSITK*>(img))
    {
        itkImage = SITKImageFrom3DImage(img);
    }
    else
    {
        itkImage = dynamic_cast<CImageSITK*>(img)->GetSItkImage();
    }

    sitk::ImageFileWriter writer;
    writer.SetFileName(filename);

    try
    {
        writer.Execute(itkImage);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';

        return false;
    }
    
    return true;
}

CImageSITK::CImageSITK()
{

}

CImageSITK::CImageSITK(int nx, int ny, int nz, int pixelType)
{
    switch (pixelType)
    {
    case CImage::UINT_8:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkUInt8);
        break;
    case CImage::INT_8:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkInt8);
        break;
    case CImage::UINT_16:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkUInt16);
        break;
    case CImage::INT_16:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkInt16);
        break;
    case CImage::UINT_32:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkUInt32);
        break;
    case CImage::INT_32:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkInt32);
        break;
    case CImage::UINT_RGB8:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkVectorUInt8);
        break;
    case CImage::INT_RGB8:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkVectorInt8);
        break;
    case CImage::UINT_RGB16:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkVectorUInt16);
        break;
    case CImage::INT_RGB16:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkVectorInt16);
        break;
    case CImage::REAL_32:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkFloat32);
        break;
    case CImage::REAL_64:
        m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkFloat64);
        break;
    default:
        assert(false);
    }

    m_pixelType = pixelType;

    FinalizeImage();
}

CImageSITK::~CImageSITK()
{
    m_pb = nullptr;
}

bool CImageSITK::CreateFrom3DImage(C3DImage* im)
{
#ifdef HAS_ITK
	if (im == nullptr) return false;

	int nx = im->Width();
	int ny = im->Height();
	int nz = im->Depth();

	m_sitkImage = sitk::Image(nx, ny, nz, sitk::PixelIDValueEnum::sitkUInt8);
	uint8_t* pb = m_sitkImage.GetBufferAsUInt8();
	uint8_t* ps = (uint8_t*) im->GetBytes();
	memcpy(pb, ps, nx * ny * nz);
	
	FinalizeImage();

	return true;
#else
	return false;
#endif
}

void CImageSITK::FinalizeImage()
{
    m_pb = (uint8_t*)m_sitkImage.GetBufferAsVoid();

    m_cx = m_sitkImage.GetWidth();
    m_cy = m_sitkImage.GetHeight();
    m_cz = m_sitkImage.GetDepth();
}

BOX CImageSITK::GetBoundingBox()
{
    std::vector<unsigned int> size = m_sitkImage.GetSize();
	std::vector<double> origin = m_sitkImage.GetOrigin();
	std::vector<double> spacing = m_sitkImage.GetSpacing();

	return BOX(origin[0],origin[1],origin[2],spacing[0]*size[0]+origin[0],spacing[1]*size[1]+origin[1],spacing[2]*size[2]+origin[2]);
}

void CImageSITK::SetBoundingBox(BOX& box)
{
    m_sitkImage.SetOrigin({box.x0, box.y0, box.z0});

    std::vector<unsigned int> size = m_sitkImage.GetSize();

	try {
	    m_sitkImage.SetSpacing({(box.x1 - box.x0)/size[0], (box.y1 - box.y0)/size[1], (box.z1 - box.z0)/size[2]});
	}
	catch (...)
	{
		// ITK doesn't like zero spacing.
	}

    C3DImage::SetBoundingBox(box);
}

mat3d CImageSITK::GetOrientation()
{
    std::vector<double> orient = m_sitkImage.GetDirection();

    return mat3d(orient[0], orient[1], orient[2], orient[3], orient[4], orient[5], orient[6], orient[7], orient[8]);
}

void CImageSITK::SetOrientation(mat3d& orientation)
{
    std::vector<double> orient {*orientation[0], *orientation[1], *orientation[2], *orientation[3], 
        *orientation[4], *orientation[5], *orientation[6], *orientation[7], *orientation[8]};

    m_sitkImage.SetDirection(orient);

    C3DImage::SetOrientation(orientation);
}

std::vector<unsigned int> CImageSITK::GetSize()
{
    return m_sitkImage.GetSize();
}

std::vector<double> CImageSITK::GetOrigin()
{
    return m_sitkImage.GetOrigin();
}

std::vector<double> CImageSITK::GetSpacing()
{
    return m_sitkImage.GetSpacing();
}

itk::simple::Image CImageSITK::GetSItkImage()
{
    return m_sitkImage;
}

void CImageSITK::SetItkImage(itk::simple::Image image)
{
    m_sitkImage = image;

    m_cx = m_sitkImage.GetWidth();
    m_cy = m_sitkImage.GetHeight();
    m_cz = m_sitkImage.GetDepth();
    if(m_cz == 0) m_cz = 1;

    // These 2 functions set these values so that if they're used internally, they're right
    m_box = GetBoundingBox();
    m_orientation = GetOrientation();

    m_pb = (uint8_t*)m_sitkImage.GetBufferAsVoid();

    switch (m_sitkImage.GetPixelID())
    {
    case sitk::sitkInt8:
        m_pixelType = CImage::INT_8;
        m_bps = 1;
        break;
    case sitk::sitkUInt8:
        m_pixelType = CImage::UINT_8;
        m_bps = 1;
        break;
    case sitk::sitkInt16:
        m_pixelType = CImage::INT_16;
        m_bps = 2;
        break;
    case sitk::sitkUInt16:
        m_pixelType = CImage::UINT_16;
        m_bps = 2;
        break;
    case sitk::sitkInt32:
        m_pixelType = CImage::INT_32;
        m_bps = 4;
        break;
    case sitk::sitkUInt32:
        m_pixelType = CImage::UINT_32;
        m_bps = 4;
        break;
    case sitk::sitkVectorInt8:
        m_pixelType = CImage::INT_RGB8;
        m_bps = 3;
        break;
    case sitk::sitkVectorUInt8:
        m_pixelType = CImage::UINT_RGB8;
        m_bps = 3;
        break;
    case sitk::sitkVectorInt16:
        m_pixelType = CImage::INT_RGB16;
        m_bps = 6;
        break;
    case sitk::sitkVectorUInt16:
        m_pixelType = CImage::UINT_RGB16;
        m_bps = 6;
        break;
    case sitk::sitkFloat32:
        m_pixelType = CImage::REAL_32;
        m_bps = 4;
        break;
    case sitk::sitkFloat64:
        m_pixelType = CImage::REAL_64;
        m_bps = 8;
        break;
    default:
        break;
    }
}

#endif