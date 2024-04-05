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
#include "FEBioJobManager.h"
#include "FEBioThread.h"
#include "FEBioJob.h"
#include "LocalJobProcess.h"
#ifdef HAS_SSH
#include "SSHHandler.h"
#include "SSHThread.h"
#endif
#include <QProcess>
#include <QMessageBox>
#include <QToolButton>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QFormLayout>
#include "MainWindow.h"

#ifdef WIN32
#include <Windows.h>
#endif

class CFEBioJobManager::Impl
{
public:
	CMainWindow*	wnd;
	QProcess*		process;
	CFEBioThread*	febThread;
	bool			bkillJob;
};

CFEBioJobManager::CFEBioJobManager(CMainWindow* wnd) : im(new CFEBioJobManager::Impl)
{
	im->wnd = wnd;
	im->process = nullptr;
	im->bkillJob = false;
	im->febThread = nullptr;
}

bool CFEBioJobManager::StartJob(CFEBioJob* job)
{
	// make sure no model is running
	if (CFEBioJob::GetActiveJob() != nullptr) return false;

	// set this as the active job
	CFEBioJob::SetActiveJob(job);

	if (job == nullptr) return true;

	// don't forget to reset the kill flag
	im->bkillJob = false;

	job->ClearProgress();
	job->StartTimer();

	// launch the job
	if (job->GetLaunchConfig()->type == launchTypes::DEFAULT)
	{
		if (im->febThread) return false;
		im->febThread = new CFEBioThread(im->wnd, job, this);
		im->febThread->start();
	}
	else if (job->GetLaunchConfig()->type == LOCAL)
	{
		if (im->process) return false;
		// create new process
		CLocalJobProcess* process = new CLocalJobProcess(im->wnd, job, this);
		im->process = process;

		// go! 
		process->run();
	}
	else
	{
#ifdef HAS_SSH
		CSSHHandler* handler = job->GetSSHHandler();

		if (!handler->IsBusy())
		{
			handler->SetTargetFunction(STARTREMOTEJOB);

			CSSHThread* sshThread = new CSSHThread(handler, STARTSSHSESSION);
			QObject::connect(sshThread, &CSSHThread::FinishedPart, im->wnd, &CMainWindow::NextSSHFunction);
			sshThread->start();
		}

		// NOTE: Why are we doing this?
		CFEBioJob::SetActiveJob(nullptr);
#endif
	}

	return true;
}

bool CFEBioJobManager::IsJobRunning() const
{
	return (CFEBioJob::GetActiveJob() != nullptr);
}

void CFEBioJobManager::KillJob()
{
	if (im->process && (im->process->state() == QProcess::Running))
	{
		im->bkillJob = true;
		im->process->kill();

		CFEBioJob* job = CFEBioJob::GetActiveJob();
		if (job)
		{
			job->SetStatus(CFEBioJob::CANCELLED);
			CFEBioJob::SetActiveJob(nullptr);
		}
	}
	else if (im->febThread && im->febThread->isRunning())
	{
		im->bkillJob = true;
		im->febThread->KillThread();
		im->febThread = nullptr;

		CFEBioJob* job = CFEBioJob::GetActiveJob();
		if (job)
		{
			job->SetStatus(CFEBioJob::CANCELLED);
			CFEBioJob::SetActiveJob(nullptr);
		}
	}
}

void CFEBioJobManager::onRunFinished(int exitCode, QProcess::ExitStatus es)
{
	CFEBioJob* job = CFEBioJob::GetActiveJob();
	if (job)
	{
		job->StopTimer();
		job->SetStatus(exitCode == 0 ? CFEBioJob::COMPLETED : CFEBioJob::FAILED);
		CFEBioJob::SetActiveJob(nullptr);

		QString sret = (exitCode == 0 ? "NORMAL TERMINATION" : "ERROR TERMINATION");
		QString jobName = QString::fromStdString(job->GetName());
		QString logmsg = QString("FEBio job \"%1 \" has finished: %2\n").arg(jobName).arg(sret);
		im->wnd->AddLogEntry(logmsg);

		CDlgJobMonitor dlg(im->wnd);
		dlg.SetFEBioJob(job);
		if (dlg.exec())
		{
			im->wnd->OpenFile(QString::fromStdString(job->GetPlotFileName()), false, false);
		}

		im->wnd->UpdateTab(job->GetDocument());
	}
	else
	{
		// Not sure if we should ever get here.
		QMessageBox::information(im->wnd, "FEBio Studio", "FEBio is done.");
	}
	CFEBioJob::SetActiveJob(nullptr);

	im->febThread = nullptr;
	delete im->process;
	im->process = nullptr;
}

