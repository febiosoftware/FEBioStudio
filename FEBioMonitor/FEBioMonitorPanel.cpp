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
#include "FEBioMonitorPanel.h"
#include "../FEBioStudio/MaterialPanel.h"
#include "../FEBioStudio/PostDataPanel.h"
#include "../FEBioStudio/PostModelPanel.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTabWidget>

class CFEBioMonitorPanel::Ui
{
public:
	CMainWindow* wnd;

	QTabWidget* tab;
	CMaterialPanel* matPanel;
	CPostDataPanel* dataPanel;
	CPostModelPanel* modelPanel;

public:
	void setup(CFEBioMonitorPanel* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);

		tab = new QTabWidget;
		tab->addTab(modelPanel = new CPostModelPanel(wnd), "Model");
		tab->addTab(matPanel   = new CMaterialPanel (wnd), "Materials");
		tab->addTab(dataPanel  = new CPostDataPanel (wnd), "Data");

		l->addWidget(tab);
		w->setLayout(l);
	}
};

CFEBioMonitorPanel::CFEBioMonitorPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new CFEBioMonitorPanel::Ui)
{
	ui->wnd = wnd;
	ui->setup(this);
}

void CFEBioMonitorPanel::Update(bool breset)
{
	ui->modelPanel->Update(breset);
	ui->matPanel  ->Update(breset);
	ui->dataPanel ->Update(breset);
}
