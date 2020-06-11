/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "DlgLameConvertor.h"
#include "DlgUnitConverter.h"
#include "DlgKinemat.h"
#include "DlgPlotMix.h"
#include "DlgSettings.h"
#include "DlgRun.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <FEBio/FEBioExport25.h>
#include "DlgFEBioOptimize.h"
#include <QFileDialog>
#include <QApplication>
#include <FSCore/FSDir.h>
#include "SSHHandler.h"
#include <GeomLib/MeshLayer.h>
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>
#include "ModelDocument.h"

void CMainWindow::on_actionCurveEditor_triggered()
{
	ui->showCurveEditor();
}

void CMainWindow::on_actionMeshInspector_triggered()
{
	ui->showMeshInspector();
}

void CMainWindow::on_actionElasticityConvertor_triggered()
{
	CDlgLameConvertor dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionUnitConverter_triggered()
{
	CDlgUnitConverter dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionKinemat_triggered()
{
	CDlgKinemat dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionPlotMix_triggered()
{
	CDlgPlotMix dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionFEBioRun_triggered()
{
	// First, check that a job is not running yet
	if (ui->m_process && (ui->m_process->state()!=QProcess::NotRunning))
	{
		QMessageBox::information(this, "FEBio Studio", "An FEBio job is already running.\nYou must wait till the job is finished or stop it.");
		return;
	}

	// get the document
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	QString docName = QString::fromStdString(doc->GetDocFileBase());
	QString docFolder = QString::fromStdString(doc->GetDocFolder());

	// make sure that docFolder is valid
	if (docFolder.isEmpty())
	{
		QMessageBox::warning(this, "Run FEBio", "You have to save the model before you can run it in FEBio.");
		return;
	}

	// get the list of all the job names so far
	QStringList jobList;
	for (int i = 0; i < doc->FEBioJobs(); ++i)
	{
		CFEBioJob* job = doc->GetFEBioJob(i);
		jobList.append(QString::fromStdString(job->GetName()));
	}

	// By default, the job path will be the project folder
	// unless the project folder is not defined, in which case we'll reuse the last path
	QString jobPath = docFolder + "/jobs";

	// we'll take the last job's name or create a new one if the job list is empty
	QString jobName;
	if (jobList.empty())
	{
		// create a name for this job
		jobName = docName;
	}
	else
	{
		jobName = jobList.last();
	}

	// this keeps track of the FEBio selection that was used last
	static int lastLaunchConfigIndex = 0;
	static int lastFEBioFileVersion = 0;

	// setup the run dialog
	CDlgRun dlg(this);
	dlg.SetWorkingDirectory(jobPath);
	if (jobList.isEmpty() == false) dlg.SetJobNames(jobList);
	if (jobName.isEmpty() == false) dlg.SetJobName(jobName);
	dlg.SetLaunchConfig(ui->m_launch_configs, lastLaunchConfigIndex);
	dlg.SetFEBioFileVersion(lastFEBioFileVersion);

	if (doc->FEBioJobs() > 0)
	{
		CFEBioJob* job = doc->GetFEBioJob(0);
		std::string s = job->GetConfigFileName();
		if (s.empty() == false)
		{
			dlg.SetConfigFileName(QString::fromStdString(s));
		}
	}

	if (dlg.exec())
	{
		// get the working directory and job name
		jobPath = dlg.GetWorkingDirectory();
		jobName = dlg.GetJobName();

		// do string replacement
		QString absDir = QString::fromStdString(FSDir::expandMacros(jobPath.toStdString()));

		// create job directory if it doesn't exist.
		if(!QFile(absDir).exists())
		{
			bool b = QDir(absDir).mkpath(absDir);
			if (b == false)
			{
				QMessageBox::critical(this, "FEBioStudio", "Failed creating working directory.\nCannot run job.");
				return;
			}
		}

		// find the relative path with respect to the model's folder
		QDir modelDir(QString::fromStdString(doc->GetDocFolder()));
		string relPath = modelDir.relativeFilePath(jobPath).toStdString();

		// see if a job with this name already exists
		CFEBioJob* job = doc->FindFEBioJob(jobName.toStdString());

		// update with the selected launch configuration index
		lastLaunchConfigIndex = dlg.GetLaunchConfig();

		// if not, create a new job
		if (job == nullptr)
		{
			// create a new new job
			job = new CFEBioJob(doc, jobName.toStdString(), relPath, ui->m_launch_configs.at(lastLaunchConfigIndex));
			doc->AddFEbioJob(job);

			// show it in the model viewer
			UpdateModel(job);
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
		RunFEBioJob(job, dlg.DoAutoSave());
	}
}

void CMainWindow::on_actionFEBioStop_triggered()
{
	if (ui->m_process && (ui->m_process->state() == QProcess::Running))
	{
		ui->m_bkillProcess = true;
		ui->m_process->kill();

		CFEBioJob* job = CFEBioJob::GetActiveJob();
		if (job)
		{
			job->SetStatus(CFEBioJob::CANCELLED);
			CFEBioJob::SetActiveJob(nullptr);
			ShowInModelViewer(job);
		}
	}
	else QMessageBox::information(this, "FEBio Studio", "No FEBio job is running.");
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
	}
}

void CMainWindow::on_actionOptions_triggered()
{
	CDlgSettings dlg(this);
	dlg.exec();
}

void CMainWindow::on_actionLayerInfo_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CLogPanel* log = ui->logPanel;
	log->AddText("\nMesh Layer Info:\n===================\n");
	GModel* gm = doc->GetGModel();
	int nobjs = gm->Objects();
	int layers = gm->MeshLayers();
	MeshLayerManager* mlm = gm->GetMeshLayerManager();

	// get the longest layer name
	int l = 0;
	for (int i = 0; i < layers; ++i)
	{
		const std::string& s = gm->GetMeshLayerName(i);
		int sl = s.length();
		if (sl > l) l = sl;
	}

	int activeLayer = gm->GetActiveMeshLayer();
	for (int i = 0; i < layers; ++i)
	{
		log->AddText(QString::number(i) + ".");
		QString name = QString::fromStdString(gm->GetMeshLayerName(i));
		log->AddText(QString("%1:").arg(name, -l));

		int nc = nobjs;
		int meshes = mlm->FEMeshes(i);
		if (meshes != nobjs)
		{
			log->AddText(QString("(Incorrect number of meshes: %1 / %2)").arg(meshes).arg(nobjs));
		}
		else
		{
			for (int j = 0; j < nc; ++j)
			{
				const FEMesher* mesher = nullptr;
				const FEMesh*	mesh = nullptr;

				if (i == activeLayer)
				{
					mesher = gm->Object(j)->GetFEMesher();
					mesh = gm->Object(j)->GetFEMesh();
				}
				else
				{
					mesher = mlm->GetFEMesher(i, j);
					mesh = mlm->GetFEMesh(i, j);
				}

				ulong pmesher = (ulong)mesher;
				ulong pmesh = (ulong)mesh;

				QString s1; s1.setNum(pmesher, 16);
				QString s2; s2.setNum(pmesh, 16);

				log->AddText(QString("%1|%2").arg(s1, 8, '0').arg(s2, 8, '0'));

				if (j != nobjs - 1) log->AddText(",");
			}
		}

		if (i == activeLayer)
			log->AddText("***\n");
		else
			log->AddText("\n");
	}
}

void CMainWindow::onRunFinished(int exitCode, QProcess::ExitStatus es)
{
	CFEBioJob* job = CFEBioJob::GetActiveJob();
	if (job)
	{
		job->SetStatus(exitCode == 0 ? CFEBioJob::COMPLETED : CFEBioJob::FAILED);
		ShowInModelViewer(job);
		CFEBioJob::SetActiveJob(nullptr);

		QString sret = (exitCode == 0 ? "NORMAL TERMINATION" : "ERROR TERMINATION");
		QString jobName = QString::fromStdString(job->GetName());
		QString msg = QString("FEBio job \"%1 \" has finished:\n\n%2\n").arg(jobName).arg(sret);

		QString logmsg = QString("FEBio job \"%1 \" has finished: %2\n").arg(jobName).arg(sret);

		if (exitCode == 0)
		{
			QMessageBox::information(this, "Run FEBio", msg);
			AddLogEntry(logmsg);
		}
		else
		{
			QMessageBox::critical(this, "Run FEBio", msg);
			AddLogEntry(logmsg);
		}
	}
	else
	{
		// Not sure if we should ever get here.
		QMessageBox::information(this, "FEBio Studio", "FEBio is done.");
	}
	CFEBioJob::SetActiveJob(nullptr);

	delete ui->m_process;
	ui->m_process = 0;
}

void CMainWindow::onReadyRead()
{
	if (ui->m_process == 0) return;

	QByteArray output = ui->m_process->readAll();
	QString s(output);
	AddOutputEntry(s);
}

void CMainWindow::onErrorOccurred(QProcess::ProcessError err)
{
	// make sure we don't have an active job since onRunFinished will not be called!
	CFEBioJob::SetActiveJob(nullptr);

	// suppress an error if user stopped FEBio job
	if (ui->m_bkillProcess && (err==QProcess::Crashed))
	{
		return;
	}

	QString errString;
	switch (err)
	{
	case QProcess::FailedToStart: errString = "Failed to start"; break;
	case QProcess::Crashed      : errString = "Crashed"; break;
	case QProcess::Timedout     : errString = "Timed out"; break;
	case QProcess::WriteError   : errString = "Write error"; break;
	case QProcess::ReadError    : errString = "Read error"; break;
	case QProcess::UnknownError : errString = "Unknown error"; break;
	default:
		errString = QString("Error code = %1").arg(err);
	}

	QString t = "An error has occurred.\nError = " + errString;
	AddLogEntry(t);
	QMessageBox::critical(this, "Run FEBio", t);
}
