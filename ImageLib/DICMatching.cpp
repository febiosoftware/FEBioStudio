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
#include <stdlib.h>

#include <sitkFFTNormalizedCorrelationImageFilter.h>
#include <sitkNormalizedCorrelationImageFilter.h>
#include <algorithm>

namespace sitk = itk::simple;

//Constructor
CDICMatching::CDICMatching(CDICImage& ref_img, CDICImage& def_img, int iter)
	: m_ref_img(ref_img), m_def_img(def_img), m_iter(iter), m_subSize(ref_img.GetSubSize()), //m_subs_per_row(ref_img.GetSubsPerRow()), m_subs_per_col(ref_img.GetSubsPerCol())
	m_subs_per_row((m_ref_img.GetWidth() / m_subSize) - 1), m_subs_per_col((m_ref_img.GetHeight() / m_subSize) - 1)
{
	//save reference center points
	m_ref_center_points = GetRefCenters(m_ref_img.GetWidth(), m_ref_img.GetHeight(), m_subSize);
	CreateMovingImages();//create subsets
	m_moving_mask = CreateSubsetMask();//create subset mask for fft
	CreateFixedMasks(); //create search areas in deformed image
	m_fixed_SITK_img = *m_def_img.GetSITKImage(); //fixed image

	FFTCorrelation(); //perform "template matching"
}

void CDICMatching::FFTCorrelation()
{
	for (int n = 0; n < m_ref_center_points.size(); n++)
	{
		sitk::Image extraMask = sitk::ReadImage("C:\\Users\\elana\\Documents\\FEBio DIC\\DEBUG\\binary.tif", sitk::sitkFloat32);
		sitk::Image temp = m_searchAreas[n].GetSItkImage();
		sitk::Image cas = sitk::Cast(temp, sitk::sitkFloat32);

		sitk::Image searchArea = sitk::Subtract(extraMask, sitk::InvertIntensity(cas));

		std::vector<int> results = FFT_TemplateMatching(m_fixed_SITK_img.GetSItkImage(), m_movingImages[n].GetSItkImage(),
			searchArea, m_moving_mask.GetSItkImage(), 0.8, n);

		vec2i match;
		match.x = results[0];
		match.y = results[1];

		m_match_points.push_back(match);

		//save metric (NCC) values
		m_NCC.push_back(results[5]);


	}
}

CDICMatching::~CDICMatching()
{

}

std::vector<vec2i> CDICMatching::GetRefCenters(int ref_width, int ref_height, int subSize)
{
	//create vector of reference subset center points
	std::vector<vec2i> ref_sub_centers;

	for (int j = subSize / 2; j < ref_height - subSize; j += subSize)
	{
		for (int i = subSize / 2; i < ref_width - subSize; i += subSize)
		{
			vec2i pt(i, j);
			ref_sub_centers.push_back(pt);
		}
	}

	return ref_sub_centers;
}

CImageSITK CDICMatching::CreateSubsetMask()
{
	sitk::Image moving_mask(m_subSize, m_subSize, sitk::sitkFloat32);

	for (unsigned int j = 0; j < moving_mask.GetHeight(); j++)
	{
		for (unsigned int i = 0; i < moving_mask.GetWidth(); i++)
		{
			float v = 255;
			moving_mask.SetPixelAsFloat({ i,j }, v);
		}
	}

	CImageSITK mm(m_subSize, m_subSize, 1);
	mm.SetItkImage(moving_mask);

	return mm;

}

