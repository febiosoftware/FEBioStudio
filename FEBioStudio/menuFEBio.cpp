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
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "ModelDocument.h"
#include "DlgRun.h"
#include "DlgFEBioOptimize.h"
#include <FSCore/FSDir.h>
#include <QMessageBox>
#include <QFileDialog>
#include "DlgFEBioInfo.h"
#include "DlgFEBioPlugins.h"
#include <FEBioMonitor/DlgMonitorSettings.h>
#include <FEBioMonitor/FEBioMonitorDoc.h>

void CMainWindow::on_actionFEBioRun_triggered()
{
	// First, check that a job is not running yet
	if (ui->m_jobManager->IsJobRunning())
	{
		QMessageBox::information(this, "FEBio Studio", "An FEBio job is already running.\nYou must wait till the job is finished or stop it.");
		return;
	}

	// name of last job that was run
	static QString lastJobName;

	// these will store defaults for job name, job path, and previous job list
	QString jobName;
	QString jobPath;
	QStringList jobList;

	// get the document
	CDocument* doc = GetDocument();
	CModelDocument* modelDoc = dynamic_cast<CModelDocument*>(doc);

	// get default name and path
	if (doc)
	{
		// make sure that docFolder is valid
		QString docFolder = QString::fromStdString(doc->GetDocFolder());
		if (docFolder.isEmpty())
		{
			QMessageBox::warning(this, "Run FEBio", "You have to save the model before you can run it in FEBio.");
			return;
		}

		jobName = QString::fromStdString(doc->GetDocFileName());
		jobPath = docFolder;
	}

	// fill in job list
	if (modelDoc)
	{
		QString docName = QString::fromStdString(modelDoc->GetDocFileBase());
		QString docFolder = QString::fromStdString(modelDoc->GetDocFolder());

		// make sure that docFolder is valid
		if (docFolder.isEmpty())
		{
			QMessageBox::warning(this, "Run FEBio", "You have to save the model before you can run it in FEBio.");
			return;
		}

		// By default, the job path will be the project folder
		// unless the project folder is not defined, in which case we'll reuse the last path
		jobPath = docFolder + "/jobs";

		// get the list of all the job names so far
		jobName.clear();
		for (int i = 0; i < modelDoc->FEBioJobs(); ++i)
		{
			CFEBioJob* job = modelDoc->GetFEBioJob(i);
			QString jobNamei = QString::fromStdString(job->GetName());
			if (jobNamei == lastJobName) jobName = lastJobName;
			jobList.append(jobNamei);
		}

		// we'll take the last job's name or create a new one if the job list is empty
		if (jobList.empty())
		{
			// create a name for this job
			jobName = docName;
		}
		else if (jobName.isEmpty())
		{
			jobName = jobList.last();
		}
	}

	// this keeps track of the FEBio selection that was used last
	static int lastLaunchConfigIndex = 0;
	static int lastFEBioFileVersion = 0x0400;	// default to 4.0

	// setup the run dialog
	CDlgRun dlg(this);
	dlg.SetWorkingDirectory(jobPath);
	if (jobList.isEmpty() == false) dlg.SetJobNames(jobList);
	if (jobName.isEmpty() == false) dlg.SetJobName(jobName);
	dlg.SetLaunchConfig(ui->m_launch_configs, lastLaunchConfigIndex);
	dlg.SetFEBioFileVersion(lastFEBioFileVersion);

	static bool showAdvancedSettings = false;
	static bool debugFlag = false;
	dlg.ShowAdvancedSettings(showAdvancedSettings);
	dlg.SetDebugFlag(debugFlag);

	if (modelDoc && modelDoc->FEBioJobs() > 0)
	{
		CFEBioJob* job = modelDoc->GetFEBioJob(0);
		std::string s = job->GetConfigFileName();
		if (s.empty() == false)
		{
			dlg.SetConfigFileName(QString::fromStdString(s));
		}
	}

	if (modelDoc == nullptr)
	{
		dlg.ShowFEBioSaveOptions(false);
		dlg.EnableJobSettings(false);
	}

	if (dlg.exec())
	{
		showAdvancedSettings = dlg.AdvancedSettingsShown();
		debugFlag = dlg.HasDebugFlag();

		// get the working directory and job name
		jobPath = dlg.GetWorkingDirectory();
		jobName = dlg.GetJobName();

		// do string replacement
		QString absDir = QString::fromStdString(FSDir::expandMacros(jobPath.toStdString()));

		// create job directory if it doesn't exist.
		if (!QFile(absDir).exists())
		{
			bool b = QDir(absDir).mkpath(absDir);
			if (b == false)
			{
				QMessageBox::critical(this, "FEBioStudio", "Failed creating working directory.\nCannot run job.");
				return;
			}
		}

		// for default jobs we want to change the working directory to the jobs folder
		QDir::setCurrent(absDir);

		CFEBioJob* job = nullptr;

		QDir modelDir(QString::fromStdString(doc->GetDocFolder()));
		string relPath = modelDir.relativeFilePath(jobPath).toStdString();

		// find the relative path with respect to the model's folder
		if (modelDoc)
		{
			// see if a job with this name already exists
			job = modelDoc->FindFEBioJob(jobName.toStdString());
		}

		// update with the selected launch configuration index
		lastLaunchConfigIndex = dlg.GetLaunchConfig();

		// if not, create a new job
		if (job == nullptr)
		{
			// create a new new job
			job = new CFEBioJob(doc, jobName.toStdString(), relPath, ui->m_launch_configs.at(lastLaunchConfigIndex));

			if (modelDoc)
			{
				modelDoc->AddFEbioJob(job);
				// show it in the model viewer
				UpdateModel(job);
			}
		}
		else
		{
			job->UpdateWorkingDirectory(relPath);
			job->UpdateLaunchConfig(ui->m_launch_configs.at(lastLaunchConfigIndex));

			// show it in the model viewer
			UpdateModel(job);
		}

		QString configFile = dlg.GetConfigFileName();
		if (configFile.isEmpty() == false) job->SetConfigFileName(configFile.toStdString());

		// get the selected FEBio file version
		lastFEBioFileVersion = dlg.GetFEBioFileVersion();

		job->m_febVersion = lastFEBioFileVersion;
		job->m_writeNotes = dlg.WriteNotes();
		job->m_cmd = dlg.CommandLine().toStdString();

		// do a model check
		if (modelDoc && (DoModelCheck(modelDoc) == false)) return;

		if (doc)
		{
			// auto-save the document
			if (dlg.DoAutoSave() && doc->IsModified())
			{
				AddLogEntry(QString("saving %1 ...").arg(QString::fromStdString(doc->GetDocFilePath())));
				bool b = doc->SaveDocument();
				UpdateTab(doc);
				AddLogEntry(b ? "success\n" : "FAILED\n");
			}
		}

		// store the name of the job
		lastJobName = jobName;

		// export to FEBio
		if (modelDoc)
		{
			string febFile = job->GetFEBFileName(false);

			// see if the feb file already exists
			QFile file(QString::fromStdString(febFile));
			if (file.exists())
			{
				QString msg = QString("The job \"%1\" was already run. Re-running it may overwrite existing results.\nDo you want to continue?").arg(jobName);
				int n = QMessageBox::warning(this, "Run FEBio", msg, QMessageBox::Yes, QMessageBox::No);
				if (n != QMessageBox::Yes) return;
			}

			// save the FEBio file
			if (ExportFEBioFile(modelDoc, febFile, lastFEBioFileVersion) == false)
			{
				return;
			}
		}

		// run the job
		RunFEBioJob(job);
	}
}

