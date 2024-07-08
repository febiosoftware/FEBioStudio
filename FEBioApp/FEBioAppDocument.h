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
#include "../FEBioStudio/Document.h"
#include <FECore/FEParam.h>
#include <vector>

class FEBioModel;
class FEModel;

class FEBioAppWidget;

class CFEBioModelDataSource
{
public:
	CFEBioModelDataSource() {}
	virtual ~CFEBioModelDataSource() {}

	virtual void Clear() = 0;
	virtual void Update(double time) = 0;
};

class FEBioAppDocument : public CDocument
{
	Q_OBJECT

public:
	FEBioAppDocument(CMainWindow* wnd);
	~FEBioAppDocument();

	bool LoadModelFromFile(QString fileName);

	void SetTask(const std::string& taskName, const std::string& taskInputFilename = "");

	std::vector<FEParamValue> GetFEBioParameterList(const char* szparams);
	FEParamValue GetFEBioParameter(const char* szparams);

	FEBioModel* GetFEBioModel();

	void AddModelDataSource(CFEBioModelDataSource* dataSrc);

	void SetUI(FEBioAppWidget* w);
	FEBioAppWidget* GetUI();

public slots:
	void runScript(const QString& script); // called by action buttons

signals:
	void modelStarted();
	void modelFinished(bool returnCode);
	void dataChanged();

private:
	static bool febio_cb(FEModel* fem, unsigned int nevent, void* pd);
	bool ProcessFEBioEvent(int nevent);

	// this function actuallys run FEBio, but is called from a separate thread
	void RunFEBioModel();

public:
	void runModel();
	void stopModel();

private:
	bool	m_isInitialized;
	bool	m_isRunning;
	bool	m_forceStop;
	FEBioModel* m_fem;
	std::vector<CFEBioModelDataSource*>	m_dataSources;

	std::string m_taskName;
	std::string m_taskInputFile;

	FEBioAppWidget* m_ui;

	friend class CFEBioAppThread;
};
