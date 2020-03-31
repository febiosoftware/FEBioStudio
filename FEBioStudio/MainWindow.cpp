#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "Document.h"
#include <QApplication>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QMessageBox>
#include <QtCore/QMimeData>
#include <FSCore/FSObject.h>
#include <MeshTools/PRVArchive.h>
#include <MeshIO/FileReader.h>
#include <QtCore/QTimer>
#include <QFileDialog>
#include <FEBio/FEBioImport.h>
#include "DocTemplate.h"
#include "CreatePanel.h"
#include "FileThread.h"
#include "GLHighlighter.h"
#include <Nike3D/NikeImport.h>
#include <Abaqus/AbaqusImport.h>
#include <QStyleFactory>
#include "DlgImportAbaqus.h"
#include "DlgAddMeshData.h"
#include "GraphWindow.h"
#include "PostDoc.h"
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

#ifdef HAS_QUAZIP
#include "ZipFiles.h"
#endif

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
}

//-----------------------------------------------------------------------------
CMainWindow::CMainWindow(bool reset, QWidget* parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
	m_doc = 0;

	m_fileThread = nullptr;
	m_postFileThread = nullptr;

	CResource::Init(this);

	setDockOptions(dockOptions() | QMainWindow::AllowNestedDocks | QMainWindow::GroupedDragging);

	// read the theme option, before we build the UI
	readThemeSetting();

	// setup the GUI
	ui->setupUi(this);

	// create a new document
	m_doc = new CDocument(this);

	// read the settings
	if (reset == false)
	{
		readSettings();
	}

	// activate dark style
	if (ui->m_theme == 1) 
	{
		darkStyle();

		// NOTE: I'm not sure if I can set the dark theme before I can create the document.
		//       Since the bg colors are already set, I need to do this here. Make sure
		//       the values set here coincide with the values from CDocument::NewDocument
		VIEW_SETTINGS& v = m_doc->GetViewSettings();
		v.m_col1 = GLColor(83, 83, 83);
		v.m_col2 = GLColor(128, 128, 128);
		v.m_nbgstyle = BG_HORIZONTAL;

		GLWidget::set_base_color(GLColor(255, 255, 255));
	}

	// allow drop events
	setAcceptDrops(true);

	// make sure the file viewer is visible
	ui->showFileViewer();

	// update the post toolbar (so it stays hidden on startup)
	UpdatePostToolbar();

	// load templates
	TemplateManager::Init();

	UpdateModel();

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
	delete m_doc;
}

