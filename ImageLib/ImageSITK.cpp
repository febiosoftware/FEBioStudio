/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


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

        sitk::CastImageFilter castFilter;
        castFilter.SetOutputPixelType(sitk::sitkUInt8);
        m_sitkImage = castFilter.Execute(m_sitkImage);
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
    // finalImage = originalImage;
    m_pb = m_sitkImage.GetBufferAsUInt8();

    m_cx = m_sitkImage.GetWidth();
    m_cy = m_sitkImage.GetHeight();
    m_cz = m_sitkImage.GetDepth();

    std::vector<double> spacing = m_sitkImage.GetSpacing();

    std::cout << spacing[0] << " " << spacing[1] << " " << spacing[2] << std::endl;

    std::vector<double> origin = m_sitkImage.GetOrigin();
    std::cout << origin[0] << " " << origin[1] << " " << origin[2] << std::endl;
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