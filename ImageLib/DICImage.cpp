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

#include "DICImage.h"
#include "ImageSITK.h"

#include <sitkImage.h>

namespace sitk = itk::simple;

CDICSubset::CDICSubset(CDICImage* image, int x0, int y0, int x1, int y1)
    : m_image(image), m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1),
        m_subset_image(x1 - x0, y1 - y0, 1)
{
    m_width = m_x1 - m_x0;
    m_height = m_y1 - m_y0;

    m_center = vec2i(m_x0 + m_width/2, m_y0 + m_height/2);

    Byte* originalBytes = m_image->GetSITKImage()->GetBytes();
    Byte* subsetBytes = m_subset_image.GetBytes();

    int originalWidth = m_image->GetSITKImage()->Width();

    for(int j = 0; j < m_height; j++)
    {
        for(int i = 0; i < m_width; i++)
        {
            subsetBytes[j*m_width+i] = originalBytes[(j+y0)*originalWidth+i+x0];
        }
    }

}

CDICImage::CDICImage(CImageSITK* image, double window, double sigma, int subSize, int subSpacing, double searchAreaRatio)
    : m_image(image), m_windowSize(window), m_sigma(sigma), m_subSize(subSize), m_subSpacing(subSpacing), 
        m_searchAreaRatio(searchAreaRatio)
{
    MakeSubsets();
}

void CDICImage::MakeSubsets()
{
    int M = m_subSize;
	int N = m_subSize;
	int img_w = m_image->Width();
	int img_h = m_image->Height();
	int sub_size = M * N;

	int x1 = 0;
	int y1 = 0;

	int i = 0; //to sum
	int c = 0;

	for (int y = 0; y < img_h; y = y + N)
	{
		for (int x = 0; x < img_w; x = x + M)
		{
			if ((img_h - y) < M || (img_w - x) < N)
			{
				c++;
				break;
			}
			y1 = y + N;
			x1 = x + M;

            CDICSubset subset(this, x, y, x + N, y + M);

			if (subset.Height() * subset.Width() < sub_size) { continue; }

			m_subsets.push_back(subset);

			i++;
		}
	}

	int r = i / c;

	m_num_subs = i;
	m_sub_cols = c - 1;
	m_sub_rows = r + 1;
}

int CDICImage::GetWidth()
{
	return m_image->Width();
}

int CDICImage::GetHeight()
{
	return m_image->Height();
}