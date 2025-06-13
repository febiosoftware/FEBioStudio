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
#include "PythonTool.h"
#include <FEBioStudio/PropertyListForm.h>
#include <FEBioStudio/MainWindow.h>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "PythonRunner.h"
#include "PyRunContext.h"

CPythonTool::CPythonTool(CMainWindow* wnd, const QString& name) : CAbstractTool(wnd, name)
{
	m_props = nullptr;
	form = nullptr;
	label = nullptr;
}

CPythonTool::~CPythonTool()
{
	delete m_props;
}

void CPythonTool::SetProperties(CCachedPropertyList* props)
{
	if (form) form->setPropertyList(props);
	delete m_props;
	m_props = props;
}

void CPythonTool::SetToolInfo(const QString& info)
{
	CAbstractTool::SetInfo(info);
	if (label) label->setText(info);
}

CCachedPropertyList* CPythonTool::GetProperties()
{
	return m_props;
}

QString CPythonTool::GetFilePath()
{
	return m_fileName;
}

void CPythonTool::SetFilePath(const QString& filepath)
{
	m_fileName = filepath;
}

QWidget* CPythonTool::createUi()
{
	QWidget* w = new QWidget;
	QVBoxLayout* l = new QVBoxLayout();
	QString info = GetInfo();
	label = new QLabel(info);
	l->addWidget(label);
	form = new CPropertyListForm;
	form->setPropertyList(m_props);
	l->addWidget(form);
	QPushButton* pb = new QPushButton("Run");
	l->addWidget(pb);
	l->addStretch();
	w->setLayout(l);

	CPythonRunner* pyrun = CPythonRunner::GetInstance();

	connect(pb, &QPushButton::clicked, this, &CPythonTool::onRun);
	connect(this, &CPythonTool::runTool, pyrun, &CPythonRunner::runTool);

	return w;
}

void CPythonTool::onRun()
{
	if (m_props == nullptr) return;
	PyRunContext::Init();
	GetMainWindow()->AddPythonLogEntry(QString(">>> Running tool %1 ...\n").arg(name()));
	emit runTool(m_props);
}
