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
#include <QWidget>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

FEBioAppModel::FEBioAppModel() : m_fem(new FEBioModel)
{
	m_isInitialized = false;
	m_isRunning = false;
};

FEBioAppModel::~FEBioAppModel()
{
	delete m_fem;
}

bool FEBioAppModel::Solve()
{
	if (m_fem == nullptr) return false;
	bool b = false;
	if (m_isInitialized) b = m_fem->Reset();
	else
	{
		m_isInitialized = m_fem->Init();
		b = m_isInitialized;
	}
	if (b) b = m_fem->Solve();
	return b;
}

CFEBioAppThread::CFEBioAppThread(FEBioAppModel* fem, FEBioAppDocument* parent) : QThread(parent), m_fem(fem)
{
	QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
	QObject::connect(this, SIGNAL(FEBioFinished(bool)), parent, SLOT(onFEBioFinished(bool)));
}

void CFEBioAppThread::run()
{
	if (m_fem && !m_fem->IsRunning())
	{
		bool b = m_fem->Solve();
		emit FEBioFinished(b);
	}
}

FEBioAppDocument::FEBioAppDocument(CMainWindow* wnd) : CDocument(wnd), m_fbm(nullptr)
{
	m_forceStop = false;
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
			data->Update(m_fbm->GetFEBioModel()->GetCurrentTime());
			break;
		}
	}

	emit dataChanged();

	if (m_forceStop) return false;

	return true;
}

bool FEBioAppDocument::LoadModelFromFile(QString fileName)
{
	if (m_fbm) delete m_fbm;
	m_fbm = new FEBioAppModel;
	FEBioModel& fem = *m_fbm->GetFEBioModel();
	fem.SetName("fem");

	fem.AddCallback(febio_cb, CB_ALWAYS, this);
	string sfile = fileName.toStdString();
	FEBio::BlockCreateEvents(true);
	bool b = fem.Input(sfile.c_str());
	FEBio::BlockCreateEvents(false);
	return b;
}

FEBioModel* FEBioAppDocument::GetFEBioModel()
{
	return m_fbm->GetFEBioModel();
}

void FEBioAppDocument::AddModelDataSource(CFEBioModelDataSource* dataSrc)
{
	m_dataSources.push_back(dataSrc);
}

std::vector<FEParamValue> FEBioAppDocument::GetFEBioParameterList(const char* szparams)
{
	std::vector<FEParamValue> paramList;
	if (m_fbm == nullptr) return paramList;
	FEBioModel& fem = *m_fbm->GetFEBioModel();

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
	return m_fbm->GetFEBioModel()->GetParameterValue(ps);
}

void FEBioAppDocument::runModel()
{
	m_forceStop = false;
	if (m_fbm)
	{
		CFEBioAppThread* thread = new CFEBioAppThread(m_fbm, this);
		thread->start();
		emit modelStarted();
	}
}

void FEBioAppDocument::stopModel()
{
	m_forceStop = true;
}

void FEBioAppDocument::onFEBioFinished(bool b)
{
	emit modelFinished(b);
}
