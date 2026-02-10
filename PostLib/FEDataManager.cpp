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

#include "stdafx.h"
#include "FEDataManager.h"
#include "FEPostModel.h"
#include "constants.h"
using namespace Post;
using namespace std;

FEDataManager::FEDataManager(FEPostModel* pm)
{
	m_pm = pm;
}

FEDataManager::~FEDataManager(void)
{
	Clear();
}

FEDataFieldPtr FEDataManager::FirstDataField() 
{ 
	return m_Data.begin(); 
}

int FEDataManager::DataFields() const
{ 
	return (int) m_Data.size(); 
}

void FEDataManager::Clear()
{
	vector<ModelDataField*>::iterator pi;
	for (pi = m_Data.begin(); pi != m_Data.end(); ++pi) delete (*pi);
	m_Data.clear();
}

void FEDataManager::AddDataField(ModelDataField* pd, const std::string& name, const char* szunits)
{
	if (name.empty() == false) pd->SetName(name);
	if (szunits) pd->SetUnits(szunits);
	m_Data.push_back(pd);
	int fieldID = BUILD_FIELD(pd->DataClass(), DataFields() - 1, 0);
	pd->SetFieldID(fieldID);
}

void FEDataManager::DeleteDataField(ModelDataField* pd)
{
	FEDataFieldPtr it;
	for (it = m_Data.begin(); it != m_Data.end(); ++it)
	{
		if (*it == pd)
		{
			m_Data.erase(it);
			return;
		}
	}
}

int FEDataManager::FindDataField(const std::string& fieldName)
{
	vector<ModelDataField*>::iterator pe = m_Data.begin();
	for (int i=0; i<(int) m_Data.size(); ++i, ++pe)
	{
		if ((*pe)->GetName() == fieldName) return i;
	}

	return -1;
}

int FEDataManager::GetFieldCode(const std::string& fieldName)
{
	int n = FindDataField(fieldName);
	if (n >= 0)
	{
		auto p = DataField(n);
		return (*p)->GetFieldID();
	}
	return -1;
}

FEDataFieldPtr FEDataManager::DataField(int i)
{
	if ((i < 0) || (i >= m_Data.size())) return m_Data.end();
	return m_Data.begin() + i;
}

bool FEDataManager::IsValid(FEDataFieldPtr pdf) const
{
	return (pdf != m_Data.end());
}

std::string FEDataManager::getDataString(int nfield, Data_Tensor_Type ntype)
{
	if (IS_VALID(nfield))
	{
		int ndata = FIELD_CODE(nfield);
		int ncomp = FIELD_COMP(nfield);
		if ((ndata>=0) && (ndata< m_Data.size()))
		{
			ModelDataField* pd = m_Data[ndata];
			return pd->componentName(ncomp, ntype);
		}
	}
	return "";
}

const char* FEDataManager::getDataUnits(int nfield)
{
	if (IS_VALID(nfield))
	{
		int ndata = FIELD_CODE(nfield);
		if ((ndata >= 0) && (ndata < m_Data.size()))
		{
			ModelDataField* pd = m_Data[ndata];
			return pd->GetUnits();
		}
	}
	return nullptr;
}

// see if a field ID is valid
bool FEDataManager::IsValid(int fieldId) const
{
	int ndata = FIELD_CODE(fieldId);
	if ((ndata < 0) || (ndata >= m_Data.size())) return false;

	ModelDataField* pd = m_Data[ndata];

	// strip the component
	fieldId = (fieldId & ~0xFF);
	return (pd->GetFieldID() == fieldId);
}
