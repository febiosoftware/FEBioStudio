/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "ImageFilter.h"
#include <PostLib/ImageModel.h>
#include <ImageLib/ITKImage.h>

#include "itkImage.h"
#include "itkMeanImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include <chrono>
#include <ctime>
#include <iostream>


CImageFilter::CImageFilter(Post::CImageModel* model) : model(model)
{

}

MeanImageFilter::MeanImageFilter(Post::CImageModel* model) : CImageFilter(model)
{

}

void MeanImageFilter::ApplyFilter()
{
    CITKImage* image = dynamic_cast<CITKImage*>(model->GetImageSource()->Get3DImage());

    if(!image) return;

    // Byte* bytes = image->GetBytes();

    // int size = image->Width()*image->Height()*image->Depth();

    // for(int index = 0; index < size; index++)
    // {
    //     bytes[index] = 0;
    // }

    auto start = std::chrono::system_clock::now();

    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType, 3>;

    using FilterType = itk::MeanImageFilter<ImageType, ImageType>;
    FilterType::Pointer filter = FilterType::New();

    ImageType::SizeType indexRadius;

    indexRadius[0] = 1; // radius along x
    indexRadius[1] = 1; // radius along y
    indexRadius[2] = 1; // radius along z

    filter->SetRadius(indexRadius);

    itk::SmartPointer<itk::Image<unsigned char, 3>> itkImage = image->GetItkImage();

    filter->SetInput(itkImage);
    filter->Update();

    image->SetItkImage(filter->GetOutput());

    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

    // image->SetBytes(itkImage->GetBufferPointer());

    // itkImage->Update();

}

GaussianImageFilter::GaussianImageFilter(Post::CImageModel* model)
    : CImageFilter(model)
{

}

void GaussianImageFilter::ApplyFilter()
{
    CITKImage* image = dynamic_cast<CITKImage*>(model->GetImageSource()->Get3DImage());

    if(!image) return;

    auto start = std::chrono::system_clock::now();

    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType, 3>;

    using FilterType = itk::SmoothingRecursiveGaussianImageFilter<ImageType, ImageType>;

    FilterType::Pointer filter = FilterType::New();
    filter->SetNormalizeAcrossScale(false);

    itk::SmartPointer<itk::Image<unsigned char, 3>> itkImage = image->GetItkImage();

    filter->SetInput(itkImage);

    const double sigma = 2.0;
    filter->SetSigma(sigma);

    filter->Update();

    image->SetItkImage(filter->GetOutput());

    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";


}