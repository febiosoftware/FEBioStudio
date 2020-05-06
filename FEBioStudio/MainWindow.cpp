#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "GLView.h"
#include "ModelDocument.h"
#include "ModelFileReader.h"
#include <QApplication>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QMessageBox>
#include <QDirIterator>
#include <QDeskTopServices>
#include <QtCore/QMimeData>
#include <FSCore/FSObject.h>
#include <QtCore/QTimer>
#include <QFileDialog>
#include "DocTemplate.h"
#include "CreatePanel.h"
#include "FileThread.h"
#include "GLHighlighter.h"
#include <QStyleFactory>
#include "DlgAddMeshData.h"
#include "GraphWindow.h"
#include "DlgTimeSettings.h"
#include <PostGL/GLModel.h>
#include "DlgWidgetProps.h"
#include <FEBio/FEBioExport25.h>
#include <FEBio/FEBioExport3.h>
#include "FEBioJob.h"
#include <PostLib/ColorMap.h>
#include <FSCore/FSDir.h>
#include <QInputDialog>
#include "DlgCheck.h"
#include "Logger.h"
#include "SSHHandler.h"
#include "SSHThread.h"
#include "Encrypter.h"
#include "DlgImportXPLT.h"
#include "Commands.h"
#include <XPLTLib/xpltFileReader.h>
#include <MeshTools/GModel.h>
#include "DocManager.h"
#include "PostDocument.h"
#include "ModelDocument.h"
#ifdef HAS_QUAZIP
#include "ZipFiles.h"
#endif
#include "welcomePage.h"

extern GLColor col[];

CMainWindow*	CResource::m_wnd = nullptr;

void CResource::Init(CMainWindow* wnd) { m_wnd = wnd; }

QIcon CResource::Icon(const QString& iconName)
{
	assert(m_wnd);
	return m_wnd->GetResourceIcon(iconName);
}

// create a dark style theme (work in progress)
void darkStyle()
{
	qApp->setStyle(QStyleFactory::create("Fusion"));
	QPalette palette = qApp->palette();
	palette.setColor(QPalette::Window, QColor(53, 53, 53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(30, 30, 30));
	palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::black);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53, 53, 53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Highlight, QColor(51, 153, 255));
	palette.setColor(QPalette::HighlightedText, Qt::white);
	palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
	qApp->setPalette(palette);

	qApp->setStyleSheet("QMenu {margin: 2px} QMenu::separator {height: 1px; background: gray; margin-left: 10px; margin-right: 5px;}");
}

//-----------------------------------------------------------------------------
CMainWindow::CMainWindow(bool reset, QWidget* parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
	m_DocManager = new CDocManager(this);

	m_fileThread = nullptr;

	CResource::Init(this);

	setDockOptions(dockOptions() | QMainWindow::AllowNestedDocks | QMainWindow::GroupedDragging);

	// read the theme option, before we build the UI
	readThemeSetting();

	// setup the GUI
	ui->setupUi(this);

	// read the settings
	if (reset == false)
	{
		readSettings();
	}

	// get the welcome page
	ui->welcome->Refresh();

	// activate dark style
	if (ui->m_theme == 1)
	{
		darkStyle();

		// NOTE: I'm not sure if I can set the dark theme before I can create the document.
		//       Since the bg colors are already set, I need to do this here. Make sure
		//       the values set here coincide with the values from CDocument::NewDocument
/*		VIEW_SETTINGS& v = m_doc->GetViewSettings();
		v.m_col1 = GLColor(83, 83, 83);
		v.m_col2 = GLColor(128, 128, 128);
		v.m_nbgstyle = BG_HORIZONTAL;
*/
		GLWidget::set_base_color(GLColor(255, 255, 255));
	}
#ifdef LINUX
	if(ui->m_theme == 2)
	{
		qApp->setStyle(QStyleFactory::create("adwaita"));
	}
	else if(ui->m_theme == 3)
	{
		qApp->setStyle(QStyleFactory::create("adwaita-dark"));

		VIEW_SETTINGS& v = m_doc->GetViewSettings();
		v.m_col1 = GLColor(83, 83, 83);
		v.m_col2 = GLColor(128, 128, 128);
		v.m_nbgstyle = BG_HORIZONTAL;

		GLWidget::set_base_color(GLColor(255, 255, 255));
	}
#endif

	// allow drop events
	setAcceptDrops(true);

	// make sure the file viewer is visible
	ui->showFileViewer();

	// update the UI configuration
	UpdateUIConfig();

	// load templates
	TemplateManager::Init();

	// Instantiate Logger singleton
	CLogger::Instantiate(this);

	//Initialize the DatabasePanel. This requires information read in from the settings
	ui->databasePanel->Init(ui->m_repositoryFolder);
}

QIcon CMainWindow::GetResourceIcon(const QString& iconName)
{
	QString rs(iconName);
	if (ui->m_theme == 1)
	{
		rs += "_neg";
	}
	QString url = ":/icons/" + rs + ".png";

	// make sure the icon exists
	if (ui->m_theme == 1)
	{
		QFile f(url);
		if (!f.exists())
		{
			// use the regular version instead
			url = ":/icons/" + iconName + ".png";
		}
	}

	int dpr = devicePixelRatio();
	QPixmap pixmap(url);
//	pixmap.setDevicePixelRatio(dpr);
	QIcon icon;
	icon.addPixmap(pixmap);
	return icon;
}

//-----------------------------------------------------------------------------
CMainWindow::~CMainWindow()
{
	// delete document
	delete m_DocManager;
}

//-----------------------------------------------------------------------------
// get the current theme
int CMainWindow::currentTheme() const
{
	return ui->m_theme;
}

//-----------------------------------------------------------------------------
// clear command stack on save
bool CMainWindow::clearCommandStackOnSave() const
{
	return ui->m_clearUndoOnSave;
}

//-----------------------------------------------------------------------------
// set clear command stack on save
void CMainWindow::setClearCommandStackOnSave(bool b)
{
	ui->m_clearUndoOnSave = b;
}

