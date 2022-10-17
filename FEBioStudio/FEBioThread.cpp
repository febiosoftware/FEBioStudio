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
		emit m_thread->sendLog(sz);
	}

private:
	CFEBioThread* m_thread;
};

CFEBioThread::CFEBioThread(CMainWindow* wnd, CFEBioJob* job, QObject* parent) : m_wnd(wnd), m_job(job)
{
	QObject::connect(this, SIGNAL(finished()), this, SIGNAL(QObject::deleteLater()));
	QObject::connect(this, SIGNAL(resultsReady(int, QProcess::ExitStatus)), parent, SLOT(onRunFinished(int, QProcess::ExitStatus)));
	QObject::connect(this, SIGNAL(sendLog(const QString&)), wnd, SLOT(updateOutput(const QString&)));
}

void CFEBioThread::run()
{
	// get the FEBio job file path
	string febFile = m_job->GetFEBFileName();

	// get ready ...
	m_wnd->AddLogEntry(QString("Starting FEBio: %1\n").arg(QString::fromStdString(febFile)));

	// set ...
	m_job->SetStatus(CFEBioJob::RUNNING);

	QString Cmd = QString::fromStdString(m_job->m_cmd);
	Cmd.replace("$(Filename)", QString::fromStdString(m_job->GetFEBFileName()));
	string cmd = Cmd.toStdString();

	// go!
	FEBioThreadOutput threadOutput(this);
	int n = FEBio::runModel(cmd, &threadOutput);

	emit resultsReady(n, QProcess::ExitStatus::NormalExit);
}

void CFEBioThread::KillThread()
{
	FEBio::TerminateRun();
}