void CMainWindow::on_actionFEBioMonitor_triggered()
{
	if (dynamic_cast<FEBioMonitorDoc*>(GetDocument()))
	{
		QMessageBox::information(this, "FEBio Studio", "The FEBio Monitor is already running.");
		return;
	}
	else
	{
		bool exportFEBioFile = false;
		FEBioMonitorDoc* monitorDoc = new FEBioMonitorDoc(this);
		CModelDocument* doc = GetModelDocument();
		if (doc)
		{
			// make sure the model is saved
			if (doc->IsModified())
			{
				on_actionSave_triggered();
			}

			// write the FEBio input file
			std::string febFilename = doc->GetDocFilePath();

			exportFEBioFile = true;
			size_t n = febFilename.rfind(".fs2");
			if (n == string::npos) exportFEBioFile = false;
			else febFilename.replace(n, 4, ".feb");

			monitorDoc->SetFEBioInputFile(QString::fromStdString(febFilename));
		}

		CDlgMonitorSettings dlg(monitorDoc, this);
		if (doc && exportFEBioFile) dlg.SetFileMode(CDlgMonitorSettings::SAVE_FILE);
		else dlg.SetFileMode(CDlgMonitorSettings::OPEN_FILE);

		if (doc && (exportFEBioFile == false)) dlg.CanEditFilename(false);

		if (dlg.exec())
		{
			std::string febFilename = monitorDoc->GetFEBioInputFile().toStdString();
			if (doc && exportFEBioFile && ExportFEBioFile(doc, febFilename, 0x0400) == false)
			{
				QMessageBox::critical(this, "FEBio Studio", "Failed to export model to feb file.");
				return;
			}

			AddDocument(monitorDoc);
			monitorDoc->RunJob();
		}
		else delete monitorDoc;
	}
}

