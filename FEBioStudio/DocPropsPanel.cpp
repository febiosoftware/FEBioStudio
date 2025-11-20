/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "DocPropsPanel.h"
#include <QBoxLayout>
#include "PropertyListView.h"
#include "MainWindow.h"
#include "Document.h"

class Ui::CDocPropsPanel
{
public:
	CDocument* doc = nullptr;
	::CPropertyListView* props;

public:
	void setup(::CDocPropsPanel* w)
	{
		QVBoxLayout* l = new QVBoxLayout;

		props = new ::CPropertyListView();
		l->addWidget(props);

		w->setLayout(l);
	}
};

CDocPropsPanel::CDocPropsPanel(CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CDocPropsPanel)
{
	ui->setup(this);
}

void CDocPropsPanel::Update(bool breset)
{
	CDocument* doc = GetMainWindow()->GetDocument();
	if ((doc != ui->doc) || breset)
	{
		ui->doc = doc;
		CPropertyList* plOld = ui->props->GetPropertyList();
		CPropertyList* pl = (doc ? doc->GetDocProperties() : nullptr);
		ui->props->Update(pl);
		delete plOld;
	}
}

void CDocPropsPanel::Clear()
{

}
