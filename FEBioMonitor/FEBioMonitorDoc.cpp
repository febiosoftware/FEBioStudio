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
#include "../FEBioStudio/PropertyList.h"
#include <FECore/FEModelParam.h>
#include <FECore/FEAnalysis.h>
#include <FECore/FEGlobalMatrix.h>
#include <FECore/FESolver.h>
#include <FECore/LinearSolver.h>
#include <FECore/FENewtonSolver.h>
#include "FEBioMonitorPanel.h"
#include "FEBioMonitorView.h"
#include "GLMonitorScene.h"
#include <QWaitCondition>

// winbase.h defines a GetCurrentTime macro, 
// so we need to undef this before we include FEModel.h
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif // GetCurrentTime


#include "../FEBioStudio/FEBioJobManager.h"
#include "../FEBioStudio/FEBioJob.h"
#include "FEBioReportDoc.h"


#include <FECore/FEModel.h>
#include <FECore/Timer.h>


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

FEBioMonitorThread::FEBioMonitorThread(FEBioMonitorDoc* doc, CFEBioJob* job) : m_doc(doc), m_job(job)
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
	fem.CreateReport(true);
	fem.GetLogFile().SetLogStream(new FEBLogStream(m_doc));
	fem.SetDebugLevel(m_doc->GetDebugLevel());

	fem.AddCallback(cb_processEvent, CB_ALWAYS, (void*)m_doc);

	if (fem.Input(filename.c_str()) == false)
	{
		emit jobFinished(false);
		return;
	}

	bool b = fem.Init();

	std::string plotFileName = fem.GetPlotFileName();
	m_job->SetPlotFileName(plotFileName);

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
	if (m_job)
	{
		m_job->m_jobReport = fem.GetReport();
		m_job->m_timingInfo = fem.GetTimingInfo();
		m_job->m_modelStats = fem.GetModelStats();
		m_job->m_stepStats = fem.GetStepStats();
	}

	emit jobFinished(b);
}

class FEBioMonitorDoc::Imp
{
public:
	QString febFile;
	QString	outputBuffer;
	bool	startPaused = false;
	bool	isOutputReady;
	bool	isStopped;
	bool	isRunning;
	bool	isPaused;
	bool	pauseRequested;
	bool	usePauseTime;
	double	pauseTime;
	double	progressPct;
	double	time;
	unsigned int pauseEvents;
	int		currentEvent;
	int		debugLevel = 0;
	bool	recordStates = false;
	int		updateEvents = Update_Major_Iters;
	QMutex	mutex;
	Timer	timer;
	FEBioModel* fem = nullptr;
	CFEBioJob* job = nullptr;

	QVector<FEBioWatchVariable*>	watches;
	FSConvergenceInfo conv;
};

FEBioMonitorDoc::FEBioMonitorDoc(CMainWindow* wnd) : CGLModelDocument(wnd), m(new FEBioMonitorDoc::Imp)
{
	SetDocTitle("[FEBio Monitor]");
	SetIcon(":/icons/febiomonitor.png");

	m->isOutputReady = false;
	m->isRunning = false;
	m->isPaused  = false;
	m->pauseRequested = false;
	m->isStopped = false;
	m->startPaused = false;
	m->progressPct = 0.0;
	m->pauseEvents = CB_ALWAYS;
	m->currentEvent = 0;
	m->usePauseTime = false;
	m->pauseTime = 0.0;
	m->time = 0.0;
	m->debugLevel = 0;
	m->recordStates = false;

	m_scene = new CGLMonitorScene(this);
	m_bValid = false;
	m_showTitle = true;
	m_showSubtitle = true;

	connect(this, &FEBioMonitorDoc::outputReady, this, &FEBioMonitorDoc::readOutput);
	connect(this, &FEBioMonitorDoc::updateViews, this, &FEBioMonitorDoc::onUpdateViews);
	connect(this, &FEBioMonitorDoc::modelInitialized, this, &FEBioMonitorDoc::onModelInitialized);
}

FEBioMonitorDoc::~FEBioMonitorDoc()
{
	CMainWindow* wnd = GetMainWindow();
	wnd->GetFEBioMonitorPanel()->Clear();
	qDeleteAll(m->watches);
	delete m;
}

