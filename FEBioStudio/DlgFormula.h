#pragma once
#include <QDialog>
#include <FSCore/LoadCurve.h>

class QLineEdit;

class CDlgFormula : public QDialog
{
	Q_OBJECT

public:
	CDlgFormula(QWidget* parent);

	void SetMath(const QString& math);

	QString GetMath();
	double GetMin();
	double GetMax();
	int GetSamples();

	std::vector<LOADPOINT> GetPoints();

public slots:
	void accept();

private:
	QLineEdit*	m_math;
	QLineEdit*	m_min;
	QLineEdit*	m_max;
	QLineEdit*	m_samples;
};
