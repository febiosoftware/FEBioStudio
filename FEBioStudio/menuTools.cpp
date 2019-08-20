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
	if (ui->m_process && (ui->m_process->state()!=QProcess::NotRunning))
	{
		QMessageBox::information(this, "PreView2", "An FEBio job is already running.\nYou must wait till the job is finished or stop it.");
		return;
	}

	CDocument* doc = GetDocument();
	QString projectFolder = QString::fromStdString(doc->GetDocFolder());

	QString fileBase = QString::fromStdString(doc->GetDocFileBase());
	QString file = fileBase;
	file += ".feb";

	static QString lastPath, lastFile;
	QString path;
	if (projectFolder.isEmpty()) path = lastPath;
	else path = "$(ProjectFolder)";

	if (fileBase.isEmpty() && (lastFile.isEmpty() == false)) file = lastFile;

	static int lastPathIndex = 0;

	CDlgRun dlg(this);
	dlg.SetWorkingDirectory(path);
	dlg.SetFileName(file);
	dlg.SetFEBioPath(ui->m_febio_path, ui->m_febio_info, lastPathIndex);

	dlg.Init();
	if (dlg.exec())
	{
		QString dir = dlg.GetWorkingDirectory();
		QString file = dlg.GetFileName();

		dir.replace("$(ProjectFolder)", projectFolder);

		lastPath = dir;
		lastFile = file;

		if(!(dir.endsWith("/") | dir.endsWith("\\")))
		{
			#ifdef WIN32
				dir += "\\";
			#else
				dir += "/";
			#endif
		}
		
		QString path = dir + file;

		std::string sfile = path.toStdString();

		// try to save the file first
		AddLogEntry(QString("Saving to %1 ...").arg(path));
		FEBioExport25 feb;
		if (feb.Export(doc->GetProject(), sfile.c_str()) == false)
		{
			QMessageBox::critical(this, "Run FEBio", "Failed saving FEBio file.");
			AddLogEntry("FAILED\n");
			return;
		}
		else AddLogEntry("SUCCESS!\n");

		// see if a job with this name already exists
		CFEBioJob* job = doc->FindFEBioJob(sfile);

		// create a new new job
		if (job == nullptr)
		{
			job = new CFEBioJob(sfile, CFEBioJob::RUNNING);
			doc->AddFEbioJob(job);
		}
		else
		{
			job->SetStatus(CFEBioJob::RUNNING);
			doc->SetActiveJob(job);
		}

		// show it in the model viewer
		UpdateModel(job);

		// clear output for next job
		ClearOutput();

		// create new process
		ui->m_process = new QProcess(this);
		ui->m_process->setProcessChannelMode(QProcess::MergedChannels);
		ui->m_process->setWorkingDirectory(dir);

		lastPathIndex = dlg.GetFEBioPath();
        QString program = ui->m_febio_path.at(lastPathIndex);

		// replace $(FEBioStudioDir)
		if (program.contains("$(FEBioStudioDir)"))
		{
			program.replace("$(FEBioStudioDir)", QApplication::applicationDirPath());
		}

		// get the command line
		QString cmd = dlg.CommandLine();

		// extract the arguments
		QStringList args = cmd.split(" ", QString::SkipEmptyParts);
		
		args.replaceInStrings("$(Filename)", dlg.GetFileName());
		
		// get ready 
		AddLogEntry(QString("Starting FEBio: %1\n").arg(args.join(" ")));
		QObject::connect(ui->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRunFinished(int, QProcess::ExitStatus)));
		QObject::connect(ui->m_process, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		QObject::connect(ui->m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onErrorOccurred(QProcess::ProcessError)));

		// don't forget to reset the kill flag
		ui->m_bkillProcess = false;

		// go!
		ui->m_process->start(program, args);

		// show the output window
		ui->logPanel->ShowOutput();
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
	else QMessageBox::information(this, "PreView2", "No FEBio job is running.");
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

void CMainWindow::onRunFinished(int exitCode, QProcess::ExitStatus es)
{
	if (exitCode == 0)
	{
		QMessageBox::information(this, "Run FEBio", "Normal termination");
		AddLogEntry("FEBio exited: Normal termination\n");
	}
	else
	{
		QMessageBox::information(this, "Run FEBio", "Error termination");
		AddLogEntry("FEBio exited: Error termination\n");
	}

	CFEBioJob* job = GetDocument()->GetActiveJob();
	if (job)
	{
		job->SetStatus(exitCode == 0 ? CFEBioJob::COMPLETED : CFEBioJob::FAILED);
		ShowInModelViewer(job);
		GetDocument()->SetActiveJob(nullptr);
	}

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
