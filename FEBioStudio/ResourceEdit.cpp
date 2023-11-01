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
#include "ResourceEdit.h"
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QFileDialog>

class Ui::CResourceEdit
{
public:
	QLineEdit*		m_name;
	QPushButton*	m_pick;

	QStringList		m_resFlt;

	::CResourceEdit::ResourceType m_resType = ::CResourceEdit::FILE_RESOURCE;

public:
	void setup(QWidget* w)
	{
		m_name = new QLineEdit;
		m_pick = new QPushButton("...");
		m_pick->setFixedWidth(20);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(m_name);
		h->addWidget(m_pick);
		h->setSpacing(0);
		h->setContentsMargins(0,0,0,0);
		w->setLayout(h);

		m_resFlt << "All files (*)";

		QObject::connect(m_name, SIGNAL(editingFinished()), w, SLOT(nameChanged()));
		QObject::connect(m_pick, SIGNAL(clicked(bool)), w, SLOT(buttonPressed()));
	}
};


//=================================================================================================

CResourceEdit::CResourceEdit(QWidget* parent) : QWidget(parent), ui(new Ui::CResourceEdit)
{
	ui->setup(this);
}

void CResourceEdit::setResourceFilter(const QStringList& flt)
{
	ui->m_resFlt = flt;
}

void CResourceEdit::setResourceType(ResourceType resourceType)
{
	ui->m_resType = resourceType;
}

QString CResourceEdit::resourceName() const
{
	return ui->m_name->text();
}

void CResourceEdit::setResourceName(const QString& t)
{
	ui->m_name->setText(t);
}

void CResourceEdit::nameChanged()
{
	emit resourceChanged();
}

void CResourceEdit::buttonPressed()
{
	if (ui->m_resType == FILE_RESOURCE)
	{
		QFileDialog dlg(this);
		dlg.setFileMode(QFileDialog::ExistingFile);
		dlg.setNameFilters(ui->m_resFlt);
		if (dlg.exec())
		{
			QStringList files = dlg.selectedFiles();
			if (files.size() == 1)
			{
				QString fileName = files.at(0);
				ui->m_name->setText(fileName);
				emit resourceChanged();
			}
		}
	}
	else if (ui->m_resType == FOLDER_RESOURCE)
	{
		QString folderName = QFileDialog::getExistingDirectory(this);
		if (folderName.isEmpty() == false)
		{
			ui->m_name->setText(folderName);
			emit resourceChanged();
		}
	}
}
