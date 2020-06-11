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

#include "stdafx.h"
#include "DragBox.h"
#include <QMouseEvent>
#include <QLineEdit>

class MyLineEdit : public QLineEdit
{
public:
	MyLineEdit(QWidget* parent) : QLineEdit(parent)
	{
		m_step = 1.0;
	}

	void setSingleStep(double v) { m_step = v; }

	void mousePressEvent(QMouseEvent* ev)
	{
		m_p0 = ev->pos();
		QLineEdit::mousePressEvent(ev);
	}

	void mouseMoveEvent(QMouseEvent* ev) 
	{
		ev->accept();
		double val = text().toDouble();

		if (ev->buttons() & Qt::LeftButton)
		{
			const QValidator* v = validator();
			if (v == 0) return;

			QPoint p1 = ev->pos();
			if (p1.x() > m_p0.x())
				val += m_step;
			else if (p1.x() < m_p0.x())
				val -= m_step;
			else return;

			QString newText = QString("%1").arg(val);
			int npos = 0;
			if (v->validate(newText, npos) != QValidator::Invalid)
				setText(newText);
			m_p0 = p1;
		}
	}

private:
	QPoint	m_p0;
	double	m_step;
};

CDragBox::CDragBox(QWidget* parent) : QDoubleSpinBox(parent)
{
	setLineEdit(new MyLineEdit(this));
	setDecimals(4);
}

void CDragBox::SetSingleStep(double v)
{
	setSingleStep(v);
	MyLineEdit* edit = dynamic_cast<MyLineEdit*>(lineEdit());
	if (edit)
	{
		edit->setSingleStep(v);
	}
}
