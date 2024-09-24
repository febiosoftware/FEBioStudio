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
#include "DlgRun.h"
#include <QAction>
#include <QFileDialog>
#include <QToolButton>
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QGroupBox>
#include <FSCore/FSDir.h>
#include <string.h>
#include "DlgEditLConfigs.h"
#include "DlgExportFEBio.h"
#include <FEBioLink/FEBioClass.h>
#include <FECore/fecore_enum.h>


class Ui::CDlgRun
{
public:
	QLineEdit*	cwd;
	QComboBox*	jobName;
	QComboBox*	launchConfig;
	QCheckBox*	debug;
	QCheckBox*	writeNotes;
	QCheckBox*	allowMixedMesh;
	QLineEdit*	configFile;
	QComboBox*	taskName;
	QLineEdit*	taskFile;
	QCheckBox*	autoSave;

	CFEBioFormatSelector* febioFormat;
	
	QCheckBox* editCmd;
	QLineEdit*	cmd;

	QGroupBox*	febops;
	QWidget*	jobFolder;

	QGroupBox*	advSettings;
	QPushButton* more;

	std::vector<CLaunchConfig>* m_launch_configs;

	int m_last_index = -1;

public:
	void setup(QDialog* dlg)
	{
		jobName = new QComboBox;
		jobName->setEditable(true);

		launchConfig = new QComboBox;
		QAction* editLaunchConfigs = new QAction;
		editLaunchConfigs->setIcon(QIcon(":/icons/edit.png"));
		QToolButton* editLCBtn = new QToolButton;
		editLCBtn->setDefaultAction(editLaunchConfigs);
		QBoxLayout* launchConfigLayout = new QHBoxLayout;
		launchConfigLayout->addWidget(launchConfig);
		launchConfigLayout->addWidget(editLCBtn);
		

		cwd = new QLineEdit;
		QAction* setCWD = new QAction;
		setCWD->setIcon(QIcon(":/icons/open.png"));
		QToolButton* setCWDBtn = new QToolButton;
		setCWDBtn->setDefaultAction(setCWD);
		QHBoxLayout* cwdLayout = new QHBoxLayout;
		cwdLayout->setContentsMargins(0,0,0,0);
		cwdLayout->addWidget(cwd);
		cwdLayout->addWidget(setCWDBtn);

		jobFolder = new QWidget;
		jobFolder->setLayout(cwdLayout);

		febioFormat = new CFEBioFormatSelector;
		febioFormat->setFEBioFormat(0x0400);

		autoSave = new QCheckBox("Save model before running FEBio");
		autoSave->setChecked(true);

		configFile = new QLineEdit;
		configFile->setWhatsThis("Specify the location of an FEBio configuration file. (Optional)");
		QToolButton* selectConfigFile = new QToolButton;
		selectConfigFile->setIcon(QIcon(":/icons/open.png"));
		QHBoxLayout* configLayout = new QHBoxLayout;
		configLayout->addWidget(configFile);
		configLayout->addWidget(selectConfigFile);

		taskName = new QComboBox; taskName->setEditable(true);
		std::vector<FEBio::FEBioClassInfo> ci = FEBio::FindAllClasses(-1, FETASK_ID);
		for (int i = 0; i < ci.size(); ++i)
		{
			taskName->addItem(ci[i].sztype);
		}
		taskName->setCurrentIndex(-1);

		taskFile = new QLineEdit;

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Job name:", jobName);
		form->addRow("Working directory:", jobFolder);
		form->addRow("Launch Configuration:", launchConfigLayout);
		form->addRow("", autoSave);

		QGroupBox* settings = new QGroupBox("Job settings:");
		settings->setLayout(form);

		QFormLayout* febl = new QFormLayout;
		febl->addRow("FEBio file format:", febioFormat);
		febl->addRow("", writeNotes = new QCheckBox("Write Notes"));
		febl->addRow("", allowMixedMesh = new QCheckBox("Allow degenerate elements"));

		febops = new QGroupBox("FEBio export settings:");
		febops->setLayout(febl);

		QFormLayout* ext = new QFormLayout;
		ext->setLabelAlignment(Qt::AlignRight);
		ext->addRow("Debug mode:", debug = new QCheckBox(""));
		ext->addRow("Config file:", configLayout);
		ext->addRow("Task name:", taskName);
		ext->addRow("Task control file:", taskFile);

		writeNotes->setChecked(true);

		editCmd = new QCheckBox("override command");
		cmd = new QLineEdit; cmd->setEnabled(false);
		cmd->setText("-i $(Filename)");

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(ext);
		v->addWidget(editCmd);
		v->addWidget(cmd);

		advSettings = new QGroupBox("Advanced settings:");
		advSettings->setLayout(v);
		advSettings->hide();

		QPushButton* run = new QPushButton("Run");
		QPushButton* cnl = new QPushButton("Cancel");
		more = new QPushButton("Advanced");
		more->setCheckable(true);

		QHBoxLayout* h = new QHBoxLayout;
		h->addStretch();
		h->addWidget(run);
		h->addWidget(cnl);
		h->addWidget(more);
		
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(settings);
		l->addWidget(febops);
		l->addStretch();
		l->addWidget(advSettings);
		l->addLayout(h);
		dlg->setLayout(l);

		dlg->adjustSize();
		
		QObject::connect(editLCBtn, SIGNAL(clicked()), dlg, SLOT(on_editLCBtn_Clicked()));
		QObject::connect(setCWDBtn, SIGNAL(clicked()), dlg, SLOT(on_setCWDBtn_Clicked()));
		QObject::connect(run, SIGNAL(clicked()), dlg, SLOT(accept()));
		QObject::connect(cnl, SIGNAL(clicked()), dlg, SLOT(reject()));
		QObject::connect(more, SIGNAL(toggled(bool)), advSettings, SLOT(setVisible(bool)));
//		QObject::connect(launchConfig, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onPathChanged(int)));

		QObject::connect(debug, SIGNAL(toggled(bool)), dlg, SLOT(updateDefaultCommand()));
		QObject::connect(editCmd, SIGNAL(toggled(bool)), cmd, SLOT(setEnabled(bool)));
		QObject::connect(selectConfigFile, SIGNAL(clicked()), dlg, SLOT(on_selectConfigFile()));
		QObject::connect(configFile, SIGNAL(textChanged(const QString&)), dlg, SLOT(updateDefaultCommand()));
		QObject::connect(taskName, SIGNAL(currentTextChanged(const QString&)), dlg, SLOT(updateDefaultCommand()));
		QObject::connect(taskFile, SIGNAL(textChanged(const QString&)), dlg, SLOT(updateDefaultCommand()));
	}
};

