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
#include "InputWidgets.h"

QString vecToString(const vec3f& v)
{
	return QString("%1,%2,%3").arg(v.x).arg(v.y).arg(v.z);
}

vec3f stringToVec(const QString& s)
{
	QStringList l = s.split(',');
	int N = l.size();
	vec3f v(0.f, 0.f, 0.f);
	v.x = (N > 0 ? l[0].toFloat() : 0.f);
	v.y = (N > 1 ? l[1].toFloat() : 0.f);
	v.z = (N > 2 ? l[2].toFloat() : 0.f);

	return v;
}

CVec3Input::CVec3Input(QWidget* parent) : QLineEdit(parent) {}

void CVec3Input::setValue(const vec3f& v)
{
	setText(vecToString(v));
}

vec3f CVec3Input::value() const
{
	return stringToVec(text());
}

CDoubleSlider::CDoubleSlider(QWidget* parent) : QWidget(parent), m_min(0), m_max(1), m_steps(100)
{
	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);

	box = new QDoubleSpinBox(parent);
	box->setRange(m_min, m_max);
	box->setSingleStep((m_max - m_min) / m_steps);
	box->setMinimumWidth(150);

	slider = new QSlider(parent);
	slider->setOrientation(Qt::Horizontal);
	slider->setRange(0, m_steps);

	layout->addWidget(box);
	layout->addWidget(slider);

	setLayout(layout);

	connect(box, &QDoubleSpinBox::valueChanged, this, &CDoubleSlider::boxValueChanged);
	connect(slider, &QSlider::valueChanged, this, &CDoubleSlider::sliderValueChanged);
}

void CDoubleSlider::setRange(double min, double max)
{
	m_min = min;
	m_max = max;

	box->setRange(min, max);
	box->setSingleStep((max - min) / m_steps);
}

void CDoubleSlider::setValue(double val)
{
	if (val > m_max) val = m_max;
	else if (val < m_min) val = m_min;

	box->setValue(val);

	slider->setValue(static_cast<int>(m_steps * (val - m_min) / (m_max - m_min)));
}

void CDoubleSlider::setSingleStep(double stepSize)
{
	if (stepSize <= 0) return;
	m_steps = (m_max - m_min) / stepSize;
	if (m_steps < 1) m_steps = 1;
	if (m_steps > 100) m_steps = 100;
	double step = (m_max - m_min) / m_steps;
	box->setSingleStep(step);
	slider->setRange(0, m_steps);
}

double CDoubleSlider::getValue()
{
	return box->value();
}

void CDoubleSlider::boxValueChanged(double val)
{
	slider->blockSignals(true);
	slider->setValue(m_steps * (val - m_min) / (m_max - m_min));
	slider->blockSignals(false);

	emit valueChanged(val);
}

void CDoubleSlider::sliderValueChanged(int val)
{
	double realVal = (double)val / m_steps * (m_max - m_min) + m_min;

	box->blockSignals(true);
	box->setValue(realVal);
	box->blockSignals(false);

	emit valueChanged(realVal);
}
