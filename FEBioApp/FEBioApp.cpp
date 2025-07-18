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
#include "FEBioApp.h"
#include <FEBioLib/FEBioModel.h>
#include <QMessageBox>
#include "FEBioAppScript.h"	
#include <FECore/FECoreTask.h>
#include <FECore/FECoreKernel.h>
#include <QThread>

class CFEBioAppThread : public QThread
{
public:
	CFEBioAppThread(FEBioApp* app) : QThread(app), m_app(app)
	{
		QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
	}
	void run() Q_DECL_OVERRIDE
	{
		if (m_app) m_app->RunFEBioModel();
	}
private:
	FEBioApp* m_app;
};


FEBioApp::FEBioApp()
{

}

FEBioApp::~FEBioApp()
{
	delete m_fem;
}

void FEBioApp::SetTask(const std::string& taskName, const std::string& taskInputFilename)
{
	m_taskName = taskName;
	m_taskInputFile = taskInputFilename;
}

void FEBioApp::AddModelDataSource(CFEBioModelDataSource* dataSrc)
{
	m_dataSources.push_back(dataSrc);
}

bool FEBioApp::ProcessFEBioEvent(int nevent)
{
	for (auto data : m_dataSources)
	{
		switch (nevent)
		{
		case CB_INIT:
		case CB_RESET:
			data->Clear();
			data->Update(0.0);
			break;
		case CB_MAJOR_ITERS:
			data->Update(m_fem->GetCurrentTime());
			break;
		}
	}

	emit dataChanged();

	if (m_forceStop) return false;

	return true;
}

bool febio_cb(FEModel* fem, unsigned int nevent, void* pd)
{
	FEBioApp* app= (FEBioApp*)(pd);
	return app->ProcessFEBioEvent(nevent);
}

bool FEBioApp::LoadModelFromFile(const std::string& fileName)
{
	if (m_fem) delete m_fem;
	m_fem = new FEBioModel;
	FEBioModel& fem = *m_fem;
	fem.SetName("fem");
	bool b = fem.Input(fileName.c_str());

	fem.AddCallback(febio_cb, CB_ALWAYS, this);

	// call this so that the parameter list is initialized
	if (b) fem.GetParameterList();

	return b;
}

std::string FEBioApp::GetModelFilename()
{
	if (m_fem == nullptr) return std::string();
	return m_fem->GetInputFileName();
}

std::vector<FEParamValue> FEBioApp::GetFEBioParameterList(const char* szparams)
{
	std::vector<FEParamValue> paramList;
	FEBioModel* fem = GetFEBioModel();
	if (fem == nullptr) return paramList;

	ParamString ps(szparams);

	string modelName = "fem";
	if (ps == modelName)
	{
		FECoreBase* pc = fem->FindComponent(ps);

		FEParameterList& pl = pc->GetParameterList();
		int n = pl.Parameters();
		list<FEParam>::iterator it = pl.first();
		for (int i = 0; i < n; ++i, ++it)
		{
			FEParam& pi = *it;

			if (pi.dim() == 1)
			{
				FEParamValue val = pi.paramValue();

				switch (val.type())
				{
				case FE_PARAM_DOUBLE:
				case FE_PARAM_INT:
				case FE_PARAM_BOOL:
				{
					paramList.push_back(val);
				}
				break;
				}
			}
		}
	}

	return paramList;
}

FEParamValue FEBioApp::GetFEBioParameter(const char* szparams)
{
	ParamString ps(szparams);
	return m_fem->GetParameterValue(ps);
}

void FEBioApp::runScript(const QString& script)
{
	FEBioAppScript appScript(this);
	if (!appScript.run(script))
	{
		QMessageBox::critical(nullptr, "FEBioApp", QString("Failed to execute script.\n%1").arg(appScript.errorString()));
	}
}

// This is the actual function that runs the FEBio model. It is called from a
// separate thread.
void FEBioApp::RunFEBioModel()
{
	FEBioModel* fem = GetFEBioModel();
	if (fem == nullptr) return;

	assert(m_isRunning == false);
	m_isRunning = true;
	m_forceStop = false;

	if (m_taskName.empty() == false)
	{
		std::unique_ptr<FECoreTask> task(fecore_new<FECoreTask>(m_taskName.c_str(), fem));
		if (task == nullptr) emit modelFinished(false);

		const char* sztaskfile = (m_taskInputFile.empty() ? nullptr : m_taskInputFile.c_str());
		if (task->Init(sztaskfile) == false)
		{
			emit modelFinished(false);
		}
		else
		{
			bool b = task->Run();
			emit modelFinished(b);
		}
	}
	else
	{
		bool b = false;
		if (m_isInitialized) b = fem->Reset();
		else
		{
			m_isInitialized = fem->Init();
			b = m_isInitialized;
		}
		if (b)
		{
			emit modelStarted();
			b = fem->Solve();
			if (m_forceStop) b = true;
			emit modelFinished(b);
		}
	}
	m_isRunning = false;
}


void FEBioApp::runModel()
{
	if (GetFEBioModel() && !m_isRunning)
	{
		CFEBioAppThread* thread = new CFEBioAppThread(this);
		thread->start();
	}
}

void FEBioApp::stopModel()
{
	m_forceStop = true;
}
