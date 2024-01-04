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
#include <QMessageBox>
#include "FEBioMonitorDoc.h"
#include "../FEBioStudio/MainWindow.h"
#include "../FEBioStudio/GLView.h"
#include "GLMonitorScene.h"
#include <QWaitCondition>

// winbase.h defines a GetCurrentTime macro, 
// so we need to undef this before we include FEModel.h
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif // GetCurrentTime

#include <FECore/FEModel.h>


// NOTE: make sure this is included last, 
// otherwise we may get compilation errors related to Qt
#include <FEBioLib/FEBioModel.h>
#include <FECore/log.h>

QWaitCondition jobIsPaused;

class FEBLogStream : public LogStream
{
public:
	FEBLogStream(FEBioMonitorDoc* doc) : m_doc(doc) {}

	void print(const char* sz) override
	{
		m_doc->appendLog(sz);
	}

private:
	FEBioMonitorDoc* m_doc;
};

FEBioMonitorThread::FEBioMonitorThread(FEBioMonitorDoc* doc) : m_doc(doc)
{
	connect(this, &QThread::finished, this, &QObject::deleteLater);
	connect(this, &FEBioMonitorThread::jobFinished, doc, &FEBioMonitorDoc::onJobFinished);
}

bool cb_processEvent(FEModel* fem, unsigned int nwhen, void* pd)
{
	FEBioMonitorDoc* doc = (FEBioMonitorDoc*)pd;
	return doc->processFEBioEvent(fem, nwhen);
}

void FEBioMonitorThread::run()
{
	std::string filename = m_doc->GetFEBioInputFile().toStdString();
	if (filename.empty()) {
		emit jobFinished(false);
		return;
	}

	FEBioModel fem;
	fem.GetLogFile().SetLogStream(new FEBLogStream(m_doc));

	fem.AddCallback(cb_processEvent, CB_ALWAYS, (void*)m_doc);

	if (fem.Input(filename.c_str()) == false)
	{
		emit jobFinished(false);
		return;
	}

	bool b = fem.Init();
	if (b == false)
	{
		emit jobFinished(false);
		return;
	}

	try {
		b = fem.Solve();
	}
	catch (...)
	{
		b = false;
	}

	emit jobFinished(b);
}

FEBioMonitorDoc::FEBioMonitorDoc(CMainWindow* wnd) : CGLDocument(wnd)
{
	SetDocTitle("[FEBio Monitor]");
	m_isOutputReady = false;
	m_isRunning = false;
	m_isPaused  = false;
	m_isStopped = false;
	m_startPaused = false;
	m_progressPct = 0.0;
	m_scene = new CGLMonitorScene(this);

	m_bValid = false;
	m_time = 0.0;

	CGLView* view = wnd->GetGLView();

	connect(this, &FEBioMonitorDoc::outputReady, this, &FEBioMonitorDoc::readOutput);
	connect(this, &FEBioMonitorDoc::updateView, view, &CGLView::updateView);
}

FEBioMonitorDoc::~FEBioMonitorDoc()
{

}

void FEBioMonitorDoc::SetFEBioInputFile(QString febFile)
{
	assert(m_febFile.isEmpty());
	m_febFile = febFile;
}

QString FEBioMonitorDoc::GetFEBioInputFile() const
{
	return m_febFile;
}

bool FEBioMonitorDoc::IsRunning() const
{
	return m_isRunning;
}

bool FEBioMonitorDoc::IsPaused() const
{
	return m_isPaused;
}

void FEBioMonitorDoc::StartPaused(bool b)
{
	m_startPaused = b;
}

void FEBioMonitorDoc::RunJob()
{
	if (m_isRunning)
	{
		if (m_isPaused)
		{
			m_isPaused = false;
			jobIsPaused.wakeAll();
			return;
		}
		else assert(false);
		return;
	}

	m_isOutputReady = false;
	m_isRunning = true;
	m_isPaused  = m_startPaused;
	m_isStopped = false;
	m_progressPct = 0.0;
	FEBioMonitorThread* thread = new FEBioMonitorThread(this);
	thread->start();
}

void FEBioMonitorDoc::FEBioMonitorDoc::KillJob()
{
	if (m_isRunning) m_isStopped = true;
	if (m_isPaused)
	{
		m_isPaused = false;
		jobIsPaused.wakeAll();
	}
	updateWindowTitle();
}

void FEBioMonitorDoc::PauseJob()
{
	QMutexLocker lock(&m_mutex);
	if (m_isRunning && !m_isStopped)
		m_isPaused = true;
	updateWindowTitle();
}

void FEBioMonitorDoc::AdvanceJob()
{
	QMutexLocker lock(&m_mutex);
	if (m_isRunning && m_isPaused)
	{
		jobIsPaused.wakeAll();
	}
	updateWindowTitle();
}

