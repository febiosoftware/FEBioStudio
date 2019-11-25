#pragma once
#include <QtCharts/QChartView>
using namespace QtCharts;

namespace Post {
	class CImageModel;
}

class CHistogramViewer : public QWidget
{
	Q_OBJECT

public:
	CHistogramViewer(QWidget* parent = nullptr);

	void SetImageModel(Post::CImageModel* img);

public slots:
	void SetLogMode(bool b);

protected:
	void Update();

private:
	Post::CImageModel*	m_img;
	bool	m_logMode;

private:
	QChartView*		m_chart;
};
