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
#include "FEBioAppView.h"
#include "../FEBioStudio/MainWindow.h"
#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "FEBioAppUIBuilder.h"
#include "FEBioAppDocument.h"

FEBioAppView::FEBioAppView(CMainWindow* wnd) : QWidget(wnd)
{
	m_wnd = wnd;
	QVBoxLayout* l = new QVBoxLayout;
	ui = new QLabel("Hello, world!");
	l->addWidget(ui);
	setLayout(l);
}

void FEBioAppView::setSource(QString filePath, FEBioAppDocument* app)
{
	delete ui;
	FEBioAppUIBuilder UIBuilder;
	ui = UIBuilder.BuildUIFromFile(filePath, app);
	if (ui == nullptr) ui = new QLabel("FAILED TO READ FILE!!");
	layout()->addWidget(ui);
}

void FEBioAppView::onModelStarted()
{
	m_wnd->setWindowTitle("[RUNNING]");
}

void FEBioAppView::onModelFinished(bool returnCode)
{
	if (returnCode)
	{
		QMessageBox::information(m_wnd, "FEBio App", "FEBio completed successfully!");
	}
	else
	{
		QMessageBox::critical(m_wnd, "FEBio App", "FEBio failed!");
	}
	repaint();
	m_wnd->UpdateTitle();
}