void CDICMatching::CreateMovingImages()
{
	sitk::Image ref = m_ref_img.GetSITKImage()->GetSItkImage();
	if (ref.GetPixelIDValue() != sitk::sitkFloat32)
	{
		sitk::CastImageFilter caster;
		caster.SetOutputPixelType(sitk::sitkFloat32);
		ref = caster.Execute(ref);
	}

	for (int ii = 0; ii < m_ref_center_points.size(); ii++)
	{
		int px = m_ref_center_points[ii].x;
		int py = m_ref_center_points[ii].y;

		sitk::Image sub(m_subSize, m_subSize, sitk::sitkFloat32);

		int range_x_low = px - m_subSize / 2;
		int range_x_upper = px + m_subSize / 2;
		int range_y_low = py - m_subSize / 2;
		int range_y_upper = py + m_subSize / 2;

		for (unsigned int j = 0; j < m_subSize; j++)
		{
			for (unsigned int i = 0; i < m_subSize; i++)
			{
				auto val = ref.GetPixelAsFloat({ range_x_low + i,range_y_low + j });
				sub.SetPixelAsFloat({ i,j }, val);
			}
		}

		CImageSITK SUB(sub.GetWidth(), sub.GetHeight(), 1);
		SUB.SetItkImage(sub);

		m_movingImages.push_back(SUB);
	}

}

void CDICMatching::CreateFixedMasks()
{
	//difference in reference image vs deformed image dimensions
	int dim_dif_x = m_def_img.GetWidth() - m_ref_img.GetWidth();
	double dimX_scale = 1 + std::abs((double)dim_dif_x) / m_ref_img.GetWidth();
	int dim_dif_y = m_def_img.GetHeight() - m_ref_img.GetHeight();
	double dimY_scale = 1 + std::abs((double)dim_dif_y) / m_ref_img.GetHeight();

	//Subset size and inflation size (USER INPUTS)
	double user_inf_x = 0.7;
	double user_inf_y = 0.7;
	double infl_x = dimX_scale + user_inf_x;
	double infl_y = dimY_scale + user_inf_y;

	//Create range of search area for each subset
	for (int ii = 0; ii < m_ref_center_points.size(); ii++)
	{
		sitk::Image img(m_def_img.GetWidth(), m_def_img.GetHeight(), sitk::sitkFloat32);

		int px = m_ref_center_points[ii].x;
		px = px + dim_dif_x / 2;
		int py = m_ref_center_points[ii].y;
		py = py + dim_dif_y / 2;

		int range_x_low, range_y_low, range_x_upper, range_y_upper;

		int INF_x = (infl_x * m_subSize);
		int INF_y = (infl_y * m_subSize);

		if (px - INF_x < m_subSize / 2 | ii == 0 | ii % m_subs_per_row == 0)
		{
			range_x_low = 0.0;
		}
		else
		{
			range_x_low = px - INF_x;
		}

		if (py - INF_y < m_subSize / 2)
		{
			range_y_low = 0.0;
		}
		else
		{
			range_y_low = py - INF_y;
		}


		if (px + INF_x > img.GetWidth() - m_subSize / 2 | (ii + 1) % m_subs_per_row == 0)
		{
			range_x_upper = img.GetWidth();
		}
		else
		{
			range_x_upper = px + INF_x;
		}

		if (py + INF_y > img.GetHeight() - m_subSize / 2)
		{
			range_y_upper = img.GetHeight();
		}
		else
		{
			range_y_upper = py + INF_y;
		}

		float t = 255;

		for (unsigned int j = range_y_low; j < range_y_upper; j++)
		{
			for (unsigned int i = range_x_low; i < range_x_upper; i++)
			{
				img.SetPixelAsFloat({ i,j }, t);
			}
		}

		CImageSITK IMG(img.GetWidth(), img.GetHeight(), 1);
		IMG.SetItkImage(img);

		m_searchAreas.push_back(IMG);

	}

}