void FEBioMonitorDoc::SetFEBioInputFile(QString febFile)
{
	m->febFile = febFile;
}

QString FEBioMonitorDoc::GetFEBioInputFile() const
{
	return m->febFile;
}

bool FEBioMonitorDoc::IsRunning() const
{
	return m->isRunning;
}

bool FEBioMonitorDoc::IsPaused() const
{
	return m->isPaused;
}

int FEBioMonitorDoc::GetCurrentEvent() const
{
	return m->currentEvent;
}

void FEBioMonitorDoc::StartPaused(bool b)
{
	m->startPaused = b;
}

bool FEBioMonitorDoc::StartPaused() const
{
	return m->startPaused;
}

void FEBioMonitorDoc::SetPauseEvents(unsigned int nevents)
{
	m->pauseEvents = nevents;
}

unsigned int FEBioMonitorDoc::GetPauseEvents() const
{
	return m->pauseEvents;
}

void FEBioMonitorDoc::SetPauseTime(double ftime, bool benable)
{
	m->pauseTime = ftime;
	m->usePauseTime = benable;
}

bool FEBioMonitorDoc::IsPauseTimeEnabled() const
{
	return m->usePauseTime;
}

double FEBioMonitorDoc::GetPauseTime() const
{
	return m->pauseTime;
}

int FEBioMonitorDoc::GetDebugLevel() const
{
	return m->debugLevel;
}

void FEBioMonitorDoc::SetDebugLevel(int n)
{
	m->debugLevel = n;
	if (m->fem) m->fem->SetDebugLevel(n);
}

void FEBioMonitorDoc::SetRecordStatesFlag(bool b)
{
	m->recordStates = b;
}

bool FEBioMonitorDoc::GetRecordStatesFlag() const
{
	return m->recordStates;
}

void FEBioMonitorDoc::SetUpdateEvents(int updateEvents)
{
	m->updateEvents = updateEvents;
}

int FEBioMonitorDoc::GetUpdateEvents() const
{
	return m->updateEvents;
}

void FEBioMonitorDoc::RunJob()
{
	if (m->isRunning)
	{
		if (m->isPaused)
		{
			m->pauseRequested = false;
			jobIsPaused.wakeAll();
			return;
		}
		else assert(false);
		return;
	}

	GetMainWindow()->ClearOutput();

	m->isOutputReady = false;
	m->isRunning = true;
	m->isPaused = false;
	m->pauseRequested = m->startPaused;
	m->isStopped = false;
	m->progressPct = 0.0;
	m->timer.reset();
	m->timer.start();
	assert(m->job == nullptr);
	m->job = new CFEBioJob(this);
	m->job->SetName("febio monitor");
	m->job->SetFEBFileName(GetFEBioInputFile().toStdString());
	m->job->StartTimer();
	CFEBioJob::SetActiveJob(m->job);

	GetMainWindow()->GetFEBioMonitorView()->Update(true);

	FEBioMonitorThread* thread = new FEBioMonitorThread(this, m->job);
	thread->start();
}

void FEBioMonitorDoc::FEBioMonitorDoc::KillJob()
{
	if (m->isRunning) m->isStopped = true;
	if (m->isPaused)
	{
		m->pauseRequested = false;
		m->usePauseTime = false;
		jobIsPaused.wakeAll();
	}
	updateWindowTitle();
}

void FEBioMonitorDoc::PauseJob()
{
	QMutexLocker lock(&m->mutex);
	if (m->isRunning && !m->isStopped)
		m->pauseRequested = true;
	updateWindowTitle();
}

void FEBioMonitorDoc::AdvanceJob()
{
	QMutexLocker lock(&m->mutex);
	if (m->isRunning && m->isPaused)
	{
		jobIsPaused.wakeAll();
	}
	updateWindowTitle();
}

