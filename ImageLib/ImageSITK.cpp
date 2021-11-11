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
#include <QFileInfo>

#pragma once

namespace sitk = itk::simple;

SITKImage::SITKImage()
{

}

SITKImage::SITKImage(int nx, int ny, int nz)
    : m_sitkImage(nx, ny, nz, sitk::sitkUInt8)
{
    m_cx = nx;
    m_cy = ny;
    m_cz = nz;

    FinalizeImage();
}

SITKImage::~SITKImage()
{
    m_pb = nullptr;
}

bool SITKImage::LoadFromFile(const char* filename, bool isDicom)
{
    m_filename = filename;

    if(isDicom)
    {
        sitk::ImageSeriesReader reader;

        QFileInfo info(filename);
        const std::vector<std::string> dicom_names = sitk::ImageSeriesReader::GetGDCMSeriesFileNames( info.absolutePath().toStdString().c_str() );
        reader.SetFileNames( dicom_names );

        m_sitkImage = reader.Execute();
    }
    else
    {
        sitk::ImageFileReader reader;
        reader.SetFileName(filename);

        m_sitkImage = reader.Execute();
    }

    sitk::RescaleIntensityImageFilter rescaleFiler;
    rescaleFiler.SetOutputMinimum(0);
    rescaleFiler.SetOutputMaximum(255);
    m_sitkImage = rescaleFiler.Execute(m_sitkImage);

    sitk::CastImageFilter castFilter;
    castFilter.SetOutputPixelType(sitk::sitkUInt8);
    m_sitkImage = castFilter.Execute(m_sitkImage);

    FinalizeImage();

    return true;
}

void SITKImage::FinalizeImage()
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

std::vector<unsigned int> SITKImage::GetSize()
{
    return m_sitkImage.GetSize();
}

std::vector<double> SITKImage::GetOrigin()
{
    return m_sitkImage.GetOrigin();
}

std::vector<double> SITKImage::GetSpacing()
{
    return m_sitkImage.GetSpacing();
}

itk::simple::Image SITKImage::GetSItkImage()
{
    return m_sitkImage;
}

void SITKImage::SetItkImage(itk::simple::Image image)
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