void CDlgRun::on_selectConfigFile()
{
	QString s = QFileDialog::getOpenFileName(this, "Open");
	if (s.isEmpty() == false)
	{ 
#ifdef WIN32
		s.replace('/', '\\');
#endif
		ui->configFile->setText(s);
		updateDefaultCommand();
	}
}

void CDlgRun::ShowAdvancedSettings(bool b)
{
	ui->more->setChecked(b);
}

bool CDlgRun::AdvancedSettingsShown()
{
	return ui->more->isChecked();
}

void CDlgRun::SetDebugFlag(bool b)
{
	ui->debug->setChecked(b);
}

bool CDlgRun::HasDebugFlag()
{
	return ui->debug->isChecked();
}

void CDlgRun::ShowFEBioSaveOptions(bool b)
{
	ui->febops->setVisible(b);
}

void CDlgRun::EnableJobSettings(bool b)
{
	ui->jobName->setEnabled(b);
	ui->jobFolder->setEnabled(b);
}

void CDlgRun::updateDefaultCommand()
{
	bool bg = ui->debug->isChecked();

	if (ui->cmd->isEnabled() == false)
	{
		QString t("-i $(Filename)");
		if (bg) t += " -g";

		QString configFile = ui->configFile->text();
		if (configFile.isEmpty() == false) t += " -config $(ConfigFile)";

		QString taskName = ui->taskName->currentText();
		if (taskName.isEmpty() == false)
		{
			t += " -task=\"" + taskName + "\"";
			QString taskFile = ui->taskFile->text();
			t += " " + taskFile;
		}

		ui->cmd->setText(t);
	}
}

