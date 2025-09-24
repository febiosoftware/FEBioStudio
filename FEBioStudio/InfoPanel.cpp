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

#include "stdafx.h"
#include "InfoPanel.h"
#include <QBoxLayout>
#include <QPlainTextEdit>
#include <QLabel>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <FSCore/FSObject.h>
#include "MainWindow.h"
#include "GLDocument.h"

class Ui::CInfoPanel
{
public:
	::CMainWindow*	m_wnd;
	FSObject*	po;
	QLabel*		name;
	QPlainTextEdit*	edit;

public:
	void setup(QWidget* w)
	{
		QToolButton* b1 = new QToolButton; b1->setIcon(QIcon(":/icons/save.png")); b1->setAutoRaise(true); b1->setObjectName("infoSave"); b1->setToolTip("<font color=\"black\">Save Text");
		QToolButton* b2 = new QToolButton; b2->setIcon(QIcon(":/icons/clear.png")); b2->setAutoRaise(true); b2->setObjectName("infoClear"); b2->setToolTip("<font color=\"black\">Clear Text");

		name = new QLabel;
		name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(b1);
		h->addWidget(b2);
		h->addWidget(name);
		h->addStretch();
		h->setContentsMargins(0,0,0,0);
		h->setSpacing(0);

		edit = new QPlainTextEdit;
		edit->setPlaceholderText("(Enter notes here)");
		edit->setDisabled(true);
		edit->setObjectName("edit");
		edit->setFont(QFont("Courier", 12));

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(h);
		v->addWidget(edit);
		w->setLayout(v);
		v->setContentsMargins(0,0,0,0);
		v->setSpacing(0);

		QMetaObject::connectSlotsByName(w);
	}
};

CInfoPanel::CInfoPanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new Ui::CInfoPanel)
{
	ui->m_wnd = wnd;
	ui->setup(this);
	SetObject(0);
}

void CInfoPanel::SetObject(FSObject* po)
{
	if (po == nullptr)
	{
		ui->po = po;
		ui->edit->clear();
		ui->edit->setDisabled(true);
		ui->name->setText("(select an item in the model tree to add notes)");
	}
	else
	{
		if (ui->edit->isEnabled() == false) ui->edit->setEnabled(true);
		ui->po = po;

		QString s = QString::fromStdString(po->GetName());
		std::string type = CGLDocument::GetTypeString(po);
		QString t = QString::fromStdString(type);

		s = "Notes for: <b>" + s + " <i>(" + t + ")</i></b>";
		ui->name->setText(s);
		ui->edit->setPlainText(QString::fromStdString(po->GetInfo()));
	}
}

void CInfoPanel::on_edit_textChanged()
{
	if (ui->po)
	{
		QString txt = ui->edit->toPlainText();
		std::string s = txt.toStdString();
		ui->po->SetInfo(s);
		ui->m_wnd->UpdateModel(ui->po, false);
	}
}

void CInfoPanel::on_infoSave_clicked(bool b)
{
	QString txt = ui->edit->toPlainText();

	QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "Text Files (*.txt)");
	if (fileName.isEmpty() == false)
	{
		// convert to const char*
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();

		// open the file
		FILE* fp = fopen(szfile, "wb");
		if (fp == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed saving log");
			return;
		}

		// convert data to string
		std::string s = txt.toStdString();
		size_t len = s.length();
		size_t nwritten = fwrite(s.c_str(), sizeof(char), len, fp);

		// close the file
		fclose(fp);
	}
}

void CInfoPanel::on_infoClear_clicked(bool b)
{
	ui->edit->clear();
}
