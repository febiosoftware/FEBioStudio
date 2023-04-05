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

#include "DICMatching.h"
#include "ImageSITK.h"
#include <fstream>

#include <sitkFFTNormalizedCorrelationImageFilter.h>
#include <sitkNormalizedCorrelationImageFilter.h>

namespace sitk = itk::simple;

//Constructor
CDICMatching::CDICMatching(CDICImage& ref_img, CDICImage& def_img, int iter)
	: m_ref_img(ref_img), m_def_img(def_img), m_iter(iter), m_subSize(ref_img.GetSubSize())
{
	//CreateSearchAreas();
	//TemplateMatching();

	std::vector<CDICSubset> ref_subsets = m_ref_img.GetSubsets();

	m_ref_center_points = GetRefCenters(m_ref_img.GetWidth(), m_ref_img.GetHeight(), m_subSize);

    for (int i = 0; i < ref_subsets.size(); i++)
    {
		CDICSubset sub = ref_subsets[i];

		std::string path;
		if (i < 10)
		{
			path = "C:\\Users\\elana\\Documents\\FEBio DIC\\DEBUG\\subsets\\sub0" + std::to_string(i) + ".tif";
		}
		else
		{
			path = "C:\\Users\\elana\\Documents\\FEBio DIC\\DEBUG\\subsets\\sub" + std::to_string(i) + ".tif";

		}
		
		sitk::WriteImage(sub.GetSubsetImage()->GetSItkImage(), path);

    }


}

CDICMatching::~CDICMatching()
{
    for(int i = 0; i < m_searchAreas.size(); i++)
    {
        delete m_searchAreas[i];
    }
}

std::vector<vec2i> CDICMatching::GetRefCenters(int ref_width, int ref_height, int subSize)
{
	//create vector of reference subset center points
	std::vector<vec2i> ref_sub_centers;

	for (int j = subSize / 2; j < ref_height; j += subSize)
	{
		for (int i = subSize / 2; i < ref_width; i += subSize)
		{
			vec2i pt(i, j);
			ref_sub_centers.push_back(pt);
		}
	}

	return ref_sub_centers;
}

CImageSITK CDICMatching::CreateMovingImage(int sub_idx)
{
	int px = m_ref_center_points[sub_idx].x;
	int py = m_ref_center_points[sub_idx].y;

	//sitk::Image sub(m_subSize, m_subSize, sitk::sitkFloat32);
	CImageSITK sub(m_subSize, m_subSize,1);

	int range_x_low = px - m_subSize / 2;
	int range_x_upper = px + m_subSize / 2;
	int range_y_low = py - m_subSize / 2;
	int range_y_upper = py + m_subSize / 2;
	
	std::ofstream debug;
	debug.open("C:\\Users\\elana\\Documents\\FEBio DIC\\DEBUG\\debug.txt");

	Byte* originalBytes = m_ref_img.GetSITKImage()->GetBytes();
	Byte* subsetBytes = sub.GetBytes();

	int originalWidth = m_ref_img.GetWidth();

	for (int j = 0; j < sub.Height(); j++)
	{
		for (int i = 0; i < sub.Width(); i++)
		{
			subsetBytes[j * sub.Width() + i] = originalBytes[(j + py) * originalWidth + i + px];
		}
	}

	return sub;

}



//void CDICMatching::CreateSearchAreas()
//{
//	// for (int i = 0; i < m_ref_img.GetSubsets(); i++)
//    double subSize = m_ref_img.GetSubSize();
//    double searchAreaRatio = m_ref_img.GetSearchAreaRatio();
//
//    for(auto subset :  m_ref_img.GetSubsets())
//	{
//		// cv::Point p = m_ref_subCenters[i];
//        vec2i p = subset.Center();
//
//		//std::cout << p << std::endl; 
//
//		vec2i tl(p.x - subSize / 2, p.y - subSize / 2);
//		vec2i br(p.x + subSize / 2, p.y + subSize / 2);
//
//        int infl = searchAreaRatio*subSize;
//        
//        tl.x - infl/2;
//        tl.y -= infl/2;
//        br.x += infl/2;
//        br.y += infl/2;
//
//        // int inflSize = infl + subSize;
//
//        int width = m_def_img.GetSITKImage()->Width();
//        int height = m_def_img.GetSITKImage()->Height();
//
//        CImageSITK* searchArea = new CImageSITK(width, height, 1);
//        Byte* deformedBytes = m_def_img.GetSITKImage()->GetBytes();
//        Byte* searchBytes = searchArea->GetBytes();
//
//
//        for(int j = 0; j < height; j++)
//        {
//            for(int i = 0; i < width; i++)
//            {
//                int index = j*width+i;
//                // if(i >= tl.x && i <= br.x && j >= tl.y && j <= br.y)   
//                // {
//                    searchBytes[index] = deformedBytes[index];
//                // }
//                // else
//                // {
//                //     searchBytes[index] = 0;
//                // }
//            }
//        }
//
//		m_searchAreas.push_back(searchArea);
//
//	}
//
//	// std::cout << "SEARCH AREAS: " << m_searchAreas.size() << std::endl;
//	// std::cout << "REF SUBSETS: " << m_ref_subsets.size() << std::endl;
//
//}

