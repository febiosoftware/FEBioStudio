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
#include <QStackedWidget>
#include "FEBioAppUIBuilder.h"
#include "FEBioAppDocument.h"

class FEBioAppViewUI
{
public:
	CMainWindow* wnd;
	QStackedWidget* stack;
	QMap<FEBioAppDocument*, QWidget*> widgets;

public:
	void setup(QWidget* parent)
	{
		QVBoxLayout* l = new QVBoxLayout;
		stack = new QStackedWidget;
		stack->addWidget(new QLabel("No UI available"));
		l->addWidget(stack);
		parent->setLayout(l);
	}
};

FEBioAppView::FEBioAppView(CMainWindow* wnd) : QWidget(wnd), ui(new FEBioAppViewUI)
{
	ui->wnd = wnd;
	ui->setup(this);
}

void FEBioAppView::setActiveDocument(FEBioAppDocument* app)
{
	// see if we already have a UI for this document
	QWidget* w = nullptr;
	if (ui->widgets.contains(app))
	{
		w = ui->widgets.value(app);
	}
	else
	{
		// build a new widget
		FEBioAppUIBuilder UIBuilder;
		w = UIBuilder.BuildUIFromFile(QString::fromStdString(app->GetDocFilePath()), app);
		if (w == nullptr) QMessageBox::critical(this, "FEBio Studio", "Failed to read file!");
		else
		{
			ui->widgets[app] = w;
			ui->stack->addWidget(w);
		}
	}

	if (w) ui->stack->setCurrentWidget(w);
	else ui->stack->setCurrentIndex(0);
}

void FEBioAppView::removeDocument(FEBioAppDocument* app)
{
	if (ui->widgets.contains(app))
	{
		QWidget* w = ui->widgets.value(app);
		ui->stack->setCurrentIndex(0);
		ui->stack->removeWidget(w);
		ui->widgets.remove(app);
		delete w;
	}
}