std::vector<int> CDICMatching::FFT_TemplateMatching(sitk::Image fixed, sitk::Image moving, sitk::Image fixed_mask, sitk::Image moving_mask, double overlapFraction, int idx)
{
	//Ensure Images of same type
	if (moving.GetSpacing() != fixed.GetSpacing() | moving.GetDirection() != fixed.GetDirection() | moving.GetOrigin() != fixed.GetOrigin())
	{
		sitk::ResampleImageFilter resampler;
		resampler.SetReferenceImage(fixed);
		moving = resampler.Execute(moving);
	}

	if (fixed_mask.GetPixelIDValue() != sitk::sitkFloat32)
	{
		sitk::CastImageFilter caster;
		caster.SetOutputPixelType(sitk::sitkFloat32);
		fixed_mask = caster.Execute(fixed_mask);
	}

	if (moving_mask.GetPixelIDValue() != sitk::sitkFloat32)
	{
		sitk::CastImageFilter caster;
		caster.SetOutputPixelType(sitk::sitkFloat32);
		moving_mask = caster.Execute(moving_mask);
	}

	//Apply Gaussian Smoothing Filter with appropriate pixel type
	auto sigma = fixed.GetSpacing();
	auto pixel_type = sitk::sitkFloat32;

	//FFT input images
	sitk::Image fft_fixed = sitk::Cast(sitk::SmoothingRecursiveGaussian(fixed, sigma), pixel_type);
	sitk::Image fft_moving = sitk::Cast(sitk::SmoothingRecursiveGaussian(moving, sigma), pixel_type);

	//Execute FFT NCC
	sitk::Image out = sitk::MaskedFFTNormalizedCorrelation(fft_fixed, fft_moving, fixed_mask, moving_mask, 0, overlapFraction);
	out = sitk::SmoothingRecursiveGaussian(out); //Smooth

	//Find maximum (aka best match)
	sitk::MinimumMaximumImageFilter minMax;
	minMax.Execute(out);
	double max = minMax.GetMaximum();

	//Index max location
	sitk::Image cc = sitk::ConnectedComponent(sitk::RegionalMaxima(out, 0.0, 1.0, true));
	sitk::LabelStatisticsImageFilter stats;
	stats.Execute(out, cc);
	auto labels = stats.GetLabels();

	std::vector<std::pair<double, int64_t>> pairs;

	for (int i = 0; i < labels.size(); i++)
	{
		pairs.push_back(std::make_pair(stats.GetMean(labels[i]), labels[i]));
	}

	std::sort(pairs.begin(), pairs.end());


	for (int i = 0; i < pairs.size(); i++)
	{
		auto bb = stats.GetBoundingBox(pairs[i].second);
		double pkx = bb[1];
		double pky = bb[2];

		auto pk = out.TransformContinuousIndexToPhysicalPoint({ pkx,pky });

	}

	auto bb = stats.GetBoundingBox(pairs[pairs.size() - 1].second);

	//determine peak point
	double peak_x = bb[1];
	double peak_y = bb[2];

	std::vector<double> peak_pt;

	peak_pt.push_back(peak_x);
	peak_pt.push_back(peak_y);

	//convert to physical point
	auto peak_phys = out.TransformContinuousIndexToPhysicalPoint(peak_pt);

	//determine center of image (displacement w.r.t. center)
	std::vector<double> center_pt;

	auto out_size = out.GetSize();

	center_pt.push_back(out_size[0] / 2);
	center_pt.push_back(out_size[1] / 2);

	auto center_phys = out.TransformContinuousIndexToPhysicalPoint(center_pt); //convert to physical point

	//Calculate displacement/translation
	auto disp_x = peak_phys[0] - center_phys[0];
	auto disp_y = peak_phys[1] - center_phys[1];

	std::vector<int> results;

	results.push_back(peak_phys[0]);
	results.push_back(peak_phys[1]);
	results.push_back(center_phys[0]);
	results.push_back(center_phys[1]);
	results.push_back(disp_x);
	results.push_back(disp_y);
	results.push_back(max);

	return results;
}

CDICImage& CDICMatching::GetRefImage()
{
	return m_ref_img;
}

CDICImage& CDICMatching::GetDefImage()
{
	return m_def_img;
}

std::vector<vec2i> CDICMatching::GetMatchResults()
{
	return m_match_points;
}

std::vector<vec2i> CDICMatching::GetRefCenterPoints()
{
	return m_ref_center_points;
}

int CDICMatching::GetSubsPerRow()
{
	return m_subs_per_row;
}

int CDICMatching::GetSubsPerCol()
{
	return m_subs_per_col;
}

std::vector<double> CDICMatching::GetNCCVals()
{
	return m_NCC;
}