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
#include "LinkPropertyEdit.h"
#include <QDesktopServices>
#include <QLineEdit>
#include <QToolButton>
#include <QBoxLayout>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QUrl>
#include <FSCore/FSDir.h>
#include "MainWindow.h"

class Ui::CLinkPropertyEdit
{
public:
	QLineEdit*		m_relativePathEdit;
	QToolButton*	m_open;

	QString fullPath;
	QString relativePath;
	bool internal;

public:
	CLinkPropertyEdit(QStringList& paths, bool internal)
		: internal(internal)
	{
		fullPath = paths[0];
		relativePath = paths[1];
	}

	void setup(QWidget* w)
	{
		m_relativePathEdit = new QLineEdit(relativePath);
		m_relativePathEdit->setDisabled(true);
		m_relativePathEdit->setToolTip(fullPath);

		m_open = new QToolButton;
		m_open->setIcon(QIcon(":/icons/open.png"));

		if(internal) m_open->setToolTip("Open file");
		else m_open->setToolTip("Open in external editor");

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(m_relativePathEdit);
		h->addWidget(m_open);
		h->setSpacing(0);
		h->setContentsMargins(0,0,0,0);
		w->setLayout(h);

		QObject::connect(m_open, SIGNAL(clicked(bool)), w, SLOT(buttonPressed()));
	}
};


//=================================================================================================

CLinkPropertyEdit::CLinkPropertyEdit(QStringList& paths, bool internal, QWidget* parent)
	: QWidget(parent), ui(new Ui::CLinkPropertyEdit(paths, internal))
{
	ui->setup(this);
}

void CLinkPropertyEdit::buttonPressed()
{
	QString filename = ui->fullPath;
	QFileInfo info = QFileInfo(ui->fullPath);

	if(!info.exists())
	{
		// for plot and log files we'll check for remote versions
		QString ext = info.suffix();
		if ((ext == "xplt") || (ext == "log"))
		{
			QString remoteFile = QString("%1.remote").arg(filename);
			QFileInfo info2(remoteFile);
			if (!info2.exists())
			{
				// no luck either
				QMessageBox::critical(this, "FEBio Studio", QString("Cannot open file.\n%1 does not exist.").arg(filename));
				return;
			}
			filename = remoteFile;
		}
		else
		{
			QMessageBox::critical(this, "FEBio Studio", QString("Cannot open file.\n%1 does not exist.").arg(filename));
			return;
		}
	}

	if(ui->internal)
	{
		CMainWindow* wnd = dynamic_cast<CMainWindow*>(QApplication::activeWindow()); assert(wnd);
		wnd->OpenFile(filename, false);
	}
	else
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
	}
}