void CFEBioJobManager::onReadyRead()
{
	if (im->process)
	{
		QByteArray output = im->process->readAll();
		QString s(output);
		im->wnd->AddOutputEntry(s);
	}
	else if (im->febThread)
	{
		QString s = im->febThread->GetOutput();
		im->wnd->AddOutputEntry(s);
	}
}

void CFEBioJobManager::onErrorOccurred(QProcess::ProcessError err)
{
	// make sure we don't have an active job since onRunFinished will not be called!
	CFEBioJob::SetActiveJob(nullptr);

	// suppress an error if user stopped FEBio job
	if (im->bkillJob && (err == QProcess::Crashed))
	{
		return;
	}

	// check for FailedToStart
	if (err == QProcess::FailedToStart)
	{
		QMessageBox::critical(im->wnd, "Run FEBio", "FEBio failed to start.\nCheck the launch configuration and make sure that the path to the FEBio executable is correct.");
	}
	else
	{
		QString errString;
		switch (err)
		{
		case QProcess::FailedToStart: errString = "Failed to start"; break;
		case QProcess::Crashed: errString = "Crashed"; break;
		case QProcess::Timedout: errString = "Timed out"; break;
		case QProcess::WriteError: errString = "Write error"; break;
		case QProcess::ReadError: errString = "Read error"; break;
		case QProcess::UnknownError: errString = "Unknown error"; break;
		default:
			errString = QString("Error code = %1").arg(err);
		}

		QString t = "An error has occurred.\nError = " + errString;
		im->wnd->AddLogEntry(t);
		QMessageBox::critical(im->wnd, "Run FEBio", t);
	}
}

//==================================================================================

QString FormatTimeString(double sec)
{
	int nsec = (int)sec;
	int nhr  = (nsec / 3600);
	nsec -= nhr * 3600;
	int nmin = (nsec / 60);
	nsec -= 60 * nmin;

	return QString("%1:%2:%3").arg(nhr).arg(nmin, 2, 10, QChar('0')).arg(nsec, 2, 10, QChar('0'));
}

class Ui::CDlgJobMonitor
{
public:
	QLabel* jobName;
	QLabel* jobStatus;
	QLabel* runTime;
	QLabel* completion;
	QPlainTextEdit* log;

	QPushButton* openPlt;
	QPushButton* close;
	QToolButton* showWarnings;
	QToolButton* showErrors;

	struct LogEntry
	{
		int	 ntype;
		QString	msg;
		int	count;
	};

	QList<LogEntry> m_log;

public:
	void setup(::CDlgJobMonitor* dlg)
	{
		QGridLayout* g = new QGridLayout;
		g->addWidget(new QLabel("Job:"), 0, 0);
		g->addWidget(jobName = new QLabel(), 0, 1); jobName->setMinimumWidth(200);
		QFont font = jobName->font();
		font.setBold(true);
		jobName->setAlignment(Qt::AlignLeft);
		jobName->setFrameShape(QFrame::Shape::Box);
		jobName->setFont(font);

		g->addWidget(new QLabel("Status:"), 0, 2);
		g->addWidget(jobStatus = new QLabel(), 0, 3); jobStatus->setMinimumWidth(200);
		jobStatus->setAlignment(Qt::AlignLeft);
		jobStatus->setFrameShape(QFrame::Shape::Box);
		jobStatus->setFont(font);

		g->addWidget(new QLabel("Runtime :"), 1, 0);
		g->addWidget(runTime = new QLabel, 1, 1);
		runTime->setAlignment(Qt::AlignLeft);
		runTime->setFrameShape(QFrame::Box);
		g->addWidget(new QLabel("Completion :"), 1, 2);
		g->addWidget(completion = new QLabel, 1, 3); 
		completion->setAlignment(Qt::AlignLeft);
		completion->setFrameShape(QFrame::Box);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(g);

		QHBoxLayout* h = new QHBoxLayout;
		showWarnings = new QToolButton;
		showWarnings->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		showWarnings->setText("Warnings (0)");
		showWarnings->setIcon(QIcon(":/icons/warning.png"));
		showWarnings->setCheckable(true);
		showWarnings->setChecked(true);
		showWarnings->setAutoRaise(true);

		showErrors = new QToolButton;
		showErrors->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		showErrors->setText("Errors (0)");
		showErrors->setIcon(QIcon(":/icons/clear.png"));
		showErrors->setCheckable(true);
		showErrors->setChecked(true);
		showErrors->setAutoRaise(true);

		h->addWidget(showWarnings);
		h->addWidget(showErrors);
		h->addStretch();
		l->addLayout(h);

		openPlt = new QPushButton("Open results");
		log = new QPlainTextEdit;
		log->setFont(QFont("Courier", 12));
		log->setReadOnly(true);

		l->addWidget(log);
		l->addWidget(openPlt);

		l->addStretch();
		QHBoxLayout* hc = new QHBoxLayout;
		hc->addStretch();
		hc->addWidget(close = new QPushButton("Close"));
		l->addLayout(hc);

		dlg->setLayout(l);

		QObject::connect(openPlt, &QPushButton::clicked, dlg, &QDialog::accept);
		QObject::connect(close, &QPushButton::clicked, dlg, &QDialog::reject);
		QObject::connect(showWarnings, &QPushButton::toggled, dlg, &::CDlgJobMonitor::UpdateReport);
		QObject::connect(showErrors, &QPushButton::toggled, dlg, &::CDlgJobMonitor::UpdateReport);

#ifdef WIN32
		MessageBeep(MB_ICONASTERISK);
#endif
	}

