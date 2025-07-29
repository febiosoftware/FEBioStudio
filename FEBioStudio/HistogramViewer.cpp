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

#include "stdafx.h"
#include "HistogramViewer.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include <QBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <CUILib/PlotWidget.h>
#include <limits>

using namespace Post;

CHistogramViewer::CHistogramViewer(QWidget* parent) : QWidget(parent)
{
	m_logMode = false;
	m_img = nullptr;

	m_chart = new CPlotWidget;
	m_chart->showLegend(false);
	m_chart->setChartStyle(BARCHART_PLOT);

	QHBoxLayout* topLayout = new QHBoxLayout;
	topLayout->setContentsMargins(0, 0, 0, 0);

	QCheckBox* cb = new QCheckBox("Logarithmic");
	topLayout->addWidget(cb);
	topLayout->addStretch();

	topLayout->addWidget(new QLabel("Bins:"));

	m_bins = new QSpinBox;
	m_bins->setRange(2, 65536);
	m_bins->setValue(256);

	topLayout->addWidget(m_bins);

	QVBoxLayout* v = new QVBoxLayout;
	v->addLayout(topLayout);
	v->addWidget(m_chart);

	setLayout(v);

	connect(m_bins, &QSpinBox::editingFinished, this, &CHistogramViewer::Update);
	connect(cb, &QCheckBox::clicked, this, &CHistogramViewer::SetLogMode);
}

void CHistogramViewer::SetLogMode(bool b)
{
	if (b != m_logMode)
	{
		m_logMode = b;
		Update();
	}
}


void CHistogramViewer::SetImageModel(CImageModel* img)
{
	if (m_img != img)
	{
		m_img = img;
		Update();
	}
}

template<class pType> void GetValues(C3DImage* im, histogram& values)
{
	pType* data = (pType*)im->GetBytes();

	size_t N = im->Width() * im->Height() * im->Depth();
	if (im->PixelType() == CImage::INT_RGB8 || im->PixelType() == CImage::UINT_RGB8
		|| im->PixelType() == CImage::INT_RGB16 || im->PixelType() == CImage::UINT_RGB16)
	{
		N *= 3;
	}

	double min, max;
	im->GetMinMax(min, max);
	double range = max - min;
	if (range == 0) return;

	int bins = (int)values.size();
	for (int i = 0; i < bins; i++)
	{
		values[i].first = (range * i) / (bins - 1) + min;
	}

	int nx = im->Width();
	int ny = im->Height();
	int nz = im->Depth();

#pragma omp parallel firstprivate(data)
	{
		std::vector<uint64_t> ytmp(bins, 0);

#pragma omp for
		for (int i = 0; i < N; ++i)
		{
			int n = (data[i] - min) / range * (bins - 1);
			ytmp[n]++;
		}

#pragma omp critical
		{
			for (size_t n = 0; n < bins; ++n)
				values[n].second += ytmp[n];
		}
	}
}

void CHistogramViewer::CalculateHistogram(histogram& values)
{
	C3DImage* im = m_img->Get3DImage();
	if (im == nullptr) return;

	switch (im->PixelType())
	{
	case CImage::UINT_8    : GetValues<uint8_t> (im, values); break;
	case CImage::INT_8     : GetValues<int8_t>  (im, values); break;
	case CImage::UINT_16   : GetValues<uint16_t>(im, values); break;
	case CImage::INT_16    : GetValues<int16_t> (im, values); break;
	case CImage::UINT_32   : GetValues<uint32_t>(im, values); break;
	case CImage::INT_32    : GetValues<int32_t> (im, values); break;
	case CImage::UINT_RGB8 : GetValues<uint8_t> (im, values); break;
	case CImage::INT_RGB8  : GetValues<int8_t>  (im, values); break;
	case CImage::UINT_RGB16: GetValues<uint16_t>(im, values); break;
	case CImage::INT_RGB16 : GetValues<int16_t> (im, values); break;
	case CImage::REAL_32   : GetValues<float>   (im, values); break;
	case CImage::REAL_64   : GetValues<double>  (im, values); break;
	default:
		assert(false);
	}
}

void CHistogramViewer::Update()
{
	if (m_img == nullptr) return;

	C3DImage* im = m_img->Get3DImage();
	if (im == nullptr) return;

	int bins = m_bins->value();

	if (bins < 2)
	{
		bins = 2;
		m_bins->setValue(2);
	}

	histogram values;
	values.resize(bins);
	CalculateHistogram(values);

	double N = im->Depth() * im->Width() * im->Height();
	if (im->PixelType() == CImage::INT_RGB8 || im->PixelType() == CImage::UINT_RGB8
		|| im->PixelType() == CImage::INT_RGB16 || im->PixelType() == CImage::UINT_RGB16)
	{
		N *= 3;
	}

	if (m_logMode)
	{
		for (int i = 0; i < bins; ++i)
		{
			double y = values[i].second;
			values[i].second = log10(y + 1.0);
		}
	}

	m_chart->clear();

	CPlotData* data = new CPlotData;
	for (int i = 0; i < bins; ++i)
		data->addPoint(values[i].first, values[i].second);

	m_chart->addPlotData(data);

	// Find some good bounds for the graph
	// Sometimes theres a single bin that's significantly bigger than the others.
	// To aviod squashing the rest of the data, we find the highest data point that's
	// within 3 std devs of the mean, and then base the vertical range on that. 
	double avg = 0;
	for (auto& val : values)
	{
		avg += val.second;
	}
	avg /= bins;

	double stdDev = 0;
	for (auto& val : values)
	{
		double y = val.second;
		stdDev += (y - avg) * (y - avg);
	}
	stdDev = sqrt(stdDev / bins);

	double newMax = 0;
	double realMax = 0;
	for (auto& val : values)
	{
		double y = val.second;
		if (y > realMax) realMax = y;
		if (y > (avg + stdDev * 3))
		{
			continue;
		}

		if (y > newMax)
		{
			newMax = y;
		}
	}
	if (newMax == 0) newMax = realMax;

	double xmin = values[0].first;
	double xmax = values[bins - 1].first;
	double range = (xmax - xmin);
	QRectF rect(xmin - range * 0.05, 0, range * 1.1, newMax * 1.3);
	m_chart->setViewRect(rect);

	m_chart->update();
}
