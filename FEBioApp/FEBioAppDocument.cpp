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
#include "FEBioAppDocument.h"
#include <FEBioLib/FEBioModel.h>
#include <FEBioLink/FEBioModule.h>
#include <FECore/FECoreTask.h>
#include <FECore/FECoreKernel.h>
#include <QWidget>
#include <QThread>

class CFEBioAppThread : public QThread
{
public:
	CFEBioAppThread(FEBioAppDocument* doc) : QThread(doc), m_doc(doc)
	{
		QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
	}
	void run() Q_DECL_OVERRIDE
	{
		if (m_doc) m_doc->RunFEBioModel();
	}
private:
	FEBioAppDocument* m_doc;
};

FEBioAppDocument::FEBioAppDocument(CMainWindow* wnd) : CDocument(wnd), m_fem(nullptr)
{
	SetIcon(":/icons/febiorun.png");
	m_forceStop = false;
	m_isInitialized = false;
	m_isRunning = false;
}

FEBioAppDocument::~FEBioAppDocument()
{
	delete m_fem;
}

bool FEBioAppDocument::febio_cb(FEModel* fem, unsigned int nevent, void* pd)
{
	FEBioAppDocument* doc = (FEBioAppDocument*)(pd);
	return doc->ProcessFEBioEvent(nevent);
}

bool FEBioAppDocument::ProcessFEBioEvent(int nevent)
{
	// process all model data sources
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

bool FEBioAppDocument::LoadModelFromFile(QString fileName)
{
	if (m_fem) delete m_fem;
	m_fem = new FEBioModel;
	FEBioModel& fem = *m_fem;
	fem.SetName("fem");

	fem.AddCallback(febio_cb, CB_ALWAYS, this);
	string sfile = fileName.toStdString();
	FEBio::BlockCreateEvents(true);
	bool b = fem.Input(sfile.c_str());
	FEBio::BlockCreateEvents(false);

	// call this so that the parameter list is initialized
	if (b) fem.GetParameterList();

	return b;
}

void FEBioAppDocument::SetTask(const std::string& taskName, const std::string& taskInputFilename)
{
	m_taskName = taskName;
	m_taskInputFile = taskInputFilename;
}

FEBioModel* FEBioAppDocument::GetFEBioModel()
{
	return m_fem;
}

void FEBioAppDocument::AddModelDataSource(CFEBioModelDataSource* dataSrc)
{
	m_dataSources.push_back(dataSrc);
}

std::vector<FEParamValue> FEBioAppDocument::GetFEBioParameterList(const char* szparams)
{
	std::vector<FEParamValue> paramList;
	if (m_fem == nullptr) return paramList;
	FEBioModel& fem = *m_fem;

	ParamString ps(szparams);

	string modelName = "fem";
	if (ps == modelName)
	{
		FECoreBase* pc = fem.FindComponent(ps);

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

FEParamValue FEBioAppDocument::GetFEBioParameter(const char* szparams)
{
	ParamString ps(szparams);
	return m_fem->GetParameterValue(ps);
}

void FEBioAppDocument::runModel()
{
	if (m_fem && !m_isRunning)
	{
		CFEBioAppThread* thread = new CFEBioAppThread(this);
		thread->start();
	}
}

void FEBioAppDocument::stopModel()
{
	m_forceStop = true;
}

// This is the actual function that runs the FEBio model. It is called from a
// separate thread.
void FEBioAppDocument::RunFEBioModel()
{
	if (m_fem == nullptr) return;

	assert(m_isRunning == false);
	m_isRunning = true;
	m_forceStop = false;

	if (m_taskName.empty() == false)
	{
		std::unique_ptr<FECoreTask> task(fecore_new<FECoreTask>(m_taskName.c_str(), m_fem));
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
		if (m_isInitialized) b = m_fem->Reset();
		else
		{
			m_isInitialized = m_fem->Init();
			b = m_isInitialized;
		}
		if (b)
		{
			emit modelStarted();
			b = m_fem->Solve();
			if (m_forceStop) b = true;
			emit modelFinished(b);
		}
	}
	m_isRunning = false;
}
