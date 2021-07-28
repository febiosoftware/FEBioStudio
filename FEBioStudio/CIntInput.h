/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <QSlider>
#include <QLineEdit>
#include <QValidator>
#include <MathLib/math3d.h>

//-----------------------------------------------------------------------------
class CIntInput : public QLineEdit
{
public:
	CIntInput(QWidget* parent = 0) : QLineEdit(parent)
	{
		setValidator(new QIntValidator);
	}

	void setValue(int m) { setText(QString("%1").arg(m)); }
	int value() const { return text().toInt(); }
};

//-----------------------------------------------------------------------------
class CFloatInput : public QLineEdit
{
public:
	CFloatInput(QWidget* parent = 0) : QLineEdit(parent)
	{
		QDoubleValidator* pv = new QDoubleValidator;
		setValidator(pv);
	}

	void setValue(double v) { setText(QString("%1").arg(v)); }
	double value() const { return text().toDouble(); }
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
	CIntSlider(QWidget* parent) : QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout;
		layout->setContentsMargins(0,0,0,0);

		box = new QSpinBox(parent);
		slider = new QSlider(parent);
		slider->setOrientation(Qt::Horizontal);

		layout->addWidget(box);
		layout->addWidget(slider);

		setLayout(layout);

		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

		QObject::connect(box, &QSpinBox::valueChanged, slider, &QSlider::setValue);
		QObject::connect(slider, &QSlider::valueChanged, box, &QSpinBox::setValue);
		
		QObject::connect(box, &QSpinBox::valueChanged, this, &CIntSlider::valueChanged);
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
		return box->value();
	}

signals:

	void valueChanged(int val);

private:
	QSpinBox* box;
	QSlider* slider;

};