void FEBioMonitorDoc::onJobFinished(bool b)
{
	m->timer.stop();
	m->isRunning = false;

	assert(m->job);
	m->job->StopTimer();
	m->job->SetStatus(b ? CFEBioJob::COMPLETED : CFEBioJob::FAILED);

	if (m->isStopped)
	{
		QMessageBox::information(m_wnd, "FEBio Studio", "Job cancelled.");
	}
	else
	{
		CDlgJobReport dlg(GetMainWindow());
		dlg.SetFEBioJob(m->job);
		if (dlg.exec())
		{
			GetMainWindow()->OpenFile(QString::fromStdString(m->job->GetPlotFileName()), false, false);
		}
	}

	// generate detailed report
	CMainWindow* wnd = GetMainWindow();
	CFEBioReportDoc* doc = new CFEBioReportDoc(wnd);
	doc->setJob(m->job);
	wnd->AddDocument(doc);

	delete m->job;
	m->job = nullptr;

	updateWindowTitle();
}

void FEBioMonitorDoc::appendLog(const char* sz)
{
	if ((sz == nullptr) || (sz[0] == 0)) return;

	bool doEmit = false;

	m->mutex.lock();
	if (m->outputBuffer.isEmpty()) m->outputBuffer = sz;
	else m->outputBuffer.append(sz);
	if (m->isOutputReady == false)
	{
		m->isOutputReady = true;
		doEmit = true;
	}
	m->mutex.unlock();

	if (doEmit)
	{
		emit outputReady();
	}
}

void FEBioMonitorDoc::readOutput()
{
	QString s;
	m->mutex.lock();
	s = m->outputBuffer;
	m->outputBuffer.clear();
	m->isOutputReady = false;
	m->mutex.unlock();

	m_wnd->AddOutputEntry(s);

	updateWindowTitle();
}

void FEBioMonitorDoc::updateWindowTitle()
{
	double p = m->progressPct;
	int pn = (int)(10.0 * p);
	p = pn / 10.0;

	if (m->job) m->job->SetProgress(p);
	m_wnd->UpdateTitle();
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

Post::CGLModel* FEBioMonitorDoc::GetGLModel()
{
	if (IsValid() == false) return nullptr;
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(GetScene());
	if (scene == nullptr) return nullptr;
	return scene->GetGLModel();
}

Post::FEPostModel* FEBioMonitorDoc::GetFSModel()
{
	if (IsValid() == false) return nullptr;
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(GetScene());
	if (scene == nullptr) return nullptr;
	return scene->GetFSModel();
}

GObject* FEBioMonitorDoc::GetActiveObject()
{
	if (!IsValid()) return nullptr;
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(GetScene());
	if (scene == nullptr) return nullptr;
	return scene->GetPostObject();
}

double FEBioMonitorDoc::GetTimeValue() const
{
	return m->time;
}

void FEBioMonitorDoc::SetProgress(double p)
{
	m->progressPct = p;
}

bool FEBioMonitorDoc::AddDataField(const std::string& dataField)
{
	if (m->isPaused == false) return false;
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(m_scene);
	if (scene == nullptr) return false;
	return scene->AddDataField(dataField);
}

void FEBioMonitorDoc::SetCurrentState(int n)
{
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(m_scene);
	if (scene == nullptr) return;
	scene->GetFSModel()->SetCurrentTimeIndex(n);
	scene->UpdateScene();
}

QString eventToString(int nevent)
{
	QString s;
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
	case CB_TIMESTEP_FAILED : s = "TIMESTEP_FAILED"; break;
	case CB_USER1           : s = "USER1"; break;
	}
	return s;
}