//-----------------------------------------------------------------------------
// set the current theme
void CMainWindow::setCurrentTheme(int n)
{
	ui->m_theme = n;
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateTitle()
{
	QString projectName = ProjectName();
	if (projectName.isEmpty() == false)
	{
		setWindowTitle(projectName);
	}
	else setWindowTitle("");
}

//-----------------------------------------------------------------------------
//! update the tab's text
void CMainWindow::UpdateTab(CDocument* doc)
{
	if (doc == nullptr) return;

	int n = ui->tab->findView(doc);
	assert(n >= 0);
	if (n >= 0)
	{
		QString file = QString::fromStdString(doc->GetDocTitle());
		if (doc->IsModified()) file += "*";
		ui->tab->setTabText(n, file);

		QString path = QString::fromStdString(doc->GetDocFilePath());
		if (path.isEmpty() == false) ui->tab->setToolTip(path); else ui->tab->setToolTip("");
	}

	ui->fileViewer->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_clearProject()
{
	if (ui->m_project.Files() == 0) return;

	if (QMessageBox::question(this, "Clear Project", "Are you sure you want to clear the current project?\nThis cannot be undone.") == QMessageBox::Yes)
	{
		ui->m_project.Clear();
		ui->fileViewer->Update();
		UpdateTitle();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_closeProject()
{
	if (ui->m_project.Files() == 0) return;

	ui->m_project.Close();
	ui->fileViewer->Update();
	UpdateTitle();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_removeFromProject(const QString& file)
{
	ui->m_project.Remove(file);
	ui->fileViewer->Update();
	UpdateTitle();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_closeFile(const QString& file)
{
	CDocument* doc = FindDocument(file.toStdString());
	if (doc)
	{
		CloseView(doc);
		ui->fileViewer->Update();
		UpdateTitle();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_addToProject(const QString& file)
{
	ui->m_project.AddFile(file);
	ui->fileViewer->Update();
	UpdateTitle();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_welcome_anchorClicked(const QUrl& link)
{
	QString ref = link.toString();
	if      (ref == "#new") on_actionNewModel_triggered();
	else if (ref == "#open") on_actionOpen_triggered();
	else if (ref == "#openproject") on_actionOpenProject_triggered();
	else if (ref == "#febio") on_actionFEBioURL_triggered();
	else if (ref == "#help") on_actionFEBioResources_triggered();
	else
	{
		string s = ref.toStdString();
		const char* sz = s.c_str();
		if (strncmp(sz, "#recent_", 8) == 0)
		{
			int n = atoi(sz + 8);

			QStringList recentFiles = GetRecentFileList();
			OpenFile(recentFiles.at(n));
		}
	}
}

//-----------------------------------------------------------------------------
// Read a file asynchronously
// doc        : the document that is being modified 
// fileName   : the file name of the file that is read
// fileReader : the object that reads the file's content
// flags      : flags indicating what to do after the file is read
void CMainWindow::ReadFile(CDocument* doc, const QString& fileName, FileReader* fileReader, int flags)
{
	m_fileQueue.push_back(QueuedFile(doc, fileName, fileReader, flags));
	ReadNextFileInQueue();
}

void CMainWindow::OpenFile(const QString& filePath, bool showLoadOptions)
{
	// stop any animation
	if (ui->m_isAnimating) ui->postToolBar->CheckPlayButton(false);

	// convert to native separators
	QString fileName = QDir::toNativeSeparators(filePath);

	// check to extension to see what to do
	QString ext = QFileInfo(fileName).suffix();
	if ((ext.compare("fsm", Qt::CaseInsensitive) == 0) ||
		(ext.compare("prv", Qt::CaseInsensitive) == 0) ||
		(ext.compare("fsprj", Qt::CaseInsensitive) == 0))
	{
		OpenDocument(fileName);
	}
	else if (ext.compare("xplt", Qt::CaseInsensitive) == 0)
	{
		// load the plot file
		OpenPlotFile(fileName, showLoadOptions);
	}
	else if ((ext.compare("feb", Qt::CaseInsensitive) == 0) ||
		(ext.compare("inp", Qt::CaseInsensitive) == 0) ||
		(ext.compare("n", Qt::CaseInsensitive) == 0))
	{
		// load the feb file
		OpenFEModel(fileName);
	}
	else if (ext.compare("prj", Qt::CaseInsensitive) == 0)
	{
		// Extract the project archive and open it
		ImportProjectArchive(fileName);
	}
	else if (ext.compare("fsp", Qt::CaseInsensitive) == 0)
	{
		OpenProject(fileName);
	}
	else
	{
		// Open any other files (e.g. log files) with the system's associated program
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
		//		assert(false);
		//		QMessageBox::critical(this, "FEBio Studio", "Does not compute!");
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::ReadFile(QueuedFile& qfile)
{
	// if this is a reload, clear the document
	if (qfile.m_doc && (qfile.m_flags & QueuedFile::RELOAD_DOCUMENT))
	{
		qfile.m_doc->Clear();
		Update(nullptr, true);
	}

	// read the file, either threaded or directly
	if (qfile.m_flags & QueuedFile::NO_THREAD)
	{
		bool bret = false;
		QString errorString;
		if (qfile.m_fileReader == nullptr)
		{
			AddLogEntry("Don't know how to read file.");
		}
		else
		{
			string sfile = qfile.m_fileName.toStdString();
			bret = qfile.m_fileReader->Load(sfile.c_str());
			std::string err = qfile.m_fileReader->GetErrorMessage();
			errorString = QString::fromStdString(err);
		}
		finishedReadingFile(bret, qfile, errorString);
	}
	else
	{
		assert(m_fileThread == nullptr);
		m_fileThread = new CFileThread(this, qfile);
		m_fileThread->start();
		ui->statusBar->showMessage(QString("Reading file %1 ...").arg(qfile.m_fileName));
		ui->fileProgress->setValue(0);
		ui->statusBar->addPermanentWidget(ui->fileProgress);
		ui->fileProgress->show();
		AddLogEntry(QString("Reading file %1 ... ").arg(qfile.m_fileName));
		QTimer::singleShot(100, this, SLOT(checkFileProgress()));
	}
}

//-----------------------------------------------------------------------------
// read a list of files
void CMainWindow::ImportFiles(const QStringList& files)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr)
	{
		QMessageBox::critical(this, "Import Files", "No active document.");
		return;
	}

	// set the queue
	for (int i = 0; i < files.size(); ++i)
	{
		QString fileName = files[i];
		FileReader* fileReader = CreateFileReader(fileName);
		if (fileReader)
		{
			m_fileQueue.push_back(QueuedFile(doc, fileName, fileReader, 0));
		}
		else
		{
			AddLogEntry(QString("Don't know how to read: %1\n").arg(fileName));
			if (files.size() == 1)
			{
				QMessageBox::critical(this, "Import Geometry", "Don't know how to read this file.");
				return;
			}
		}
	}

	// start the process
	ReadNextFileInQueue();

	for (int i=0; i<files.count(); ++i)
		ui->addToRecentGeomFiles(files[i]);
}

#ifdef HAS_QUAZIP
//-----------------------------------------------------------------------------
// Import Project
void CMainWindow::ImportProjectArchive(const QString& fileName)
{
	QFileInfo fileInfo(fileName);

	// get the parent directory's name
	QString parentDirName = fileInfo.path();

	// create folder in which to unzip
	QDir parentDir(parentDirName);
	parentDir.mkdir(fileInfo.completeBaseName());
	QString destDir = parentDirName + "/" + fileInfo.completeBaseName();

	// extract files
	QStringList extractedFiles = extractAllFiles(fileName, destDir);

	// open first .fsprj file
	bool found = false;
	for(QString str : extractedFiles)
	{
		if(QFileInfo(str).suffix().compare("fsprj", Qt::CaseInsensitive) == 0)
		{
			found = true;
			OpenDocument(str);
			break;
		}
	}

	if(!found)
	{
		for(QString str : extractedFiles)
			{
				if(QFileInfo(str).suffix().compare("feb", Qt::CaseInsensitive) == 0)
				{
					found = true;
					OpenFEModel(str);
					break;
				}
			}
	}
}
#else
void CMainWindow::ImportProjectArchive(const QString& fileName) {}
#endif

//-----------------------------------------------------------------------------
void CMainWindow::ReadNextFileInQueue()
{
	// If a file is being processed, just wait
	if (m_fileThread) return;

	// make sure we have a file
	if (m_fileQueue.empty()) return;

	// get the next file name
	QueuedFile nextFile = m_fileQueue[0];

	// remove the last file that was read
	m_fileQueue.erase(m_fileQueue.begin());

	// start reading the file
	ReadFile(nextFile);
}

//-----------------------------------------------------------------------------
// Open a project
bool CMainWindow::OpenProject(const QString& projectFile)
{
	bool b = ui->m_project.Open(projectFile);
	if (b == false)
	{
		QMessageBox::critical(this, "ERROR", "Failed to open the project file.");
		return false;
	}

	ui->fileViewer->Update();
	ui->addToRecentFiles(projectFile);
	ui->fileViewer->parentWidget()->show();
	ui->fileViewer->parentWidget()->raise();
	UpdateTitle();

	return b;
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenDocument(const QString& fileName)
{
	QString filePath = QDir::toNativeSeparators(QDir::cleanPath(fileName));

	// Stop the timer if it's running
	if (ui->m_isAnimating) ui->m_isAnimating = false;

	// see if the file is already open
	int docs = m_DocManager->Documents();
	for (int i = 0; i < docs; ++i)
	{
		CDocument* doc = m_DocManager->GetDocument(i);
		std::string sfile = doc->GetDocFilePath();
		QString fileName_i = QString::fromStdString(sfile);
		if (filePath == fileName_i)
		{
			ui->tab->setCurrentIndex(i);
			on_actionRefresh_triggered();
			QApplication::alert(this);
			return;
		}
	}

	// create a new document
	CModelDocument* doc = new CModelDocument(this);
	doc->SetDocFilePath(filePath.toStdString());

	// start reading the file
	ReadFile(doc, filePath, new ModelFileReader(doc), QueuedFile::NEW_DOCUMENT);

	// add file to recent list
	ui->addToRecentFiles(filePath);
}

//! add a document 
void CMainWindow::AddDocument(CDocument* doc)
{
	// add it to the doc manager
	m_DocManager->AddDocument(doc);

	// make it the active one
	SetActiveDocument(doc);

	// add it to the project
	CModelDocument* modelDoc = dynamic_cast<CModelDocument*>(doc);
	if (modelDoc)
	{
		ui->m_project.AddFile(QString::fromStdString(modelDoc->GetDocFilePath()));
		ui->fileViewer->Update();
	}
}

//-----------------------------------------------------------------------------
bool compare_filename(const std::string& file1, const std::string& file2)
{
	if (file1.size() != file2.size()) return false;

	int l = file1.size();
	for (int i = 0; i < l; ++i)
	{
		char c1 = file1[i];
		char c2 = file2[i];

		if ((c1 == '/') || (c1 == '\\'))
		{
			if ((c2 != '/') && (c2 != '\\')) return false;
		}
		else if (c1 != c2) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
CDocument* CMainWindow::FindDocument(const std::string& filePath)
{
	for (int i = 0; i < m_DocManager->Documents(); ++i)
	{
		CDocument* doc = m_DocManager->GetDocument(i);
		std::string file_i = doc->GetDocFilePath();
		if (compare_filename(filePath, file_i)) return doc;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
//! Open a plot file
void CMainWindow::OpenPlotFile(const QString& fileName, bool showLoadOptions)
{
	// see if this file is already open
	CPostDocument* doc = dynamic_cast<CPostDocument*>(FindDocument(fileName.toStdString()));
	if (doc == nullptr)
	{
		doc = new CPostDocument(this);
		xpltFileReader* xplt = new xpltFileReader(doc->GetFEModel());
		doc->SetFileReader(xplt);
		if (showLoadOptions)
		{
			CDlgImportXPLT dlg(this);
			if (dlg.exec())
			{
				xplt->SetReadStateFlag(dlg.m_nop);
				xplt->SetReadStatesList(dlg.m_item);
			}
			else
			{
				delete doc;
				return;
			}
		}
		ReadFile(doc, fileName, doc->GetFileReader(), QueuedFile::NEW_DOCUMENT);
	}
	else
	{
		ReadFile(doc, fileName, doc->GetFileReader(), QueuedFile::RELOAD_DOCUMENT);
	}
}

//-----------------------------------------------------------------------------
//! get the mesh mode
int CMainWindow::GetMeshMode()
{
	if (ui->buildPanel->IsEditPanelVisible()) return MESH_MODE_SURFACE;
	else return MESH_MODE_VOLUME;
}

//-----------------------------------------------------------------------------
void CMainWindow::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::dropEvent(QDropEvent *e)
{
	foreach (const QUrl &url, e->mimeData()->urls()) {
		QString fileName = url.toLocalFile();
		// create a file reader
		FileReader* fileReader = CreateFileReader(fileName);
		// make sure we have one
		if (fileReader)
		{
			CDocument* doc = GetDocument();
			if (doc)
				ReadFile(doc, fileName, fileReader, 0);
		}
		else {
			OpenFile(fileName, false);
		}
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::checkFileProgress()
{
	float f = 1.f;
	if (m_fileThread) f = m_fileThread->getFileProgress();
	else return;

	int n = (int)(100.f*f);
	ui->fileProgress->setValue(n);
	if (f < 1.0f) QTimer::singleShot(100, this, SLOT(checkFileProgress()));
}

//-----------------------------------------------------------------------------
void CMainWindow::on_finishedReadingFile(bool success, const QString& errorString)
{
	assert(m_fileThread);
	if (m_fileThread == nullptr) return;
	QueuedFile qfile = m_fileThread->GetFile();
	m_fileThread = nullptr;
	finishedReadingFile(success, qfile, errorString);
}

//-----------------------------------------------------------------------------
void CMainWindow::finishedReadingFile(bool success, QueuedFile& file, const QString& errorString)
{
	ui->statusBar->clearMessage();
	ui->statusBar->removeWidget(ui->fileProgress);

	if (success == false)
	{
		if (m_fileQueue.empty())
		{
			QString err = QString("Failed reading file :\n%1").arg(errorString);
			QMessageBox::critical(this, "FEBio Studio", err);
		}

		QString err = QString("FAILED:\n%1\n").arg(errorString);
		AddLogEntry(err);

		// if this was a new document, we need to delete the document
		if (file.m_flags & QueuedFile::NEW_DOCUMENT)
		{
			delete file.m_doc;
		}

		return;
	}
	else
	{
		if (errorString.isEmpty() == false)
		{
			if (m_fileQueue.empty())
			{
				QMessageBox::information(this, "FEBio Studio", errorString);
			}
			AddLogEntry("success!\n");
			AddLogEntry("Warnings were generated:\n");
			AddLogEntry(errorString);
			AddLogEntry("\n");
		}
		else
		{
			AddLogEntry("success!\n");
		}

		// if this was a new document, make it the active one 
		if (file.m_flags & QueuedFile::NEW_DOCUMENT)
		{
			CDocument* doc = file.m_doc; assert(doc);
			doc->SetFileReader(file.m_fileReader);
			doc->SetDocFilePath(file.m_fileName.toStdString());
			bool b = doc->Initialize();
			if (b == false)
			{
				AddLogEntry("Document initialization failed!\n");
			}
			AddDocument(doc);

			// for fsprj files we set the "project" directory. 
			FSDir path(file.m_fileName.toStdString());
			if (path.fileExt() == "fsprj")
			{
				FSDir::setMacro("ProjectDir", ".");
			}
		}
		else if (file.m_flags & QueuedFile::RELOAD_DOCUMENT)
		{
			CDocument* doc = file.m_doc; assert(doc);
			bool b = doc->Initialize();
			if (b == false)
			{
				AddLogEntry("File could not be reloaded!\n");
			}
			SetActiveDocument(file.m_doc);
		}
		else
		{
			SetActiveDocument(file.m_doc);
		}
	}

	if (m_fileQueue.empty() == false)
	{
		ReadNextFileInQueue();
	}
	else
	{
		Reset();
		UpdatePhysicsUi();
		UpdateModel();
		UpdateToolbar();
		UpdatePostToolbar();
		Update();
		if (ui->modelViewer) ui->modelViewer->Show();

		// If the main window is not active, this will alert the user that the file has been read. 
		QApplication::alert(this, 0);
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::Update(QWidget* psend, bool breset)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	UpdateTab(doc);

	if (breset)
	{
		//	m_pGLView->OnZoomExtents(0,0);
		UpdateModel();
	}

	// redraw the GL view
	RedrawGL();

	// update the GL control bar
	UpdateGLControlBar();

	// Update the command windows
	if (ui->buildPanel->isVisible() && (psend != ui->buildPanel)) ui->buildPanel->Update(breset);

	//	if (m_pCurveEdit->visible() && (m_pCurveEdit != psend)) m_pCurveEdit->Update();
	if (ui->meshWnd && ui->meshWnd->isVisible()) ui->meshWnd->Update();

	if (ui->postPanel && ui->postPanel->isVisible()) ui->postPanel->Update(breset);

	if (ui->measureTool && ui->measureTool->isVisible()) ui->measureTool->Update();

	// update graph windows
	QList<::CGraphWindow*>::iterator it = ui->graphList.begin();
	for (int i = 0; i < ui->graphList.size(); ++i, ++it)
	{
		(*it)->Update(breset);
	}
}

//-----------------------------------------------------------------------------
CGLView* CMainWindow::GetGLView()
{
	return ui->glview;
}

//-----------------------------------------------------------------------------
CBuildPanel* CMainWindow::GetBuildPanel()
{
	return ui->buildPanel;
}

//-----------------------------------------------------------------------------
CCreatePanel* CMainWindow::GetCreatePanel()
{
	return ui->buildPanel->CreatePanel();
}

//-----------------------------------------------------------------------------
//! close the current open project
void CMainWindow::CloseProject()
{
	// close all views first
	int n = ui->tab->count();
	for (int i = 0; i < n; ++i) ui->tab->closeView(0);
}

//-----------------------------------------------------------------------------
//! This function resets the GL View. It is called when creating a new file new
//! (CWnd::OnFileNew) or when opening a file (CWnd::OnFileOpen). 
//! \sa CGLView::Reset
void CMainWindow::Reset()
{
	ui->glview->Reset();
	ui->glview->ZoomExtents(false);
}

//-----------------------------------------------------------------------------
//! Get the active document
CDocument* CMainWindow::GetDocument() 
{ 
	return ui->tab->getActiveDoc();
}

CModelDocument* CMainWindow::GetModelDocument() { return dynamic_cast<CModelDocument*>(GetDocument()); }
CPostDocument* CMainWindow::GetPostDocument() { return dynamic_cast<CPostDocument*>(GetDocument()); }

//-----------------------------------------------------------------------------
// get the document manager
CDocManager* CMainWindow::GetDocManager()
{
	return m_DocManager;
}

//-----------------------------------------------------------------------------
bool CMainWindow::maybeSave()
{
	// are there any unsaved documents open?
	int docs = m_DocManager->Documents();
	int unsavedDocs = 0;
	for (int i=0; i<docs; ++i)
	{ 
		CDocument* doc = m_DocManager->GetDocument(i);
		if (doc->IsModified()) unsavedDocs++;
	}

	if (unsavedDocs > 0)
	{
		QMessageBox::StandardButton b = QMessageBox::question(this, "", "There are unsaved documents open.\nAre you sure you want to exit?", QMessageBox::Yes | QMessageBox::No);
		if (b == QMessageBox::No) return false;
	}
	return true;
}

bool CMainWindow::maybeSave(CDocument* doc)
{
	if (doc && doc->IsModified())
	{
		QMessageBox::StandardButton b = QMessageBox::question(this, "", "The file was changed. Do you want to save it?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (b == QMessageBox::Cancel) return false;

		if (b == QMessageBox::Yes)
		{
			on_actionSave_triggered();
		}
		return true;
	}
	else return true;
}

void CMainWindow::ReportSelection()
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return;

	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0)) 
	{
		ClearStatusMessage();
		return;
	}

	GetCreatePanel()->setInput(sel);
	int N = sel->Size();
	QString msg;
	switch (sel->Type())
	{
	case SELECT_OBJECTS:
	{
		GObjectSelection& s = dynamic_cast<GObjectSelection&>(*sel);
		if (N == 1)
		{
			GObject* po = s.Object(0);
			msg = QString("Object \"%1\" selected (Id = %2)").arg(QString::fromStdString(po->GetName())).arg(po->GetID());
		}
		else msg = QString("%1 Objects selected").arg(N);
	}
	break;
	case SELECT_PARTS:
	{
		if (N == 1)
		{
			GPartSelection& ps = dynamic_cast<GPartSelection&>(*sel);
			GPartSelection::Iterator it(&ps);
			msg = QString("Part \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Parts selected").arg(N);
	}
	break;
	case SELECT_SURFACES:
	{
		if (N == 1)
		{
			GFaceSelection& fs = dynamic_cast<GFaceSelection&>(*sel);
			GFaceSelection::Iterator it(&fs);
			msg = QString("Surface \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Surfaces selected").arg(N);
	}
	break;
	case SELECT_CURVES:
	{
		GEdgeSelection& es = dynamic_cast<GEdgeSelection&>(*sel);
		if (N == 1)
		{
			GEdgeSelection::Iterator it(&es);
			msg = QString("Curve \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Curves selected").arg(N);
	}
	break;
	case SELECT_NODES:
	{
		if (N == 1)
		{
			GNodeSelection& ns = dynamic_cast<GNodeSelection&>(*sel);
			GNodeSelection::Iterator it(&ns);
			msg = QString("Node \"%1\" selected (Id = %2)").arg(QString::fromStdString(it->GetName())).arg(it->GetID());
		}
		else msg = QString("%1 Nodes selected").arg(N);
	}
	break;
	case SELECT_DISCRETE_OBJECT:
	{
		if (N == 1)
		{
			msg = QString("1 discrete object selected");
		}
		else msg = QString("%1 discrete objects selected").arg(N);
	}
	break;
	case SELECT_FE_ELEMENTS:
	{
		msg = QString("%1 elements selected").arg(N);
	}
	break;
	case SELECT_FE_FACES:
	{
		msg = QString("%1 faces selected").arg(N);
	}
	break;
	case SELECT_FE_EDGES:
	{
		msg = QString("%1 edges selected").arg(N);
	}
	break;
	case SELECT_FE_NODES:
	{
		msg = QString("%1 nodes selected").arg(N);
	}
	break;
	}
	SetStatusMessage(msg);
	AddLogEntry(msg + "\n");

	FEElementSelection* es = dynamic_cast<FEElementSelection*>(sel);
	if (es)
	{
		if (es->Size() == 1)
		{
			FEElement_* el = es->Element(0);
			switch (el->Type())
			{
			case FE_HEX8   : AddLogEntry("  Type = HEX8"   ); break;
			case FE_TET4   : AddLogEntry("  Type = TET4"   ); break;
			case FE_TET5   : AddLogEntry("  Type = TET5"   ); break;
			case FE_PENTA6 : AddLogEntry("  Type = PENTA6" ); break;
			case FE_QUAD4  : AddLogEntry("  Type = QUAD4"  ); break;
			case FE_TRI3   : AddLogEntry("  Type = TRI3"   ); break;
			case FE_BEAM2  : AddLogEntry("  Type = BEAM2"  ); break;
			case FE_HEX20  : AddLogEntry("  Type = HEX20"  ); break;
			case FE_QUAD8  : AddLogEntry("  Type = QUAD8"  ); break;
			case FE_BEAM3  : AddLogEntry("  Type = BEAM3"  ); break;
			case FE_TET10  : AddLogEntry("  Type = TET10"  ); break;
			case FE_TRI6   : AddLogEntry("  Type = TRI6"   ); break;
			case FE_TET15  : AddLogEntry("  Type = TET15"  ); break;
			case FE_HEX27  : AddLogEntry("  Type = HEX27"  ); break;
			case FE_TRI7   : AddLogEntry("  Type = TRI7"   ); break;
			case FE_QUAD9  : AddLogEntry("  Type = QUAD9"  ); break;
			case FE_PENTA15: AddLogEntry("  Type = PENTA15"); break;
			case FE_PYRA5  : AddLogEntry("  Type = PYRA5"  ); break;
			case FE_TET20  : AddLogEntry("  Type = TET20"  ); break;
			case FE_TRI10  : AddLogEntry("  Type = TRI10"  ); break;
			}
			AddLogEntry("\n");

			AddLogEntry("  nodes: ");
			int n = el->Nodes();
			for (int i=0; i<n; ++i)
			{
				AddLogEntry(QString::number(el->m_node[i]));
				if (i < n - 1) AddLogEntry(", ");
				else AddLogEntry("\n");
			}

			AddLogEntry("  neighbors: ");
			n = 0;
			if (el->IsSolid()) n = el->Faces();
			else if (el->IsShell()) n = el->Edges();

			for (int i=0; i<n; ++i)
			{
				AddLogEntry(QString::number(el->m_nbr[i]));
				if (i < n - 1) AddLogEntry(", ");
				else AddLogEntry("\n");
			}
		}
	}

	FEFaceSelection* fs = dynamic_cast<FEFaceSelection*>(sel);
	if (fs)
	{
		if (fs->Size() == 1)
		{
			FEFaceSelection::Iterator it = fs->begin();
			FEFace* pf = it;
			switch (pf->Type())
			{
			case FE_FACE_TRI3 : AddLogEntry("  Type = TRI3"); break;
			case FE_FACE_QUAD4: AddLogEntry("  Type = QUAD4"); break;
			case FE_FACE_TRI6 : AddLogEntry("  Type = TRI6"); break;
			case FE_FACE_TRI7 : AddLogEntry("  Type = TRI7"); break;
			case FE_FACE_QUAD8: AddLogEntry("  Type = QUAD8"); break;
			case FE_FACE_QUAD9: AddLogEntry("  Type = QUAD9"); break;
			case FE_FACE_TRI10: AddLogEntry("  Type = TRI10"); break;
			}
			AddLogEntry("\n");

			AddLogEntry("  neighbors: ");
			int n = pf->Edges();
			for (int i = 0; i<n; ++i)
			{
				AddLogEntry(QString::number(pf->m_nbr[i]));
				if (i < n - 1) AddLogEntry(", ");
				else AddLogEntry("\n");
			}
		}
	}
}

void CMainWindow::closeEvent(QCloseEvent* ev)
{
	if (maybeSave())
	{
		writeSettings();
		ev->accept();
	}
	else
		ev->ignore();
}

void CMainWindow::keyPressEvent(QKeyEvent* ev)
{
	if ((ev->key() == Qt::Key_Backtab) || (ev->key() == Qt::Key_Tab))
	{
		if (ev->modifiers() & Qt::ControlModifier)
		{
			ev->accept();

			int docs = ui->tab->count();
			if (docs > 1)
			{
				// activate the next document
				int i = ui->tab->currentIndex();

				// activate next or prev one 
				if (ev->key() == Qt::Key_Backtab)
				{
					i--;
					if (i < 0) i = docs - 1;
				}
				else
				{
					++i;
					if (i >= docs) i = 0;
				}
				ui->tab->setCurrentIndex(i);
			}
		}
	}
	else if (ev->key() == Qt::Key_Escape)
	{
		// give the build panels a chance to react first
		if (ui->buildPanel->OnEscapeEvent()) return;

		CModelDocument* doc = GetModelDocument();
		if (doc)
		{
			// if the build panel didn't process it, clear selection
			FESelection* ps = doc->GetCurrentSelection();
			if ((ps == 0) || (ps->Size() == 0))
			{
				if (doc->GetItemMode() != ITEM_MESH) doc->SetItemMode(ITEM_MESH);
				else ui->SetSelectionMode(SELECT_OBJECT);
				Update();
				UpdateUI();
			}
			else on_actionClearSelection_triggered();
			ev->accept();
			GLHighlighter::ClearHighlights();
			GLHighlighter::setTracking(false);
		}
	}
	else if ((ev->key() == Qt::Key_1) && (ev->modifiers() && Qt::CTRL)) 
	{
		ui->showFileViewer();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_2) && (ev->modifiers() && Qt::CTRL))
	{
		ui->showModelViewer();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_3) && (ev->modifiers() && Qt::CTRL))
	{
		ui->showBuildPanel();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_4) && (ev->modifiers() && Qt::CTRL))
	{
		ui->showPostPanel();
		ev->accept();
	}
}

void CMainWindow::SetCurrentFolder(const QString& folder)
{
	ui->currentPath = folder;
}

// get the current project
FEBioStudioProject* CMainWindow::GetProject()
{
	return &ui->m_project;
}

void CMainWindow::writeSettings()
{
	QSettings settings("MRLSoftware", "FEBio Studio");
	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.setValue("theme", ui->m_theme);
	settings.setValue("showProjectManangerOnStart", ui->m_showNewOnStartup);
	QRect rt;
	rt = CCurveEditor::preferredSize(); if (rt.isValid()) settings.setValue("curveEditorSize", rt);
	rt = CGraphWindow::preferredSize(); if (rt.isValid()) settings.setValue("graphWindowSize", rt);
	settings.endGroup();

	settings.beginGroup("PostSettings");
	settings.setValue("defaultMap", Post::ColorMapManager::GetDefaultMap());
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	settings.setValue("currentPath", ui->currentPath);

	settings.setValue("defaultProjectFolder", ui->m_defaultProjectParent);
	settings.setValue("repositoryFolder", ui->m_repositoryFolder);

	settings.setValue("recentFiles", ui->m_recentFiles);
	settings.setValue("recentGeomFiles", ui->m_recentGeomFiles);

	settings.endGroup();

	settings.beginGroup("LaunchConfigurations");
	// Create and save a list of launch config names
	QStringList launch_config_names;
	for(CLaunchConfig conf : ui->m_launch_configs)
	{
		launch_config_names.append(QString::fromStdString(conf.name));
	}
	settings.setValue("launchConfigNames", launch_config_names);

	// Save launch configs
	for(int i = 0; i < launch_config_names.count(); i++)
	{
		QString configName = "launchConfigs/" + launch_config_names[i];

		settings.setValue(configName + "/type", ui->m_launch_configs[i].type);
		settings.setValue(configName + "/path", ui->m_launch_configs[i].path.c_str());
		settings.setValue(configName + "/server", ui->m_launch_configs[i].server.c_str());
		settings.setValue(configName + "/port", ui->m_launch_configs[i].port);
		settings.setValue(configName + "/userName", ui->m_launch_configs[i].userName.c_str());
		settings.setValue(configName + "/remoteDir", ui->m_launch_configs[i].remoteDir.c_str());
//		settings.setValue(configName + "/jobName", ui->m_launch_configs[i].jobName.c_str());
//		settings.setValue(configName + "/walltime", ui->m_launch_configs[i].walltime.c_str());
//		settings.setValue(configName + "/procNum", ui->m_launch_configs[i].procNum);
//		settings.setValue(configName + "/ram", ui->m_launch_configs[i].ram);
		settings.setValue(configName + "/customFile", ui->m_launch_configs[i].customFile.c_str());
		settings.setValue(configName + "/text", ui->m_launch_configs[i].getText().c_str());
	}
	settings.endGroup();
}

void CMainWindow::readThemeSetting()
{
	QSettings settings("MRLSoftware", "FEBio Studio");
	settings.beginGroup("MainWindow");
	ui->m_theme = settings.value("theme", 0).toInt();
	settings.endGroup();
}


void CMainWindow::readSettings()
{
	QSettings settings("MRLSoftware", "FEBio Studio");
	settings.beginGroup("MainWindow");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	ui->m_theme = settings.value("theme", 0).toInt();
	ui->m_showNewOnStartup = settings.value("showProjectManangerOnStart", true).toBool();

	QRect rt;
	rt = settings.value("curveEditorSize", QRect()).toRect();
	if (rt.isValid()) CCurveEditor::setPreferredSize(rt);
	rt = settings.value("graphWindowSize", QRect()).toRect();
	if (rt.isValid()) CGraphWindow::setPreferredSize(rt);

	settings.endGroup();

	settings.beginGroup("PostSettings");
	Post::ColorMapManager::SetDefaultMap(settings.value("defaultMap", Post::ColorMapManager::JET).toInt());
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	ui->currentPath = settings.value("currentPath", QDir::homePath()).toString();

	ui->m_defaultProjectParent = settings.value("defaultProjectFolder", ui->m_defaultProjectParent).toString();
	ui->m_repositoryFolder = settings.value("repositoryFolder", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/modelRepo").toString();

	QStringList recentFiles = settings.value("recentFiles").toStringList(); ui->setRecentFiles(recentFiles);
	QStringList recentGeomFiles = settings.value("recentGeomFiles").toStringList(); ui->setRecentGeomFiles(recentGeomFiles);

	settings.endGroup();

	settings.beginGroup("LaunchConfigurations");

	QStringList launch_config_names;
	launch_config_names = settings.value("launchConfigNames", launch_config_names).toStringList();

	// Overwrite the default if they have launch configurations saved.
	if(launch_config_names.count() > 0)
	{
		ui->m_launch_configs.clear();
	}

	for(QString conf : launch_config_names)
	{
		QString configName = "launchConfigs/" + conf;

		ui->m_launch_configs.push_back(CLaunchConfig());

		ui->m_launch_configs.back().name = conf.toStdString();
		ui->m_launch_configs.back().type = settings.value(configName + "/type").toInt();
		ui->m_launch_configs.back().path = settings.value(configName + "/path").toString().toStdString();
		ui->m_launch_configs.back().server = settings.value(configName + "/server").toString().toStdString();
		ui->m_launch_configs.back().port = settings.value(configName + "/port").toInt();
		ui->m_launch_configs.back().userName = settings.value(configName + "/userName").toString().toStdString();
		ui->m_launch_configs.back().remoteDir = settings.value(configName + "/remoteDir").toString().toStdString();
//		ui->m_launch_configs.back().jobName = settings.value(configName + "/jobName").toString().toStdString();
//		ui->m_launch_configs.back().walltime = settings.value(configName + "/walltime").toString().toStdString();
//		ui->m_launch_configs.back().procNum = settings.value(configName + "/procNum").toInt();
//		ui->m_launch_configs.back().ram = settings.value(configName + "/ram").toInt();
		ui->m_launch_configs.back().customFile = settings.value(configName + "/customFile").toString().toStdString();
		ui->m_launch_configs.back().setText(settings.value(configName + "/text").toString().toStdString());

	}
	settings.endGroup();
}


//-----------------------------------------------------------------------------
void CMainWindow::UpdateUI()
{
	/*	m_pToolbar->Update();
	m_pCmdWnd->Update();
	if (m_pCurveEdit->visible()) m_pCurveEdit->Update();
	 */
	ui->glc->Update();
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateToolbar()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (doc->IsValid() == false) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	if (view.m_bfiber != ui->actionShowFibers->isChecked()) ui->actionShowFibers->trigger();
	if (view.m_blma   != ui->actionShowMatAxes->isChecked()) ui->actionShowMatAxes->trigger();
	if (view.m_bmesh  != ui->actionShowMeshLines->isChecked()) ui->actionShowMeshLines->trigger();
	if (view.m_bgrid  != ui->actionShowGrid->isChecked()) ui->actionShowGrid->trigger();
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenInCurveEditor(FSObject* po)
{
	//	OnToolsCurveEditor(0, 0);
	//	m_pCurveEdit->Select(po);
}

//-----------------------------------------------------------------------------
void CMainWindow::ShowInModelViewer(FSObject* po)
{
	ui->modelViewer->parentWidget()->raise();
	ui->modelViewer->Select(po);
	ui->modelViewer->UpdateObject(po);
}

//-----------------------------------------------------------------------------
//! set the selection mode
void CMainWindow::SetSelectionMode(int nselect)
{
	ui->SetSelectionMode(nselect);
}

//-----------------------------------------------------------------------------
//! Updates the model editor and selects object po.
//! \param po pointer to object that will be selected in the model editor
void CMainWindow::UpdateModel(FSObject* po, bool bupdate)
{
	if (ui->modelViewer)
	{
		if (bupdate)
		{
			ui->modelViewer->Update();
			if (po)
			{
				//				ui->showModelViewer();
				ui->modelViewer->Select(po);
			}
		}
		else ui->modelViewer->UpdateObject(po);
	}
}

//-----------------------------------------------------------------------------
//! Updates the GLView control bar
void CMainWindow::UpdateGLControlBar()
{
	ui->glc->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateUIConfig()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr)
	{
		Update(0, true);

		CDocument* doc = GetDocument();
		if (doc)
		{
			// Build Mode
			ui->setUIConfig(1);
			ui->modelViewer->parentWidget()->raise();
		}
		else
		{
			// no open docs
			// we need to update the welcome page since the recent
			// file list might have changed
			ui->welcome->Refresh();
			ui->setUIConfig(0);
			ui->fileViewer->parentWidget()->raise();
		}
		return;
	}
	else
	{
		// Post Mode
		ui->setUIConfig(2);

		UpdatePostPanel();
		if (postDoc->IsValid()) ui->postToolBar->Update();
		else ui->postToolBar->setDisabled(true);

		ui->postPanel->parentWidget()->raise();
	}
}

//-----------------------------------------------------------------------------
//! Update the post tool bar
void CMainWindow::UpdatePostToolbar()
{
	if (ui->postToolBar->isVisible())
		ui->postToolBar->Update();
}

//-----------------------------------------------------------------------------
//! set the post doc that will be rendered in the GL view
void CMainWindow::SetActiveDocument(CDocument* doc)
{
	int view = ui->tab->findView(doc);
	if (view == -1)
	{
		AddView(doc->GetDocFileName(), doc);
	}
	else
	{
		SetActiveView(view);
	}
}

//-----------------------------------------------------------------
int CMainWindow::Views()
{
	return ui->tab->views();
}

//-----------------------------------------------------------------
void CMainWindow::SetActiveView(int n)
{
	ui->tab->setActiveView(n);
	ui->glview->UpdateWidgets(false);
	UpdateUIConfig();
	RedrawGL();
}

//-----------------------------------------------------------------
int CMainWindow::FindView(CDocument* doc)
{
	return ui->tab->findView(doc);
}

//-----------------------------------------------------------------------------
GObject* CMainWindow::GetActiveObject()
{
	CDocument* doc = GetDocument();
	if (doc) return doc->GetActiveObject();
	return nullptr;
}

//-----------------------------------------------------------------
void CMainWindow::AddView(const std::string& viewName, CDocument* doc, bool makeActive)
{
	ui->tab->addView(viewName, doc, makeActive);
	ui->glview->ZoomExtents(false);
	ui->glview->UpdateWidgets(false);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_currentChanged(int n)
{
	UpdateUIConfig();
	UpdateGLControlBar();
	ui->updateMeshInspector();
	RedrawGL();

	if (n == 0) ui->modelViewer->parentWidget()->raise();
	else ui->postPanel->parentWidget()->raise();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_tabCloseRequested(int n)
{
	// Okay, remove the view
	CloseView(n);
	ui->fileViewer->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::OnPostObjectStateChanged()
{
	Post::CGLModel* mdl = GetCurrentModel();
	if (mdl == nullptr) return;
	bool b = mdl->GetColorMap()->IsActive();
	ui->postToolBar->CheckColorMap(b);
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::OnPostObjectPropsChanged(FSObject* po)
{
	Post::CGLModel* mdl = GetCurrentModel();
	if (mdl == nullptr) return;

	Post::CGLColorMap* colorMap = dynamic_cast<Post::CGLColorMap*>(po);
	if (colorMap)
	{
		int dataField = mdl->GetColorMap()->GetEvalField();
		ui->postToolBar->SetDataField(dataField);
	}
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::CloseView(int n)
{
	CDocument* doc = ui->tab->getDocument(n);

	if (doc->IsModified())
	{
		if (maybeSave(doc) == false) return;
	}

	m_DocManager->RemoveDocument(n);

	ui->tab->closeView(n);
	UpdateUIConfig();
}

//-----------------------------------------------------------------------------
void CMainWindow::CloseView(CDocument* doc)
{
	int n = FindView(doc);
	if (n >= 0) CloseView(n);
}

//-----------------------------------------------------------------------------
//! Update the post panel
void CMainWindow::UpdatePostPanel(bool braise, Post::CGLObject* po)
{
	ui->postPanel->Reset();
	if (braise) ui->postPanel->parentWidget()->raise();
	ui->postPanel->SelectObject(po);
}

//-----------------------------------------------------------------------------
void CMainWindow::RedrawGL()
{
	ui->glview->repaint();
}

//-----------------------------------------------------------------------------
//! Zoom to an FSObject
void CMainWindow::ZoomTo(const BOX& box)
{
	ui->glview->ZoomTo(box);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectObjects_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_OBJECT);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectParts_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_PART);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectSurfaces_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_FACE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectCurves_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_EDGE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectNodes_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_NODE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectDiscrete_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_DISCRETE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectRect_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_BOX);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectCircle_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_CIRCLE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectFree_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_FREE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_postSelectRect_toggled(bool b) { on_selectRect_toggled(b); }
void CMainWindow::on_postSelectCircle_toggled(bool b) { on_selectCircle_toggled(b); }
void CMainWindow::on_postSelectFree_toggled(bool b) { on_selectFree_toggled(b); }
void CMainWindow::on_postActionMeasureTool_triggered() { on_actionMeasureTool_triggered(); }

//-----------------------------------------------------------------------------
void CMainWindow::StopAnimation()
{
	ui->stopAnimation();
}

//-----------------------------------------------------------------------------
// add to the log 
void CMainWindow::AddLogEntry(const QString& txt)
{
	ui->logPanel->AddText(txt);
}

//-----------------------------------------------------------------------------
// add to the output window
void CMainWindow::AddOutputEntry(const QString& txt)
{
	ui->logPanel->AddText(txt, 1);
}

//-----------------------------------------------------------------------------
// clear the log
void CMainWindow::ClearLog()
{
	ui->logPanel->ClearLog();
}

//-----------------------------------------------------------------------------
// clear the output window
void CMainWindow::ClearOutput()
{
	ui->logPanel->ClearOutput();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_glview_pointPicked(const vec3d& r)
{
	GetCreatePanel()->setInput(r);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_glview_selectionChanged()
{
}

//-----------------------------------------------------------------------------
void CMainWindow::SetStatusMessage(const QString& message)
{
	ui->statusBar->showMessage(message);
}

//-----------------------------------------------------------------------------
void CMainWindow::ClearStatusMessage()
{
	ui->statusBar->clearMessage();
}

//-----------------------------------------------------------------------------
void CMainWindow::BuildContextMenu(QMenu& menu)
{
	menu.addAction(ui->actionZoomSelect);
	menu.addAction(ui->actionShowGrid);
	menu.addAction(ui->actionShowMeshLines);
	menu.addAction(ui->actionShowEdgeLines);
	menu.addAction(ui->actionOrtho);
	menu.addSeparator();

	QMenu* view = new QMenu("Standard views");
	view->addAction(ui->actionFront);
	view->addAction(ui->actionBack);
	view->addAction(ui->actionLeft);
	view->addAction(ui->actionRight);
	view->addAction(ui->actionTop);
	view->addAction(ui->actionBottom);

	menu.addAction(view->menuAction());
	menu.addSeparator();

	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr)
	{
		menu.addAction(ui->actionShowNormals);
		menu.addAction(ui->actionShowFibers);
		menu.addAction(ui->actionShowMatAxes);
		menu.addAction(ui->actionShowDiscrete);
		menu.addSeparator();

		// NOTE: Make sure the texts match the texts in OnSelectObjectTransparencyMode
		VIEW_SETTINGS& vs = ui->glview->GetViewSettings();
		QMenu* display = new QMenu("Object transparency mode");
		QAction* a;
		a = display->addAction("None"); a->setCheckable(true); if (vs.m_transparencyMode == 0) a->setChecked(true);
		a = display->addAction("Selected only"); a->setCheckable(true); if (vs.m_transparencyMode == 1) a->setChecked(true);
		a = display->addAction("Unselected only"); a->setCheckable(true); if (vs.m_transparencyMode == 2) a->setChecked(true);
		QObject::connect(display, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectObjectTransparencyMode(QAction*)));
		menu.addAction(display->menuAction());
		menu.addSeparator();

		CModelDocument* doc = GetModelDocument();
		if (doc)
		{
			GModel* gm = doc->GetGModel();
			int layers = gm->MeshLayers();
			if (layers > 1)
			{
				QMenu* sub = new QMenu("Set Active Mesh Layer");
				int activeLayer = gm->GetActiveMeshLayer();
				for (int i = 0; i < layers; ++i)
				{
					string s = gm->GetMeshLayerName(i);
					QAction* a = sub->addAction(QString::fromStdString(s));
					a->setCheckable(true);
					if (i == activeLayer) a->setChecked(true);
				}
				QObject::connect(sub, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectMeshLayer(QAction*)));
				menu.addAction(sub->menuAction());
				menu.addSeparator();
			}
		}
	}
	menu.addAction(ui->actionOptions);
}

//-----------------------------------------------------------------------------
void CMainWindow::OnSelectMeshLayer(QAction* ac)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel* gm = doc->GetGModel();

	string s = ac->text().toStdString();

	int layer = gm->FindMeshLayer(s); assert(layer >= 0);

	if (layer != gm->GetActiveMeshLayer())
	{
		// since objects may not have meshes in the new layer, we make sure we are 
		// in object selection mode
		if (doc->GetItemMode() != ITEM_MESH)
		{
			doc->SetItemMode(ITEM_MESH);
			UpdateGLControlBar();
		}

		// change the mesh layer
		doc->DoCommand(new CCmdSetActiveMeshLayer(gm, layer));
		UpdateModel();
		UpdateGLControlBar();
		ui->buildPanel->Update(true);
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::OnSelectObjectTransparencyMode(QAction* ac)
{
	VIEW_SETTINGS& vs = ui->glview->GetViewSettings();

	if      (ac->text() == "None"           ) vs.m_transparencyMode = 0;
	else if (ac->text() == "Selected only"  ) vs.m_transparencyMode = 1;
	else if (ac->text() == "Unselected only") vs.m_transparencyMode = 2;

	RedrawGL();
}

//-----------------------------------------------------------------------------
// Update the physics menu based on active modules
void CMainWindow::UpdatePhysicsUi()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	int module = prj.GetModule();

	ui->actionAddRigidConstraint->setVisible(module & MODULE_MECH);
	ui->actionAddRigidConnector->setVisible(module & MODULE_MECH);
	ui->actionSoluteTable->setVisible(module & MODULE_SOLUTES);
	ui->actionSBMTable->setVisible(module & MODULE_SOLUTES);
	ui->actionAddReaction->setVisible(module & MODULE_REACTIONS);
}

//-----------------------------------------------------------------------------
void CMainWindow::onExportAllMaterials()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEModel& fem = *doc->GetFEModel();

	vector<GMaterial*> matList;
	for (int i=0; i<fem.Materials(); ++i)
	{
		matList.push_back(fem.GetMaterial(i));
	}

	onExportMaterials(matList);
}

//-----------------------------------------------------------------------------
void CMainWindow::onExportMaterials(const vector<GMaterial*>& matList)
{
	if (matList.size() == 0)
	{
		QMessageBox::information(this, "Export Materials", "This project does not contain any materials");
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this, "Export Materials", "", "PreView Materials (*.pvm)");
	if (fileName.isEmpty() == false)
	{
		CDocument* doc = GetDocument();
//		if (doc->ExportMaterials(fileName.toStdString(), matList) == false)
		{
			QMessageBox::critical(this, "Export Materials", "Failed exporting materials");
		}
	}	
}

//-----------------------------------------------------------------------------
void CMainWindow::onImportMaterials()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Import Materials", "", "PreView Materials (*.pvm)");
	if (fileNames.isEmpty() == false)
	{
		for (int i=0; i<fileNames.size(); ++i)
		{
			QString file = fileNames.at(i);
			if (doc->ImportMaterials(file.toStdString()) == false)
			{
				QString msg = QString("Failed importing materials from\n%1").arg(file);
				QMessageBox::critical(this, "Import Materials", msg);
			}
		}

		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllMaterials()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all materials?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllMaterials();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllBC()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all boundary conditions?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllBC();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllLoads()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all loads?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllLoads();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllIC()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all initial conditions?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllIC();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllContact()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all contact interfaces?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllContact();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllConstraints()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all constraints?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllConstraints();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllRigidConstraints()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all rigid constraints?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllRigidConstraints();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllRigidConnectors()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all rigid connectors?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllRigidConnectors();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllSteps()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all steps?\nThis will also delete all boundary conditions, etc., associated with the steps.\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllSteps();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::ClearRecentProjectsList()
{
	ui->m_recentFiles.clear();
}

//-----------------------------------------------------------------------------
void CMainWindow::GenerateMap(FSObject* po)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgAddMeshData dlg(po, this);
	if (dlg.exec())
	{
		std::string mapName = dlg.GetMapName();
		std::string paramName = dlg.GetParamName();
		Param_Type paramType = dlg.GetParamType();

		FSObject* data = doc->CreateDataMap(po, mapName, paramName, paramType);
		if (data == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "It pains me to inform you that your command could not be executed.");
		}
		else UpdateModel(data);
	}
}

void CMainWindow::OnCameraChanged()
{
	if (ui->postPanel->isVisible())
	{
		ui->postPanel->OnViewChanged();
	}
}

// remove a graph from the list
void CMainWindow::RemoveGraph(::CGraphWindow* graph)
{
	ui->graphList.removeOne(graph);
}

// Add a graph to the list of managed graph windows
void CMainWindow::AddGraph(CGraphWindow* graph)
{
	graph->setWindowTitle(QString("Graph%1").arg(ui->graphList.size() + 1));
	ui->graphList.push_back(graph);
}

void CMainWindow::on_fontStyle_currentFontChanged(const QFont& font)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont old_font = pw->get_font();
		std::string s = font.family().toStdString();
		QFont new_font(font.family(), old_font.pointSize());
		new_font.setBold(old_font.bold());
		new_font.setItalic(old_font.italic());
		pw->set_font(new_font);
		RedrawGL();
	}
}

void CMainWindow::on_fontSize_valueChanged(int i)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		font.setPointSize(i);
		pw->set_font(font);
		RedrawGL();
	}
}

void CMainWindow::on_fontBold_toggled(bool checked)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		font.setBold(checked);
		pw->set_font(font);
		RedrawGL();
	}
}

void CMainWindow::on_fontItalic_toggled(bool bchecked)
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		font.setItalic(bchecked);
		pw->set_font(font);
		RedrawGL();
	}
}

void CMainWindow::on_actionProperties_triggered()
{
	// get the selected widget
	GLWidget* pglw = GLWidget::get_focus();
	if (pglw == 0) return;

	// edit the properties
	if (dynamic_cast<GLBox*>(pglw))
	{
		CDlgBoxProps dlg(pglw, this);
		dlg.exec();
	}
	else if (dynamic_cast<GLLegendBar*>(pglw))
	{
		CDlgLegendProps dlg(pglw, this);
		dlg.exec();
	}
	else if (dynamic_cast<GLTriad*>(pglw))
	{
		CDlgTriadProps dlg(pglw, this);
		dlg.exec();
	}
	else if (dynamic_cast<GLSafeFrame*>(pglw))
	{
		CDlgCaptureFrameProps dlg(pglw, this);
		dlg.exec();
	}
	else
	{
		QMessageBox::information(this, "Properties", "No properties available");
	}

	UpdateFontToolbar();

	RedrawGL();
}

void CMainWindow::UpdateFontToolbar()
{
	GLWidget* pw = GLWidget::get_focus();
	if (pw)
	{
		QFont font = pw->get_font();
		ui->pFontStyle->setCurrentFont(font);
		ui->pFontSize->setValue(font.pointSize());
		ui->actionFontBold->setChecked(font.bold());
		ui->actionFontItalic->setChecked(font.italic());
		ui->pFontToolBar->setEnabled(true);
	}
	else ui->pFontToolBar->setDisabled(true);
}

bool CMainWindow::DoModelCheck(CModelDocument* doc)
{
	if (doc == nullptr) return false;

	vector<MODEL_ERROR> warnings = doc->CheckModel();

	if (warnings.empty() == false)
	{
		CDlgCheck dlg(this);
		dlg.SetWarnings(warnings);
		if (dlg.exec() == 0)
		{
			return false;
		}
	}

	return true;
}

void CMainWindow::RunFEBioJob(CFEBioJob* job, bool autoSave)
{
	CModelDocument* doc = job->GetDocument();
	assert(doc);
	if (doc == nullptr) return;

	bool febioFileVersion = job->m_febVersion;
	bool writeNotes = job->m_writeNotes;
	QString cmd = QString::fromStdString(job->m_cmd);

	// see if we already have a job running.
	if (CFEBioJob::GetActiveJob())
	{
		QMessageBox::critical(this, "FEBio Studio", "Cannot start job since a job is already running");
		return;
	}

	// check the model first for issues
	if (DoModelCheck(doc) == false) return;

	// auto-save the document
	if (autoSave && doc->IsModified())
	{
		AddLogEntry(QString("saving %1 ...").arg(QString::fromStdString(doc->GetDocFilePath())));
		bool b = doc->SaveDocument();
		AddLogEntry(b ? "success\n" : "FAILED\n");
	}

	// get the FEBio job (relative) file path
	string febFile = job->GetFEBFileName();
	// do string substitution
	febFile = FSDir::expandMacros(febFile);

	// convert to an absolute path
	QDir modelDir(QString::fromStdString(doc->GetDocFolder()));
	QString absPath = modelDir.absoluteFilePath(QString::fromStdString(febFile));

	febFile = QDir::toNativeSeparators(absPath).toStdString();

	// try to save the file first
	AddLogEntry(QString("Saving to %1 ...").arg(QString::fromStdString(febFile)));

	if (febioFileVersion == 0)
	{
		FEBioExport25 feb(doc->GetProject());
		if (feb.Write(febFile.c_str()) == false)
		{
			QMessageBox::critical(this, "Run FEBio", "Failed saving FEBio file.");
			AddLogEntry("FAILED\n");
			return;
		}
		else AddLogEntry("SUCCESS!\n");
	}
	else if (febioFileVersion == 1)
	{
		FEBioExport3 feb(doc->GetProject());
		if (feb.Write(febFile.c_str()) == false)
		{
			QMessageBox::critical(this, "Run FEBio", "Failed saving FEBio file.");
			AddLogEntry("FAILED\n");
			return;
		}
		else AddLogEntry("SUCCESS!\n");
	}
	else
	{
		assert(false);
		QMessageBox::critical(this, "Run FEBio", "Don't know what file version to save.");
		AddLogEntry("FAILED\n");
		return;
	}

	// clear output for next job
	ClearOutput();

	// extract the working directory and file title from the file path
	QFileInfo fileInfo(absPath);
	QString workingDir = fileInfo.absolutePath();
	QString fileName = fileInfo.fileName();

	// set this as the active job
	CFEBioJob::SetActiveJob(job);

	if(job->GetLaunchConfig()->type == LOCAL)
	{
		// create new process
		ui->m_process = new QProcess(this);
		ui->m_process->setProcessChannelMode(QProcess::MergedChannels);
		if (workingDir.isEmpty() == false)
		{
			AddLogEntry(QString("Setting current working directory to: %1\n").arg(workingDir));
			ui->m_process->setWorkingDirectory(workingDir);
		}
		QString program = QString::fromStdString(job->GetLaunchConfig()->path);

		// do string substitution
		string sprogram = program.toStdString();
		sprogram = FSDir::expandMacros(sprogram);
		program = QString::fromStdString(sprogram);

		// extract the arguments
		QStringList args = cmd.split(" ", QString::SkipEmptyParts);

		std::string configFile = job->GetConfigFileName();

		args.replaceInStrings("$(Filename)", fileName);
		args.replaceInStrings("$(ConfigFile)", QString::fromStdString(configFile));

		// get ready
		AddLogEntry(QString("Starting FEBio: %1\n").arg(args.join(" ")));
		QObject::connect(ui->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRunFinished(int, QProcess::ExitStatus)));
		QObject::connect(ui->m_process, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		QObject::connect(ui->m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onErrorOccurred(QProcess::ProcessError)));

		// don't forget to reset the kill flag
		ui->m_bkillProcess = false;

		// go!
		ui->m_process->start(program, args);

		// show the output window
		ui->logPanel->ShowOutput();
	}
	else
	{
#ifdef HAS_SSH
		CSSHHandler* handler = job->GetSSHHandler();

		if(!handler->IsBusy())
		{
			handler->SetTargetFunction(STARTREMOTEJOB);

			CSSHThread* sshThread = new CSSHThread(handler, STARTSSHSESSION);
			QObject::connect(sshThread, &CSSHThread::FinishedPart, this, &CMainWindow::NextSSHFunction);
			sshThread->start();
		}

		// show the output window
		ui->logPanel->ShowOutput();

		CFEBioJob::SetActiveJob(nullptr);
#endif
	}

}

void CMainWindow::NextSSHFunction(CSSHHandler* sshHandler)
{
	if(!HandleSSHMessage(sshHandler))
	{
		sshHandler->EndSSHSession();

		return;
	}

	CSSHThread* sshThread = new CSSHThread(sshHandler, sshHandler->GetNextFunction());
	QObject::connect(sshThread, &CSSHThread::FinishedPart, this, &CMainWindow::NextSSHFunction);
	sshThread->start();
}


bool CMainWindow::HandleSSHMessage(CSSHHandler* sshHandler)
{
	QString QPasswd;
	QMessageBox::StandardButton reply;

	switch(sshHandler->GetMsgCode())
	{
	case FAILED:
		QMessageBox::critical(this, "FEBio Studio", sshHandler->GetMessage());
		return false;
	case NEEDSPSWD:
		bool ok;
		QPasswd = QInputDialog::getText(NULL, "Password", sshHandler->GetMessage(), QLineEdit::Password, "", &ok);

		if(ok)
		{
			std::string password = QPasswd.toStdString();
			sshHandler->SetPasswordLength(password.length());
			sshHandler->SetPasswdEnc(CEncrypter::Instance()->Encrypt(password));
		}
		else
		{
			return false;
		}
		break;
	case YESNODIALOG:
		 reply = QMessageBox::question(this, "FEBio Studio", sshHandler->GetMessage(),
				 QMessageBox::Yes|QMessageBox::No);

		 return reply == QMessageBox::Yes;
	case DONE:
		return false;
	}

	return true;
}

void CMainWindow::ShowProgress(bool show, QString message)
{
	if(show)
	{
		ui->statusBar->showMessage(message);
		ui->statusBar->addPermanentWidget(ui->fileProgress);
		ui->fileProgress->show();
	}
	else
	{
		ui->statusBar->clearMessage();
		ui->statusBar->removeWidget(ui->fileProgress);
	}
}

void CMainWindow::ShowIndeterminateProgress(bool show, QString message)
{
	if(show)
	{
		ui->statusBar->showMessage(message);
		ui->statusBar->addPermanentWidget(ui->indeterminateProgress);
		ui->indeterminateProgress->show();
	}
	else
	{
		ui->statusBar->clearMessage();
		ui->statusBar->removeWidget(ui->indeterminateProgress);
	}
}

void CMainWindow::UpdateProgress(int n)
{
	ui->fileProgress->setValue(n);
}


void CMainWindow::on_modelViewer_currentObjectChanged(FSObject* po)
{
	ui->infoPanel->SetObject(po);
}

void CMainWindow::toggleOrtho()
{
	ui->actionOrtho->trigger();
}

QStringList CMainWindow::GetRecentFileList()
{
	return ui->m_recentFiles;
}

QString CMainWindow::ProjectFolder()
{
	return "";// m_projectFolder;
}

QString CMainWindow::ProjectName()
{
	QString projectFile = ui->m_project.GetProjectFileName();
	if (projectFile.isEmpty()) return "";
	QFileInfo fi(projectFile);
	return fi.fileName();
}
