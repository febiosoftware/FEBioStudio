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

#include "ImageFilterSITK.h"
#include "ImageModel.h"
#include "ImageSource.h"
#include "SITKTools.h"

#ifdef HAS_ITK
#include <sitkSmoothingRecursiveGaussianImageFilter.h>
#include <sitkMeanImageFilter.h>
#include <sitkAdaptiveHistogramEqualizationImageFilter.h>

namespace sitk = itk::simple;


class ITKException : public std::exception
{
public:
    ITKException(std::exception& e)
    {
        std::string str = e.what();
        int pos = str.find("\n");

        if(pos == str.npos)
        {
            m_what = str.c_str();
        }
        else
        {
            m_what = str.substr(pos+1, str.npos).c_str();
        }
    }

    const char* what() const noexcept override
    {
        return m_what.c_str();
    }

private:
    std::string m_what;
};


SITKImageFiler::SITKImageFiler()
{

}

itk::simple::Image SITKImageFiler::GetSITKImage()
{
    C3DImage* image = m_model->GetImageSource()->Get3DImage();
    
    return SITKImageFrom3DImage(image);
} 


MeanImageFilter::MeanImageFilter()
{
    static int n = 1;
	 
    char sz[64];
    sprintf(sz, "MeanImageFilter%02d", n);
    n += 1;
    SetName(sz);

    AddIntParam(1, "x Radius")->SetIntRange(0, 9999999);
    AddIntParam(1, "y Radius")->SetIntRange(0, 9999999);
    AddIntParam(1, "z Radius")->SetIntRange(0, 9999999);
}

void MeanImageFilter::ApplyFilter()
{
    if(!m_model) return;

    sitk::Image original = GetSITKImage();
    
    C3DImage* filteredImage = m_model->GetImageSource()->GetImageToFilter();

    sitk::MeanImageFilter filter;

    std::vector<unsigned int> indexRadius;

    indexRadius.push_back(GetIntValue(0)); // radius along x
    indexRadius.push_back(GetIntValue(1)); // radius along y
    indexRadius.push_back(GetIntValue(2)); // radius along z

    filter.SetRadius(indexRadius);

    try
    {
        CopyTo3DImage(filteredImage, filter.Execute(original));
    }
    catch(std::exception& e)
    {
        throw ITKException(e);
    }
}

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

    sitk::Image original = GetSITKImage();

    C3DImage* filteredImage = m_model->GetImageSource()->GetImageToFilter();

    sitk::SmoothingRecursiveGaussianImageFilter filter;

    filter.SetSigma(GetFloatValue(0));

    try
    {
        CopyTo3DImage(filteredImage, filter.Execute(original));
    }
    catch(std::exception& e)
    {
        throw ITKException(e);
    }
}

AdaptiveHistogramEqualizationFilter::AdaptiveHistogramEqualizationFilter()
{
    static int n = 1;

    char sz[64];
    sprintf(sz, "AdaptiveHistogramEqualization%02d", n);
    n += 1;
    SetName(sz);

    AddDoubleParam(0.3, "Aplha")->SetFloatRange(0, 1);
    AddDoubleParam(0.3, "Beta")->SetFloatRange(0, 1);

    AddIntParam(5, "x Radius")->SetIntRange(0, 9999999);
    AddIntParam(5, "y Radius")->SetIntRange(0, 9999999);
    AddIntParam(5, "z Radius")->SetIntRange(0, 9999999);
}

void AdaptiveHistogramEqualizationFilter::ApplyFilter()
{
    if(!m_model) return;

    sitk::Image original = GetSITKImage();

    C3DImage* filteredImage = m_model->GetImageSource()->GetImageToFilter();

    sitk::AdaptiveHistogramEqualizationImageFilter filter;
    filter.SetAlpha(GetFloatValue(0));
    filter.SetBeta(GetFloatValue(1));
    filter.SetRadius({(unsigned int)GetIntValue(2), (unsigned int)GetIntValue(3), (unsigned int)GetIntValue(4)});

    try
    {
        CopyTo3DImage(filteredImage, filter.Execute(original));

    }
    catch(std::exception& e)
    {
        throw ITKException(e);
    }
}

#endif