//-----------------------------------------------------------------------------
// get the current theme
int CMainWindow::currentTheme() const
{
	return ui->m_theme;
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
	// get the file name
	if (m_doc)
	{
		std::string wndTitle = m_doc->GetDocFileName();
		if (wndTitle.empty()) wndTitle = "Untitled";

		if (m_doc->IsModified()) wndTitle += "*";

		wndTitle += " - FEBio Studio";
		setWindowTitle(QString::fromStdString(wndTitle));
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::ReadFile(const QString& fileName, FileReader* fileReader, bool clearDoc)
{
	if (clearDoc) GetDocument()->Clear();

	m_fileThread = new CFileThread(this, fileReader, false, fileName);
	m_fileThread->start();
	ui->statusBar->showMessage(QString("Reading file %1 ...").arg(fileName));
	ui->fileProgress->setValue(0);
	ui->statusBar->addPermanentWidget(ui->fileProgress);
	ui->fileProgress->show();
	AddLogEntry(QString("Reading file %1 ... ").arg(fileName));
	QTimer::singleShot(100, this, SLOT(checkFileProgress()));
}

//-----------------------------------------------------------------------------
// read a list of files
void CMainWindow::ImportFiles(const QStringList& files)
{
	// set the queue
	m_fileQueue = files;

	// start the process
	ReadNextFileInQueue();

	for (int i=0; i<files.count(); ++i)
		ui->addToRecentGeomFiles(files[i]);
}

#ifdef HAS_QUAZIP
//-----------------------------------------------------------------------------
// Import Project
void CMainWindow::ImportProject(const QString& fileName)
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
void CMainWindow::ImportProject(const QString& fileName) {}
#endif

//-----------------------------------------------------------------------------
void CMainWindow::ReadNextFileInQueue()
{
	// make sure we have a file
	if (m_fileQueue.isEmpty()) return;

	// get the next file name
	QString fileName = m_fileQueue.at(0);

	// remove the last file that was read
	m_fileQueue.removeAt(0);

	// create a file reader
	FileReader* fileReader = CreateFileReader(fileName);

	// make sure we have one
	if (fileReader)
	{
		ReadFile(fileName, fileReader, false);
	}
	else
	{
		QMessageBox::critical(this, "Read file", QString("Cannot read file\n{0}").arg(fileName));
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenDocument(const QString& fileName)
{
	m_fileQueue.clear();

	// check to see if the document is modified or not
	if (m_doc->IsModified())
	{
		QString msg("The project was modified since the last save.\nDo you want to save the project ?");
		int n = QMessageBox::question(this, "FEBio Studio", msg, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
		if (n == QMessageBox::Yes)
		{
			on_actionSave_triggered();
		}
		else if (n == QMessageBox::Cancel) return;
	}

	// start reading the file
	ReadFile(fileName, 0, true);

	// add file to recent list
	ui->addToRecentFiles(fileName);
}

//-----------------------------------------------------------------------------
//! Open a plot file
void CMainWindow::OpenPlotFile(const QString& fileName, bool showLoadOptions)
{
	xpltFileReader* xplt = nullptr;
	if (showLoadOptions)
	{
		CDlgImportXPLT dlg(this);
		if (dlg.exec())
		{
			xplt = new xpltFileReader;
			xplt->SetReadStateFlag(dlg.m_nop);
			xplt->SetReadStatesList(dlg.m_item);
		}
		else return;
	}

	CDocument* doc = GetDocument();
	std::string sfile = fileName.toStdString();

	// try to turn this into a relative path to the project folder
	string relPath = FSDir::toRelativePath(sfile);

	// create a dummy job
	CFEBioJob* job = new CFEBioJob(doc);
	job->SetPlotFileName(relPath);

	// set the filename as the job's name
	string fileBase = FSDir::fileBase(sfile);
	job->SetName(fileBase);

	// add it to the document
	doc->AddFEbioJob(job);

	// open the job's plotfile
	OpenPlotFile(job, xplt);
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenPlotFile(CFEBioJob* job, xpltFileReader* xpltReader)
{
	if (xpltReader == nullptr) xpltReader = new xpltFileReader();

	// create the file reading thread and run it
	m_postFileThread = new CPostFileThread(this, job, xpltReader);
	m_postFileThread->start();

	string plotFile = FSDir::toAbsolutePath(job->GetPlotFileName());
	QString fileName = QString::fromStdString(plotFile);
	ui->statusBar->showMessage(QString("Reading file %1 ...").arg(fileName));
	AddLogEntry(QString("Reading file %1 ...").arg(fileName));

	int H = ui->statusBar->height() - 5;
	ui->fileProgress->setFixedHeight(H);

	ui->fileProgress->setValue(0);
	ui->statusBar->addPermanentWidget(ui->fileProgress);
	ui->fileProgress->show();
	QTimer::singleShot(100, this, SLOT(checkFileProgress()));
}

//-----------------------------------------------------------------------------
//! get the mesh mode
int CMainWindow::GetMeshMode()
{
	if (ui->buildPanel->IsEditPanelVisible()) return MESH_MODE_SURFACE;
	else return MESH_MODE_VOLUME;
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenFEModel(const QString& fileName)
{
	m_fileQueue.clear();

	// create a file reader
	FileReader* reader = 0;
	QString ext = QFileInfo(fileName).suffix();
	if      (ext.compare("feb", Qt::CaseInsensitive) == 0) reader = new FEBioImport();
	else if (ext.compare("n"  , Qt::CaseInsensitive) == 0) reader = new FENIKEImport();
	else if (ext.compare("inp", Qt::CaseInsensitive) == 0) 
	{
		AbaqusImport* abaqusReader = new AbaqusImport();

		CDlgImportAbaqus dlg(abaqusReader, this);
		if (dlg.exec() == 0)
		{
			return;
		}

		abaqusReader->m_breadPhysics = true;
		reader = abaqusReader;
	}
	else
	{
		QMessageBox::critical(this, "Read File", QString("Cannot reade file\n%0").arg(fileName));
		return;
	}

	/*	// remove quotes from the filename
	char* ch = strchr((char*)szfile, '"');
	if (ch) szfile++;
	ch = strrchr((char*)szfile, '"');
	if (ch) *ch = 0;
	 */

	// start reading the file
	ReadFile(fileName, reader, true);

	// add file to recent list
	ui->addToRecentFEFiles(fileName);
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
			ReadFile(fileName, fileReader, false);
		}
		else {
			QString ext = QFileInfo(fileName).suffix();
			if ((ext.compare("prv", Qt::CaseInsensitive) == 0) ||
					(ext.compare("fsprj", Qt::CaseInsensitive) == 0))
				OpenDocument(fileName);
		}
	}
}
//-----------------------------------------------------------------------------
void CMainWindow::on_recentFiles_triggered(QAction* action)
{
	QString fileName = action->text();
	OpenDocument(fileName);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_recentFEFiles_triggered(QAction* action)
{
	QString fileName = action->text();
	OpenFEModel(fileName);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_recentGeomFiles_triggered(QAction* action)
{
	QString fileName = action->text();
	ImportFiles(QStringList(fileName));
}

//-----------------------------------------------------------------------------
void CMainWindow::checkFileProgress()
{
	float f = 1.f;
	if (m_fileThread) f = m_fileThread->getFileProgress();
	else if (m_postFileThread) f = m_postFileThread->getFileProgress();
	else return;

	int n = (int)(100.f*f);
	ui->fileProgress->setValue(n);
	if (f < 1.0f) QTimer::singleShot(100, this, SLOT(checkFileProgress()));
}

//-----------------------------------------------------------------------------
void CMainWindow::checkPostFileProgress()
{
	if (m_postFileThread)
	{
		float f = m_postFileThread->getFileProgress();
		int n = (int)(100.f*f);
		ui->fileProgress->setValue(n);
		if (f < 1.0f) QTimer::singleShot(100, this, SLOT(checkPostFileProgress()));
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::finishedReadingFile(bool success, const QString& errorString)
{
	m_fileThread = 0;
	ui->statusBar->clearMessage();
	ui->statusBar->removeWidget(ui->fileProgress);

	if (success == false)
	{
		if (m_fileQueue.isEmpty())
		{
			QString err = QString("Failed reading file :\n%1").arg(errorString);
			QMessageBox::critical(this, "FEBio Studio", err);
		}

		QString err = QString("FAILED:\n%1\n").arg(errorString);
		AddLogEntry(err);
		return;
	}
	else if (errorString.isEmpty() == false)
	{
		if (m_fileQueue.isEmpty())
		{
			QMessageBox::information(this, "FEBio Studio", errorString);
		}
		AddLogEntry("success!\n");
		AddLogEntry("Warnings were generated:\n");
		AddLogEntry(errorString);
	}
	else 
	{
		AddLogEntry("success!\n");
	}

	if (m_fileQueue.isEmpty() == false)
	{
		ReadNextFileInQueue();
	}
	else
	{
		UpdateTitle();
		Reset();
		UpdateModel();
		UpdateToolbar();
		Update();
		if (ui->modelViewer) ui->modelViewer->Show();

		// If the main window is not active, this will alert the user that the file has been read. 
		QApplication::alert(this, 0);
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::finishedReadingPostFile(bool success, const QString& errorString)
{
	if (m_postFileThread == nullptr) return;

	CFEBioJob* job = m_postFileThread->GetFEBioJob();
	m_postFileThread = nullptr;
	ui->statusBar->clearMessage();
	ui->statusBar->removeWidget(ui->fileProgress);

	if (success == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed loading plot file.");
		AddLogEntry("FAILED!\n");
	}
	else
	{
		AddLogEntry("success!\n");

		UpdateModel();
		ui->modelViewer->Select(job);
		SetActivePostDoc(job->GetPostDoc());

		UpdatePostPanel();
		UpdatePostToolbar();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::Update(QWidget* psend, bool breset)
{
	static bool bmodified = !m_doc->IsModified();

	if (bmodified != m_doc->IsModified())
	{
		UpdateTitle();
		bmodified = m_doc->IsModified();
	}

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
	if (ui->buildPanel->isVisible() && (psend != ui->buildPanel)) ui->buildPanel->Update();

	//	if (m_pCurveEdit->visible() && (m_pCurveEdit != psend)) m_pCurveEdit->Update();
	if (ui->meshWnd && ui->meshWnd->isVisible()) ui->meshWnd->Update();

	if (ui->postPanel && ui->postPanel->isVisible()) ui->postPanel->Update();
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
//! This function resets the GL View. It is called when creating a new file new
//! (CWnd::OnFileNew) or when opening a file (CWnd::OnFileOpen). 
//! \sa CGLView::Reset
void CMainWindow::Reset()
{
	ui->glview->Reset();
	ui->glview->ZoomExtents(false);
}

bool CMainWindow::maybeSave()
{
	CDocument* doc = GetDocument();
	if (doc->IsModified())
	{
		// If the model does not have a file associated and only contains
		// jobs, we will not ask the question
		std::string fileName = doc->GetDocFilePath();
		if (fileName.empty() && (doc->GetGModel()->Objects() == 0)) return true;
		
		QMessageBox::StandardButton b = QMessageBox::question(this, "", "The project was changed. Do you want to save it?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (b == QMessageBox::Cancel) return false;

		if (b==QMessageBox::Yes)
		{
			on_actionSave_triggered();
		}
		return true;
	}
	else return true;
}

void CMainWindow::ReportSelection()
{
	CDocument* doc = GetDocument();
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

	FEElementSelection* es = dynamic_cast<FEElementSelection*>(sel);
	if (es)
	{
		if (es->Size() == 1)
		{
			FEElement_* el = es->Element(0);
			AddLogEntry("1 element selected:\n");

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
			AddLogEntry("1 face selected:\n");
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

		// if the build panel didn't process it, clear selection
		FESelection* ps = m_doc->GetCurrentSelection();
		if ((ps == 0) || (ps->Size() == 0))
		{
			if (m_doc->GetItemMode() != ITEM_MESH) m_doc->SetItemMode(ITEM_MESH);
			else ui->SetSelectionMode(SELECT_OBJECT);
			Update();
			UpdateUI();
		}
		else on_actionClearSelection_triggered();
		ev->accept();
		GLHighlighter::ClearHighlights();
		GLHighlighter::setTracking(false);
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

void CMainWindow::writeSettings()
{
	QSettings settings("MRLSoftware", "FEBio Studio");
	settings.beginGroup("MainWindow");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("state", saveState());
	settings.setValue("theme", ui->m_theme);
	settings.endGroup();

	settings.beginGroup("PostSettings");
	settings.setValue("defaultMap", Post::ColorMapManager::GetDefaultMap());
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	settings.setValue("currentPath", ui->currentPath);

	settings.setValue("defaultProjectFolder", ui->m_defaultProjectFolder);
	settings.setValue("repositoryFolder", ui->m_repositoryFolder);

	QStringList folders = ui->fileViewer->FolderList();
	settings.setValue("folders", folders);

	settings.setValue("recentFiles", ui->m_recentFiles);
	settings.setValue("recentFEFiles", ui->m_recentFEFiles);
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
	settings.endGroup();

	settings.beginGroup("PostSettings");
	Post::ColorMapManager::SetDefaultMap(settings.value("defaultMap", Post::ColorMapManager::JET).toInt());
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	ui->currentPath = settings.value("currentPath", QDir::homePath()).toString();

	ui->m_defaultProjectFolder = settings.value("defaultProjectFolder", ui->m_defaultProjectFolder).toString();
	ui->m_repositoryFolder = settings.value("repositoryFolder", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/modelRepo").toString();

	QStringList folders = settings.value("folders").toStringList();
	if (folders.isEmpty() == false)
	{
		ui->fileViewer->SetFolderList(folders);
	}

	ui->fileViewer->setCurrentPath(ui->currentPath);

	QStringList recentFiles = settings.value("recentFiles").toStringList(); ui->setRecentFiles(recentFiles);
	QStringList recentFEFiles = settings.value("recentFEFiles").toStringList(); ui->setRecentFEFiles(recentFEFiles);
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
	if (m_doc->IsValid() == false) return;

	VIEW_SETTINGS& view = m_doc->GetViewSettings();
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
//! Update the post tool bar
void CMainWindow::UpdatePostToolbar()
{
	CPostDoc* doc = GetActiveDocument();
	if ((doc == nullptr) || (doc->IsValid() == false))
	{
		ui->postToolBar->setDisabled(true);
		ui->postToolBar->hide();
		return;
	}

	Post::CGLModel* mdl = doc->GetGLModel();
	if (mdl == 0)
	{
		ui->postToolBar->setDisabled(true);
		return;
	}

	Post::CGLColorMap* map = mdl->GetColorMap();

	// rebuild the menu
	Post::FEModel* pfem = doc->GetFEModel();
	ui->selectData->BuildMenu(pfem, Post::DATA_SCALAR);
	ui->selectData->blockSignals(true);
	ui->selectData->setCurrentValue(map->GetEvalField());
	ui->selectData->blockSignals(false);

	// update the color map state
	if (map->IsActive()) ui->actionColorMap->setChecked(true);
	else ui->actionColorMap->setChecked(false);

	// update the state indicator
	int ntime = mdl->CurrentTimeIndex() + 1;

	Post::FEModel* fem = mdl->GetFEModel();
	int states = fem->GetStates();
	QString suff = QString("/%1").arg(states);
	ui->pspin->setSuffix(suff);
	ui->pspin->setRange(1, states);
	ui->pspin->setValue(ntime);
	ui->postToolBar->setEnabled(true);
	if (ui->postToolBar->isHidden()) ui->postToolBar->show();
}

//-----------------------------------------------------------------------------
//! set the post doc that will be rendered in the GL view
void CMainWindow::SetActivePostDoc(CPostDoc* postDoc)
{
	if (postDoc == nullptr) 
		SetActiveView(0);
	else
	{
		int view = ui->tab->findView(postDoc);
		if (view == -1)
		{
			AddView(postDoc->GetName(), postDoc);
		}
		else
		{
			SetActiveView(view);
		}

		// raise the post panel
		ui->postPanel->parentWidget()->raise();
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
	RedrawGL();
}

//-----------------------------------------------------------------
CPostDoc* CMainWindow::GetActiveDocument()
{
	return ui->tab->getActiveDoc();
}

//-----------------------------------------------------------------
int CMainWindow::FindView(CPostDoc* postDoc)
{
	return ui->tab->findView(postDoc);
}

//-----------------------------------------------------------------------------
GObject* CMainWindow::GetActiveObject()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc == nullptr) return GetDocument()->GetActiveObject();
	return postDoc->GetPostObject();
}

//-----------------------------------------------------------------
void CMainWindow::AddView(const std::string& viewName, CPostDoc* doc, bool makeActive)
{
	ui->tab->addView(viewName, doc, makeActive);
	ui->glview->ZoomExtents(false);
	ui->glview->UpdateWidgets(false);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_currentChanged(int n)
{
	UpdatePostPanel();
	UpdatePostToolbar();
	ui->updateMeshInspector();
	RedrawGL();

	if (n == 0) ui->modelViewer->parentWidget()->raise();
	else ui->postPanel->parentWidget()->raise();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_tabCloseRequested(int n)
{
	// Don't close the first view, which is the model view
	if (n == 0) return;

	// Okay, remove the view
	CloseView(n);
}

//-----------------------------------------------------------------------------
void CMainWindow::OnPostObjectStateChanged()
{
	Post::CGLModel* mdl = GetCurrentModel();
	if (mdl == nullptr) return;
	bool b = mdl->GetColorMap()->IsActive();
	if (b != ui->actionColorMap->isChecked()) ui->actionColorMap->setChecked(b);
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::OnPostObjectPropsChanged(Post::CGLObject* po)
{
	Post::CGLModel* mdl = GetCurrentModel();
	if (mdl == nullptr) return;

	Post::CGLColorMap* colorMap = dynamic_cast<Post::CGLColorMap*>(po);
	if (colorMap)
	{
		int dataField = mdl->GetColorMap()->GetEvalField();
		if (ui->selectData->currentValue() != dataField) ui->selectData->setCurrentValue(dataField);
	}
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::CloseView(int n)
{
	ui->tab->closeView(n);
}

//-----------------------------------------------------------------------------
void CMainWindow::CloseView(CPostDoc* postDoc)
{
	int n = FindView(postDoc);
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
	if (b) m_doc->SetSelectionMode(SELECT_OBJECT); 
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectParts_toggled(bool b)
{
	if (b) m_doc->SetSelectionMode(SELECT_PART);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectSurfaces_toggled(bool b)
{
	if (b) m_doc->SetSelectionMode(SELECT_FACE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectCurves_toggled(bool b)
{
	if (b) m_doc->SetSelectionMode(SELECT_EDGE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectNodes_toggled(bool b)
{
	if (b) m_doc->SetSelectionMode(SELECT_NODE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectDiscrete_toggled(bool b)
{
	if (b) m_doc->SetSelectionMode(SELECT_DISCRETE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectRect_toggled(bool b)
{
	if (b) m_doc->SetSelectionStyle(REGION_SELECT_BOX);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectCircle_toggled(bool b)
{
	if (b) m_doc->SetSelectionStyle(REGION_SELECT_CIRCLE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectFree_toggled(bool b)
{
	if (b) m_doc->SetSelectionStyle(REGION_SELECT_FREE);
	Update();
}

//-----------------------------------------------------------------------------
// set the current time value
void CMainWindow::SetCurrentTimeValue(float ftime)
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	int n0 = doc->GetActiveState();
	doc->SetCurrentTimeValue(ftime);
	int n1 = doc->GetActiveState();

	if (n0 != n1)
	{
		ui->pspin->blockSignals(true);
		ui->pspin->setValue(n1 + 1);
		ui->pspin->blockSignals(false);
	}

	// update the rest
//	UpdateTools(false);
//	UpdateGraphs(false);
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::onTimer()
{
	if (ui->m_isAnimating == false) return;

	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();

	int N = doc->GetFEModel()->GetStates();
	int N0 = time.m_start;
	int N1 = time.m_end;

	int nstep = doc->GetActiveState();

	if (time.m_bfix)
	{
		float f0 = doc->GetTimeValue(N0);
		float f1 = doc->GetTimeValue(N1);

		float ftime = doc->GetTimeValue();

		if (time.m_mode == MODE_FORWARD)
		{
			ftime += time.m_dt;
			if (ftime > f1)
			{
				if (time.m_bloop) ftime = f0;
				else { ftime = f1; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_REVERSE)
		{
			ftime -= time.m_dt;
			if (ftime < f0)
			{
				if (time.m_bloop) ftime = f1;
				else { ftime = f0; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_CYLCE)
		{
			ftime += time.m_dt*time.m_inc;
			if (ftime > f1)
			{
				time.m_inc = -1;
				ftime = f1;
				if (time.m_bloop == false) StopAnimation();
			}
			else if (ftime < f0)
			{
				time.m_inc = 1;
				ftime = f0;
				if (time.m_bloop == false) StopAnimation();
			}
		}

		SetCurrentTimeValue(ftime);
	}
	else
	{
		if (time.m_mode == MODE_FORWARD)
		{
			nstep++;
			if (nstep > N1)
			{
				if (time.m_bloop) nstep = N0;
				else { nstep = N1; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_REVERSE)
		{
			nstep--;
			if (nstep < N0)
			{
				if (time.m_bloop) nstep = N1;
				else { nstep = N0; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_CYLCE)
		{
			nstep += time.m_inc;
			if (nstep > N1)
			{
				time.m_inc = -1;
				nstep = N1;
				if (time.m_bloop == false) StopAnimation();
			}
			else if (nstep < N0)
			{
				time.m_inc = 1;
				nstep = N0;
				if (time.m_bloop == false) StopAnimation();
			}
		}
		ui->pspin->setValue(nstep+1);
	}

	// TODO: Should I start the event before or after the view is redrawn?
	if (ui->m_isAnimating)
	{
		if (doc == nullptr) return;
		if (doc->IsValid())
		{
			TIMESETTINGS& time = doc->GetTimeSettings();
			double fps = time.m_fps;
			if (fps < 1.0) fps = 1.0;
			double msec_per_frame = 1000.0 / fps;
			QTimer::singleShot(msec_per_frame, this, SLOT(onTimer()));
		}
	}
}

void CMainWindow::on_selectData_currentValueChanged(int index)
{
	//	if (index == -1)
	//		ui->actionColorMap->setDisabled(true);
	//	else
	{
		//		if (ui->actionColorMap->isEnabled() == false)
		//			ui->actionColorMap->setEnabled(true);

		int nfield = ui->selectData->currentValue();
		CPostDoc* doc = GetActiveDocument();
		if (doc == nullptr) return;
		doc->SetDataField(nfield);

		// turn on the colormap
		if (ui->actionColorMap->isChecked() == false)
		{
			ui->actionColorMap->toggle();
		}

		ui->postPanel->SelectObject(doc->GetGLModel()->GetColorMap());

		ui->glview->UpdateWidgets(false);
		RedrawGL();
	}

	//	UpdateGraphs(false);

	//	if (ui->modelViewer->isVisible()) ui->modelViewer->Update(false);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionPlay_toggled(bool bchecked)
{
	CPostDoc* doc = GetActiveDocument();
	if (doc && doc->IsValid())
	{
		if (bchecked)
		{
			TIMESETTINGS& time = doc->GetTimeSettings();
			double fps = time.m_fps;
			if (fps < 1.0) fps = 1.0;
			double msec_per_frame = 1000.0 / fps;

			ui->m_isAnimating = true;
			QTimer::singleShot(msec_per_frame, this, SLOT(onTimer()));
		}
		else ui->m_isAnimating = false;
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionRefresh_triggered()
{
	ui->actionColorMap->setDisabled(true);
	ui->postToolBar->setDisabled(true);

	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	xpltFileReader xplt;
	if (doc->ReloadPlotfile(&xplt) == false)
	{
		QMessageBox::critical(this, tr("FEBio Studio"), "Failed updating the model");
	}
	else if (doc->IsValid())
	{
		int N = doc->GetFEModel()->GetStates();
		if (N > 1) ui->postToolBar->setEnabled(true);

		Post::FEModel* fem = doc->GetFEModel();
		int nfield = doc->GetEvalField();

		// we need to update the model viewer before we rebuild the selection menu
		ui->modelViewer->Update();

		// now, we can rebuild the 
		ui->selectData->BuildMenu(doc->GetFEModel(), Post::DATA_SCALAR);
		ui->selectData->blockSignals(true);
		ui->selectData->setCurrentValue(nfield);
		ui->selectData->blockSignals(false);
		ui->actionColorMap->setDisabled(false);

		// update the UI
		UpdatePostPanel();
		UpdatePostToolbar();
		ui->glview->UpdateCamera(true);
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionFirst_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	ui->pspin->setValue(time.m_start+1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionPrev_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	int nstep = doc->GetActiveState();
	nstep--;
	if (nstep < time.m_start) nstep = time.m_start;
	ui->pspin->setValue(nstep+1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionNext_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	int nstep = doc->GetActiveState();
	nstep++;
	if (nstep > time.m_end) nstep = time.m_end;
	ui->pspin->setValue(nstep+1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionLast_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	ui->pspin->setValue(time.m_end+1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionTimeSettings_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	CDlgTimeSettings dlg(doc, this);
	if (dlg.exec())
	{
		TIMESETTINGS& time = doc->GetTimeSettings();
		//		ui->timePanel->SetRange(time.m_start, time.m_end);

		int ntime = doc->GetActiveState();
		if ((ntime < time.m_start) || (ntime > time.m_end))
		{
			if (ntime < time.m_start) ntime = time.m_start;
			if (ntime > time.m_end) ntime = time.m_end;
		}

		ui->pspin->setValue(ntime + 1);
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectTime_valueChanged(int n)
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	doc->SetActiveState(n - 1);
	RedrawGL();
	ui->modelViewer->RefreshProperties();

	int graphs = ui->graphList.size();
	QList<CGraphWindow*>::iterator it = ui->graphList.begin();
	for (int i = 0; i < graphs; ++i, ++it)
	{
		CGraphWindow* w = *it;
		w->Update(false);
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::SetCurrentState(int n)
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	ui->pspin->setValue(n + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionColorMap_toggled(bool bchecked)
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	doc->ActivateColormap(bchecked);
	ui->postPanel->SelectObject(doc->GetGLModel()->GetColorMap());
	RedrawGL();
}

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
	CPostDoc* postDoc = GetActiveDocument();
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

	if (postDoc == nullptr)
	{
		menu.addAction(ui->actionShowNormals);
		menu.addAction(ui->actionShowFibers);
		menu.addAction(ui->actionShowMatAxes);
		menu.addAction(ui->actionShowDiscrete);
		menu.addSeparator();

		// NOTE: Make sure the texts match the texts in OnSelectObjectTransparencyMode
		VIEW_SETTINGS& vs = GetDocument()->GetViewSettings();
		QMenu* display = new QMenu("Object transparency mode");
		QAction* a;
		a = display->addAction("None"); a->setCheckable(true); if (vs.m_transparencyMode == 0) a->setChecked(true);
		a = display->addAction("Selected only"); a->setCheckable(true); if (vs.m_transparencyMode == 1) a->setChecked(true);
		a = display->addAction("Unselected only"); a->setCheckable(true); if (vs.m_transparencyMode == 2) a->setChecked(true);
		QObject::connect(display, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectObjectTransparencyMode(QAction*)));
		menu.addAction(display->menuAction());
		menu.addSeparator();

		if (GetActiveDocument() == nullptr)
		{
			GModel* gm = GetDocument()->GetGModel();
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
	CDocument* doc = GetDocument();
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
		ui->buildPanel->Update();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::OnSelectObjectTransparencyMode(QAction* ac)
{
	VIEW_SETTINGS& vs = GetDocument()->GetViewSettings();

	if      (ac->text() == "None"           ) vs.m_transparencyMode = 0;
	else if (ac->text() == "Selected only"  ) vs.m_transparencyMode = 1;
	else if (ac->text() == "Unselected only") vs.m_transparencyMode = 2;

	RedrawGL();
}

//-----------------------------------------------------------------------------
// Update the physics menu based on active modules
void CMainWindow::UpdatePhysicsUi()
{
	FEProject& prj = GetDocument()->GetProject();
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
	CDocument* doc = GetDocument();
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
		if (doc->ExportMaterials(fileName.toStdString(), matList) == false)
		{
			QMessageBox::critical(this, "Export Materials", "Failed exporting materials");
		}
	}	
}

//-----------------------------------------------------------------------------
void CMainWindow::onImportMaterials()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Import Materials", "", "PreView Materials (*.pvm)");
	if (fileNames.isEmpty() == false)
	{
		CDocument* doc = GetDocument();

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
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all materials?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllMaterials();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllBC()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all boundary conditions?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllBC();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllLoads()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all loads?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllLoads();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllIC()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all initial conditions?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllIC();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllContact()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all contact interfaces?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllContact();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllConstraints()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all constraints?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllConstraints();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllRigidConstraints()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all rigid constraints?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllRigidConstraints();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllRigidConnectors()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all rigid connectors?\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
		FEModel& fem = *doc->GetFEModel();
		fem.DeleteAllRigidConnectors();
		UpdateModel();
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::DeleteAllSteps()
{
	if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete all steps?\nThis will also delete all boundary conditions, etc., associated with the steps.\nThis cannot be undone.", QMessageBox::Ok | QMessageBox::Cancel))
	{
		CDocument* doc = GetDocument();
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
	CDlgAddMeshData dlg(po, this);
	if (dlg.exec())
	{
		std::string mapName = dlg.GetMapName();
		std::string paramName = dlg.GetParamName();
		Param_Type paramType = dlg.GetParamType();

		CDocument* doc = GetDocument();
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

bool CMainWindow::DoModelCheck()
{
	CDocument* doc = GetDocument();

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

void CMainWindow::RunFEBioJob(CFEBioJob* job)
{
	CDocument* doc = GetDocument();

	bool febioFileVersion = job->m_febVersion;
	bool writeNotes = job->m_writeNotes;
	QString cmd = QString::fromStdString(job->m_cmd);

	// see if we already have a job running.
	if (doc->GetActiveJob())
	{
		QMessageBox::critical(this, "FEBio Studio", "Cannot start job since a job is already running");
		return;
	}

	// get the FEBio job file path
	string filePath = job->GetFileName();

	// do string substitution
	filePath = FSDir::toAbsolutePath(filePath);

	// check the model first for issues
	if (DoModelCheck() == false) return;

	// try to save the file first
	AddLogEntry(QString("Saving to %1 ...").arg(QString::fromStdString(filePath)));

	if (febioFileVersion == 0)
	{
		FEBioExport25 feb;
		if (feb.Export(doc->GetProject(), filePath.c_str()) == false)
		{
			QMessageBox::critical(this, "Run FEBio", "Failed saving FEBio file.");
			AddLogEntry("FAILED\n");
			return;
		}
		else AddLogEntry("SUCCESS!\n");
	}
	else if (febioFileVersion == 1)
	{
		FEBioExport3 feb;
		if (feb.Export(doc->GetProject(), filePath.c_str()) == false)
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
	size_t n = filePath.rfind('/');
	if (n == string::npos) n = filePath.rfind('\\');

	string cwd, fileName;
	if (n != string::npos)
	{
		cwd = filePath.substr(0, n);
		fileName = filePath.substr(n + 1, string::npos);
	}
	else fileName = filePath;

	// set this as the active job
	doc->SetActiveJob(job);

	if(job->GetLaunchConfig()->type == LOCAL)
	{
		// create new process
		ui->m_process = new QProcess(this);
		ui->m_process->setProcessChannelMode(QProcess::MergedChannels);
		if (cwd.empty() == false)
		{
			QString wd = QString::fromStdString(cwd);
			AddLogEntry(QString("Setting current working directory to: %1\n").arg(wd));
			ui->m_process->setWorkingDirectory(wd);
		}
		QString program = QString::fromStdString(job->GetLaunchConfig()->path);

		// do string substitution
		string sprogram = program.toStdString();
		sprogram = FSDir::toAbsolutePath(sprogram);
		program = QString::fromStdString(sprogram);

		// extract the arguments
		QStringList args = cmd.split(" ", QString::SkipEmptyParts);

		std::string configFile = job->GetConfigFileName();

		args.replaceInStrings("$(Filename)", QString::fromStdString(fileName));
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

		GetDocument()->SetActiveJob(nullptr);
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
