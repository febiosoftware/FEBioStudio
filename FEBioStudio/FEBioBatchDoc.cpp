/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include "FEBioBatchDoc.h"
#include "LocalJobProcess.h"
#include "FEBioJob.h"
#include <FEBioMonitor/FEBioReportDoc.h>
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
#include <chrono>
#include <queue>
#include "MainWindow.h"
using namespace std::chrono;

class FEBioBatchProcess : public CLocalJobProcess
{
public:
	FEBioBatchProcess(CMainWindow* wnd, QObject* parent) : CLocalJobProcess(wnd, &job, "febio4", parent), job(nullptr) {}
	CFEBioJob job;
	int jobId = -1;
};

class FEBioBatchDoc::Impl
{
public:
	enum {MAX_PROCESSES = 32};

public:
	std::vector<JobInfo> fileList;
	std::queue<JobInfo*> jobQueue;
	double	m_tic = 0, m_toc = 0;

	FEBioBatchProcess* processes[MAX_PROCESSES] = { nullptr };

	Options options;

	void deleteProcess(FEBioBatchProcess* p)
	{
		bool bfound = false;
		for (int i = 0; i < MAX_PROCESSES; ++i)
		{
			if (p == processes[i])
			{
				processes[i] = nullptr;
				bfound = true;
				break;
			}
		}
		assert(bfound);
		delete p;
	}

	void StartTimer()
	{
		time_point<steady_clock> tp = steady_clock::now();
		m_tic = m_toc = duration_cast<duration<double>>(tp.time_since_epoch()).count();
	}

	void StopTimer()
	{
		time_point<steady_clock> tp = steady_clock::now();
		m_toc = duration_cast<duration<double>>(tp.time_since_epoch()).count();
	}

	double ElapsedTime() const { return m_toc - m_tic; }
};

FEBioBatchDoc::FEBioBatchDoc(CMainWindow* wnd) : CDocument(wnd), m(*new Impl)
{
	static int n = 1;
	SetDocTitle(QString::asprintf("Batch%d", n++).toStdString());
}

FEBioBatchDoc::~FEBioBatchDoc()
{
}

FEBioBatchDoc::Options FEBioBatchDoc::GetOptions() const { return m.options; }
void FEBioBatchDoc::SetOptions(const Options& opt) { m.options = opt; }

void FEBioBatchDoc::SetFileList(const QStringList& fileList)
{
	m.fileList.clear();
	for (const QString& s : fileList)
	{
		JobInfo di;
		di.fileName = s;
		di.status = IDLE;
		di.jobId = (int)m.fileList.size();
		m.fileList.push_back(di);
	}
	SetModifiedFlag(true);
}

int FEBioBatchDoc::Files() const
{
	return (int)m.fileList.size();
}

std::vector<FEBioBatchDoc::JobInfo> FEBioBatchDoc::GetFileList() const 
{ 
	return m.fileList; 
}

FEBioBatchDoc::JobInfo FEBioBatchDoc::GetJobInfo(int jobId) const
{
	JobInfo job;
	if ((jobId >= 0) && (jobId < m.fileList.size())) job = m.fileList[jobId];
	return job;
}

void FEBioBatchDoc::StartBatch()
{
	if (!m.jobQueue.empty()) return;

	std::vector<int> jobIDs;
	for (int i = 0; i < m.fileList.size(); ++i) jobIDs.push_back(i);
	StartBatch(jobIDs);
}

void FEBioBatchDoc::StartBatch(const std::vector<int>& jobIDs)
{
	if (jobIDs.empty()) return;

	if (!m.jobQueue.empty()) return;

	for (int n : jobIDs)
	{
		if ((n >= 0) && (n < m.fileList.size()))
		{
			m.fileList[n].stats.clear();
			m.fileList[n].status = PENDING;
			m.jobQueue.push(&m.fileList[n]);
		}
	}

	CMainWindow* wnd = GetMainWindow();
	if (wnd) wnd->AddLogEntry(QString(">>> Starting batch run ...\n"));

	m.StartTimer();

	// start the next pending job
	StartNextJob();
}

void FEBioBatchDoc::StartNextJob()
{
	if (m.jobQueue.empty())
	{
		// check if all processes are done
		for (int l = 0; l < m.options.nprocs; ++l)
			if (m.processes[l]) return;

		// we're done
		m.StopTimer();

		CMainWindow* wnd = GetMainWindow();
		if (wnd) wnd->AddLogEntry(QString(">>> Batch finished (elapsed time = %1 s)\n").arg(m.ElapsedTime(), 0, 'f', 1));

		emit batchFinished();
		return;
	}

	// find an available process
	for (int l=0; l<m.options.nprocs; ++l)
	{
		FEBioBatchProcess* p = m.processes[l];
		if (p == nullptr)
		{
			if (!m.jobQueue.empty())
			{
				JobInfo& di = *m.jobQueue.front(); m.jobQueue.pop();
				assert(di.status == PENDING);

				// start this job
				di.status = RUNNING;

				QString baseName = di.fileName;
				int n = baseName.lastIndexOf('.');
				if (n > 0) baseName = baseName.left(n);

				QString logName = baseName + ".log";

				p = new FEBioBatchProcess(m_wnd, this);
				m.processes[l] = p;

				p->job.SetFEBFileName(di.fileName.toStdString());
				p->job.SetLogFileName(logName.toStdString());
				p->jobId = di.jobId;
				p->job.m_cmd = "-i $(Filename) -task=test";

				if (m.options.nthreads > 0)
				{
					QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
					env.insert("OMP_NUM_THREADS", QString::number(m.options.nthreads));
					p->setProcessEnvironment(env);
				}

				p->job.StartTimer();
				p->run();

				emit jobStatusChanged(p->jobId);
			}
			else
			{
				break;
			}
		}
	}
}

