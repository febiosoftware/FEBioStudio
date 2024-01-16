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
#include "FEBioThread.h"
#include "FEBioJob.h"
#include "MainWindow.h"
#include <FEBioLink/FEBioClass.h>
#include <string>
#include <QFileInfo>
using namespace std;

class FEBioThreadOutput : public FEBio::FEBioOutputHandler
{
public:
	FEBioThreadOutput(CFEBioThread* thread) : m_thread(thread) {}

	void write(const char* sz) override
	{
		m_thread->appendLog(sz);
	}

private:
	CFEBioThread* m_thread;
};

class FEBioThreadProgress : public FEBio::FEBioProgressTracker
{
public:
	FEBioThreadProgress(CFEBioJob* job) : m_job(job) {}
	void SetProgress(double pct) override { m_job->SetProgress(pct); }

private:
	CFEBioJob* m_job;
};

CFEBioThread::CFEBioThread(CMainWindow* wnd, CFEBioJob* job, QObject* parent) : m_wnd(wnd), m_job(job)
{
	m_isOutputReady = false;

	QObject::connect(this, SIGNAL(finished()), this, SIGNAL(QObject::deleteLater()));
	QObject::connect(this, SIGNAL(resultsReady(int, QProcess::ExitStatus)), parent, SLOT(onRunFinished(int, QProcess::ExitStatus)));
	QObject::connect(this, SIGNAL(outputReady()), parent, SLOT(onReadyRead()));
}

void CFEBioThread::run()
{
	// get the FEBio job file path
	string febFile = m_job->GetFEBFileName();

	// get ready ...
	m_wnd->AddLogEntry(QString("Starting FEBio: %1\n").arg(QString::fromStdString(febFile)));

	// set ...
	m_job->SetStatus(CFEBioJob::RUNNING);
	m_job->SetProgress(0.0);

	QString Cmd = QString::fromStdString(m_job->m_cmd);

	// get the job file name (NOTE that we put quotes around it to deal with spaces)
	QString fileName = QString("\"%1\"").arg(QString::fromStdString(m_job->GetFEBFileName()));

	Cmd.replace("$(Filename)", fileName);
	string cmd = Cmd.toStdString();

	// go!
	FEBioThreadOutput threadOutput(this);
	FEBioThreadProgress progressTracker(m_job);
	int n = FEBio::runModel(cmd, &threadOutput, &progressTracker);

	emit resultsReady(n, QProcess::ExitStatus::NormalExit);
}

void CFEBioThread::KillThread()
{
	FEBio::TerminateRun();
}

void CFEBioThread::appendLog(const char* sz)
{
	if ((sz == nullptr) || (sz[0] == 0)) return;

	bool doEmit = false;

	m_mutex.lock();
	if (m_outputBuffer.isEmpty()) m_outputBuffer = sz;
	else m_outputBuffer.append(sz);
	if (m_isOutputReady == false)
	{
		m_isOutputReady = true;
		doEmit = true;
	}
	m_mutex.unlock();

	if (doEmit)
	{
		emit outputReady();
	}
}

QString CFEBioThread::GetOutput()
{
	QString s;
	m_mutex.lock();
	s = m_outputBuffer;
	m_outputBuffer.clear();
	m_isOutputReady = false;
	m_mutex.unlock();
	return s;
}
