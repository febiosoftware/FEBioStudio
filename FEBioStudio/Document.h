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
#include <FSCore/Serializable.h>
#include "PropertyList.h"
#include <QtCore/QString>
#include <QObject>

class CMainWindow;
class CDocument;

//-----------------------------------------------------------------------------
// Class that can be used to monitor changes to the document
class CDocObserver
{
public:
	CDocObserver(CDocument* doc);
	virtual ~CDocObserver();

	// bnewFlag is set when a new model was loaded
	virtual void DocumentUpdate(bool bnewFlag) {}

	// this function is called when the document is about to be deleted
	virtual void DocumentDelete();

	// get the document
	CDocument* GetDocument() { return m_doc; }

private:
	CDocument*	m_doc;
};

//-----------------------------------------------------------------------------
// Document class stores data and implements serialization of data to and from file.
//
class CDocument : public QObject, public CSerializable
{
	Q_OBJECT

public:
	// --- constructor/destructor ---
	CDocument(CMainWindow* wnd);
	virtual ~CDocument();

	// clear the model
	virtual void Clear();

	// initialize the model
	// this is called after a document was loaded
	virtual bool Initialize();

	// update internals
	virtual void Update();

	// will be called when the document is activated
	virtual void Activate();

	// will be called when the document is deactivate
	virtual void Deactivate();

	CMainWindow* GetMainWindow();

public:
	// --- Document validation ---
	bool IsModified() const;
	virtual void SetModifiedFlag(bool bset = true);
	bool IsValid() const;

public:
	// --- I/O-routines ---
	// Save the document
	bool SaveDocument(const std::string& fileName);

	virtual bool SaveDocument();

	// Autosave
	virtual bool AutoSaveDocument();
	bool loadPriorAutoSave();

	// set the document's title
	void SetDocTitle(const std::string& t);

	// get the document's title
	std::string GetDocTitle();

	// get the complete file path
	std::string GetDocFilePath();

	// set the document file path
	void SetDocFilePath(const std::string& fileName);

	// get and set the document autosave path
	std::string GetAutoSaveFilePath();
	void SetAutoSaveFilePath();

	void SetUnsaved();

	// get just the file name
	std::string GetDocFileName();

	// get the file folder
	std::string GetDocFolder();

	// get the base of the file name
	std::string GetDocFileBase();

	// return the absolute path from the relative path w.r.t. to the model's folder
	QString ToAbsolutePath(const QString& relativePath);
	QString ToAbsolutePath(const std::string& relativePath);

	virtual CPropertyList* GetDocProperties() { return nullptr; }

public:
	std::string GetIcon() const;
	void SetIcon(const std::string& iconName);

public:
	// --- Document observers ---
	void AddObserver(CDocObserver* observer);
	void RemoveObserver(CDocObserver* observer);
	void UpdateObservers(bool bnew);

public:
	static CDocument* GetActiveDocument();
	static void SetActiveDocument(CDocument* doc);

protected:
	// Modified flag
	bool	m_bModified;	// is document modified since last saved ?
	bool	m_bValid;		// is the current document in a valid state for rendering

	// title
	std::string		m_title;

	// file path
	std::string		m_filePath;
	std::string		m_autoSaveFilePath;

	std::string		m_iconName;

	CMainWindow*	m_wnd;
	std::vector<CDocObserver*>	m_Observers;

	static CDocument*	m_activeDoc;
};
