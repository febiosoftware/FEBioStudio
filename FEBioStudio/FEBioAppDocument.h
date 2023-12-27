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
#include "Document.h"
#include <FECore/FEParam.h>
#include <vector>
#include <QMutex>
#include <QThread>

class FEBioModel;
class FEModel;

class CFEBioModelDataSource
{
public:
	CFEBioModelDataSource() {}
	virtual ~CFEBioModelDataSource() {}

	virtual void Clear() = 0;
	virtual void Update(double time) = 0;
};

class FEBioAppModel
{
public:
	FEBioAppModel();
	~FEBioAppModel();

	bool IsRunning() const { return m_isRunning; }

	bool Solve();

	FEBioModel* GetFEBioModel() { return m_fem; }

private:
	bool	m_isInitialized;
	bool	m_isRunning;
	FEBioModel* m_fem;
};

class FEBioAppDocument : public CDocument
{
	Q_OBJECT

public:
	FEBioAppDocument(CMainWindow* wnd);

	bool LoadModelFromFile(QString fileName);

	std::vector<FEParamValue> GetFEBioParameterList(const char* szparams);
	FEParamValue GetFEBioParameter(const char* szparams);

	FEBioModel* GetFEBioModel();

	void AddModelDataSource(CFEBioModelDataSource* dataSrc);

public slots:
	void runModel();
	void stopModel();
	void onFEBioFinished(bool b);

signals:
	void modelStarted();
	void modelFinished(bool returnCode);
	void dataChanged();

private:
	static bool febio_cb(FEModel* fem, unsigned int nevent, void* pd);
	bool ProcessFEBioEvent(int nevent);

private:
	FEBioAppModel*	m_fbm;
	bool	m_forceStop;
	std::vector<CFEBioModelDataSource*>	m_dataSources;
};

class CFEBioAppThread : public QThread
{
	Q_OBJECT

public:
	CFEBioAppThread(FEBioAppModel* fem, FEBioAppDocument* parent);

	void run() Q_DECL_OVERRIDE;

signals:
	void FEBioFinished(bool);

private:
	FEBioAppModel* m_fem;
};