//void CDICMatching::TemplateMatching()
//{
////     sitk::FFTNormalizedCorrelationImageFilter filter;
//
////     // filter.set
//
////     using FilterType = itk::FFTNormalizedCorrelationImageFilter< ShortImageType, DoubleImageType >;
//
//
//// // FilterType::Pointer filter = FilterType::New();
//
//// // filter->SetFixedImage( fixedImage );
//
//    // sitk::FFTNormalizedCorrelationImageFilter filter;
//    sitk::NormalizedCorrelationImageFilter filter;
//
//    // int height = m_searchAreas[index]->GetSItkImage()
//
//	for (int index = 0; index < m_searchAreas.size(); index++)
//	{
//		// Now perform template matching
//        auto size = m_searchAreas[index]->GetSize();
//
//        CImageSITK mask(size[0], size[1], 1);
//        Byte* maskBuffer = mask.GetBytes();
//
//        for(int i = 0; i < size[0]*size[1]; i++)
//        {
//            maskBuffer[i] = 1;
//        }
//
//        // auto result = filter.Execute(m_searchAreas[index]->GetSItkImage(), m_ref_img.GetSubsetImage(index)->GetSItkImage());
//        // auto result = filter.Execute(m_def_img.GetSITKImage()->GetSItkImage(), m_ref_img.GetSubsetImage(index)->GetSItkImage());
//
//        auto result = filter.Execute(m_searchAreas[index]->GetSItkImage(), mask.GetSItkImage(), m_ref_img.GetSubsetImage(index)->GetSItkImage());
//        // auto result = filter.Execute(m_def_img.GetSITKImage()->GetSItkImage(), mask.GetSItkImage(), m_ref_img.GetSubsetImage(index)->GetSItkImage());
//
//        sitk::RescaleIntensityImageFilter rescaleFiler;
//        rescaleFiler.SetOutputMinimum(0);
//        rescaleFiler.SetOutputMaximum(255);
//        auto temp = rescaleFiler.Execute(result);
//
//        sitk::CastImageFilter castFilter;
//        castFilter.SetOutputPixelType(sitk::sitkUInt8);
//        temp = castFilter.Execute(temp);
//
//        // sitk::WriteImage(m_searchAreas[index]->GetSItkImage(), "/home/mherron/Desktop/search.tif");
//        // sitk::WriteImage(m_ref_img.GetSubsetImage(index)->GetSItkImage(), "/home/mherron/Desktop/subset.tif");
//
//        //sitk::WriteImage(temp, "/home/mherron/Desktop/test.png");
//
//
//        size = result.GetSize();
//        int width = size[0];
//        int height = size[1];
//
//
//
//        double* resultBuffer = result.GetBufferAsDouble();
//
//        
//        double min = -1;
//        vec2i location;
//
//        for(int j = 0; j < height; j++)
//        {
//            for(int i = 0; i < width; i++)
//            {
//                double val = resultBuffer[j*width+i];
//                if(val > min)
//                {
//                    min = val;
//                    location.x = i;
//                    location.y = j;
//                }
//
//            }
//
//        }
//
//        vec2i fixedSize;
//        fixedSize.x = size[0]/2 - location.x;
//        fixedSize.y = size[1]/2 - location.y;
//
//
//		// cv::Mat result;
//		// matchTemplate(m_searchAreas[i], m_ref_subsets[i]->GetSubsetImg(), result, cv::TM_SQDIFF_NORMED);
//
//		// double minV, maxV;
//		// cv::Point minLoc, maxLoc;
//
//		// normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
//
//		// minMaxLoc(result, &minV, &maxV, &minLoc, &maxLoc, cv::Mat());
//
//		// //m_match_topLeft.push_back(minLoc);//top left corner of matched subset
//
//		// int xc = minLoc.x + m_subSize / 2;
//		// int yc = minLoc.y + m_subSize / 2;
//
//        location.x += m_ref_img.GetSubSize() / 2;
//        location.y += m_ref_img.GetSubSize() / 2;
//
//		m_match_center.push_back(location); //match subset in deformed center
//	}
//
//	// ViewMatchResults();
//}

// void Matching::ViewMatchResults()
// {
// 	cv::Mat img_display;
// 	m_def_img.GetImage().copyTo(img_display);

// 	for (int i = 0; i < m_match_center.size(); i++)
// 	{
// 		int distance = cv::norm(m_ref_subCenters[i] - m_match_center[i]);

// 		//std::cout << "distance = " << distance << std::endl;

// 		//if (distance > m_subSize*2) { continue; }

// 		//cv::rectangle(img_display, m_match_topLeft[i], cv::Point(m_match_topLeft[i].x + m_subSize, m_match_topLeft[i].y + m_subSize), cv::Scalar::all(0), 2, 8, 0);
// 		//cv::rectangle(m_match_result, m_match_topLeft[i], cv::Point(m_match_topLeft[i].x + m_subSize, m_match_topLeft[i].y + m_subSize), cv::Scalar::all(0), 2, 8, 0);
// 		//cv::circle(img_display, m_match_topLeft[i], 4, cv::Scalar(255, 0, 0), 4); //top left corner
// 		//cv::circle(img_display, m_match_center[i], 4, cv::Scalar(0, 255, 0), 4); //match center position
// 		//cv::circle(img_display, m_ref_subCenters[i], 4, cv::Scalar(0, 0, 255), 4);
// 		cv::line(img_display, m_ref_subCenters[i], m_match_center[i], cv::Scalar(0, 255, 0), 6);
// 	}

// 	//cv::imshow("RESULTS", img_display);

// 	std::string str = "matchImg_" + std::to_string(m_iter) + ".jpg";
// 	cv::imwrite(str, img_display);


// 	//cv::imshow("Ref", m_ref_img.GetImage());
// 	//cv::imshow("def", m_def_img.GetImage());
// 	//cv::waitKey(0);

// }

CDICImage& CDICMatching::GetRefImage()
{
	return m_ref_img;
}

CDICImage& CDICMatching::GetDefImage()
{
	return m_def_img;
}


// std::vector<cv::Point> Matching::GetRefCenters()
// {
// 	return m_ref_subCenters;
// }

// std::vector<cv::Point> Matching::GetMatchCenters()
// {
// 	return m_match_center;
// }