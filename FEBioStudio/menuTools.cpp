#include "stdafx.h"
#include "MainWindow.h"
#include "DlgLameConvertor.h"
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

void CMainWindow::on_actionFEBioRun_triggered()
{
	// First, check that a job is not running yet
	if (ui->m_process && (ui->m_process->state()!=QProcess::NotRunning))
	{
		QMessageBox::information(this, "FEBio Studio", "An FEBio job is already running.\nYou must wait till the job is finished or stop it.");
		return;
	}

	// get the document
	CDocument* doc = GetDocument();

	QString projectName = QString::fromStdString(doc->GetDocFileBase());
	QString projectFolder = QString::fromStdString(doc->GetDocFolder());

	// get the list of all the job names so far
	QStringList jobList;
	for (int i = 0; i < doc->FEBioJobs(); ++i)
	{
		CFEBioJob* job = doc->GetFEBioJob(i);
		jobList.append(QString::fromStdString(job->GetName()));
	}

	// get the project folder and name
	static QString jobName;
	if (jobName.isEmpty() && jobList.isEmpty())
	{
		// create a name for this job
		jobName = projectName;
		jobName += QString("_job");
	}

	// By default, the job path will be the project folder
	// unless the project folder is not defined, in which case we'll reuse the last path
	static QString lastPath;
	QString jobPath;
	if (projectFolder.isEmpty()) jobPath = lastPath;
#ifdef WIN32
	else jobPath = "$(ProjectDir)\\jobs";
#else
	else jobPath = "$(ProjectDir)/jobs";
#endif

	// this keeps track of the FEBio selection that was used last
	static int lastLaunchConfigIndex = 0;

	static int lastFEBioFileVersion = 0;

	// setup the run dialog
	CDlgRun dlg(this);
	dlg.SetWorkingDirectory(jobPath);
	if (jobList.isEmpty() == false) dlg.SetJobNames(jobList);
	if (jobName.isEmpty() == false) dlg.SetJobName(jobName);
	dlg.SetLaunchConfig(ui->m_launch_configs, lastLaunchConfigIndex);

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

		// store the last path
		lastPath = jobPath;

		// do string replacement
		FSDir dir(jobPath.toStdString());
		QString realPath = QString::fromStdString((dir.toAbsolutePath()));

		// create job directory if it doesn't exist.
		if(!QFile(realPath).exists()){
			QDir(realPath).mkpath(realPath);

		}

		// see if a job with this name already exists
		CFEBioJob* job = doc->FindFEBioJob(jobName.toStdString());

		// update with the selected launch configuration index
		lastLaunchConfigIndex = dlg.GetLaunchConfig();

		// if not, create a new job
		if (job == nullptr)
		{
			// create a new new job
			job = new CFEBioJob(doc, jobName.toStdString(), jobPath.toStdString(), ui->m_launch_configs.at(lastLaunchConfigIndex));
			doc->AddFEbioJob(job);

			// show it in the model viewer
			UpdateModel(job);
		}
		else
		{
			job->UpdateWorkingDirectory(jobPath.toStdString());
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
		RunFEBioJob(job);
	}
}

void CMainWindow::on_actionFEBioStop_triggered()
{
	if (ui->m_process && (ui->m_process->state() == QProcess::Running))
	{
		ui->m_bkillProcess = true;
		ui->m_process->kill();

		CFEBioJob* job = GetDocument()->GetActiveJob();
		if (job)
		{
			job->SetStatus(CFEBioJob::CANCELLED);
			GetDocument()->SetActiveJob(nullptr);
			ShowInModelViewer(job);
		}
	}
	else QMessageBox::information(this, "FEBio Studio", "No FEBio job is running.");
}

void CMainWindow::on_actionFEBioOptimize_triggered()
{
	CDlgFEBioOptimize dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "*.feb");
		if (fileName.isEmpty() == false)
		{
			FEBioOpt opt = dlg.GetFEBioOpt();
			if (GetDocument()->GenerateFEBioOptimizationFile(fileName.toStdString(), opt) == false)
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
	CLogPanel* log = ui->logPanel;
	log->AddText("\nMesh Layer Info:\n===================\n");
	GModel* gm = GetDocument()->GetGModel();
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

	for (int i = 0; i < layers; ++i)
	{
		log->AddText(QString::number(i) + ".");
		QString name = QString::fromStdString(gm->GetMeshLayerName(i));
		log->AddText(QString("%1:").arg(name, -l));

		for (int j = 0; j < nobjs; ++j)
		{
			const FEMesher* mesher = nullptr;
			const FEMesh*	mesh = nullptr;

			if (i == gm->GetActiveMeshLayer())
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
			ulong pmesh   = (ulong)mesh;

			QString s1; s1.setNum(pmesher, 16);
			QString s2; s2.setNum(pmesh, 16);

			log->AddText(QString("%1|%2").arg(s1, 8, '0').arg(s2, 8, '0'));

			if (j != nobjs - 1) log->AddText(",");
		}

		if (i == gm->GetActiveMeshLayer())
			log->AddText("***\n");
		else
			log->AddText("\n");
	}
}

void CMainWindow::onRunFinished(int exitCode, QProcess::ExitStatus es)
{
	CFEBioJob* job = GetDocument()->GetActiveJob();
	if (job)
	{
		job->SetStatus(exitCode == 0 ? CFEBioJob::COMPLETED : CFEBioJob::FAILED);
		ShowInModelViewer(job);
		GetDocument()->SetActiveJob(nullptr);

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
	GetDocument()->SetActiveJob(nullptr);

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
	GetDocument()->SetActiveJob(nullptr);

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
