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
#include "DlgPickColor.h"
#include "MainWindow.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QColorDialog>

class UIDlgPickColor
{
public:
	QColorDialog* colordlg;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		colordlg = new QColorDialog;
		colordlg->setWindowFlag(Qt::Widget);
		colordlg->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);
		l->addWidget(colordlg);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		l->addWidget(bb);
		dlg->setLayout(l);

		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::close);
	}
};

CDlgPickColor::CDlgPickColor(CMainWindow* wnd) : QDialog(wnd), ui(new UIDlgPickColor)
{
	setWindowTitle("Color Picker");
	ui->setup(this);
}

CDlgPickColor::~CDlgPickColor()
{

}

QColor CDlgPickColor::GetColor() const
{
	return ui->colordlg->currentColor();
}
