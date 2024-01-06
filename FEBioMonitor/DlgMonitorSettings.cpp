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
#include <QMessageBox>

class CDlgMonitorSettings::Ui
{
private:
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

		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Pause event:"));
		h->addWidget(m_pauseEvents = new QComboBox);
		h->addStretch();
		l->addLayout(h);
		l->addWidget(m_startPaused = new QCheckBox("Start job in paused state."));
		l->addStretch();
		l->addWidget(bb);
		dlg->setLayout(l);
		connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
	}

	void EnableFileInput(bool b) { m_fileInput->setEnabled(b); }

	void AddPauseEvent(QString name, int nevent) { m_pauseEvents->addItem(name, QVariant(nevent)); }

	QString GetInputFilename() { return m_fileInput->text(); }
	bool InputIsEnabled() const { return m_fileInput->isEnabled(); }
	unsigned int GetPauseEvents() { return (unsigned int)(m_pauseEvents->currentData().toInt()); }
	bool GetStartPausedOption() { return m_startPaused->isChecked(); }

	void SetFEBioInputFile(QString febfile) { m_fileInput->setText(febfile); }

	void SetStartPausedOption(bool b) { m_startPaused->setChecked(b); }

	void SetPauseEvents(unsigned int nevents) 
	{
		int n = m_pauseEvents->findData(nevents);
		m_pauseEvents->setCurrentIndex(n);
	}
};

CDlgMonitorSettings::CDlgMonitorSettings(FEBioMonitorDoc* doc, QWidget* parent) : QDialog(parent), ui(new CDlgMonitorSettings::Ui), m_doc(doc)
{
	setMinimumSize(600, 300);
	setWindowTitle("FEBio Monitor Settings");
	ui->setup(this);

	ui->AddPauseEvent("(All events)", 0x0FFFFFFF); // should match CB_ALWAYS!
	for (int i = 0; i < 32; ++i)
	{
		unsigned int n = (1 << i);
		QString eventName = eventToString(n);
		if (eventName.isEmpty() == false)
			ui->AddPauseEvent(eventName, n);
	}

	ui->SetFEBioInputFile(doc->GetFEBioInputFile());
	ui->SetStartPausedOption(doc->StartPaused());
	ui->SetPauseEvents(doc->GetPauseEvents());
}

void CDlgMonitorSettings::CanEditFilename(bool b)
{
	ui->EnableFileInput(b);
}

void CDlgMonitorSettings::accept()
{
	assert(m_doc);
	m_doc->StartPaused(ui->GetStartPausedOption());
	m_doc->SetPauseEvents(ui->GetPauseEvents());
	if (ui->InputIsEnabled())
	{
		QString filename = ui->GetInputFilename();
		if (filename.isEmpty())
		{
			QMessageBox::critical(this, "FEBio Monitor", "You need to specify a valid filename.");
			return;
		}
		m_doc->SetFEBioInputFile(ui->GetInputFilename());
	}

	QDialog::accept();
}
