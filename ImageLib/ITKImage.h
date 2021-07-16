/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifdef HAS_ITK
#pragma once

#include "3DImage.h"

#include "itkImage.h"
#include <itkImageFileReader.h>
#include <itkGDCMImageIO.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageSeriesReader.h>
#include <itkImageIOBase.h>
#include <QFileInfo>

// enum IMAGETYPE {DICOMTYPE, TIFFTYPE};

// template <typename pType>
class CITKImage : public C3DImage
{
    using IOPixelType = itk::IOPixelEnum;
    using IOComponentType = itk::IOComponentEnum;

    // using ImageType = itk::Image<pType, 3>;
    // using ReaderType = itk::ImageFileReader<ImageType>;
    // using ReaderType = itk::ImageSeriesReader<ImageType>;

    using FinalImageType = itk::Image<unsigned char, 3>;
    // using CastType = itk::CastImageFilter<ImageType, FinalImageType>;

public:
    CITKImage() {}
    ~CITKImage() override {}

    bool LoadFromFile(const char* filename);

private:
    int ReadScalarImage();

    template<class TImage>
    bool ReadImage();


private:
    const char* m_filename;
    bool m_isDicom;
    IOPixelType pixelType;
    IOComponentType componentType;
    // itk::ImageBase<3> originalImage;
    // itk::SmartPointer<ImageType> image;
    itk::SmartPointer<FinalImageType> finalImage;
};

#endif