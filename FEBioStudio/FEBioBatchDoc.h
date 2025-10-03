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
#pragma once
#include "Document.h"
#include <QProcess>

class CMainWindow;

class FEBioBatchDoc : public CDocument
{
	Q_OBJECT

	class Impl;

public:
	enum JobStatus {
		IDLE,
		PENDING,
		RUNNING,
		FINISHED,
		FAILED,
		CANCELLED
	};

	struct JobStats
	{
		double	elapsedTime = 0;
		int		timeSteps = -1;
		int		iterations = -1;
		int		nrhs = -1;
		int		reformations = -1;
		double	solutionNorm = -1.0;

		void clear()
		{
			elapsedTime = 0;
			timeSteps = 0;
			iterations = 0;
			nrhs = 0;
			reformations = 0;
			solutionNorm = 0.0;
		}
	};

	struct JobInfo
	{
		QString		fileName;
		int			jobId = -1;
		JobStatus	status = JobStatus::IDLE;
		JobStats	stats;
		JobStats	oldStats;
	};

	struct Options {
		int nthreads = 0;	// number of threads (0 = auto)
		int nprocs = 1;	// number of concurrent processes
	};

public:
	FEBioBatchDoc(CMainWindow* wnd);
	~FEBioBatchDoc();

	void SetFileList(const QStringList& fileList);

	int Files() const;
	std::vector<JobInfo> GetFileList() const;
	JobInfo GetJobInfo(int jobId) const;

	void StartBatch();
	void StartBatch(const std::vector<int>& jobIDs);

	void CancelPending();

	void CancelAll();

	bool SaveDocument() override;

	bool LoadDocument(const QString& fileName);

	Options GetOptions() const;
	void SetOptions(const Options& opt);

public slots:
	void onRunFinished(int returnCode, QProcess::ExitStatus status);
	void onReadyRead();
	void onErrorOccurred(QProcess::ProcessError error);

signals:
	void jobStatusChanged(int jobId);
	void batchFinished();

private:
	void StartNextJob();

private:
	Impl& m;
};
