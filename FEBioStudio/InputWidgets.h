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
#include <QHBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QValidator>
#include <FSCore/math3d.h>

//-----------------------------------------------------------------------------
class CIntInput : public QLineEdit
{
	Q_OBJECT

public:
	CIntInput(QWidget* parent = 0) : QLineEdit(parent)
	{
		setValidator(new QIntValidator);
		QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	}

	void setValue(int m) { setText(QString("%1").arg(m)); }
	int value() const { return text().toInt(); }

public slots:
	void onEditingFinished()
	{
		emit valueChanged(text().toInt());
	}

signals:
	void valueChanged(int newValue);
};

//-----------------------------------------------------------------------------
class CFloatInput : public QLineEdit
{
	Q_OBJECT

public:
	CFloatInput(QWidget* parent = 0) : QLineEdit(parent)
	{
		QDoubleValidator* pv = new QDoubleValidator;
		setValidator(pv);

		QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	}

	void setValue(double v) { setText(QString("%1").arg(v)); }
	double value() const { return text().toDouble(); }

public slots:
	void onEditingFinished()
	{
		emit valueChanged(text().toDouble());
	}

signals:
	void valueChanged(double newValue);
};

//-----------------------------------------------------------------------------
class CVec3Input : public QLineEdit
{
public:
	CVec3Input(QWidget* parent = 0);

	void setValue(const vec3f& v);

	vec3f value() const;
};

QString vecToString(const vec3f& f);

vec3f stringToVec(const QString& s);

//-----------------------------------------------------------------------------
class CIntSlider : public QWidget
{

	Q_OBJECT

public:
	CIntSlider(QWidget* parent = nullptr) : QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout;
		layout->setContentsMargins(0,0,0,0);

		box = new QSpinBox(parent);
		slider = new QSlider(parent);
		slider->setOrientation(Qt::Horizontal);

		layout->addWidget(box);
		layout->addWidget(slider);

		setLayout(layout);

		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

		connect(box, &QSpinBox::valueChanged, slider, &QSlider::setValue);
		connect(slider, &QSlider::valueChanged, box, &QSpinBox::setValue);
		
		connect(box, &QSpinBox::valueChanged, this, &CIntSlider::valueChanged);
	}

    void passEvent(QEvent* event)
    {
        slider->event(event);
    }

	void setRange(int min, int max)
	{
		box->setRange(min, max);
		slider->setRange(min, max);
	}

	void setValue(int val)
	{
		box->setValue(val);
		slider->setValue(val);
	}

	int getValue()
	{
		return slider->value();
	}

    int maximum()
    {
        return slider->maximum();
    }

    int minimum()
    {
        return slider->minimum();
    }

signals:

	void valueChanged(int val);

private:
	QSpinBox* box;
	QSlider* slider;

};

//-----------------------------------------------------------------------------
class CDoubleSlider : public QWidget
{
	Q_OBJECT

public:
	CDoubleSlider(QWidget* parent = nullptr)
        : QWidget(parent), m_min(0), m_max(1), m_steps(100)
	{
		QHBoxLayout* layout = new QHBoxLayout;
		layout->setContentsMargins(0,0,0,0);

		box = new QDoubleSpinBox(parent);
        box->setRange(m_min, m_max);
        box->setSingleStep((m_max - m_min)/m_steps);

		slider = new QSlider(parent);
		slider->setOrientation(Qt::Horizontal);
		slider->setRange(0,m_steps);

		layout->addWidget(box);
		layout->addWidget(slider);

		setLayout(layout);

		connect(box, &QDoubleSpinBox::valueChanged, this, &CDoubleSlider::boxValueChanged);
		connect(slider, &QSlider::valueChanged, this, &CDoubleSlider::sliderValueChanged);
	}

	void setRange(double min, double max)
	{
        m_min = min;
        m_max = max;

		box->setRange(min, max);
		box->setSingleStep((max - min)/m_steps);
	}

	void setValue(double val)
	{
		if(val > m_max) val = m_max;
		else if(val < m_min) val = m_min;

		box->setValue(val);

		slider->setValue(static_cast<int>(m_steps * (val-m_min)/(m_max-m_min)));
	}

	void setSingleStep(double stepSize)
	{
		if (stepSize <= 0) return;
		m_steps = (m_max - m_min) / stepSize;
		if (m_steps < 1) m_steps = 1;
		if (m_steps > 100) m_steps = 100;
		double step = (m_max - m_min) / m_steps;
		box->setSingleStep(step);
		slider->setRange(0, m_steps);
	}

	double getValue()
	{
		return box->value();
	}

signals:

	void valueChanged(double val);

private slots:
    void boxValueChanged(double val)
    {
        slider->blockSignals(true);
        slider->setValue(m_steps * (val-m_min)/(m_max-m_min));
        slider->blockSignals(false);

        emit valueChanged(val);
    }

    void sliderValueChanged(int val)
    {
        double realVal = (double)val/ m_steps *(m_max-m_min) + m_min;

        box->blockSignals(true);
        box->setValue(realVal);
        box->blockSignals(false);

        emit valueChanged(realVal); 
    }

private:
	QDoubleSpinBox* box;
	QSlider* slider;

	double m_min, m_max;
	int		m_steps;

};