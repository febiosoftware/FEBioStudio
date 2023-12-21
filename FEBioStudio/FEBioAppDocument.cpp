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

FEBioAppDocument::FEBioAppDocument(CMainWindow* wnd) : CDocument(wnd), m_fem(nullptr)
{
	m_isFemInitialized = false;
}

bool FEBioAppDocument::febio_cb(FEModel* fem, unsigned int nevent, void* pd)
{
	FEBioAppDocument* doc = (FEBioAppDocument*)(pd);
	doc->ProcessFEBioEvent(nevent);
	return true;
}

void FEBioAppDocument::ProcessFEBioEvent(int nevent)
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
	return b;
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
	bool b = false;
	if (m_fem)
	{
		if (m_isFemInitialized) b = m_fem->Reset();
		else
		{
			m_isFemInitialized = m_fem->Init();
			b = m_isFemInitialized;
		}

		if (b) b = m_fem->Solve();
	}
	emit modelFinished(b);
}
