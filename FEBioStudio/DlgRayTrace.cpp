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
#include "DlgRayTrace.h"
#include "MainWindow.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QValidator>
#include <QComboBox>

class CDlgRayTraceUI
{
public:
	CMainWindow* wnd;
	QLineEdit* width;
	QLineEdit* height;
	QComboBox* sample;

	static int lastWidth;
	static int lastHeight;
	static int lastSamples;

public:
	void setup(CDlgRayTrace* dlg)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Width: ", width = new QLineEdit); width->setValidator(new QIntValidator(1, 2048));
		form->addRow("Height: ", height = new QLineEdit); height->setValidator(new QIntValidator(1, 2048));
		form->addRow("Multisample: ", sample = new QComboBox); sample->addItems({ "Off","2", "3", "4" });

		width->setText(QString::number(lastWidth));
		height->setText(QString::number(lastHeight));
		sample->setCurrentIndex(lastSamples - 1);

		QVBoxLayout* l = new QVBoxLayout;
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addLayout(form);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
	}
};

int CDlgRayTraceUI::lastWidth = 256;
int CDlgRayTraceUI::lastHeight = 256;
int CDlgRayTraceUI::lastSamples = 1;

CDlgRayTrace::CDlgRayTrace(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgRayTraceUI)
{
	setWindowTitle("Ray Trace");
	ui->setup(this);
}

int CDlgRayTrace::ImageWidth()
{
	return ui->width->text().toInt();
}

int CDlgRayTrace::ImageHeight()
{
	return ui->height->text().toInt();
}

void CDlgRayTrace::accept()
{
	ui->lastWidth = ImageWidth();
	ui->lastHeight = ImageHeight();
	ui->lastSamples = Multisample();
	QDialog::accept();
}

int CDlgRayTrace::Multisample()
{
	return ui->sample->currentIndex() + 1;
}
