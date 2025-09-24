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
#include "Document.h"
#include "MainWindow.h"
#include <FSCore/FSDir.h>
#include <QtCore/QDir>

//=============================================================================
CDocObserver::CDocObserver(CDocument* doc) : m_doc(doc)
{
	if (m_doc) m_doc->AddObserver(this);
}

CDocObserver::~CDocObserver()
{
	if (m_doc) m_doc->RemoveObserver(this);
}

void CDocObserver::DocumentDelete()
{
	m_doc = nullptr;
	DocumentUpdate(true);
}

//==============================================================================
// CDocument
//==============================================================================

CDocument* CDocument::m_activeDoc = nullptr;

CDocument::CDocument(CMainWindow* wnd) : m_wnd(wnd)
{
	// set document as not modified
	m_bModified = false;
	m_bValid = true;

	// reset the filename
	m_filePath.clear();
	m_autoSaveFilePath.clear();
}

CDocument* CDocument::GetActiveDocument()
{
	return m_activeDoc;
}

void CDocument::SetActiveDocument(CDocument* doc)
{
	if (doc == m_activeDoc) return;

	if (m_activeDoc) m_activeDoc->Deactivate();
	m_activeDoc = doc;
	if (doc) m_activeDoc->Activate();
}

CDocument::~CDocument()
{
	// make sure it's not the active doc
	if (GetActiveDocument() == this) SetActiveDocument(nullptr);

	// remove all observers
	for (int i = 0; i < m_Observers.size(); ++i)
		m_Observers[i]->DocumentDelete();
	m_Observers.clear();
}

void CDocument::Clear()
{
	// reset the filename
	m_filePath.clear();
	m_autoSaveFilePath.clear();

	// set document as not modified
	m_bModified = false;
	m_bValid = true;
}

bool CDocument::Initialize()
{
	return true;
}

void CDocument::Update()
{

}

// will be called when the document is activated
void CDocument::Activate()
{

}

// will be called when the document is deactivate
void CDocument::Deactivate()
{

}

CMainWindow* CDocument::GetMainWindow() 
{ 
	return m_wnd; 
}

bool CDocument::IsModified() const
{
	return m_bModified;
}

//-----------------------------------------------------------------------------
void CDocument::SetModifiedFlag(bool bset)
{
	m_bModified = bset;
}

bool CDocument::IsValid() const { return m_bValid; }

//-----------------------------------------------------------------------------
void CDocument::AddObserver(CDocObserver* observer)
{
	// no duplicates allowed
	for (int i = 0; i < m_Observers.size(); ++i)
	{
		if (m_Observers[i] == observer)
		{
			assert(false);
			return;
		}
	}

	m_Observers.push_back(observer);
}

//-----------------------------------------------------------------------------
void CDocument::RemoveObserver(CDocObserver* observer)
{
	for (int i = 0; i < m_Observers.size(); ++i)
	{
		if (m_Observers[i] == observer)
		{
			m_Observers.erase(m_Observers.begin() + i);
			return;
		}
	}
	assert(false);
}

//-----------------------------------------------------------------------------
void CDocument::UpdateObservers(bool bnew)
{
	if (m_Observers.empty()) return;

	for (int i = 0; i < m_Observers.size(); ++i)
	{
		CDocObserver* observer = m_Observers[i];
		if (observer) observer->DocumentUpdate(bnew);
	}
}

//-----------------------------------------------------------------------------
std::string CDocument::GetDocFilePath()
{
	return m_filePath;
}

//-----------------------------------------------------------------------------
// set the document's title
void CDocument::SetDocTitle(const std::string& t)
{
	m_title = t;
}

//-----------------------------------------------------------------------------
std::string CDocument::GetDocTitle()
{
	return m_title;
}

//-----------------------------------------------------------------------------
void CDocument::SetDocFilePath(const std::string& filePath)
{
	m_filePath = FSDir::filePath(filePath);
	m_title = GetDocFileName();

	SetAutoSaveFilePath();
}

//-----------------------------------------------------------------------------
std::string CDocument::GetAutoSaveFilePath()
{
	return m_autoSaveFilePath;
}

//-----------------------------------------------------------------------------
void CDocument::SetAutoSaveFilePath()
{
	//Construct the new autosave file path
	FSDir fsDir(m_filePath);
	QString newAutoSave = QString("%1/~%2_auto.%3").arg(fsDir.fileDir().c_str()).arg(fsDir.fileBase().c_str()).arg(fsDir.fileExt().c_str());

	// If an old autosave file exists, rename it
	QFile oldAutoSaveFile(m_autoSaveFilePath.c_str());
	if (oldAutoSaveFile.exists()) oldAutoSaveFile.rename(newAutoSave);

	// Set new autosave file path
	m_autoSaveFilePath = newAutoSave.toStdString();

}

void CDocument::SetUnsaved()
{
	m_filePath.clear();
	m_autoSaveFilePath.clear();

	SetModifiedFlag(true);
}

//-----------------------------------------------------------------------------
// get just the file name
std::string CDocument::GetDocFileName()
{
	if (m_filePath.empty()) return m_filePath;
	return FSDir::fileName(m_filePath);
}

//-----------------------------------------------------------------------------
std::string CDocument::GetDocFolder()
{
	if (m_filePath.empty()) return m_filePath;
	return FSDir::fileDir(m_filePath);
}

//-----------------------------------------------------------------------------
// get the base of the file name
std::string CDocument::GetDocFileBase()
{
	return FSDir::fileBase(m_filePath);
}

//-----------------------------------------------------------------------------
bool CDocument::SaveDocument(const std::string& fileName)
{
	SetDocFilePath(fileName);
	bool b = SaveDocument();
	if (b) SetModifiedFlag(false);
	return b;
}

//-----------------------------------------------------------------------------
bool CDocument::SaveDocument()
{
	return false;
}

//-----------------------------------------------------------------------------
bool CDocument::AutoSaveDocument()
{
	return true;
}

bool CDocument::loadPriorAutoSave()
{
	QFileInfo autoSaveInfo(m_autoSaveFilePath.c_str());

	if (autoSaveInfo.exists())
	{
		QFileInfo fileInfo(m_filePath.c_str());

		if (autoSaveInfo.lastModified() > fileInfo.lastModified()) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
QString CDocument::ToAbsolutePath(const QString& relativePath)
{
	return ToAbsolutePath(relativePath.toStdString());
}

//-----------------------------------------------------------------------------
QString CDocument::ToAbsolutePath(const std::string& relativePath)
{
	QString s = QString::fromStdString(FSDir::expandMacros(relativePath));
	QDir modelDir(QString::fromStdString(GetDocFolder()));
	return QDir::toNativeSeparators(QDir::cleanPath(modelDir.absoluteFilePath(s)));
}

std::string CDocument::GetIcon() const
{
	return m_iconName;
}

void CDocument::SetIcon(const std::string& iconName)
{
	m_iconName = iconName;
}
