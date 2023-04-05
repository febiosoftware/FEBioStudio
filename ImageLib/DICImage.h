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

#include <vector>
#include <FSCore/box.h>
#include "ImageSITK.h"

class CDICImage;

// class CDICRect
// {
// public:
//     CDICRect(vec2i tl = vec2i(0,0), vec2i br = vec2i(0,0));

//     CDICRect operator+(int val);
//     CDICRect operator+(vec2i val);


// private:
//     vec2i m_tl;
//     vec2i m_br;

// };

class CDICSubset
{
public:
    CDICSubset(CDICImage* image, int x0, int y0, int x1, int y1);

    int Width() { return m_width; }
    int Height() { return m_height; }

    vec2i Center() { return m_center; }

    CImageSITK* GetSubsetImage() { return &m_subset_image; }

private:
    CDICImage* m_image;
    CImageSITK m_subset_image;
    int m_x0, m_y0;
    int m_x1, m_y1;
    int m_width, m_height;
    vec2i m_center;
};

class CDICImage
{
public:
    CDICImage(CImageSITK* image, double window = 1.0, double sigma = 0.4, int subSize = 51, int subSpacing = 51, double searchAreaRatio =7.0);

    CImageSITK* GetSITKImage() { return m_image; }

    int GetSubSize() { return m_subSize; }
    int GetSubSpacing() { return m_subSpacing; }
    double GetSearchAreaRatio() { return m_searchAreaRatio; }
    double GetWindowSize() { return m_windowSize; }
    double GetSigma() { return m_sigma; }

    int GetSubsPerRow() { return m_sub_rows; }
    int GetSubsPerCol() { return m_sub_cols; }
    int GetNumSubs() { return m_num_subs; }

    CDICSubset& GetSubset(int i) { return m_subsets[i]; }
    std::vector<CDICSubset>& GetSubsets() { return m_subsets; }

    CImageSITK* GetSubsetImage(int i) { return m_subsets[i].GetSubsetImage(); }

    int GetWidth();
    int GetHeight();

private:
    void MakeSubsets();

private:
    CImageSITK* m_image;


    double m_windowSize; // used for gaussian filter
	double m_sigma; // used for gaussian filter
	// double m_corrImg_width;
	// double m_corrImg_height;
	int m_subSize; // size in pixels
	int m_subSpacing; // size in pixels. Currently not working
	double m_searchAreaRatio; // used in template matching. creates a mask around the inital location to prevent searching the whole image
	int m_sub_rows;
	int m_sub_cols;
	int m_num_subs;

    std::vector<CDICSubset> m_subsets;
};

// #endif