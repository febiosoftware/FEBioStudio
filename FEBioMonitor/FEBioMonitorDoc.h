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
class FEBioModel;
class FEGlobalMatrix;
class CFEBioJob;

class FEBioMonitorDoc;

class FSConvergenceInfo
{
public:
	class VariableNorm
	{
	public:
		std::string name;
		std::vector<double> val;
	};

public:
	double	R0;
	double	E0;
	double	U0;

	std::vector<double> Rt;
	std::vector<double> Et;
	std::vector<double> Ut;

	std::vector<VariableNorm> m_varNorms;

	void clear()
	{
		R0 = E0 = U0 = 0;
		Rt.clear();
		Et.clear();
		Ut.clear();
		m_varNorms.clear();
	}
};

class FEBioMonitorThread : public QThread
{
	Q_OBJECT

public:
	FEBioMonitorThread(FEBioMonitorDoc* doc, CFEBioJob* job);

	void run() override;

signals:
	void jobFinished(bool);

private:
	FEBioMonitorDoc* m_doc;
	CFEBioJob* m_job;
};

class FEBioWatchVariable
{
public:
	enum {
		INVALID,
		VALID
	};

public:
	FEBioWatchVariable(const QString& name) : m_name(name) {}

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	QString value() const { return m_value; }
	void setValue(const QString& val) { m_value = val; }

	int type() const { return m_type; }
	void setType(int type) { m_type = type; }

private:
	int	m_type = INVALID;
	QString m_name;
	QString m_value;
};

class FEBioMonitorDoc : public CGLModelDocument
{
	Q_OBJECT

	class Imp;

public:
	enum UpdateEvents
	{
		Update_Major_Iters,
		Update_Minor_Iters,
		Update_All_Events
	};

public:
	FEBioMonitorDoc(CMainWindow* wnd);
	~FEBioMonitorDoc();

	void StartPaused(bool b);
	bool StartPaused() const;

	void SetPauseEvents(unsigned int nevents);
	unsigned int GetPauseEvents() const;

	void SetPauseTime(double ftime, bool benable);
	bool IsPauseTimeEnabled() const;
	double GetPauseTime() const;

	int GetDebugLevel() const;
	void SetDebugLevel(int debugLevel);

	void SetRecordStatesFlag(bool b);
	bool GetRecordStatesFlag() const;

	void SetUpdateEvents(int updateEvents);
	int GetUpdateEvents() const;

	std::string GetFEBioInputFile() const;

public:
	bool RunJob(CFEBioJob* job);

	void KillJob();

	void PauseJob();

	void ContinueJob();

	void AdvanceJob();

	void appendLog(const char* sz);

	bool IsRunning() const;

	bool IsPaused() const;

	int GetCurrentEvent() const;

public:
	FEBioWatchVariable* AddWatchVariable(const QString& name);
	const FEBioWatchVariable* GetWatchVariable(int n);
	int GetWatchVariables() const;
	void SetWatchVariable(int n, const QString& name);

public:
	FEGlobalMatrix* GetStiffnessMatrix();

	double GetConditionNumber();

	void CollectVariableNorms(bool b);

	FSConvergenceInfo& GetConvergenceInfo();

	void UpdateConvergenceInfo();

private:
	void UpdateWatchVariable(FEBioWatchVariable& var);
	void UpdateAllWatchVariables();
	void InitDefaultWatchVariables();

public: // overrides for CGLModelDocument
	Post::CGLModel* GetGLModel() override;

	Post::FEPostModel* GetFSModel() override;

	GObject* GetActiveObject() override;

public:
	double GetTimeValue() const;

	bool processFEBioEvent(FEModel* fem, int event);
	void SetProgress(double percent);

	bool AddDataField(const std::string& dataField);

	void SetCurrentState(int n);

	void GenerateReport(bool b);

private:
	void updateWindowTitle();

private slots:
	void onJobFinished(bool b);
	void onModelInitialized();
	void onUpdateViews(bool updatePanel, bool updateGL);
	void readOutput();

signals:
	void outputReady();
	void modelInitialized();
	void updateViews(bool updatePanel, bool updateGL);

private:
	Imp* m;

	friend class FEBioMonitorThread;
};

QString eventToString(int nevent);