void CMainWindow::on_actionFEBioMonitorSettings_triggered()
{
	FEBioMonitorDoc* doc = dynamic_cast<FEBioMonitorDoc*>(GetDocument());
	if (doc == nullptr) return;

	if (doc->IsRunning() && !doc->IsPaused())
	{
		QMessageBox::information(this, "FEBio Studio", "Settings can only be changed when the job is paused.");
		return;
	}

	CDlgMonitorSettings dlg(doc, this);
	dlg.CanEditFilename(false);
	dlg.exec();
}

void CMainWindow::on_actionFEBioContinue_triggered()
{
	if (dynamic_cast<FEBioMonitorDoc*>(GetDocument()))
	{
		FEBioMonitorDoc* doc = dynamic_cast<FEBioMonitorDoc*>(GetDocument());
		if (doc->IsRunning() && !doc->IsPaused())
		{
			QMessageBox::information(this, "FEBio Studio", "The current job is already running.");
			return;
		}
		else doc->RunJob();
	}
}

void CMainWindow::on_actionFEBioStop_triggered()
{
	// TODO: I want to hook up the febio monitor to the job manager, but not sure how yet
	//       so until then, I have to do it this way
	if (dynamic_cast<FEBioMonitorDoc*>(GetDocument()))
	{
		FEBioMonitorDoc* doc = dynamic_cast<FEBioMonitorDoc*>(GetDocument());
		if (doc->IsRunning() == false)
		{
			QMessageBox::information(this, "FEBio Studio", "No FEBio job is running.");
		}
		else
		{
			if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to stop the current job?") == QMessageBox::Yes)
			{
				doc->KillJob();
			}
		}
	}
	else
	{
		if (ui->m_jobManager->IsJobRunning())
		{
			ui->m_jobManager->KillJob();
		}
		else QMessageBox::information(this, "FEBio Studio", "No FEBio job is running.");
	}
}

void CMainWindow::on_actionFEBioPause_triggered()
{
	if (dynamic_cast<FEBioMonitorDoc*>(GetDocument()))
	{
		FEBioMonitorDoc* doc = dynamic_cast<FEBioMonitorDoc*>(GetDocument());
		if (doc->IsRunning() == false)
		{
			QMessageBox::information(this, "FEBio Studio", "No FEBio job is running.");
		}
		else doc->PauseJob();
	}
}

void CMainWindow::on_actionFEBioNext_triggered()
{
	if (dynamic_cast<FEBioMonitorDoc*>(GetDocument()))
	{
		FEBioMonitorDoc* doc = dynamic_cast<FEBioMonitorDoc*>(GetDocument());
		if (doc->IsRunning() == false)
		{
			QMessageBox::information(this, "FEBio Studio", "No FEBio job is running.");
		}
		else doc->AdvanceJob();
	}
}

void CMainWindow::on_actionFEBioOptimize_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgFEBioOptimize dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "*.feb");
		if (fileName.isEmpty() == false)
		{
			try {
				FEBioOpt opt = dlg.GetFEBioOpt();

				if (doc->GenerateFEBioOptimizationFile(fileName.toStdString(), opt) == false)
				{
					QMessageBox::critical(this, "Generate FEBio Optimization file", "Something went terribly wrong!");
				}
				else
				{
					QMessageBox::information(this, "Generate FEBio Optimization file", "Success writing FEBio optimization file!");
				}
			}
			catch (...)
			{
				QMessageBox::critical(this, "Generate FEBio Optimization file", "Exception detection. Optimization file might be incorrect.");
			}
		}
	}
}

void CMainWindow::on_actionFEBioInfo_triggered()
{
	CDlgFEBioInfo dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionFEBioPlugins_triggered()
{
	CDlgFEBioPlugins dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionFEBioTangent_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgFEBioTangent dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "*.feb");
		if (fileName.isEmpty() == false)
		{
			try {
				FEBioTangentDiagnostic data = dlg.GetData();

				if (data.WriteDiagnosticFile(doc, fileName.toStdString()) == false)
				{
					QMessageBox::critical(this, "Generate FEBio Diagnostic file", "Something went terribly wrong!");
				}
				else
				{
					QMessageBox::information(this, "Generate FEBio Diagnostic file", "Success writing FEBio Tangent Diagnostic file!");
				}
			}
			catch (...)
			{
				QMessageBox::critical(this, "Generate FEBio Tangent Diagnostic", "Exception detection. Diagnostic file might be incorrect.");
			}
		}
	}
}
