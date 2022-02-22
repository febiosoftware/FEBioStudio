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
#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QPushButton>
#include <QTimer>

//=======================================================================================
CustomThread::CustomThread()
{

}

bool CustomThread::hasProgress()
{
	return false;
}

double CustomThread::progress()
{
	return 0.0;
}

const char* CustomThread::currentTask()
{
	return "";
}

void CustomThread::stop()
{

}

//=======================================================================================
class CDlgStartThreadUI
{
public:
	CustomThread*	m_thread;
	bool			m_bdone;
	bool			m_cancelled;
	bool			m_breturn;
	const char*		m_szcurrentTask;

public:
	QLabel*			m_task;
	QProgressBar*	m_progress;
	QPushButton*	m_stop;

public:
	CDlgStartThreadUI()
	{
		m_bdone = false;
		m_breturn = false;
		m_thread = nullptr;
		m_cancelled = false;
		m_szcurrentTask = 0;
	}

	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(new QLabel("Please wait until the operation completes."));
		l->addWidget(m_task = new QLabel(""));

		l->addWidget(m_progress = new QProgressBar);
		m_progress->setRange(0, 0);
		m_progress->setValue(0);

		QHBoxLayout* h = new QHBoxLayout;
		h->addStretch();
		h->addWidget(m_stop = new QPushButton("Cancel"));

		l->addLayout(h);
		dlg->setLayout(l);

		QObject::connect(m_stop, SIGNAL(clicked()), dlg, SLOT(cancel()));

	}
};


//=============================================================================
CDlgStartThread::CDlgStartThread(QWidget* parent, CustomThread* thread) : QDialog(parent), ui(new CDlgStartThreadUI)
{
	ui->setup(this);
	
	ui->m_thread = thread;

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect(ui->m_thread, SIGNAL(resultReady(bool)), this, SLOT(threadFinished(bool)));

	ui->m_thread->start();
	QTimer::singleShot(100, this, SLOT(checkProgress()));
}

void CDlgStartThread::setTask(const QString& taskString)
{
	ui->m_task->setText(taskString);
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

void CDlgStartThread::checkProgress()
{
	if (ui->m_bdone)
	{
		if (ui->m_cancelled)
		{
			ui->m_breturn = false;
			reject();
		}
		else accept();
	}
	else if (ui->m_cancelled == false)
	{
		if (ui->m_thread->hasProgress())
		{
			ui->m_progress->setRange(0.0, 100.0);
			double p = ui->m_thread->progress();
			ui->m_progress->setValue((int)p);

			const char* sztask = ui->m_thread->currentTask();
			if (sztask && (sztask != ui->m_szcurrentTask))
			{
				ui->m_szcurrentTask = sztask;
				ui->m_task->setText(ui->m_szcurrentTask);
			}
		}

		QTimer::singleShot(100, this, SLOT(checkProgress()));
	}
}

void CDlgStartThread::threadFinished(bool b)
{
	ui->m_bdone = true;
	ui->m_breturn = b;
	ui->m_thread->deleteLater();
	checkProgress();
}

bool CDlgStartThread::GetReturnCode()
{
	return ui->m_breturn;
}
