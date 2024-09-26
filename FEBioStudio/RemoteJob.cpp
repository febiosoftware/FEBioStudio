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
#include "RemoteJob.h"
#include "MainWindow.h"
#ifdef HAS_SSH
#include "SSHHandler.h"
#endif

class CRemoteJob::Imp 
{
public:
	CFEBioJob* job = nullptr;
	double progress = 0;
	CMainWindow* wnd;
#ifdef HAS_SSH
	CSSHHandler* sshHandler = nullptr;
#endif

	Imp() {}
	~Imp() { delete sshHandler; }
};

CRemoteJob::CRemoteJob(CFEBioJob* job, CLaunchConfig* lc, CMainWindow* wnd) : m(*(new CRemoteJob::Imp))
{ 
	m.job = job;
	m.wnd = wnd;
#ifdef HAS_SSH
	m.sshHandler = new CSSHHandler(job, lc);
	QObject::connect(m.sshHandler, &CSSHHandler::ShowProgress, wnd, &CMainWindow::ShowProgress);
	QObject::connect(m.sshHandler, &CSSHHandler::UpdateProgress, this, &CRemoteJob::updateProgress);
	QObject::connect(m.sshHandler, &CSSHHandler::sessionFinished, this, &CRemoteJob::sessionEnded);
#endif
}

CRemoteJob::~CRemoteJob() { delete &m; }

CFEBioJob* CRemoteJob::GetFEBioJob()
{
	return m.job;
}

void CRemoteJob::updateProgress(double pct)
{
	if (pct > m.progress)
	{
		m.progress = pct;
		m.job->SetProgress(pct);
		m.wnd->UpdateTitle();
	}
}

void CRemoteJob::GetRemoteFiles()
{
#ifdef HAS_SSH
	m.sshHandler->RequestRemoteFiles();
#endif
}

void CRemoteJob::GetQueueStatus()
{
#ifdef HAS_SSH
	m.sshHandler->RequestQueueStatus();
#endif
}

void CRemoteJob::StartRemoteJob()
{
#ifdef HAS_SSH
	m.sshHandler->StartRemoteSession();
#endif
}

void CRemoteJob::sessionEnded()
{
	emit jobFinished();
}
