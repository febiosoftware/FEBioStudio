#include "stdafx.h"
#include "FEDataManager.h"
#include "FEModel.h"
#include "constants.h"
using namespace Post;

FEDataManager::FEDataManager(FEModel* pm)
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
	vector<FEDataField*>::iterator pi;
	for (pi = m_Data.begin(); pi != m_Data.end(); ++pi) delete (*pi);
	m_Data.clear();
}

void FEDataManager::AddDataField(FEDataField* pd)
{
	m_Data.push_back(pd);
	pd->SetFieldID(BUILD_FIELD(pd->DataClass(), DataFields()-1, 0));
}

void FEDataManager::DeleteDataField(FEDataField* pd)
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
	vector<FEDataField*>::iterator pe = m_Data.begin();
	for (int i=0; i<(int) m_Data.size(); ++i, ++pe)
	{
		if ((*pe)->GetName() == fieldName) return i;
	}

	return -1;
}

FEDataFieldPtr FEDataManager::DataField(int i)
{
	return m_Data.begin() + i;
}

std::string FEDataManager::getDataString(int nfield, Data_Tensor_Type ntype)
{
	if (IS_VALID(nfield))
	{
		int ndata = FIELD_CODE(nfield);
		int ncomp = FIELD_COMP(nfield);
		if ((ndata>=0) && (ndata< m_Data.size()))
		{
			FEDataField* pd = m_Data[ndata];
			return pd->componentName(ncomp, ntype);
		}
	}
	return "";
}
