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
#include <vector>
#include <string>
#include <FECore/FEParam.h>

class FEBioModel;
class FEBioAppWidget;

class CFEBioModelDataSource
{
public:
	CFEBioModelDataSource() {}
	virtual ~CFEBioModelDataSource() {}

	virtual void Clear() = 0;
	virtual void Update(double time) = 0;
};

class FEBioApp : public QObject
{
	Q_OBJECT

public:
	FEBioApp();
	~FEBioApp();

	FEBioModel* GetFEBioModel() { return m_fem; }

	void SetTask(const std::string& taskName, const std::string& taskInputFilename = "");

	const std::string& GetTaskName() { return m_taskName; }
	const std::string& GetTaskInputFile() { return m_taskInputFile; }

	void AddModelDataSource(CFEBioModelDataSource* dataSrc);

	bool ProcessFEBioEvent(int nevent);

	bool LoadModelFromFile(const std::string& fileName);

	std::string GetModelFilename();

	std::vector<FEParamValue> GetFEBioParameterList(const char* szparams);
	FEParamValue GetFEBioParameter(const char* szparams);

	void SetUI(FEBioAppWidget* w) { m_ui = w; }	
	FEBioAppWidget* GetUI() { return m_ui; }

public:
	void runModel();
	void stopModel();

public slots:
	void runScript(const QString& script); // called by action buttons

signals:
	void dataChanged();
	void modelStarted();
	void modelFinished(bool returnCode);

private:
	// this function actuallys run FEBio, but is called from a separate thread
	void RunFEBioModel();

private:
	bool	m_isInitialized = false;
	bool	m_isRunning = false;
	bool	m_forceStop = false;

	FEBioModel* m_fem = nullptr;

	std::vector<CFEBioModelDataSource*>	m_dataSources;

	std::string m_taskName;
	std::string m_taskInputFile;

	FEBioAppWidget* m_ui = nullptr;


	friend class CFEBioAppThread;
};
