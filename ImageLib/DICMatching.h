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
#include "ImageLib/ImageSITK.h"

namespace sitk = itk::simple;

class CDICMatching
{
public:
	CDICMatching(CDICImage& ref_img, CDICImage& def_img, int iter);
	~CDICMatching();
	CDICImage& GetRefImage();
	CDICImage& GetDefImage();
	std::vector<vec2i> GetRefCenters(int ref_width, int ref_height, int subSize);
	CImageSITK CreateSubsetMask();
	void CreateFixedMasks();
	void CreateMovingImages();
	std::vector<int> FFT_TemplateMatching(sitk::Image fixed, sitk::Image moving, sitk::Image fixed_mask, sitk::Image moving_mask, double overlapFraction, int idx);
	void FFTCorrelation();
	std::vector<vec2i> GetMatchResults();
	std::vector<vec2i> GetRefCenterPoints();
	int GetSubsPerRow();
	int GetSubsPerCol();

private:
	CDICImage& m_ref_img;
	CDICImage& m_def_img;
	std::vector<CImageSITK> m_searchAreas;
	int m_iter;
	std::vector<vec2i> m_match_points;
	std::vector<vec2i> m_ref_center_points;
	int m_subSize;
	std::vector<CImageSITK> m_movingImages;
	CImageSITK m_fixed_SITK_img;
	CImageSITK m_moving_mask;
	int m_subs_per_row;
	int m_subs_per_col;

};