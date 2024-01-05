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
#include "DlgMonitorSettings.h"
#include <FEBioMonitor/FEBioMonitorDoc.h>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>

class CDlgMonitorSettings::Ui
{
public:
	QLineEdit* m_fileInput;
	QCheckBox* m_startPaused;
	QComboBox* m_pauseEvents;

public:
	void setup(CDlgMonitorSettings* dlg)
	{
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(new QLabel("FEBio input file:"));
		l->addWidget(m_fileInput = new QLineEdit());
		l->addWidget(m_startPaused = new QCheckBox("Start job in paused state."));

		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Pause event:"));
		h->addWidget(m_pauseEvents = new QComboBox);
		h->addStretch();
		l->addLayout(h);
		l->addStretch();
		l->addWidget(bb);
		dlg->setLayout(l);
		connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
	}
};

CDlgMonitorSettings::CDlgMonitorSettings(QWidget* parent) : QDialog(parent), ui(new CDlgMonitorSettings::Ui)
{
	setMinimumSize(600, 300);
	setWindowTitle("FEBio Monitor Settings");
	ui->setup(this);

	ui->m_pauseEvents->addItem("(All events)", QVariant(0x0FFFFFFF)); // should match CB_ALWAYS!
	for (int i = 0; i < 32; ++i)
	{
		unsigned int n = (1 << i);
		QString eventName = eventToString(n);
		if (eventName.isEmpty() == false)
			ui->m_pauseEvents->addItem(eventName, QVariant(n));
	}
}

void CDlgMonitorSettings::CanEditFilename(bool b)
{
	ui->m_fileInput->setEnabled(b);
}

void CDlgMonitorSettings::SetFEBioInputFile(QString febfile)
{
	ui->m_fileInput->setText(febfile);
}

QString CDlgMonitorSettings::GetFEBioInputFile()
{
	return ui->m_fileInput->text();
}

bool CDlgMonitorSettings::GetStartPausedOption()
{
	return ui->m_startPaused->isChecked();
}

void CDlgMonitorSettings::SetStartPausedOption(bool b)
{
	ui->m_startPaused->setChecked(b);
}

void CDlgMonitorSettings::SetPauseEvents(unsigned int nevents)
{
	int n = ui->m_pauseEvents->findData(nevents);
	ui->m_pauseEvents->setCurrentIndex(n);
}

unsigned int CDlgMonitorSettings::GetPauseEvents()
{
	return (unsigned int)(ui->m_pauseEvents->currentData().toInt());
}
