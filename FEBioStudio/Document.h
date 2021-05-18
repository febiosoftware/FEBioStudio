/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <MeshTools/FEProject.h>
#include <MeshTools/FESelection.h>
#include <FSCore/LoadCurve.h>
#include <FSCore/Serializable.h>
#include <MeshIO/FileReader.h>
#include "CommandManager.h"
#include "FEBioOpt.h"
#include <PostLib/ImageModel.h>
#include <FSCore/FSObjectList.h>
#include "modelcheck.h"
#include <QtCore/QString>
#include "ViewSettings.h"
#include <GLLib/GView.h>

//-----------------------------------------------------------------------------
// Transform Modes
#define TRANSFORM_NONE		1
#define TRANSFORM_MOVE		2
#define TRANSFORM_ROTATE	3
#define TRANSFORM_SCALE		4

//-----------------------------------------------------------------------------
// Selection Modes
#define SELECT_OBJECT	1
#define SELECT_PART		2
#define SELECT_FACE		3
#define SELECT_EDGE		4
#define SELECT_NODE		5
#define SELECT_DISCRETE	6

//-----------------------------------------------------------------------------
// Selection Styles
#define REGION_SELECT_BOX		1
#define REGION_SELECT_CIRCLE	2
#define REGION_SELECT_FREE		3

//-----------------------------------------------------------------------------
// Sub-object item Modes
#define	ITEM_MESH		1
#define ITEM_ELEM		2
#define	ITEM_FACE		3
#define ITEM_NODE		4
#define ITEM_EDGE		5

//-----------------------------------------------------------------------------
// mesh selection modes
#define MESH_MODE_VOLUME	0
#define MESH_MODE_SURFACE	1

//-----------------------------------------------------------------------------
// back ground styles
#define BG_COLOR1		0
#define BG_COLOR2		1
#define BG_HORIZONTAL	2
#define BG_VERTICAL		3

//-----------------------------------------------------------------------------
// render mode
#define RENDER_WIREFRAME	1
#define RENDER_SOLID		0

//-----------------------------------------------------------------------------
class CMainWindow;
class FEFileExport;
class CDocument;
class FEModifier;
class FESurfaceModifier;
class GSurfaceMeshObject;
class FileReader;
class FileWriter;

namespace Post {
	class CImageModel;
}

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
class CDocument : public CSerializable
{
public:
	// --- constructor/destructor ---
	CDocument(CMainWindow* wnd);
	virtual ~CDocument();

	// clear the model
	virtual void Clear();

	// initialize the model
	// this is called after a document was loaded
	virtual bool Initialize();

	// will be called when the document is activated
	virtual void Activate();

	// will be called when the document is deactivate
	virtual void Deactivate();

	CMainWindow* GetMainWindow();

public:
	// --- Document validation ---
	bool IsModified();
	void SetModifiedFlag(bool bset = true);
	bool IsValid();

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

//-----------------------------------------------------------------------------
// Base class for documents that require visualization
class CGLDocument : public CDocument
{
public:
	CGLDocument(CMainWindow* wnd);
	~CGLDocument();

	void Clear() override;

	bool SaveDocument() override;

	bool AutoSaveDocument() override;

	// set/get the file reader
	void SetFileReader(FileReader* fileReader);
	FileReader* GetFileReader();

	// set/get the file writer
	void SetFileWriter(FileWriter* fileWriter);
	FileWriter* GetFileWriter();

	// import image data
#ifdef HAS_TEEM
  Post::CImageModel* ImportTiff(const std::string& filename);
  Post::CImageModel* ImportNrrd(const std::string& filename);
#endif

#ifdef HAS_DICOM
  Post::CImageModel* ImportDicom(const std::string& filename);
#endif
	Post::CImageModel* ImportImage(const std::string& fileName, int nx, int ny, int nz, BOX box);

	// --- Command history functions ---
	bool CanUndo();
	bool CanRedo();
	void AddCommand(CCommand* pcmd);
	void AddCommand(CCommand* pcmd, const std::string& s);
	bool DoCommand(CCommand* pcmd, bool b = true);
	bool DoCommand(CCommand* pcmd, const std::string& s, bool b = true);
	void UndoCommand();
	void RedoCommand();
	const char* GetUndoCmdName();
	const char* GetRedoCmdName();
	void ClearCommandStack();
	const std::string& GetCommandErrorString() const;

	// --- view state ---
	VIEW_STATE GetViewState() { return m_vs; }
	void SetViewState(VIEW_STATE vs);

	int GetTransformMode() { return m_vs.ntrans; }
	void SetTransformMode(int mode) { m_vs.ntrans = mode; UpdateSelection(false); }

	int GetSelectionMode() { return m_vs.nselect; }
	void SetSelectionMode(int mode) { m_vs.nitem = ITEM_MESH; m_vs.nselect = mode; UpdateSelection(false); }

	void SetSelectionStyle(int nstyle) { m_vs.nstyle = nstyle; }
	int GetSelectionStyle() { return m_vs.nstyle; }

	int GetItemMode() { return m_vs.nitem; }
	void SetItemMode(int mode) { m_vs.nitem = mode; UpdateSelection(false); }

	static std::string GetTypeString(FSObject* po);

	virtual void UpdateSelection(bool breport = true);

	virtual GObject* GetActiveObject();

	CGView* GetView();

public:
	void setModelInfo(const std::string& s) { m_info = s; }
	std::string getModelInfo() const { return m_info; }

public:
	int ImageModels() const;
	void AddImageModel(Post::CImageModel* img);
	Post::CImageModel* GetImageModel(int i);
	void DeleteAllImageModels();

protected:
	void SaveResources(OArchive& ar);
	void LoadResources(IArchive& ar);

public:
	void SetUnitSystem(int unitSystem);
	int GetUnitSystem() const;

protected:
	// The command manager
	CCommandManager*	m_pCmd;		// the command manager

	CGView				m_view;

	VIEW_STATE	m_vs;	// the view state

	std::string		m_info;
	int				m_units;

	FSObjectList<Post::CImageModel>	m_img;

	FileReader*		m_fileReader;
	FileWriter*		m_fileWriter;
};