void FEBioBatchDoc::CancelAll()
{
	// first, cancel all pending jobs
	CancelPending();

	// now, let's terminate any remaining jobs
	for (int i = 0; i < Impl::MAX_PROCESSES; ++i)
	{
		FEBioBatchProcess* p = m.processes[i];
		if (p && (p->state() == QProcess::Running))
		{
			p->kill();
		}
	}
}

void FEBioBatchDoc::CancelPending()
{
	while (!m.jobQueue.empty())
	{
		JobInfo* di = m.jobQueue.front(); m.jobQueue.pop();
		if (di->status == PENDING) di->status = CANCELLED;
		di->stats = di->oldStats;
	}
}

void FEBioBatchDoc::onRunFinished(int returnCode, QProcess::ExitStatus status)
{
	FEBioBatchProcess* p = dynamic_cast<FEBioBatchProcess*>(QObject::sender());
	if (p == nullptr) return;

	assert(p->jobId >= 0);
	if (p->jobId >= 0)
	{
		CFEBioJob& job = p->job;
		job.StopTimer();

		JobInfo& di = m.fileList[p->jobId];
		if (returnCode == 0) di.status = FEBioBatchDoc::FINISHED;
		else di.status = FEBioBatchDoc::FAILED;

		CFEBioReportDoc report(nullptr);
		report.LoadFromLogFile(job.GetLogFileName().c_str());

		di.stats.timeSteps    = report.m_modelStats.ntimeSteps;
		di.stats.iterations   = report.m_modelStats.ntotalIters;
		di.stats.nrhs         = report.m_modelStats.ntotalRHS;
		di.stats.reformations = report.m_modelStats.ntotalReforms;
		di.stats.solutionNorm = report.m_modelStats.solutionNorm;

		di.stats.elapsedTime = job.ElapsedTime();

		emit jobStatusChanged(p->jobId);

		m.deleteProcess(p);
	}

	StartNextJob();
}

void FEBioBatchDoc::onReadyRead()
{

}

void FEBioBatchDoc::onErrorOccurred(QProcess::ProcessError error)
{
	// No need to do anything here, since we can check the exit status of the process in finished.
}

bool FEBioBatchDoc::SaveDocument()
{
	std::string filePath = GetDocFilePath();
	if (filePath.empty()) return false;

	XMLWriter xml;
	xml.open(filePath.c_str());
	xml.add_branch("fbs_batch");
	for (auto& job : m.fileList)
	{
		XMLElement file("file");
		file.add_attribute("path", job.fileName.toStdString());
		xml.add_branch(file);
		{
			xml.add_leaf("timesteps" , job.stats.timeSteps);
			xml.add_leaf("iterations", job.stats.iterations);
			xml.add_leaf("rhs_evals" , job.stats.nrhs);
			xml.add_leaf("reforms"   , job.stats.reformations);
			xml.add_leaf("solution"  , job.stats.solutionNorm);
			xml.add_leaf("runtime"   , job.stats.elapsedTime);
		}
		xml.close_branch();

		job.oldStats = job.stats;
	}
	xml.close_branch();

	xml.close();

	return true;
}

bool FEBioBatchDoc::LoadDocument(const QString& fileName)
{
	if (fileName.isEmpty()) return false;

	std::string sfile = fileName.toStdString();

	XMLReader xml;
	xml.Open(sfile.c_str());
	XMLTag tag;
	if (!xml.FindTag("fbs_batch", tag))
	{
		xml.Close();
		return false;
	}

	SetDocFilePath(sfile);

	m.fileList.clear();

	try {
		++tag;
		do {
			if (tag == "file")
			{
				JobInfo di;
				const char* szpath = tag.AttributeValue("path");
				di.fileName = szpath;
				++tag;
				do {
					if      (tag == "timesteps" ) tag.value(di.stats.timeSteps);
					else if (tag == "iterations") tag.value(di.stats.iterations);
					else if (tag == "rhs_evals" ) tag.value(di.stats.nrhs);
					else if (tag == "reforms"   ) tag.value(di.stats.reformations);
					else if (tag == "solution"  ) tag.value(di.stats.solutionNorm);
					else if (tag == "runtime"   ) tag.value(di.stats.elapsedTime);
					++tag;
				} while (!tag.isend());

				di.oldStats = di.stats;

				di.jobId = (int)m.fileList.size();

				m.fileList.push_back(di);
			}
			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{

	}

	xml.Close();

	return true;
}