	void setJob(CFEBioJob* job)
	{
		log->clear();
		m_log.clear();
		if (job == nullptr)
		{
			jobName->setText("");
			jobStatus->setText("");
			return;
		}

		jobName->setText(QString::fromStdString(job->GetName()));
		int status = job->GetStatus();
		switch (status)
		{
		case CFEBioJob::NONE     : jobStatus->setText("N/A"); break;
		case CFEBioJob::COMPLETED: jobStatus->setText("Normal termination"); break;
		case CFEBioJob::FAILED   : jobStatus->setText("Error termination"); break;
		case CFEBioJob::CANCELLED: jobStatus->setText("Cancelled"); break;
		case CFEBioJob::RUNNING  : jobStatus->setText("Running"); break;
		default: jobStatus->setText("(Unknown)"); break;
		}

		double elapsedTime = job->m_toc - job->m_tic;
		QString timeStr = FormatTimeString(elapsedTime);
		runTime->setText(timeStr);

		// get the job progress.
		double pct = job->GetProgress();
		if (status == CFEBioJob::COMPLETED) pct = 100.0;
		else pct = (int)(pct * 10 + 0.5) / 10.0;
		completion->setText(QString("%1 %").arg(pct));

		QString report = QString::fromStdString(job->m_jobReport);
		QStringList warningsAndErrors = report.split(QChar('\n'));
		QString last;
		int count = 0;
		for (QString next : warningsAndErrors)
		{
			if (next != last)
			{
				if (!last.isEmpty())
				{
					QString s = last;
					if (count > 1) s += QString("(x %1)").arg(count);
					AppendLog(s, count);
				}
				count = 1;
				last = next;
			}
			else count++;
		}
		if (!last.isEmpty())
		{
			QString s = last;
			if (count > 1) s += QString("(x %1)").arg(count);
			AppendLog(s, count);
		}

		UpdateLog();
	}

	void AppendLog(QString& s, int count)
	{
		if      (s.contains("Warning:", Qt::CaseInsensitive)) m_log.push_back({ 1, s, count });
		else if (s.contains("Error:"  , Qt::CaseInsensitive)) m_log.push_back({ 2, s, count });
		else m_log.push_back({ 0, s, count });
	}

	void UpdateLog()
	{
		int warningCount = 0;
		int errorCount = 0;
		log->clear();
		bool warnings = showWarnings->isChecked();
		bool errors = showErrors->isChecked();
		for (auto& item : m_log)
		{
			if (item.ntype == 1) warningCount += item.count;
			if (item.ntype == 2) errorCount += item.count;

			if (((item.ntype == 1) && warnings) ||
				((item.ntype == 2) && errors) ||
				(item.ntype == 0))
			{
				if (item.ntype == 2)
				{
					QTextCharFormat defaultFmt = log->currentCharFormat();
					QTextCharFormat fmt(defaultFmt);
					fmt.setForeground(Qt::red);
					log->setCurrentCharFormat(fmt);
					log->appendPlainText(item.msg);
					log->setCurrentCharFormat(defaultFmt);
				}
				else if (item.ntype == 1)
				{
					log->appendPlainText(item.msg);
				}
				else
					log->appendPlainText(item.msg);
			}
		}

		showWarnings->setText(QString("Warnings (%1)").arg(warningCount));
		showErrors->setText(QString("Errors (%1)").arg(errorCount));
	}
};

CDlgJobMonitor::CDlgJobMonitor(CMainWindow* wnd) : QDialog(wnd), ui(new Ui::CDlgJobMonitor())
{
	setWindowTitle("Job Monitor");
	ui->setup(this);
}

void CDlgJobMonitor::SetFEBioJob(CFEBioJob* job)
{
	ui->setJob(job);
}

void CDlgJobMonitor::UpdateReport()
{
	ui->UpdateLog();
}
