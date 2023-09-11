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
#include <FEBioStudio/PlotWidget.h>
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
    topLayout->setContentsMargins(0,0,0,0);

    QCheckBox* cb = new QCheckBox("Logarithmic");
    topLayout->addWidget(cb);
    topLayout->addStretch();

    topLayout->addWidget(new QLabel("Bins:"));

    m_bins = new QSpinBox;
    m_bins->setMaximum(65536);
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
    if(m_img != img)
    {
        m_img = img;
        Update();
    }
}

template<class pType> void CHistogramViewer::GetValues(int bins, std::vector<double>& xVals, std::vector<uint64_t>& yVals)
{
    auto im = m_img->Get3DImage();

    pType* data = (pType*)im->GetBytes();

    size_t N  = im->Width()*im->Height()*im->Depth();
    if(im->PixelType() == CImage::INT_RGB8 || im->PixelType() == CImage::UINT_RGB8 
        || im->PixelType() == CImage::INT_RGB16 || im->PixelType() == CImage::UINT_RGB16)
    {
        N *= 3;
    }

    double min, max;
    im->GetMinMax(min, max);
    double range = max - min;
    if(range == 0) return;

    for(int i = 0; i < bins; i++)
    {
        xVals[i] = range/bins * i + min;
    }

    int nx = im->Width();
    int ny = im->Height();
    int nz = im->Depth();

    #pragma omp parallel shared(yVals) firstprivate(data)
	{
		std::vector<uint64_t> ytmp(yVals.size(), 0);
        
        #pragma omp for
		for (int i=0; i<N; ++i)
		{
			int n = (data[i] - min) / range * (bins - 1);
			ytmp[n]++;
		}
        
        #pragma omp critical
		{
			for (size_t n = 0; n < ytmp.size(); ++n)
				yVals[n] += ytmp[n];
		}
	}
}


void CHistogramViewer::Update()
{
	if (m_img == nullptr) return;

	C3DImage* im = m_img->Get3DImage();
	if (im == nullptr) return;

    int bins = m_bins->value();

    if(bins <= 0)
    {
        bins = 1;
        m_bins->setValue(1);
    }

    std::vector<double> xVals(bins, 0.0);
    std::vector<uint64_t> yVals(bins, 0);

    switch (im->PixelType())
    {
    case CImage::UINT_8:
        GetValues<uint8_t>(bins, xVals, yVals);
        break;
    case CImage::INT_8:
        GetValues<int8_t>(bins, xVals, yVals);
        break;
    case CImage::UINT_16:
        GetValues<uint16_t>(bins, xVals, yVals);
        break;
    case CImage::INT_16:
        GetValues<int16_t>(bins, xVals, yVals);
        break;
    case CImage::UINT_32:
        GetValues<uint32_t>(bins, xVals, yVals);
        break;
    case CImage::INT_32:
        GetValues<int32_t>(bins, xVals, yVals);
        break;
    case CImage::UINT_RGB8:
        GetValues<uint8_t>(bins, xVals, yVals);
        break;
    case CImage::INT_RGB8:
        GetValues<int8_t>(bins, xVals, yVals);
        break;
    case CImage::UINT_RGB16:
        GetValues<uint16_t>(bins, xVals, yVals);
        break;
    case CImage::INT_RGB16:
        GetValues<int16_t>(bins, xVals, yVals);
        break;
    case CImage::REAL_32:
        GetValues<float>(bins, xVals, yVals);
        break;
    case CImage::REAL_64:
        GetValues<double>(bins, xVals, yVals);
        break;
    default:
        assert(false);
    }

    double N = im->Depth()*im->Width()*im->Height();
    if(im->PixelType() == CImage::INT_RGB8 || im->PixelType() == CImage::UINT_RGB8 
        || im->PixelType() == CImage::INT_RGB16 || im->PixelType() == CImage::UINT_RGB16)
    {
        N *= 3;
    }

    std::vector<double> finalData(bins, 0);
	if (m_logMode)
	{
		for (int i = 0; i < bins; ++i) finalData[i] = log(yVals[i]/N + 1.0);
	}
    else
    {
        for (int i = 0; i < bins; ++i) finalData[i] = yVals[i];
    }

	m_chart->clear();

	CPlotData* data = new CPlotData;
    for (int i = 0; i<bins; ++i)
        data->addPoint(xVals[i], finalData[i]);

	m_chart->addPlotData(data);


    // Find some good bounds for the graph
    // Sometimes theres a single bin that's significantly bigger than the others.
    // To aviod squashing the rest of the data, we find the highest data point that's
    // within 3 std devs of the mean, and then base the vertical range on that. 
    double avg = 0;
    for(auto val : finalData)
    {
        avg += val;
    }
    avg /= N;

    double stdDev = 0;
    for(auto val : finalData)
    {
        stdDev += (val-avg)*(val-avg);
    }
    stdDev = sqrt(stdDev/N);

    double newMax = -1;
    for(auto val : finalData)
    {
        if(val > (avg + stdDev*3))
        {
            continue;
        }

        if(val > newMax)
        {
            newMax = val;
        }
    }

    double range = (xVals[bins-1] - xVals[0]);

    QRectF rect(xVals[0] - range*0.05, 0, range*1.1, newMax*1.3);
    m_chart->setViewRect(rect);

    m_chart->update();
}
