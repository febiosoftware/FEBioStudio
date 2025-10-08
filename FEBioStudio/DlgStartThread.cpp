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
#include "DlgStartThread.h"
#include "MainWindow.h"
#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFormLayout>
#include <QTimer>
#include <chrono>
using namespace std::chrono;
using dseconds = duration<double>;

//=======================================================================================
class CDlgStartThreadUI
{
public:
	CustomThread*	m_thread;
	bool			m_bdone;
	bool			m_cancelled;
	bool			m_breturn;
	QString			m_currentTask;
	QString			m_report;

public:
	QLabel*			m_task;
	QProgressBar*	m_progress;
	QPushButton*	m_stop;
	QLabel*			m_time;
	QLabel*			m_timeLeft;

	time_point<steady_clock>	m_start, m_tic;	//!< time at start

public:
	CDlgStartThreadUI()
	{
		m_bdone = false;
		m_breturn = false;
		m_thread = nullptr;
		m_cancelled = false;
	}

	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(new QLabel("Please wait until the operation completes."));
		l->addWidget(m_task = new QLabel(""));

		l->addWidget(m_progress = new QProgressBar);
		m_progress->setRange(0, 0);
		m_progress->setValue(0);

		QFormLayout* h1 = new QFormLayout;
		h1->addRow("Time elapsed: ", m_time = new QLabel); m_time->setAlignment(Qt::AlignLeft);
		h1->addRow("Time remaining: ", m_timeLeft = new QLabel); m_timeLeft->setAlignment(Qt::AlignLeft);
		l->addLayout(h1);

		QHBoxLayout* h = new QHBoxLayout;
		h->addStretch();
		h->addWidget(m_stop = new QPushButton("Cancel"));

		l->addLayout(h);
		dlg->setLayout(l);

		QObject::connect(m_stop, SIGNAL(clicked()), dlg, SLOT(cancel()));

	}
};


//=============================================================================
CDlgStartThread::CDlgStartThread(CMainWindow* parent, CustomThread* thread) : QDialog(parent), ui(new CDlgStartThreadUI)
{
	ui->setup(this);
	
	ui->m_thread = thread;

	if (parent)
		parent->ShowLogPanel();

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect(ui->m_thread, SIGNAL(resultReady(bool)), this, SLOT(threadFinished(bool)));
	QObject::connect(ui->m_thread, SIGNAL(taskChanged(QString)), ui->m_task, SLOT(setText(QString)));
	
	if (parent)
	{
		QObject::connect(ui->m_thread, SIGNAL(writeLog(QString)), parent, SLOT(AddLogEntry(QString)));
	}
}

void CDlgStartThread::setTask(const QString& taskString)
{
	ui->m_task->setText(taskString);
}

void CDlgStartThread::showEvent(QShowEvent* ev) 
{
	ui->m_start = ui->m_tic = steady_clock::now();
	ui->m_thread->start();
	QTimer::singleShot(500, this, SLOT(checkProgress()));
}

void CDlgStartThread::closeEvent(QCloseEvent* ev)
{
	if (ui->m_bdone == false)
	{
		cancel();
		ui->m_thread->wait();
	}
	QDialog::closeEvent(ev);
}

void CDlgStartThread::accept()
{
	QDialog::accept();
}

void CDlgStartThread::cancel()
{
	ui->m_cancelled = true;
	ui->m_stop->setEnabled(false);
	ui->m_task->setText("Cancelling operation. Please wait ...");
	ui->m_progress->reset();
	ui->m_progress->setRange(0, 0);
	ui->m_thread->stop();
}

QString sec2str(double fsec)
{
	int nhour = (int)(fsec / 3600.0); fsec -= nhour * 3600;
	int nmin = (int)(fsec / 60.0); fsec -= nmin * 60;
	int nsec = (int)(fsec + 0.5);

	char sz[64] = { 0 };
	sprintf(sz, "%d:%02d:%02d", nhour, nmin, nsec);

	return QString(sz);
}

void CDlgStartThread::checkProgress()
{
	time_point<steady_clock> pause = steady_clock::now();
	double fsec = duration_cast<dseconds>(pause - ui->m_start).count();
	ui->m_time->setText(sec2str(fsec));

	if (ui->m_bdone)
	{
		if (ui->m_thread)
		{
			if (ui->m_breturn == false)
			{
				QString err = ui->m_thread->GetErrorString();
				if (err.isEmpty()) err = "An unknown error has occurred.";
				QMessageBox::critical(this, "Error", err);
			}
			ui->m_thread->deleteLater();
			ui->m_thread = nullptr;

			if (ui->m_cancelled) ui->m_breturn = false;
			if (ui->m_breturn) accept(); else reject();
		}
	}
	else
	{
		if ((ui->m_cancelled == false))
		{
			// NOTE: There could be a race condition here. What
			//       if the task string is being updated while we get here? 
			const char* sztask = ui->m_thread->currentTask();
			if (sztask)
			{
				ui->m_currentTask = QString(sztask);
				ui->m_task->setText(ui->m_currentTask);
			}

			if (ui->m_thread->hasProgress())
			{
				ui->m_progress->setRange(0.0, 100.0);
				double p = ui->m_thread->progress();
				ui->m_progress->setValue((int)p);

				double f = p / 100.0;
				if (f > 0.01)
				{
					double fsec = duration_cast<dseconds>(pause - ui->m_tic).count();
					double frem = fsec * (1.0 - f) / f;
					ui->m_timeLeft->setText(sec2str(frem));
				}
			}
			else
			{
				ui->m_tic = pause;
				ui->m_timeLeft->setText("");
			}
		}

		QTimer::singleShot(250, this, SLOT(checkProgress()));
	}
}

void CDlgStartThread::threadFinished(bool b)
{
	ui->m_breturn = b;
	ui->m_bdone = true;
}

bool CDlgStartThread::GetReturnCode()
{
	return ui->m_breturn;
}
