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

#include "ImageFilter.h"
#include <FEBioStudio/ClassDescriptor.h>
#include <PostLib/ImageModel.h>
#include <ImageLib/ImageSITK.h>
#include <sitkSmoothingRecursiveGaussianImageFilter.h>
#include <sitkMeanImageFilter.h>
#include <chrono>
#include <ctime>
#include <iostream>

namespace sitk = itk::simple;

CImageFilter::CImageFilter() : m_model(nullptr)
{

}

void CImageFilter::SetImageModel(Post::CImageModel* model)
{
    m_model = model;
}

REGISTER_CLASS(MeanImageFilter, CLASS_IMAGE_FILTER, "Mean Filter", 0);
MeanImageFilter::MeanImageFilter()
{
    static int n = 1;

    char sz[64];
    sprintf(sz, "MeanImageFilter%02d", n);
    n += 1;
    SetName(sz);

    AddIntParam(1, "x Radius");
    AddIntParam(1, "y Radius");
    AddIntParam(1, "z Radius");
}

void MeanImageFilter::ApplyFilter()
{
    if(!m_model) return;

    CImageSITK* image = dynamic_cast<CImageSITK*>(m_model->GetImageSource()->Get3DImage());

    if(!image) return;

    CImageSITK* filteredImage = m_model->GetImageSource()->GetImageToFilter();

    auto start = std::chrono::system_clock::now();

    sitk::MeanImageFilter filter;

    std::vector<unsigned int> indexRadius;

    indexRadius.push_back(GetIntValue(0)); // radius along x
    indexRadius.push_back(GetIntValue(1)); // radius along y
    indexRadius.push_back(GetIntValue(2)); // radius along z

    filter.SetRadius(indexRadius);

    filteredImage->SetItkImage(filter.Execute(image->GetSItkImage()));

    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
}

REGISTER_CLASS(GaussianImageFilter, CLASS_IMAGE_FILTER, "Gaussian Filter", 0);
GaussianImageFilter::GaussianImageFilter()
{
    static int n = 1;

    char sz[64];
    sprintf(sz, "GaussianImageFilter%02d", n);
    n += 1;
    SetName(sz);

    AddDoubleParam(2.0, "sigma");
}

void GaussianImageFilter::ApplyFilter()
{
    if(!m_model) return;

    CImageSITK* image = dynamic_cast<CImageSITK*>(m_model->GetImageSource()->Get3DImage());

    if(!image) return;

    CImageSITK* filteredImage = m_model->GetImageSource()->GetImageToFilter();

    auto start = std::chrono::system_clock::now();

    sitk::SmoothingRecursiveGaussianImageFilter filter;

    const double sigma = GetFloatValue(0);
    std::cout << sigma << std::endl;
    filter.SetSigma(sigma);

    filteredImage->SetItkImage(filter.Execute(image->GetSItkImage()));

    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

}

REGISTER_CLASS(ThresholdImageFilter, CLASS_IMAGE_FILTER, "Threshold Filter", 0);
ThresholdImageFilter::ThresholdImageFilter()
{
    static int n = 1;

    char sz[64];
    sprintf(sz, "ThresholdImageFilter%02d", n);
    n += 1;
    SetName(sz);

    AddIntParam(255, "max");
    AddIntParam(0, "min");
}

void ThresholdImageFilter::ApplyFilter()
{
    if(!m_model) return;

    C3DImage* image = m_model->GetImageSource()->Get3DImage();

    int max = GetIntValue(0);
    int min = GetIntValue(1);

    if(min >= max) return;

    auto start = std::chrono::system_clock::now();

    Byte* originalBytes = image->GetBytes();
    Byte* filteredBytes = m_model->GetImageSource()->GetImageToFilter(true)->GetBytes();

    int factor = 255;

    for(int i = 0; i < image->Width()*image->Height()*image->Depth(); i++)
    {
        if(originalBytes[i] > max || originalBytes[i] < min)
        {
            filteredBytes[i] = 0;
        }
        else
        {
            filteredBytes[i] = originalBytes[i];
        }

    }

    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
}