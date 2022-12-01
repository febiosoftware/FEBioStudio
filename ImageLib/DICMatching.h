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

#pragma once

#include "DICImage.h"

class CDICMatching
{
public:
	CDICMatching(CDICImage& ref_img, CDICImage& def_img, int iter);
    ~CDICMatching();
	void CreateSearchAreas();
	void TemplateMatching();
	// void ViewMatchResults();
	CDICImage& GetRefImage();
	CDICImage& GetDefImage();
	// std::vector<cv::Point> GetRefCenters();
	// std::vector<cv::Point> GetMatchCenters();

private:
	CDICImage& m_ref_img;
	CDICImage& m_def_img;
	// std::vector<Subset*> m_ref_subsets;
	// std::vector<cv::Point> m_ref_subCenters;
	std::vector<CImageSITK*> m_searchAreas;
	// std::vector<cv::Point> m_match_topLeft;
	std::vector<vec2i> m_match_center;
	// std::vector<cv::Mat> m_match_arrows;
	int m_iter;
};