void FEBioMonitorDoc::onJobFinished(bool b)
{
	m_isRunning = false;

	if (m_isStopped)
	{
		QMessageBox::information(m_wnd, "FEBio Studio", "Job cancelled.");
	}
	else
	{
		if (b)
			QMessageBox::information(m_wnd, "FEBio Studio", "NORMAL TERMINATION");
		else
			QMessageBox::critical(m_wnd, "FEBio Studio", "ERROR TERMINATION");
	}
	updateWindowTitle();
}

void FEBioMonitorDoc::appendLog(const char* sz)
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

void FEBioMonitorDoc::readOutput()
{
	QString s;
	m_mutex.lock();
	s = m_outputBuffer;
	m_outputBuffer.clear();
	m_isOutputReady = false;
	m_mutex.unlock();

	m_wnd->AddOutputEntry(s);

	updateWindowTitle();
}

void FEBioMonitorDoc::updateWindowTitle()
{
	double p = m_progressPct;
	int pn = (int)(10.0 * p);
	p = pn / 10.0;

	QString status;
	if      (m_isPaused ) status = "PAUSED";
	else if (m_isStopped) status = "STOPPED";
	else if (m_isRunning) status = "RUNNING";
	else status = "READY";

	m_wnd->setWindowTitle(QString("[%1: %2 %]").arg(status).arg(p));
}

double calculateFEBioProgressInPercent(FEModel* pfem)
{
	FEBioModel& fem = dynamic_cast<FEBioModel&>(*pfem);

	// get the number of steps
	int nsteps = fem.Steps();

	// calculate progress
	double starttime = fem.GetStartTime();
	double endtime = fem.GetEndTime();
	double f = 0.0;
	double ftime = pfem->GetCurrentTime();
	if (endtime != starttime) f = (ftime - starttime) / (endtime - starttime);
	else
	{
		// this only happens (I think) when the model is solved
		f = 1.0;
	}

	double pct = 0.0;
	if (nsteps > 1)
	{
		int N = nsteps;
		int n = fem.GetCurrentStepIndex();
		pct = 100.0 * ((double)n + f) / (double)N;
	}
	else
	{
		pct = 100.0 * f;
	}

	return pct;
}

double FEBioMonitorDoc::GetTimeValue() const
{
	return m_time;
}

void FEBioMonitorDoc::SetProgress(double p)
{
	m_progressPct = p;
}

QString eventToString(int nevent)
{
	QString s = "<unknown>";
	switch (nevent)
	{
	case CB_INIT            : s = "INIT"; break;
	case CB_STEP_ACTIVE     : s = "STEP_ACTIVE"; break;
	case CB_MAJOR_ITERS     : s = "MAJOR_ITERS"; break;
	case CB_MINOR_ITERS     : s = "MINOR_ITERS"; break;
	case CB_SOLVED          : s = "SOLVED"; break;
	case CB_UPDATE_TIME     : s = "UPDATE_TIME"; break;
	case CB_AUGMENT         : s = "AUGMENT"; break;
	case CB_STEP_SOLVED     : s = "STEP_SOLVED"; break;
	case CB_MATRIX_REFORM   : s = "MATRIX_REFORM"; break;
	case CB_REMESH          : s = "REMESH"; break;
	case CB_PRE_MATRIX_SOLVE: s = "PRE_MATRIX_SOLVE"; break;
	case CB_RESET           : s = "RESET"; break;
	case CB_MODEL_UPDATE    : s = "MODEL_UPDATE"; break;
	case CB_TIMESTEP_SOLVED : s = "TIMESTEP_SOLVED"; break;
	case CB_SERIALIZE_SAVE  : s = "SERIALIZE_SAVE"; break;
	case CB_SERIALIZE_LOAD  : s = "SERIALIZE_LOAD"; break;
	case CB_USER1           : s = "USER1"; break;
	}
	return s;
}

bool FEBioMonitorDoc::processFEBioEvent(FEModel* fem, int nevent)
{
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(m_scene);
	switch (nevent)
	{
	case CB_INIT:
		scene->InitScene(fem);
		m_bValid = true;
		break;
	case CB_MAJOR_ITERS:
	case CB_MINOR_ITERS:
		m_time = fem->GetTime().currentTime;
		scene->UpdateScene();
		break;
	}
	emit updateView();

	// NOTE: Even if we cancel the run,
	// the CB_SOLVED event is still triggered, which will reset the progress.
	// so we check for this
	if (nevent == CB_INIT) SetProgress(0.0);
	else if (!m_isStopped) SetProgress(calculateFEBioProgressInPercent(fem));

	m_mutex.lock();
	if (m_isPaused)
	{
		m_outputBuffer += "\n[paused in " + eventToString(nevent) + "]\n";
		emit outputReady();

		jobIsPaused.wait(&m_mutex);
	}
	m_mutex.unlock();

	if (m_isStopped) throw std::exception();

	return true;
}
