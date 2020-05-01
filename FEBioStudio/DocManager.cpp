#include "stdafx.h"
#include "DocManager.h"
#include "Document.h"

CDocManager::CDocManager(CMainWindow* wnd) : m_wnd(wnd)
{

}

// return the number of documents
int CDocManager::Documents() const
{
	return (int)m_docList.size();
}

// add a document
bool CDocManager::AddDocument(CDocument* doc)
{
	// make sure this document does not exist yet!
	for (int i = 0; i < m_docList.size(); ++i)
	{
		if (m_docList[i] == doc)
		{
			assert(false);
			return false;
		}
	}
	m_docList.push_back(doc);
	return true;
}

// remove a document
void CDocManager::RemoveDocument(int i)
{
	CDocument* doc = m_docList[i];
	m_docList.erase(m_docList.begin() + i);
	delete doc;
}

CDocument* CDocManager::GetDocument(int i)
{
	if ((i >= 0) && (i < m_docList.size()))
		return m_docList[i];
	else
		return nullptr;
}
