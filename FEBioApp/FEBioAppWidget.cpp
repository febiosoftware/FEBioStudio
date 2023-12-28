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
#include "FEBioAppWidget.h"
#include "FEBioAppDocument.h"
#include "../FEBioStudio/MainWindow.h"
#include <QMessageBox>

FEBioAppWidget::FEBioAppWidget(FEBioAppDocument* doc) : m_doc(doc)
{
	QObject::connect(doc, SIGNAL(dataChanged()), this, SLOT(onDataChanged()));
	QObject::connect(doc, SIGNAL(modelFinished(bool)), this, SLOT(onModelFinished(bool)));
	QObject::connect(doc, SIGNAL(modelStarted()), this, SLOT(onModelStarted()));
}

void FEBioAppWidget::onDataChanged()
{
	for (auto w : m_children) w->repaint();
}

void FEBioAppWidget::onModelStarted()
{
	QString fileName = QString::fromStdString(m_doc->GetDocFileName());
	window()->setWindowTitle(QString("[RUNNING %1]").arg(fileName));
}

void FEBioAppWidget::onModelFinished(bool returnCode)
{
	if (returnCode)
	{
		QMessageBox::information(this, "FEBio App", "FEBio completed successfully!");
	}
	else
	{
		QMessageBox::critical(this, "FEBio App", "FEBio failed!");
	}
	repaint();
	CMainWindow* wnd = dynamic_cast<CMainWindow*>(window());
	if (wnd) wnd->UpdateTitle();
}

void FEBioAppWidget::AddRepaintChild(QWidget* w)
{
	m_children.push_back(w);
}

CFEBioParamEdit::CFEBioParamEdit(QObject* parent) : m_editor(nullptr), QObject(parent)
{

}

void CFEBioParamEdit::SetEditor(CFloatInput* w)
{
	assert(w);
	m_editor = w;
	assert(m_param.isValid() && (m_param.type() == FE_PARAM_DOUBLE));
	w->setValue(m_param.value<double>());
	QObject::connect(w, SIGNAL(valueChanged(double)), this, SLOT(UpdateFloat(double)));
}

void CFEBioParamEdit::SetEditor(CIntInput* w)
{
	assert(w);
	m_editor = w;
	assert(m_param.isValid() && (m_param.type() == FE_PARAM_INT));
	w->setValue(m_param.value<int>());
	QObject::connect(w, SIGNAL(valueChanged(int)), this, SLOT(UpdateInt(int)));
}

void CFEBioParamEdit::SetEditor(CDoubleSlider* w)
{
	assert(w);
	m_editor = w;
	assert(m_param.isValid() && (m_param.type() == FE_PARAM_DOUBLE));
	w->setValue(m_param.value<double>());
	QObject::connect(w, SIGNAL(valueChanged(double)), this, SLOT(UpdateFloat(double)));
}

void CFEBioParamEdit::SetEditor(QCheckBox* w)
{
	assert(w);
	m_editor = w;
	assert(m_param.isValid() && (m_param.type() == FE_PARAM_BOOL));
	w->setChecked(m_param.value<bool>());
	QObject::connect(w, SIGNAL(toggled(bool)), this, SLOT(UpdateBool(bool)));
}

void CFEBioParamEdit::UpdateFloat(double newValue)
{
	assert(m_param.isValid() && (m_param.type() == FE_PARAM_DOUBLE));
	m_param.value<double>() = newValue;
}

void CFEBioParamEdit::UpdateInt(int newValue)
{
	assert(m_param.isValid() && (m_param.type() == FE_PARAM_INT));
	m_param.value<int>() = newValue;
}

void CFEBioParamEdit::UpdateBool(bool newValue)
{
	assert(m_param.isValid() && (m_param.type() == FE_PARAM_BOOL));
	m_param.value<bool>() = newValue;
}

QWidget* CFEBioParamEdit::GetEditor() const
{
	return m_editor;
}