bool FEBioMonitorDoc::processFEBioEvent(FEModel* fem, int nevent)
{
	m->time = fem->GetTime().currentTime;
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(m_scene);
	m->currentEvent = nevent;

	m->fem = dynamic_cast<FEBioModel*>(fem);

	bool updateGL = false;
	if (nevent == CB_INIT)
	{
		m->mutex.lock();
		m->conv.clear();
		m->mutex.unlock();
		scene->InitScene(fem);
		m_bValid = true;
		emit modelInitialized();
	}
	else
	{
		if (nevent == CB_MINOR_ITERS) UpdateConvergenceInfo();

		switch (m->updateEvents)
		{
		case Update_Major_Iters: updateGL = (nevent == CB_MAJOR_ITERS); break;
		case Update_Minor_Iters: updateGL = (nevent == CB_MINOR_ITERS); break;
		case Update_All_Events : updateGL = true; break;
		}

		if (updateGL)
		{
			if (m->recordStates) scene->AddState();
			scene->UpdateStateData();
		}
	}

	// NOTE: Even if we cancel the run,
	// the CB_SOLVED event is still triggered, which will reset the progress.
	// so we check for this
	if (nevent == CB_INIT) SetProgress(0.0);
	else if (!m->isStopped) SetProgress(calculateFEBioProgressInPercent(fem));

	if (nevent == CB_INIT) InitDefaultWatchVariables();
	UpdateAllWatchVariables();
	emit updateViews(false, updateGL);
	m->mutex.lock();

	constexpr double eps = std::numeric_limits<double>::epsilon();
	if ((m->pauseRequested && (m->pauseEvents & nevent)) ||
		(m->usePauseTime && (m->time + eps >= m->pauseTime)))
	{
		m->outputBuffer += "\n[paused on " + eventToString(nevent) + "]\n";
		emit outputReady();
		emit updateViews(true, true);
		m->isPaused = true;
		jobIsPaused.wait(&m->mutex);
		m->isPaused = false;
	}
	m->fem = nullptr;
	m->mutex.unlock();

	if (m->isStopped) throw std::exception();
	m->currentEvent = 0;
	return true;
}

void FEBioMonitorDoc::onModelInitialized()
{
	QMutexLocker lock(&m->mutex);
	CMainWindow* wnd = GetMainWindow();
	CFEBioMonitorPanel* monitorPanel = wnd->GetFEBioMonitorPanel(); assert(monitorPanel);
	if (monitorPanel == nullptr) return;
	monitorPanel->Update(true);
	wnd->GetFEBioMonitorView()->Update(true);
	wnd->UpdateGLControlBar();
	wnd->RedrawGL();
}

void FEBioMonitorDoc::onUpdateViews(bool updatePanel, bool updateGL)
{
	QMutexLocker lock(&m->mutex);

	CMainWindow* wnd = GetMainWindow();
	if (updateGL) wnd->GetGLView()->updateView();
	wnd->GetFEBioMonitorPanel()->Update(updatePanel);
	wnd->GetFEBioMonitorView()->Update(false);
}

const FEBioWatchVariable* FEBioMonitorDoc::GetWatchVariable(int n)
{
	return m->watches[n];
}

int FEBioMonitorDoc::GetWatchVariables() const
{
	return m->watches.size();
}

FEBioWatchVariable* FEBioMonitorDoc::AddWatchVariable(const QString& name)
{
	QMutexLocker lock(&m->mutex);
	if (!IsRunning() || !IsPaused()) return nullptr;
	FEBioWatchVariable* v = new FEBioWatchVariable(name);
	m->watches.append(v);
	UpdateWatchVariable(*v);
	return v;
}

void FEBioMonitorDoc::SetWatchVariable(int n, const QString& name)
{
	QMutexLocker lock(&m->mutex);
	if (!IsRunning() || !IsPaused()) return;
	FEBioWatchVariable* var = m->watches[n];
	var->setName(name);
	UpdateWatchVariable(*var);
}

void FEBioMonitorDoc::InitDefaultWatchVariables()
{
	m->watches.clear();
	if (m->fem == nullptr) return;
	int N = m->fem->LoadParams();
	for (int i = 0; i < N; ++i)
	{
		FEParam* pi = m->fem->GetLoadParam(i);
		if (pi)
		{
			string s = m->fem->GetParamString(pi);
			if (!s.empty()) m->watches.append(new FEBioWatchVariable(QString::fromStdString(s)));
		}
	}
}

void FEBioMonitorDoc::UpdateAllWatchVariables()
{
	if (m->fem == nullptr) return;
	for (auto w : m->watches) UpdateWatchVariable(*w);
}