void CDlgRun::on_setCWDBtn_Clicked()
{
	QFileDialog dlg(this, "Working Directory",GetWorkingDirectory());
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(GetWorkingDirectory());

	if (dlg.exec())
	{
		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		SetWorkingDirectory(fileName);
	}
}

void CDlgRun::SetWorkingDirectory(const QString& wd)
{
	QString workingDir = QDir::toNativeSeparators(wd);
	ui->cwd->setText(workingDir);
}

void CDlgRun::SetJobName(const QString& fn)
{
	ui->jobName->setEditText(fn);
}

void CDlgRun::SetJobNames(QStringList& jobNames)
{
	ui->jobName->addItems(jobNames);
}

void CDlgRun::UpdateLaunchConfigBox(int index)
{
	ui->launchConfig->clear();

	QStringList launchConfigNames;
	for(CLaunchConfig conf : *ui->m_launch_configs)
	{
		launchConfigNames.append(QString::fromStdString(conf.name));
	}

	ui->launchConfig->addItems(launchConfigNames);

	if ((index >= 0) && (index < ui->launchConfig->count()))
		ui->launchConfig->setCurrentIndex(index);
}

void CDlgRun::SetLaunchConfig(std::vector<CLaunchConfig>& launchConfigs, int ndefault)
{
	ui->m_launch_configs = &launchConfigs;

	UpdateLaunchConfigBox(ndefault);
	ui->m_last_index = ndefault;
}

void CDlgRun::SetFEBioFileVersion(int fileVersion)
{
	ui->febioFormat->setFEBioFormat(fileVersion);
}

QString CDlgRun::GetWorkingDirectory()
{
	return ui->cwd->text();
}

QString CDlgRun::GetJobName()
{
	return ui->jobName->currentText();
}

QString CDlgRun::GetConfigFileName()
{
	return ui->configFile->text();
}

void CDlgRun::SetConfigFileName(const QString& configFile)
{
	ui->configFile->setText(configFile);
}

bool CDlgRun::DoAutoSave()
{
	return ui->autoSave->isChecked();
}

int CDlgRun::GetLaunchConfig()
{
	return ui->launchConfig->currentIndex();
}

int CDlgRun::GetFEBioFileVersion()
{
	return ui->febioFormat->FEBioFormat();
}

bool CDlgRun::WriteNotes()
{
	return ui->writeNotes->isChecked();
}

bool CDlgRun::AllowMixedMesh()
{
	return ui->allowMixedMesh->isChecked();
}

QString CDlgRun::CommandLine()
{
	return ui->cmd->text();
}

CDlgRun::CDlgRun(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgRun)
{
	ui->setup(this);
	ui->jobName->setFocus();
	setWindowTitle("Run FEBio");
	resize(500, 300);
}

void CDlgRun::on_editLCBtn_Clicked()
{
	CDlgEditPath dlg(this, ui->m_launch_configs);

	if (dlg.exec())
	{
		UpdateLaunchConfigBox(dlg.GetLCIndex());
	}
}


void CDlgRun::accept()
{
	// see if the path is valid
	QString path = ui->cwd->text();
	if (path.isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter a valid working directory.");
		return;
	}

	// convert to an absolute path
	std::string spath = path.toStdString();
	FSDir dir(spath);
	spath = dir.expandMacros();

	// check if it exists
	// TODO: Commenting this out since it was annoying to see this question every time. 
/*
	QDir qdir(QString::fromStdString(spath));
	if (path.isEmpty() || (qdir.exists() == false))
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, "FEBio Studio", "The directory you specified does not exist. Create it?",
				QMessageBox::Yes|QMessageBox::No);

		if(!(reply == QMessageBox::Yes))
		{
			return;
		}
	}
*/
	// see if the job name is defined
	if (ui->jobName->currentText().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter valid job name.");
		return;
	}

	QDialog::accept();
}
