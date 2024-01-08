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
#pragma once
#include <QObject>
#include <QThread>
#include <QMutex>
#include "../FEBioStudio/GLModelDocument.h"

class FEModel; // from FEBio

class FEBioMonitorDoc;

class FEBioMonitorThread : public QThread
{
	Q_OBJECT

public:
	FEBioMonitorThread(FEBioMonitorDoc* doc);

	void run() override;

signals:
	void jobFinished(bool);

private:
	FEBioMonitorDoc* m_doc;
};

class FEBioMonitorDoc : public CGLModelDocument
{
	Q_OBJECT

public:
	FEBioMonitorDoc(CMainWindow* wnd);
	~FEBioMonitorDoc();

	void SetFEBioInputFile(QString febFile);

	QString GetFEBioInputFile() const;

	void StartPaused(bool b);
	bool StartPaused() const;

	void SetPauseEvents(unsigned int nevents);
	unsigned int GetPauseEvents() const;

	void SetPauseTime(double ftime, bool benable);
	bool IsPauseTimeEnabled() const;
	double GetPauseTime() const;

public:
	void RunJob();

	void KillJob();

	void PauseJob();

	void AdvanceJob();

	void appendLog(const char* sz);

	bool IsRunning() const;

	bool IsPaused() const;

public: // overrides for CGLModelDocument
	Post::CGLModel* GetGLModel() override;

	GObject* GetActiveObject() override;

public:
	double GetTimeValue() const;

	bool processFEBioEvent(FEModel* fem, int event);
	void SetProgress(double percent);

	bool AddDataField(const std::string& dataField);

private:
	void updateWindowTitle();

private slots:
	void onJobFinished(bool b);
	void onModelInitialized();
	void readOutput();

signals:
	void outputReady();
	void modelInitialized();
	void updateViews();

private:
	QString m_febFile;
	QString	m_outputBuffer;
	bool	m_startPaused;
	bool	m_isOutputReady;
	bool	m_isStopped;
	bool	m_isRunning;
	bool	m_isPaused;
	bool	m_pauseRequested;
	bool	m_usePauseTime;
	double	m_pauseTime;
	double	m_progressPct;
	double	m_time;
	unsigned int m_pauseEvents;
	QMutex	m_mutex;

	friend class FEBioMonitorThread;
};

QString eventToString(int nevent);