void FEBioMonitorDoc::UpdateWatchVariable(FEBioWatchVariable& var)
{
	if (m->fem == nullptr) return;

	var.setType(FEBioWatchVariable::INVALID);
	std::string s = var.name().toStdString();
	ParamString ps(s.c_str());
	FEParam* p = m->fem->FindParameter(ps);
	if (p)
	{
		QString val("[can't display]");
		switch (p->type())
		{
		case FE_PARAM_INT   : { int a = p->value<int>(); val = QString::number(a); } break;
		case FE_PARAM_BOOL  : { bool a = p->value<bool>(); val = (a ? "True" : "False"); } break;
		case FE_PARAM_DOUBLE: { double a = p->value<double>(); val = QString::number(a); } break;
		case FE_PARAM_VEC2D : { vec2d v = p->value<vec2d>(); val = Vec2dToString(v); } break;
		case FE_PARAM_VEC3D : { vec3d v = p->value<vec3d>(); val = Vec3dToString(v); } break;
		case FE_PARAM_MAT3D : { mat3d v = p->value<mat3d>(); val = Mat3dToString(v); } break;
		case FE_PARAM_MAT3DS: { mat3ds v = p->value<mat3ds>(); val = Mat3dsToString(v); } break;
		case FE_PARAM_DOUBLE_MAPPED:
		{
			FEParamDouble& v = p->value<FEParamDouble>();
			if (v.isConst())
			{
				double a = v.constValue()*v.GetScaleFactor();
				val = QString::number(a);
			}
		}
		break;
		case FE_PARAM_VEC3D_MAPPED:
		{ 
			FEParamVec3& v = p->value<FEParamVec3>();
			if (v.isConst()) 
			{
				vec3d a = v.constValue() * v.GetScaleFactor();
				val = Vec3dToString(a);
			}
		}
		break;
		case FE_PARAM_MAT3D_MAPPED:
		{
			FEParamMat3d& v = p->value<FEParamMat3d>();
			if (v.isConst())
			{
				mat3d a = v.constValue() * v.GetScaleFactor();
				val = Mat3dToString(a);
			}
		}
		break;
		case FE_PARAM_MAT3DS_MAPPED:
		{
			FEParamMat3ds& v = p->value<FEParamMat3ds>();
			if (v.isConst())
			{
				mat3ds a = v.constValue() * v.GetScaleFactor();
				val = Mat3dsToString(a);
			}
		}
		break;
		}
		var.setValue(val);
		var.setType(FEBioWatchVariable::VALID);
	}
}

FEGlobalMatrix* FEBioMonitorDoc::GetStiffnessMatrix()
{
	if (IsPaused() == false) return nullptr;
	if (m->fem == nullptr) return nullptr;

	FEAnalysis* step = m->fem->GetCurrentStep();
	if (step == nullptr) return nullptr;

	FESolver* solver = step->GetFESolver();
	FEGlobalMatrix* K = solver->GetStiffnessMatrix();
	return K;
}

double FEBioMonitorDoc::GetConditionNumber()
{
	if (IsPaused() == false) return 0.0;
	if (m->fem == nullptr) return 0.0;

	FEAnalysis* step = m->fem->GetCurrentStep();
	if (step == nullptr) return 0.0;

	FESolver* solver = step->GetFESolver();
	if (solver == nullptr) return 0.0;

	LinearSolver* ls = solver->GetLinearSolver();
	if (ls)
	{
		return ls->ConditionNumber();
	}
	return 0.0;
}

void FEBioMonitorDoc::UpdateConvergenceInfo()
{
	QMutexLocker lock(&m->mutex);

	if (m->fem == nullptr) return;
	FEAnalysis* step = m->fem->GetCurrentStep();
	if (step == nullptr) return;
	FENewtonSolver* solver = dynamic_cast<FENewtonSolver*>(step->GetFESolver());
	if (solver == nullptr) return;

	FSConvergenceInfo& info = m->conv;

	ConvergenceInfo Rnorm = solver->GetResidualConvergence();
	ConvergenceInfo Enorm = solver->GetEnergyConvergence();
	ConvergenceInfo Unorm = solver->GetSolutionConvergence(0);

	info.R0 = Rnorm.norm0;
	info.Rt.push_back(Rnorm.norm);
	info.E0 = Enorm.norm0;
	info.Et.push_back(Enorm.norm);
	info.U0 = Unorm.norm0;
	info.Ut.push_back(Unorm.norm);
}

FSConvergenceInfo& FEBioMonitorDoc::GetConvergenceInfo()
{
	return m->conv;
}
