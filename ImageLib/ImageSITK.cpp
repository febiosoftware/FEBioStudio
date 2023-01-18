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

#ifdef HAS_ITK
#include <FSCore/FSDir.h>

namespace sitk = itk::simple;

CImageSITK::CImageSITK()
{

}

CImageSITK::CImageSITK(int nx, int ny, int nz)
    : m_sitkImage(nx, ny, nz, sitk::sitkUInt8)
{
    m_cx = nx;
    m_cy = ny;
    m_cz = nz;

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

bool CImageSITK::LoadFromFile(std::string filename, bool isDicom)
{
    m_filename = filename.c_str();

    if(isDicom)
    {
        sitk::ImageSeriesReader reader;

        string absolutePath = FSDir::fileDir(filename);
        const std::vector<std::string> dicom_names = sitk::ImageSeriesReader::GetGDCMSeriesFileNames(absolutePath);
        reader.SetFileNames( dicom_names );

        m_sitkImage = reader.Execute();
    }
    else
    {
        sitk::ImageFileReader reader;
        reader.SetFileName(filename);

        m_sitkImage = reader.Execute();
    }

    if(m_sitkImage.GetPixelID() != sitk::sitkUInt8)
    {
        sitk::RescaleIntensityImageFilter rescaleFiler;
        rescaleFiler.SetOutputMinimum(0);
        rescaleFiler.SetOutputMaximum(255);
        m_sitkImage = rescaleFiler.Execute(m_sitkImage);

        try
        {
            sitk::CastImageFilter castFilter;
            castFilter.SetOutputPixelType(sitk::sitkUInt8);
            m_sitkImage = castFilter.Execute(m_sitkImage);
        }
        catch(itk::simple::GenericException& e)
        {
            throw std::runtime_error("FEBio Studio is not currently capable of reading multichannel images.");
        }
        
        
    }

    FinalizeImage();

    return true;
}

bool CImageSITK::LoadFromStack(std::vector<std::string> filenames)
{
    m_filename = filenames[0].c_str();

    sitk::RescaleIntensityImageFilter rescaleFiler;
    rescaleFiler.SetOutputMinimum(0);
    rescaleFiler.SetOutputMaximum(255);
        
    sitk::CastImageFilter castFilter;
    castFilter.SetOutputPixelType(sitk::sitkUInt8);

    sitk::ImageFileReader reader;
    reader.SetFileName(filenames[0]);
    sitk::Image slice = reader.Execute();

    unsigned int nx = slice.GetWidth();
    unsigned int ny = slice.GetHeight();
    unsigned int nz = filenames.size();

    m_sitkImage = sitk::Image(nx, ny, nz, sitk::sitkUInt8);
    Byte* imgBytes = m_sitkImage.GetBufferAsUInt8();

    // std::cout << slice.GetPixelID() << std::endl;
    // std::vector<uint32_t> index = {0,0};
    // std::cout << slice.GetPixelAsVectorUInt8(index).size() << std::endl;

    if(slice.GetPixelID() != sitk::sitkUInt8)
    {
        slice = rescaleFiler.Execute(slice);
        slice = castFilter.Execute(slice);
    }

    Byte* sliceBytes = slice.GetBufferAsUInt8();

    for(int index = 0; index < nx*ny; index++)
    {
        imgBytes[index] = sliceBytes[index];
    }
    
    for(int name = 1; name < filenames.size(); name++)
    {
        reader.SetFileName(filenames[name]);
        slice = reader.Execute();

        if(slice.GetPixelID() != sitk::sitkUInt8)
        {
            slice = rescaleFiler.Execute(slice);
            slice = castFilter.Execute(slice);
        }

        sliceBytes = slice.GetBufferAsUInt8();

        for(int index = nx*ny*name; index < nx*ny*(name+1); index++)
        {
            imgBytes[index] = sliceBytes[index];
        }
    }

    FinalizeImage();

    return true;
}

void CImageSITK::FinalizeImage()
{
    m_pb = m_sitkImage.GetBufferAsUInt8();

    m_cx = m_sitkImage.GetWidth();
    m_cy = m_sitkImage.GetHeight();
    m_cz = m_sitkImage.GetDepth();
}

BOX CImageSITK::GetBoundingBox()
{
    std::vector<unsigned int> size = m_sitkImage.GetSize();
	std::vector<double> origin = m_sitkImage.GetOrigin();
	std::vector<double> spacing = m_sitkImage.GetSpacing();

	return BOX(origin[0],origin[1],origin[2],spacing[0]*size[0],spacing[1]*size[1],spacing[2]*size[2]);
}

void CImageSITK::SetBoundingBox(BOX& box)
{
    m_sitkImage.SetOrigin({box.x0, box.y0, box.z0});

    std::vector<unsigned int> size = m_sitkImage.GetSize();

	try {
	    m_sitkImage.SetSpacing({box.x1/size[0], box.y1/size[1], box.z1/size[2]});
	}
	catch (...)
	{
		// ITK doesn't like zero spacing.
	}
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
    if(image.GetPixelID() != sitk::sitkUInt8)
    {
        sitk::CastImageFilter castFilter;
        castFilter.SetOutputPixelType(sitk::sitkUInt8);
        m_sitkImage = castFilter.Execute(image);
    }
    else
    {
        m_sitkImage = image;
    }

    m_cx = m_sitkImage.GetWidth();
    m_cy = m_sitkImage.GetHeight();;
    m_cz = m_sitkImage.GetDepth();;

    m_pb = m_sitkImage.GetBufferAsUInt8();
}

#endif