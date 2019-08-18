#include "stdafx.h"
#include "DlgFormula.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGridLayout>
#include <QValidator>
#include <QMessageBox>
#include <MeshTools/MathParser.h>

CDlgFormula::CDlgFormula(QWidget* parent) : QDialog(parent)
{
	QVBoxLayout* l = new QVBoxLayout;

	m_math = new QLineEdit;

	QGridLayout* g = new QGridLayout;

	g->addWidget(new QLabel("f(t) = "), 0, 0, Qt::AlignRight);
	g->addWidget(m_math, 0, 1, 1, 3);

	g->addWidget(new QLabel("min:"), 1, 0, Qt::AlignRight);

	m_min = new QLineEdit; m_min->setValidator(new QDoubleValidator);
	m_min->setText("0");
	g->addWidget(m_min, 1, 1);

	g->addWidget(new QLabel("max:"), 1, 2, Qt::AlignRight);

	m_max = new QLineEdit; m_max->setValidator(new QDoubleValidator);
	m_max->setText("1");
	g->addWidget(m_max, 1, 3);

	g->addWidget(new QLabel("samples:"), 2, 0, Qt::AlignRight);
	
	m_samples = new QLineEdit; m_samples->setValidator(new QIntValidator(2, 10000));
	m_samples->setText("11");
	g->addWidget(m_samples, 2, 1);

	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	l->addLayout(g);
	l->addWidget(bb);

	setLayout(l);

	QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}

QString CDlgFormula::GetMath()
{
	return m_math->text();
}

void CDlgFormula::SetMath(const QString& math)
{
	m_math->setText(math);
}

double CDlgFormula::GetMin()
{
	return m_min->text().toDouble();
}

double CDlgFormula::GetMax()
{
	return m_max->text().toDouble();
}

int CDlgFormula::GetSamples()
{
	return m_samples->text().toInt();
}

void CDlgFormula::accept()
{
	if (GetMath().isEmpty())
	{
		QMessageBox::critical(this, "", "Please enter an expresion for f(t)");
		return;
	}

	double fmin = GetMin();
	double fmax = GetMax();
	if (fmin >= fmax)
	{
		QMessageBox::critical(this, "", "Please make sure that max is larger than min");
		return;
	}

	QDialog::accept();
}

std::vector<LOADPOINT> CDlgFormula::GetPoints()
{
	QString math = GetMath();
	std::string smath = math.toStdString();

	double fmin = GetMin();
	double fmax = GetMax();
	int samples = GetSamples();

	std::vector<LOADPOINT> pts;
	CMathParser m;
	int ierr;
	LOADPOINT pt;
	for (int i = 0; i<samples; ++i)
	{
		pt.time = fmin + i*(fmax - fmin) / (samples - 1);
		m.set_variable("t", pt.time);

		pt.load = m.eval(smath.c_str(), ierr);

		pts.push_back(pt);

		if (ierr != 0)
		{
			pts.clear();
			break;
		}
	}

	return pts;
}
