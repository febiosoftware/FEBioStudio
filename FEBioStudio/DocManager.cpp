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
#include "DocManager.h"
#include "Document.h"
#include <string>
#include <sstream>

CDocManager::CDocManager(CMainWindow* wnd) : m_wnd(wnd)
{

}

// return the number of documents
int CDocManager::Documents() const
{
	return (int)m_docList.size();
}

CDocManager::~CDocManager()
{
	for (int i = 0; i < m_docList.size(); ++i) delete m_docList[i];
	m_docList.clear();
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

std::string CDocManager::GenerateNewDocName()
{
	int n = 1;
	std::string docTitle;
	bool bok = true;
	do
	{
		std::stringstream ss;
		ss << "Model" << n++;
		docTitle = ss.str();
		bok = true;
		for (int i = 0; i < Documents(); ++i)
		{
			CDocument* doci = GetDocument(i);
			if ((doci->GetDocTitle() == docTitle) || (doci->GetDocFileBase() == docTitle))
			{
				bok = false;
				break;
			}
		}
	} while (bok == false);
	return docTitle;
}
