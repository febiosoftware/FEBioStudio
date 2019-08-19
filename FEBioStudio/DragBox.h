#pragma once
#include <QDoubleSpinBox>

class CDragBox : public QDoubleSpinBox
{
public:
	CDragBox(QWidget* parent = 0);

	void SetSingleStep(double v);

private:
	QPoint	m_p0;
};
