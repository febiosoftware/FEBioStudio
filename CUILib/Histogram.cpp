#include "stdafx.h"
#include "HistogramViewer.h"
#include <PostLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include <QBoxLayout>
#include <QCheckBox>
#include <QtCharts/QLineSeries>
using namespace Post;

CHistogramViewer::CHistogramViewer(QWidget* parent) : QWidget(parent)
{
	m_logMode = false;
	m_img = nullptr;

	QLineSeries* series = new QLineSeries();
	for (int i=0; i<256; ++i)
		series->append(i, 0);

	QChart* chart = new QChart;
	chart->addSeries(series);
	chart->createDefaultAxes();
	chart->legend()->hide();

	m_chart = new QChartView;

	QCheckBox* cb = new QCheckBox("Logarithmic");

	QVBoxLayout* v = new QVBoxLayout;
	v->addWidget(cb);
	v->addWidget(m_chart);

	setLayout(v);

	m_chart->setChart(chart);
	m_chart->setRenderHint(QPainter::Antialiasing);

	QObject::connect(cb, SIGNAL(clicked(bool)), this, SLOT(SetLogMode(bool)));
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
	m_img = img;
	Update();
}

void CHistogramViewer::Update()
{
	if (m_img == nullptr) return;

	C3DImage* im = m_img->GetImageSource()->Get3DImage();

	vector<double> h(256, 0.0);
	for (int k = 0; k < im->Depth(); ++k)
	{
		for (int j = 0; j < im->Height(); ++j)
			for (int i = 0; i < im->Width(); ++i)
			{
				int n = im->value(i, j, k);
				h[n]++;
			}
	}

	if (m_logMode)
	{
		for (int i = 0; i < 256; ++i) h[i] = log(h[i] + 1.0);
	}

	double N = im->Depth()*im->Width()*im->Height();

	QLineSeries* series = new QLineSeries();
	for (int i = 0; i<256; ++i)
		series->append(i, h[i] / N);

	QChart* chart = new QChart;
	chart->addSeries(series);
	chart->createDefaultAxes();
	chart->legend()->hide();

	m_chart->setChart(chart);
}
