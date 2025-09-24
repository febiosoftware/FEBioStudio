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
#include "FEBioMonitorDoc.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QtCore/QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <CUILib/InputWidgets.h>
#include <FEBioStudio/ResourceEdit.h>

class CDlgMonitorSettings::Ui
{
public:
	QComboBox* m_jobName;
	CResourceEdit* m_workingDir;
	QCheckBox* m_startPaused;
	QComboBox* m_pauseEvents;
	QCheckBox* m_enablePauseTime;
	QCheckBox* m_variableNorms;
	QCheckBox* m_generateReport;
	CFloatInput* m_pauseTime;
	QComboBox* m_debugMode;
	QCheckBox* m_recordStates;
	QComboBox* m_updateEvents;
	QGroupBox* jobSettings;

public:
	int m_fileMode = 0;

public:
	void setup(CDlgMonitorSettings* dlg)
	{
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		QVBoxLayout* l = new QVBoxLayout;

		jobSettings = new QGroupBox("Job Settings:");
		QFormLayout* form = new QFormLayout();
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Job name:", m_jobName = new QComboBox()); m_jobName->setEditable(true);
		form->addRow("Working directory:", m_workingDir = new CResourceEdit);
		jobSettings->setLayout(form);
		l->addWidget(jobSettings);

		QGroupBox* b = new QGroupBox("Monitor Settings:");

		QVBoxLayout* settingsLayout = new QVBoxLayout;
		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Pause event:"));
		h->addWidget(m_pauseEvents = new QComboBox);
		h->addStretch();
		settingsLayout->addLayout(h);
		settingsLayout->addWidget(m_startPaused = new QCheckBox("Start job in paused state."));

		h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(m_enablePauseTime = new QCheckBox("Pause after time: "));
		h->addWidget(m_pauseTime = new CFloatInput); m_pauseTime->setValue(0.0);
		h->addStretch();
		settingsLayout->addLayout(h);

		h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Debug level:"));
		h->addWidget(m_debugMode = new QComboBox);
		h->addStretch();
		settingsLayout->addLayout(h);
		m_debugMode->addItems(QStringList() << "0 (off)" << "1" << "2");

		m_recordStates = new QCheckBox("record states");
		settingsLayout->addWidget(m_recordStates);

		h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Update events:"));
		m_updateEvents = new QComboBox();
		m_updateEvents->addItems({ "major iterations", "minor iterations", "all events" });
		h->addWidget(m_updateEvents);
		h->addStretch();

		m_variableNorms = new QCheckBox("collect variable norms");
		settingsLayout->addWidget(m_variableNorms);
		settingsLayout->addLayout(h);

		m_generateReport = new QCheckBox("generate job report");
		settingsLayout->addWidget(m_generateReport);

		b->setLayout(settingsLayout);
		l->addWidget(b);

		l->addStretch();
		l->addWidget(bb);
		dlg->setLayout(l);
		connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
	}

	void AddPauseEvent(QString name, int nevent) { m_pauseEvents->addItem(name, QVariant(nevent)); }

	unsigned int GetPauseEvents() { return (unsigned int)(m_pauseEvents->currentData().toInt()); }
	bool GetStartPausedOption() { return m_startPaused->isChecked(); }
	unsigned int GetUpdateEvents() { return m_updateEvents->currentIndex(); }
	void SetUpdateEvents(int n) { m_updateEvents->setCurrentIndex(n); }

	void SetStartPausedOption(bool b) { m_startPaused->setChecked(b); }

	void SetPauseEvents(unsigned int nevents) 
	{
		int n = m_pauseEvents->findData(nevents);
		m_pauseEvents->setCurrentIndex(n);
	}

	bool IsPauseTimeEnabled() { return m_enablePauseTime->isChecked(); }
	void EnablePauseTime(bool b) { m_enablePauseTime->setChecked(b); }

	double GetPauseTime() { return m_pauseTime->value(); }
	void SetPauseTime(double v) { m_pauseTime->setValue(v); }

	int GetDebugLevel() const { return m_debugMode->currentIndex(); }
	void SetDebugLevel(int n) { m_debugMode->setCurrentIndex(n); }

	void SetRecordStatesFlag(bool b) { m_recordStates->setChecked(b); }
	bool GetRecordStatesFlag() { return m_recordStates->isChecked(); }

	bool CollectVariableNorms() { return m_variableNorms->isChecked(); }

	bool GenerateJobReport() { return m_generateReport->isChecked(); }
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

	ui->SetStartPausedOption(doc->StartPaused());
	ui->SetPauseEvents(doc->GetPauseEvents());
	ui->SetPauseTime(doc->GetPauseTime());
	ui->EnablePauseTime(doc->IsPauseTimeEnabled());
	ui->SetDebugLevel(doc->GetDebugLevel());
	ui->SetRecordStatesFlag(doc->GetRecordStatesFlag());
	ui->SetUpdateEvents(doc->GetUpdateEvents());
}

void CDlgMonitorSettings::CanEditJob(bool b)
{
	if (!b) ui->jobSettings->hide();
}

void CDlgMonitorSettings::SetJobList(const QStringList& joblist)
{
	ui->m_jobName->addItems(joblist);
}

void CDlgMonitorSettings::SetJobName(const QString& jobName)
{
	ui->m_jobName->setEditText(jobName);
}

void CDlgMonitorSettings::SetWorkingDirectory(const QString& workingDir)
{
	ui->m_workingDir->setResourceName(workingDir);
}

QString CDlgMonitorSettings::GetWorkingDirectory()
{
	return ui->m_workingDir->resourceName();
}

QString CDlgMonitorSettings::GetJobName()
{
	return ui->m_jobName->currentText();
}

void CDlgMonitorSettings::accept()
{
	assert(m_doc);
	m_doc->StartPaused(ui->GetStartPausedOption());
	m_doc->SetPauseEvents(ui->GetPauseEvents());
	m_doc->SetPauseTime(ui->GetPauseTime(), ui->IsPauseTimeEnabled());
	m_doc->SetDebugLevel(ui->GetDebugLevel());
	m_doc->SetRecordStatesFlag(ui->GetRecordStatesFlag());
	m_doc->SetUpdateEvents(ui->GetUpdateEvents());
	m_doc->CollectVariableNorms(ui->CollectVariableNorms());
	m_doc->GenerateReport(ui->GenerateJobReport());
	QDialog::accept();
}
