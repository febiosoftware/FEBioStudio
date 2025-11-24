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
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "GLView.h"
#include "ImageSliceView.h"
#include "ModelFileReader.h"
#include <QApplication>
#include <QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QMessageBox>
#include <QDirIterator>
#include <QPushButton>
#include <QDesktopServices>
#include <QtCore/QMimeData>
#include <FSCore/FSObject.h>
#include <QtCore/QTimer>
#include <QFileDialog>
#include <QTimer>
#include "DocTemplate.h"
#include "CreatePanel.h"
#include "FileThread.h"
#include "GLHighlighter.h"
#include <QStyleFactory>
#include <QStyleHints>
#include <QDropEvent>
#include "GraphWindow.h"
#include <PostGL/GLModel.h>
#include "DlgWidgetProps.h"
#include <FEBio/FEBioExport25.h>
#include <FEBio/FEBioExport3.h>
#include <FEBio/FEBioExport4.h>
#include "FEBioJob.h"
#include <FSCore/FSDir.h>
#include <QInputDialog>
#include <QUuid>
#include "DlgCheck.h"
#include "IconProvider.h"
#include "SSHHandler.h"
#include "Encrypter.h"
#include "DlgImportXPLT.h"
#include "Commands.h"
#include <XPLTLib/xpltFileReader.h>
#include <GeomLib/GModel.h>
#include "DocManager.h"
#include "PostDocument.h"
#include "ModelDocument.h"
#include "TextDocument.h"
#include "XMLDocument.h"
#include "PostSessionFile.h"
#include "units.h"
#include "version.h"
#include "DlgStartThread.h"
#include <PostLib/VTKImport.h>
#include <PostLib/FELSDYNAPlot.h>
#include <PostLib/FELSDYNAimport.h>
#include <PostLib/FESTLimport.h>
#include "ImageThread.h"
#ifdef HAS_LIBZIP
#include "ZipFiles.h"
#endif
#include "welcomePage.h"
#include <FSCore/Palette.h>
#include <ImageLib/SITKImageSource.h>
#include <ImageLib/ImageModel.h>
#include <PostGL/GLColorMap.h>
#include <FSCore/ColorMapManager.h>
#include <GLWLib/convert.h>
#include <GLWLib/GLLabel.h>
#include <GLWLib/GLTriad.h>
#include <GLWLib/GLSafeFrame.h>
#include <GLWLib/GLLegendBar.h>
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioInit.h>
#include <qmenu.h>
#include <GLLib/GLViewSettings.h>
#include "GLModelScene.h"
#include <FEBioMonitor/FEBioMonitorDoc.h>
#include <FEBioMonitor/FEBioReportDoc.h>
#include "RemoteJob.h"
#include "DlgRemoteProgress.h"
#include <FSCore/FSLogger.h>
#include "PropertyList.h"
#include "FileProcessor.h"
#include "modelcheck.h"
#include "DlgListMaterials.h"
#include "DlgMissingPlugins.h"
#include "FEBioBatchDoc.h"

extern GLColor col[];

class FSMainWindowOutput : public FSLogOutput
{
public:
	FSMainWindowOutput(CMainWindow* w) : wnd(w) {}

	void Write(const std::string& s) override
	{
		if (wnd) wnd->AddLogEntry(QString::fromStdString(s));
	}

private:
	CMainWindow* wnd;
};

static QRhi::Implementation MapToValidAPI(GraphicsAPI graphicsApi)
{
	// map to RHI implementation
	QRhi::Implementation api = QRhi::Null;
	switch (graphicsApi)
	{
	case GraphicsAPI::API_OPENGL: api = QRhi::OpenGLES2; break;
	case GraphicsAPI::API_VULKAN: api = QRhi::Vulkan; break;
	case GraphicsAPI::API_METAL: api = QRhi::Metal; break;
	case GraphicsAPI::API_DIRECT3D11: api = QRhi::D3D11; break;
	case GraphicsAPI::API_DIRECT3D12: api = QRhi::D3D12; break;
	default:
		break;
	}

	// Use platform-specific defaults
#if defined(Q_OS_WIN)
	if ((api == QRhi::Null) ||
		((api != QRhi::D3D11) && (api != QRhi::D3D12) && (api != QRhi::Vulkan) && (api != QRhi::OpenGLES2)))
	{
		api = QRhi::OpenGLES2;
	}
#endif

	// Always use Metal on Apple platforms
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
	if ((api == QRhi::Null) ||
		((api != QRhi::Metal) && (api != QRhi::OpenGLES2)))
	{
//		api = QRhi::Metal; // TODO: Restore this when 3D textures are working on Metal
		api = QRhi::OpenGLES2;
	}
#endif

	// On Linux, prefer Vulkan if available
#if defined(Q_OS_LINUX)
	if ((api == QRhi::Null) ||
		((api != QRhi::Vulkan) && (api != QRhi::OpenGLES2)))
	{
		api = QRhi::OpenGLES2;
	}
#endif

	return api;
}

//-----------------------------------------------------------------------------
CMainWindow* CMainWindow::m_mainWnd = nullptr;

//-----------------------------------------------------------------------------
CMainWindow* CMainWindow::GetInstance()
{
	return m_mainWnd;
}

//-----------------------------------------------------------------------------
CMainWindow::CMainWindow(bool reset, GraphicsAPI api, QWidget* parent) : QMainWindow(parent), ui(new Ui::CMainWindow)
{
	m_mainWnd = this;

#ifdef LINUX
	// Set locale to avoid issues with reading and writing feb files in other languages.
	std::locale::global(std::locale::classic());
#endif

    // Preserve user's language and territory but use the C locale for everything else
    // this standardizes number formatting. 
    QLocale locale;
    QLocale forcedLocale(locale.language(), locale.territory());
    QLocale::setDefault(forcedLocale);

	m_DocManager = new CDocManager();

	m_fileProcessor = new CFileProcessor(this);

	setDockOptions(dockOptions() | QMainWindow::AllowNestedDocks | QMainWindow::GroupedDragging);

	// Instantiate IconProvider singleton
	CIconProvider::Instantiate(devicePixelRatio());

	// initialize RHI
	QRhi::Implementation rhiAPI = MapToValidAPI(api);
	RhiWindow::InitRHI(rhiAPI);
	
	// setup the GUI
	ui->setupUi(this);

	// read the settings
	if (reset == false)
	{
		readSettings();
	}
	else
	{
		// Add the default launch configuration
		ui->m_settings.m_launch_configs.push_back(new CDefaultLaunchConfig("Default"));
	}

	// allow drop events
	setAcceptDrops(true);

	// make sure the project viewer is visible
	ui->showProjectViewer();

	// show the welcome page
	ShowWelcomePage();

	// update the UI configuration
	UpdateUIConfig();

	// load templates
	TemplateManager::Init();

	// configure FEBio library
	if (ui->m_settings.loadFEBioConfigFile)
	{
		std::string fileName = ui->m_settings.febioConfigFileName.toStdString();
		FSDir dir(fileName);
		std::string filepath = dir.expandMacros();
		FEBio::ConfigureFEBio(filepath.c_str());
	}

	// Start AutoSave Timer
	ui->m_autoSaveTimer = new QTimer(this);
	QObject::connect(ui->m_autoSaveTimer, &QTimer::timeout, this, &CMainWindow::autosave);
	if (ui->m_settings.autoSaveInterval > 0)
	{
		ui->m_autoSaveTimer->start(ui->m_settings.autoSaveInterval * 1000);
	}

	// Auto Update Check
	if(ui->m_updaterPresent)
	{
		QObject::connect(&ui->m_updateWidget, &CUpdateWidget::ready, this, &CMainWindow::autoUpdateCheck);
		ui->m_updateWidget.checkForUpdate();
	}

	FSLogger::SetOutput(new FSMainWindowOutput(this));

	// Don't load plugins in Debug mode since plugins are usually built in Release mode
	// and mixing Debug and Release code can lead to all kinds of problems.
#ifdef NDEBUG
    ui->m_pluginManager.LoadXML();
    ui->m_pluginManager.LoadAllPlugins();
#endif
    ui->m_pluginManager.ReadDatabase();
    ui->m_pluginManager.Connect();

	QObject::connect(GetGLView(), &CGLView::captureFrameFinished, this, &CMainWindow::onCaptureFrameFinished);
}

//-----------------------------------------------------------------------------
CMainWindow::~CMainWindow()
{
#ifdef HAS_PYTHON
	if (ui->m_pyThread.isRunning())
	{
		ui->m_pyThread.quit();
		ui->m_pyThread.wait();
	}
#endif

	// delete document
	delete m_DocManager;
	delete m_fileProcessor;
	delete ui;
}

//-----------------------------------------------------------------------------
// clear command stack on save
bool CMainWindow::clearCommandStackOnSave() const
{
	return ui->m_settings.clearUndoOnSave;
}

//-----------------------------------------------------------------------------
// set clear command stack on save
void CMainWindow::setClearCommandStackOnSave(bool b)
{
	ui->m_settings.clearUndoOnSave = b;
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateTitle()
{
	QString title;
	QString projectName = ProjectName();
	if (projectName.isEmpty() == false)
	{
		title = projectName;
	}

	GLScreenRecorder& recorder = GetGLView()->GetScreenRecorder();
	if (recorder.HasRecording())
	{
		RECORDING_STATE state = recorder.GetRecordingState();
		switch (state)
		{
		case RECORDING_STATE::PAUSED   : title += " (RECORDING PAUSED)"; break;
		case RECORDING_STATE::RECORDING: title += " (RECORDING)"; break;
		case RECORDING_STATE::STOPPED  : title += " (RECORDING STOPPED)"; break;
		}
	}

	if (ui->m_jobManager->IsJobRunning())
	{
		CFEBioJob* job = CFEBioJob::GetActiveJob(); assert(job);
		if (job)
		{
			string name = job->GetName();
			title += " [ RUNNING: " + QString::fromStdString(name);
			if (job->HasProgress())
			{
				int pct = (int) job->GetProgress();
				title += " (" + QString::number(pct) + "%)";
			}
			title += "]";
		}
	}
	
	setWindowTitle(title);
}

//-----------------------------------------------------------------------------
//! update the tab's text
void CMainWindow::UpdateTab(CDocument* doc)
{
	if (doc == nullptr) return;

	QString tabTitle = QString::fromStdString(doc->GetDocTitle());
	if (doc->IsModified()) tabTitle += "*";

	CFEBioJob* activeJob = CFEBioJob::GetActiveJob();
	if (activeJob && (activeJob->GetDocument() == doc)) tabTitle += "[running]";

	QString tabTooltip = QString::fromStdString(doc->GetDocFilePath());

	ui->centralWidget->SetDocumentTabText(doc, tabTitle, tabTooltip);

	ui->projectViewer->Update();
}

bool CMainWindow::usingDarkTheme() const
{
	return (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_clearProject()
{
	if (ui->m_project.IsEmpty()) return;

	if (QMessageBox::question(this, "Clear Project", "Are you sure you want to clear the current project?\nThis cannot be undone.") == QMessageBox::Yes)
	{
		ui->m_project.Clear();
		ui->projectViewer->Update();
		UpdateTitle();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_closeProject()
{
	ui->m_project.Close();
	ui->projectViewer->Update();
	UpdateTitle();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_closeFile(const QString& file)
{
	CDocument* doc = FindDocument(file.toStdString());
	if (doc)
	{
		CloseView(doc);
		ui->projectViewer->Update();
		UpdateTitle();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_addToProject(const QString& file)
{
	ui->m_project.AddFile(file);
	ui->projectViewer->Update();
	UpdateTitle();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_txtedit_textChanged()
{
	QTextDocument* qtxt = ui->centralWidget->txtEdit->textDocument();
	if (qtxt == nullptr) return;

	CTextDocument* txtDoc = dynamic_cast<CTextDocument*>(GetDocument());
	if (txtDoc && txtDoc->IsValid() && (txtDoc->IsModified() == false))
	{
		if ((txtDoc->GetText() == qtxt))
		{
			txtDoc->SetModifiedFlag(qtxt->isModified());
			UpdateTab(txtDoc);
		}
	}

    CXMLDocument* xmlDoc = dynamic_cast<CXMLDocument*>(GetDocument());
	if (xmlDoc && xmlDoc->IsValid() && (xmlDoc->IsModified() == false))
	{
        xmlDoc->SetModifiedFlag(qtxt->isModified());
        UpdateTab(xmlDoc);
	}		
}

//-----------------------------------------------------------------------------
void CMainWindow::on_xmlTree_modelEdited()
{
    CXMLDocument* xmlDoc = dynamic_cast<CXMLDocument*>(GetDocument());
	if (xmlDoc && xmlDoc->IsValid() && (xmlDoc->IsModified() == false))
	{
        xmlDoc->SetModifiedFlag();
        UpdateTab(xmlDoc);
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
	m_fileProcessor->AddFile(QueuedFile(doc, fileName, fileReader, flags));
	m_fileProcessor->ReadNextFileInQueue();
}

void CMainWindow::OpenFile(const QString& filePath, bool showLoadOptions, bool openExternal, bool openInThread)
{
	// stop any animation
	if (ui->m_isAnimating) ui->postToolBar->CheckPlayButton(false);

	if(filePath.startsWith("fbs://"))
	{
		ui->databasePanel->OpenLink(filePath);
		ui->databasePanel->Raise();
		return;
	}

	// convert to native separators
	QString fileName = QDir::toNativeSeparators(filePath);

	// check to extension to see what to do
	QString ext = QFileInfo(fileName).suffix();
	if ((ext.compare("fsm", Qt::CaseInsensitive) == 0) ||
		(ext.compare("fs2", Qt::CaseInsensitive) == 0) ||
		(ext.compare("prv", Qt::CaseInsensitive) == 0) ||
		(ext.compare("fsprj", Qt::CaseInsensitive) == 0))
	{
		OpenDocument(fileName);
	}
	else if ((ext.compare("xplt", Qt::CaseInsensitive) == 0) ||
		     (ext.compare("vtk" , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("vtu" , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("pvtu", Qt::CaseInsensitive) == 0) ||
		     (ext.compare("pvd" , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("vtm" , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("k"   , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("stl" , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("fsps", Qt::CaseInsensitive) == 0))
	{
		// load the post file
		OpenPostFile(fileName, nullptr, showLoadOptions, openInThread);
	}
	else if (ext.compare("feb", Qt::CaseInsensitive) == 0)
	{
		// ask user if (s)he wants to open the feb as a model or as a file. 
		QMessageBox box;
		box.setText("How would you like to open this file?");
		QPushButton* importButton = box.addButton("Import as Model", QMessageBox::AcceptRole);
		QPushButton* textButton = box.addButton("Edit XML", QMessageBox::YesRole);
		box.addButton(QMessageBox::Cancel);
		box.setDefaultButton(importButton);

		box.exec();

		if(box.clickedButton() == importButton)
		{
			// load the feb file
			OpenFEModel(fileName);
		}
		else if(box.clickedButton() == textButton)
		{
			// open a text editor
			OpenFEBioFile(fileName);
		}
	}
	else if ((ext.compare("inp", Qt::CaseInsensitive) == 0) ||
		     (ext.compare("n"  , Qt::CaseInsensitive) == 0) ||
		     (ext.compare("dyn", Qt::CaseInsensitive) == 0) ||
		     (ext.compare("key", Qt::CaseInsensitive) == 0))
	{
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
	else if (ext.isEmpty() && (openExternal == true))
	{
		// Assume this is an LSDYNA database
		OpenPostFile(fileName, nullptr, showLoadOptions);
	}
	else if (ext == "log")
	{
		if (!OpenFEBioLogFile(fileName))
			OpenTextFile(fileName);
	}
	else if ((ext.compare("txt", Qt::CaseInsensitive) == 0) ||
			 (ext.compare("h"  , Qt::CaseInsensitive) == 0) ||
			 (ext.compare("cpp", Qt::CaseInsensitive) == 0) ||
			 (ext.compare("hpp", Qt::CaseInsensitive) == 0))
	{
		OpenTextFile(fileName);
	}
	else if (ext == "remote")
	{
		OpenRemoteFile(fileName);
	}
	else if (openExternal)
	{
		// Open any other files (e.g. log files) with the system's associated program
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
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

		// see if this file actually exists
		if (files.size() == 1)
		{
			QFileInfo fi(fileName);
			if (fi.exists() == false)
			{
				QString msg = QString("Failed reading file\n%1\nThe file does not exist.").arg(fileName);
				AddLogEntry(msg);
				QMessageBox::critical(this, "Import Geometry", msg);
				return;
			}
		}

		FileReader* fileReader = CreateFileReader(fileName);
		if (fileReader)
		{
			m_fileProcessor->AddFile(QueuedFile(doc, fileName, fileReader, 0));
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
	m_fileProcessor->ReadNextFileInQueue();

	for (int i=0; i<files.count(); ++i)
		ui->addToRecentGeomFiles(files[i]);
}

#ifdef HAS_LIBZIP
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
// Open a project
bool CMainWindow::OpenProject(const QString& projectFile)
{
	bool b = ui->m_project.Open(projectFile);
	if (b == false)
	{
		QMessageBox::critical(this, "ERROR", "Failed to open the project file.");
		return false;
	}

	ui->projectViewer->Update();
	ui->addToRecentProjects(projectFile);
	ui->projectViewer->parentWidget()->show();
	ui->projectViewer->parentWidget()->raise();
	UpdateTitle();

	CloseWelcomePage();

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
			ui->centralWidget->tab->setCurrentIndex(i);
			QApplication::alert(this);
			return;
		}
	}

	// create a new document
	CModelDocument* doc = new CModelDocument(this);
	doc->SetDocFilePath(filePath.toStdString());

	// create a model file reader
	ModelFileReader* reader = new ModelFileReader(doc);

	// try to open the file
	string sfile = filePath.toStdString();
	if (reader->Open(sfile.c_str()) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", QString("Failed to open file:\n%1").arg(filePath));
		delete reader;
		delete doc;
		return;
	}

	// get the file version
	int fileVersion = reader->GetFileVersion();

	// some checks on the file version
	if (fileVersion < MIN_FSM_VERSION)
	{
		QMessageBox::critical(this, "FEBio Studio", "This files uses an unsupported format and cannot be read in.");
		delete reader;
		delete doc;
		return;
	}

	if (fileVersion > SAVE_VERSION)
	{
		QMessageBox::critical(this, "FEBio Studio", "This file requires a newer version of FEBio Studio.");
		delete reader;
		delete doc;
		return;
	}

	if (fileVersion < FBS2_FILE)
	{
		int majv = FBS_MAJOR_VERSION(fileVersion);
		int minv = FBS_MINOR_VERSION(fileVersion);

		QString msg = QString("This file was created with an older version of FEBio Studio and needs to be converted.\nDo you wish to continue?\n(File version: %1.%2)").arg(majv).arg(minv);
		if (QMessageBox::question(this, "FEBio Studio", msg) != QMessageBox::Yes)
		{
			delete reader;
			delete doc;
			return;
		}
	}

	// we need to make this the active document
	CDocument::SetActiveDocument(doc);

	// start reading the file
	ReadFile(doc, filePath, reader, QueuedFile::NEW_DOCUMENT);

	// add file to recent list
	ui->addToRecentFiles(filePath);

	// Check if there is a more recent autosave of this file
	if(doc->loadPriorAutoSave())
	{
		int answer = QMessageBox::question(this, "FEBio Studio",
				"An autosave more recent than this file was found.\n\nWould you like to load it?",
				QMessageBox::Yes | QMessageBox::No);

		if(answer == QMessageBox::Yes)
		{
			CModelDocument* docAutoSave = new CModelDocument(this);
			ReadFile(docAutoSave, doc->GetAutoSaveFilePath().c_str(), new ModelFileReader(docAutoSave), QueuedFile::AUTO_SAVE_RECOVERY);
		}
		else
		{
			int answer2 = QMessageBox::question(this, "FEBio Studio",
					"Would you like to delete this autosave?",
					QMessageBox::Yes | QMessageBox::No);

			if(answer2 == QMessageBox::Yes)
			{
				QFile autoSave(doc->GetAutoSaveFilePath().c_str());

				if(autoSave.exists()) autoSave.remove();
			}
		}
	}
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
	if (modelDoc && (modelDoc->GetDocFilePath().empty() == false))
	{
		ui->m_project.AddFile(QString::fromStdString(modelDoc->GetDocFilePath()));
		ui->projectViewer->Update();
	}
}

//-----------------------------------------------------------------------------
bool compare_filename(const std::string& file1, const std::string& file2)
{
	if (file1.size() != file2.size()) return false;

	int l = (int)file1.size();
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
bool CMainWindow::CreateNewProject(QString fileName)
{
	// close the existing project
	ui->m_project.Close();

	// try to create a new project
	bool ret = ui->m_project.Save(fileName);

	if (ret) ui->addToRecentProjects(fileName);

	ui->projectViewer->parentWidget()->show();
	ui->projectViewer->parentWidget()->raise();

	return ret;
}

void CMainWindow::OpenRemoteFile(const QString& remoteFileName)
{
	// make sure this is a remote file
	QFileInfo fi(remoteFileName);
	if (fi.suffix() != "remote") return;

	QFile f(remoteFileName);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
	QString s(f.readAll());
	f.close();

	// should be the name of a launch configuration
	CLaunchConfig* lc = ui->findLaunchConfig(s.toStdString());
	if (lc == nullptr) return;

	// strip the ".remote" suffix
	QString fileName = remoteFileName.chopped(7);
	QFileInfo fi2(fileName);

	// go get the file
	std::unique_ptr<CRemoteJob> tmp(new CRemoteJob(nullptr, lc, this));
	CDlgRemoteProgress dlg(tmp.get(), this, false, fileName);
	dlg.exec();

	// now delete the .remote file
	QDir dir;
	dir.remove(remoteFileName);

	// and finally try to open file
	OpenFile(fileName);
}

//! Open a plot file
void CMainWindow::OpenPostFile(const QString& fileName, CModelDocument* modelDoc, bool showLoadOptions, bool openInThread)
{
	// see if this file is already open
	CPostDocument* doc = dynamic_cast<CPostDocument*>(FindDocument(fileName.toStdString()));
	if (doc == nullptr)
	{
		doc = new CPostDocument(this, modelDoc);

		QString ext = QFileInfo(fileName).suffix();
		if (ext.compare("xplt", Qt::CaseInsensitive) == 0)
		{
			xpltFileReader* xplt = new xpltFileReader(doc->GetFSModel());
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
			doc->SetFileReader(xplt);
			int flags = QueuedFile::NEW_DOCUMENT;
			if (!openInThread) flags |= QueuedFile::NO_THREAD;
			ReadFile(doc, fileName, doc->GetFileReader(), flags);

			// add file to recent list
			ui->addToRecentFiles(fileName);
		}
		else if (ext.compare("vtk", Qt::CaseInsensitive) == 0)
		{
			Post::VTKImport* vtk = new Post::VTKImport(doc->GetFSModel());
			ReadFile(doc, fileName, vtk, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.compare("vtu", Qt::CaseInsensitive) == 0)
		{
			Post::VTUImport* vtu = new Post::VTUImport(doc->GetFSModel());
			ReadFile(doc, fileName, vtu, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.compare("pvtu", Qt::CaseInsensitive) == 0)
		{
			Post::PVTUImport* pvtu = new Post::PVTUImport(doc->GetFSModel());
			ReadFile(doc, fileName, pvtu, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.compare("pvd", Qt::CaseInsensitive) == 0)
		{
			Post::PVDImport* pvd = new Post::PVDImport(doc->GetFSModel());
			ReadFile(doc, fileName, pvd, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.compare("vtm", Qt::CaseInsensitive) == 0)
		{
			Post::VTMImport* vtm = new Post::VTMImport(doc->GetFSModel());
			ReadFile(doc, fileName, vtm, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.compare("fsps", Qt::CaseInsensitive) == 0)
		{
			PostSessionFileReader* fsps = new PostSessionFileReader(doc);
			ReadFile(doc, fileName, fsps, QueuedFile::NEW_DOCUMENT);

			// add file to recent list
			ui->addToRecentFiles(fileName);
		}
		else if (ext.compare("k", Qt::CaseInsensitive) == 0)
		{
			Post::FELSDYNAimport* reader = new Post::FELSDYNAimport(doc->GetFSModel());
			ReadFile(doc, fileName, reader, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.compare("stl", Qt::CaseInsensitive) == 0)
		{
			Post::FESTLimport* reader = new Post::FESTLimport(doc->GetFSModel());
			ReadFile(doc, fileName, reader, QueuedFile::NEW_DOCUMENT);
		}
		else if (ext.isEmpty())
		{
			// Assume this is an LSDYNA database
			Post::FELSDYNAPlotImport* lsdyna = new Post::FELSDYNAPlotImport(doc->GetFSModel());
			ReadFile(doc, fileName, lsdyna, QueuedFile::NEW_DOCUMENT);
		}
	}
	else
	{
		doc->Clear();
		Update(nullptr, true);
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
void CMainWindow::on_finishedReadingFile(QueuedFile file, const QString& errorString)
{
	ui->statusBar->clearMessage();
	ui->statusBar->hideProgress();

	bool success = file.m_success;

	if (success == false)
	{
        ModelFileReader* reader = dynamic_cast<ModelFileReader*>(file.m_fileReader);
        CModelDocument* doc = dynamic_cast<CModelDocument*>(file.m_doc);
        if (reader && doc)
        {
            auto missingPlugins = reader->GetMissingPlugins();

            if (!missingPlugins.empty())
            {
                CDlgMissingPlugins dlg(this, missingPlugins);

                if(dlg.exec())
                {
                    if(dlg.SkipPlugins())
                    {
                        doc->SetSkipPluginCheck(true);
                    }
                    else
                    {
                        doc->SetSkipPluginCheck(false);
                    }

                    m_fileProcessor->ReadFile(file);
                }
            }

            return;
        }


		if (m_fileProcessor->IsQueueEmpty())
		{
			QString err = QString("Failed reading file :\n%1\n\nERROR: %2").arg(file.m_fileName).arg(errorString);
			QMessageBox::critical(this, "FEBio Studio", err);
		}

		QString err = QString("FAILED:\n%1\n").arg(errorString);
		AddLogEntry(err);

		// if this was a new document, we need to delete the document
		if (file.m_flags & QueuedFile::NEW_DOCUMENT)
		{
			delete file.m_doc;
		}

		// reset the active document
		CDocument::SetActiveDocument(GetDocument());

		return;
	}
	else
	{
		if (errorString.isEmpty() == false)
		{
			if (m_fileProcessor->IsQueueEmpty())
			{
				QStringList stringList = errorString.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
				QString err = QString("Warnings were generated while reading the file:\n%1\n\n").arg(file.m_fileName);

				err += QString("IMPORTANT: The file may NOT have been read in correctly. Please check for errors!\n\n");

				err += "WARNINGS:\n";
				int n = stringList.size();
				if (n > 10) n = 10;
				for (int i=0; i<n; ++i)
				{
					err += QString("%1\n").arg(stringList[i]);
				}
				if (stringList.size() > n)
				{
					err += QString("\n(Additional warnings on Output panel)");
				} 


				QMessageBox::information(this, "FEBio Studio", err);
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
			CGLDocument* doc = dynamic_cast<CGLDocument*>(file.m_doc); assert(doc);
			doc->SetFileReader(file.m_fileReader);
			if (doc->GetDocFilePath().empty())
			{
				doc->SetDocFilePath(file.m_fileName.toStdString());

				QFileInfo fi(file.m_fileName);
				QString path = fi.absolutePath();
				SetCurrentFolder(path);
			}
			bool b = doc->Initialize();
			if (b == false)
			{
				AddLogEntry("Document initialization failed!\n");
			}
			AddDocument(doc);

			Units::SetUnitSystem(doc->GetUnitSystem());

			// for fsprj files we set the "project" directory. 
			FSDir path(file.m_fileName.toStdString());
			if (path.fileExt() == "fsprj")
			{
				FSDir::setMacro("ProjectDir", ".");
			}
			else ui->addToRecentFiles(file.m_fileName);
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
		else if (file.m_flags & QueuedFile::AUTO_SAVE_RECOVERY)
		{
			CGLDocument* doc = dynamic_cast<CGLDocument*>(file.m_doc); assert(doc);
			doc->SetFileReader(file.m_fileReader);
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

			doc->SetDocTitle(doc->GetDocTitle() + " - Recovered");
			doc->SetUnsaved();
		}
		else
		{
			if (file.m_doc) file.m_doc->Update();
			SetActiveDocument(file.m_doc);
		}
	}

	if (m_fileProcessor->IsQueueEmpty() == false)
	{
		m_fileProcessor->ReadNextFileInQueue();
	}
	else
	{
		if ((file.m_flags & QueuedFile::RELOAD_DOCUMENT) == 0) Reset();
		UpdateModel();
		UpdateToolbar();
		UpdatePostToolbar();
		Update(nullptr, true);
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
		UpdateModel();
	}

	// redraw the GL view
	RedrawGL();

	// update the GL control bar
	UpdateGLControlBar();

	// Update the command windows
	if (ui->buildPanel->isVisible() && (psend != ui->buildPanel)) ui->buildPanel->Update(breset);

	//	if (m_pCurveEdit->visible() && (m_pCurveEdit != psend)) m_pCurveEdit->Update();
	if (ui->meshWnd && ui->meshWnd->isVisible()) ui->meshWnd->Update(breset);

	if (ui->postPanel && ui->postPanel->isVisible()) ui->postPanel->Update(breset);
	if (ui->febioMonitor && ui->febioMonitor->isVisible()) ui->febioMonitor->Update(breset);

	if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(breset);

	if (ui->measureTool && ui->measureTool->isVisible()) ui->measureTool->Update();
	if (ui->planeCutTool && ui->planeCutTool->isVisible()) ui->planeCutTool->Update();

	if (ui->docProps->isVisible() && (psend != ui->docProps)) ui->docProps->Update(breset);

	UpdateGraphs(breset);
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateMeshInspector(bool breset)
{
	if (ui->meshWnd && ui->meshWnd->isVisible()) ui->meshWnd->Update(breset);
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateGraphs(bool breset)
{
	// update graph windows
	QList<::CGraphWindow*>::iterator it = ui->graphList.begin();
	for (int i = 0; i < ui->graphList.size(); ++i, ++it)
	{
		if ((*it)->isVisible())
			(*it)->Update(breset);
	}
}

void CMainWindow::UpdateUiView()
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	switch (doc->GetUIViewMode())
	{
	case CGLDocument::MODEL_VIEW  : RedrawGL(); break;
	case CGLDocument::SLICE_VIEW  : ui->centralWidget->sliceView->Update(); RedrawGL(); break;
	case CGLDocument::TIME_VIEW_2D: ui->centralWidget->timeView2D->Update(); break;
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
CGLView* CMainWindow::GetGLView()
{
	return ui->centralWidget->glw->GetGLView();
}

CImageSliceView* CMainWindow::GetImageSliceView()
{
    return ui->centralWidget->sliceView;
}

C2DImageTimeView* CMainWindow::GetC2DImageTimeView()
{
    return ui->centralWidget->timeView2D;
}

void CMainWindow::ShowImageViewer(QImage img)
{
	if (ui->imageView == nullptr) ui->imageView = new CDlgScreenCapture(this);
	ui->imageView->SetImage(img);
	if (ui->imageView->isVisible() == false)
	{
		ui->imageView->show();
	}
}

CLogPanel* CMainWindow::GetLogPanel()
{
	return ui->logPanel;
}

CBuildPanel* CMainWindow::GetBuildPanel()
{
	return ui->buildPanel;
}

CCreatePanel* CMainWindow::GetCreatePanel()
{
	return ui->buildPanel->CreatePanel();
}

CRepositoryPanel* CMainWindow::GetDatabasePanel()
{
	return ui->databasePanel;
}

CFEBioMonitorPanel* CMainWindow::GetFEBioMonitorPanel()
{
	return ui->febioMonitor;
}

CFEBioMonitorView* CMainWindow::GetFEBioMonitorView()
{
	return ui->febioMonitorView;
}

CModelViewer* CMainWindow::GetModelViewer()
{
	return ui->modelViewer;
}
/*
CPythonToolsPanel* CMainWindow::GetPythonToolsPanel()
{
	return ui->pythonToolsPanel;
}
*/
CMainStatusBar* CMainWindow::GetStatusBar()
{
	return ui->statusBar;
}

//! close the current open project
void CMainWindow::CloseProject()
{
	// close all views first
	int n = ui->centralWidget->tab->count();
	for (int i = 0; i < n; ++i) ui->centralWidget->tab->closeView(0);
}

//-----------------------------------------------------------------------------
//! This function resets the GL View. It is called when creating a new file new
//! (CWnd::OnFileNew) or when opening a file (CWnd::OnFileOpen). 
//! \sa CGLView::Reset
void CMainWindow::Reset()
{
//	GetGLView()->Reset();
	GLHighlighter::ClearHighlights();
	on_actionZoomExtents_triggered();
}

//-----------------------------------------------------------------------------
//! Get the active document
CDocument* CMainWindow::GetDocument() 
{ 
	return ui->centralWidget->tab->getActiveDoc();
}

CGLDocument* CMainWindow::GetGLDocument() { return dynamic_cast<CGLDocument*>(GetDocument()); }
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

void CMainWindow::autosave()
{
	for(int i = 0; i < m_DocManager->Documents(); i++)
	{
		CDocument* doc = m_DocManager->GetDocument(i);

		if (doc && doc->IsModified())
		{
			doc->AutoSaveDocument();
		}
	}
}

void CMainWindow::autoUpdateCheck(bool update)
{
	ui->m_updateAvailable = update;
	ui->m_serverMessage = ui->m_updateWidget.getServerMessage();

	int n = ui->centralWidget->tab->findView("Welcome");
	if (n != -1)
	{
		ui->centralWidget->tab->getDocument(n)->Activate();
	}
}

void CMainWindow::ReportSelection()
{
	CGLDocument* doc = GetGLDocument();
	if (doc)
	{
		FESelection* sel = doc->GetCurrentSelection();
		if ((sel == nullptr) || (sel->Size() == 0))
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
#ifndef NDEBUG
				GPart* pg = it;
				msg += QString("\nnodes = (%1)[").arg(pg->m_node.size());
				for (int i = 0; i < pg->m_node.size(); ++i)
				{
					if (i != 0) msg += ",";
					msg += QString::number(pg->m_node[i]);
				}
				msg += "]\n";
#endif
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
				GDiscreteSelection& ds = dynamic_cast<GDiscreteSelection&>(*sel);
				GDiscreteSelection::Iterator it(&ds);
				GDiscreteSpringSet* dss = dynamic_cast<GDiscreteSpringSet*>(&(*it));
				if (dss)
				{
					for (int i = 0; i < dss->size(); ++i)
					{
						GDiscreteElement& de = dss->element(i);
						if (de.IsSelected())
						{
							msg = QString("discrete element [%1, %2] selected.").arg(de.Node(0)).arg(de.Node(1));
						}
					}
				}
				else msg = QString("1 discrete object selected");

			}
			else msg = QString("%1 discrete objects selected").arg(N);
		}
		break;
		case SELECT_FE_ELEMS:
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
				FSMesh* pm = es->GetMesh();
				FSElement_* el = es->Element(0);
				int eid = (el->m_nid > 0 ? el->m_nid : es->ElementIndex(0) + 1);
				AddLogEntry("  ID = " + QString::number(el->m_nid) + "\n");

				switch (el->Type())
				{
				case FE_HEX8: AddLogEntry("  Type = HEX8"); break;
				case FE_TET4: AddLogEntry("  Type = TET4"); break;
				case FE_TET5: AddLogEntry("  Type = TET5"); break;
				case FE_PENTA6: AddLogEntry("  Type = PENTA6"); break;
				case FE_QUAD4: AddLogEntry("  Type = QUAD4"); break;
				case FE_TRI3: AddLogEntry("  Type = TRI3"); break;
				case FE_BEAM2: AddLogEntry("  Type = BEAM2"); break;
				case FE_HEX20: AddLogEntry("  Type = HEX20"); break;
				case FE_QUAD8: AddLogEntry("  Type = QUAD8"); break;
				case FE_BEAM3: AddLogEntry("  Type = BEAM3"); break;
				case FE_TET10: AddLogEntry("  Type = TET10"); break;
				case FE_TRI6: AddLogEntry("  Type = TRI6"); break;
				case FE_TET15: AddLogEntry("  Type = TET15"); break;
				case FE_HEX27: AddLogEntry("  Type = HEX27"); break;
				case FE_TRI7: AddLogEntry("  Type = TRI7"); break;
				case FE_QUAD9: AddLogEntry("  Type = QUAD9"); break;
				case FE_PENTA15: AddLogEntry("  Type = PENTA15"); break;
				case FE_PYRA5: AddLogEntry("  Type = PYRA5"); break;
				case FE_TET20: AddLogEntry("  Type = TET20"); break;
				case FE_TRI10: AddLogEntry("  Type = TRI10"); break;
				case FE_PYRA13: AddLogEntry("  Type = PYRA13"); break;
				}
				AddLogEntry("\n");

				AddLogEntry("  nodes: ");
				int n = el->Nodes();
				for (int i = 0; i < n; ++i)
				{
					int ni = el->m_node[i];
					FSNode& node = pm->Node(ni);
					int nid = (node.m_nid > 0 ? node.m_nid : ni + 1);
					AddLogEntry(QString::number(nid));
					if (i < n - 1) AddLogEntry(", ");
					else AddLogEntry("\n");
				}
#ifndef NDEBUG
				AddLogEntry("  neighbors: ");
				n = 0;
				if (el->IsSolid()) n = el->Faces();
				else if (el->IsShell()) n = el->Edges();

				for (int i = 0; i < n; ++i)
				{
					AddLogEntry(QString::number(el->m_nbr[i]));
					if (i < n - 1) AddLogEntry(", ");
					else AddLogEntry("\n");
				}
#endif
                if(ui->meshWnd && ui->meshWnd->isVisible())
                {
                    auto data = es->GetMesh()->GetMeshData();

                    int n = el->Nodes();
                    AddLogEntry("  nodal values: ");
                    for (int i = 0; i < n; ++i)
                    {
                        AddLogEntry(QString::number(data.GetElementValue(es->ElementIndex(0), i)));
                        if (i < n - 1) AddLogEntry(", ");
                        else AddLogEntry("\n");
                    }

                    AddLogEntry("  avg value: ");
                    AddLogEntry(QString::number(data.GetElementAverageValue(es->ElementIndex(0))) + "\n");
                }
			}
		}

		FEFaceSelection* fs = dynamic_cast<FEFaceSelection*>(sel);
		if (fs)
		{
			if (fs->Size() == 1)
			{
				FEFaceSelection::Iterator it = fs->begin();
				FSFace* pf = it;
				switch (pf->Type())
				{
				case FE_FACE_TRI3: AddLogEntry("  Type = TRI3"); break;
				case FE_FACE_QUAD4: AddLogEntry("  Type = QUAD4"); break;
				case FE_FACE_TRI6: AddLogEntry("  Type = TRI6"); break;
				case FE_FACE_TRI7: AddLogEntry("  Type = TRI7"); break;
				case FE_FACE_QUAD8: AddLogEntry("  Type = QUAD8"); break;
				case FE_FACE_QUAD9: AddLogEntry("  Type = QUAD9"); break;
				case FE_FACE_TRI10: AddLogEntry("  Type = TRI10"); break;
				}
				AddLogEntry("\n");

				AddLogEntry("  neighbors: ");
				int n = pf->Edges();
				for (int i = 0; i < n; ++i)
				{
					AddLogEntry(QString::number(pf->m_nbr[i]));
					if (i < n - 1) AddLogEntry(", ");
					else AddLogEntry("\n");
				}
			}
		}
	}
}

void CMainWindow::closeEvent(QCloseEvent* ev)
{
	if (maybeSave())
	{
		writeSettings();

		if(ui->m_updateOnClose && ui->m_updaterPresent)
		{
			QProcess* updater = new QProcess;
			bool bret = true;
			if(ui->m_updateDevChannel)
			{
				QString s = ui->m_updateWidget.getUpdaterPath();
				bret = updater->startDetached(s, QStringList() << "--devChannel");	
			}
			else
			{
				QString s = ui->m_updateWidget.getUpdaterPath();
				bret = updater->startDetached(s, QStringList());
			}
			if (bret == false)
			{
				QMessageBox::critical(this, "FEBIo Studio", "Failed to launch updater.");
			}
		}

		ev->accept();
	}
	else
	{
		ui->m_updateOnClose = false;
		ev->ignore();
	}
}

void CMainWindow::onDropEvent(QDropEvent* e)
{
	foreach(const QUrl & url, e->mimeData()->urls()) {
		QString fileName = url.toLocalFile();

		FileReader* fileReader = nullptr;

		QFileInfo file(fileName);

		// Create a file reader
		// NOTE: For FEB files I prefer to open the file as a separate model,
		// so I need this hack. 
		if (file.suffix() != "feb") fileReader = CreateFileReader(fileName);

		CDocument* doc = GetDocument();

		// make sure we have one
		if (fileReader && doc)
		{
			ReadFile(doc, fileName, fileReader, 0);
		}
		else {
			OpenFile(fileName, false, false);
		}
	}
}

void CMainWindow::keyPressEvent(QKeyEvent* ev)
{
	if ((ev->key() == Qt::Key_Backtab) || (ev->key() == Qt::Key_Tab))
	{
		if (ev->modifiers() & Qt::ControlModifier)
		{
			ev->accept();

			int docs = ui->centralWidget->tab->count();
			if (docs > 1)
			{
				// activate the next document
				int i = ui->centralWidget->tab->currentIndex();

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
				ui->centralWidget->tab->setCurrentIndex(i);
			}
		}
	}
	else if (ev->key() == Qt::Key_Escape)
	{
		// give the build panels a chance to react first
		if (ui->buildPanel->OnEscapeEvent()) return;

		CGLDocument* doc = GetGLDocument();
		if (doc)
		{
			// if the build panel didn't process it, clear selection
			FESelection* ps = doc->GetCurrentSelection();
			if ((ps == 0) || (ps->Size() == 0))
			{
				if (doc->GetItemMode() != ITEM_MESH) doc->SetItemMode(ITEM_MESH);
				else ui->SetSelectionMode(SELECT_OBJECT);
				CGLView* glv = GetGLView();
				if (glv)
				{
					GLViewSettings& vs = glv->GetViewSettings();
					vs.m_bselbrush = false;
				}
				doc->SetTransformMode(TRANSFORM_NONE);
				Update();
				UpdateUI();
			}
			else on_actionClearSelection_triggered();
			ev->accept();
			GLHighlighter::ClearHighlights();
			GLHighlighter::setTracking(false);
		}
	}
	else if ((ev->key() == Qt::Key_1) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showProjectViewer();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_2) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showModelViewer();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_3) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showBuildPanel();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_4) && (ev->modifiers() & Qt::CTRL))
	{
		ui->showPostPanel();
		ev->accept();
	}
	else if ((ev->key() == Qt::Key_R))
	{
		if (GetPostDocument()) ui->mainMenu->actionRotate->toggle();
	}
	else if ((ev->key() == Qt::Key_T))
	{
		if (GetPostDocument()) ui->mainMenu->actionTranslate->toggle();
	}
	else if ((ev->key() == Qt::Key_A))
	{
		CModelDocument* doc = GetModelDocument();
		if (doc)
		{
			doc->ToggleActiveParts();
			RedrawGL();
		}
	}
	else if ((ev->key() == Qt::Key_F1))
	{
		if (ev->modifiers() & Qt::ControlModifier)
		{
			ev->accept();
			CModelDocument* doc = GetModelDocument();
			if (doc)
			{
				GLScene* scene = doc->GetScene();
				if (scene)
				{
					scene->Update();
					RedrawGL();
				}
			}
		}
	}
}

void CMainWindow::SetCurrentFolder(const QString& folder)
{
	ui->m_settings.m_currentPath = folder;
}

// get the current project
FEBioStudioProject* CMainWindow::GetProject()
{
	return &ui->m_project;
}

void CMainWindow::setAutoSaveInterval(int interval)
{
	ui->m_settings.autoSaveInterval = interval;

	ui->m_autoSaveTimer->stop();
	if (ui->m_settings.autoSaveInterval > 0)
	{
		ui->m_autoSaveTimer->start(ui->m_settings.autoSaveInterval * 1000);
	}
}

int CMainWindow::autoSaveInterval()
{
	return ui->m_settings.autoSaveInterval;
}

QString CMainWindow::GetServerMessage()
{
    return ui->m_serverMessage;
}

bool CMainWindow::updaterPresent()
{
	return ui->m_updaterPresent;
}

bool CMainWindow::updateAvailable()
{
	return ui->m_updateAvailable;
}

// set/get default unit system for new models
void CMainWindow::SetDefaultUnitSystem(int n)
{
	ui->m_settings.defaultUnits = n;
}

int CMainWindow::GetDefaultUnitSystem() const
{
	return ui->m_settings.defaultUnits;
}

bool CMainWindow::GetLoadConfigFlag() { return ui->m_settings.loadFEBioConfigFile; }
QString CMainWindow::GetConfigFileName() { return ui->m_settings.febioConfigFileName; }

void CMainWindow::SetLoadConfigFlag(bool b) { ui->m_settings.loadFEBioConfigFile = b; }
void CMainWindow::SetConfigFileName(QString s) { ui->m_settings.febioConfigFileName = s; }

void CMainWindow::writeSettings()
{
	GLViewSettings& vs = GetGLView()->GetViewSettings();
	FBS_SETTINGS& fbs = ui->m_settings;

	QString version = QString("%1.%2.%3").arg(FBS_VERSION).arg(FBS_SUBVERSION).arg(FBS_SUBSUBVERSION);

	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MRLSoftware", "FEBioStudio");
	settings.setValue("version", version);
	settings.beginGroup("MainWindow");
	{
		settings.setValue("geometry", saveGeometry());
		settings.setValue("state", saveState());

		QRect rt;
		rt = CCurveEditor::preferredSize(); if (rt.isValid()) settings.setValue("curveEditorSize", rt);
		rt = CGraphWindow::preferredSize(); if (rt.isValid()) settings.setValue("graphWindowSize", rt);
	}
	settings.endGroup();

	settings.beginGroup("Settings");
	{
		// background
		settings.setValue("bgColor1", (int)vs.m_col1.to_uint());
		settings.setValue("bgColor2", (int)vs.m_col2.to_uint());
		settings.setValue("fgColor", (int)vs.m_fgcol.to_uint());
		settings.setValue("bgStyle", vs.m_nbgstyle);

		// display
		settings.setValue("nodeSize", vs.m_node_size);
		settings.setValue("lineWidth", vs.m_line_size);
		settings.setValue("meshColor", vs.m_meshColor.to_uint());
		settings.setValue("normalsScaleFactor", vs.m_scaleNormals);
		settings.setValue("multiViewProjection", vs.m_nconv);
		settings.setValue("improvedTransparency", vs.m_bzsorting);
		settings.setValue("showGrid", vs.m_bgrid);
		settings.setValue("defaultFGColorOption", vs.m_defaultFGColorOption);
		settings.setValue("defaultFGColor", (int)vs.m_defaultFGColor.to_uint());
		settings.setValue("tagFontSize", vs.m_tagFontSize);
		settings.setValue("defaultWidgetFont", GLWidget::get_default_font());

		// FEBio
		settings.setValue("loadFEBioConfigFile", ui->m_settings.loadFEBioConfigFile);
		settings.setValue("febioConfigFileName", ui->m_settings.febioConfigFileName);
		settings.setValue("FEBioSDKInclude", ui->m_settings.FEBioSDKInc);
		settings.setValue("FEBioSDKLibrary", ui->m_settings.FEBioSDKLib);
		settings.setValue("createPluginPath", ui->m_settings.createPluginPath);

		// Lighting
		settings.setValue("lighting", vs.m_bLighting);
		settings.setValue("shadows", vs.m_bShadows);
		settings.setValue("shadowIntensity", vs.m_shadow_intensity);
		settings.setValue("lightDirection", Vec3fToString(vs.m_light));
		settings.setValue("environmentMap", fbs.m_envMapFile);
		settings.setValue("useEnvironmentMap", vs.m_use_environment_map);

		// Physics
		settings.setValue("fiberScaleFactor", vs.m_fiber_scale);
		settings.setValue("showFibersOnHiddenParts", vs.m_showHiddenFibers);

		// Post options
		settings.setValue("defaultMap", ColorMapManager::GetDefaultMap());
		settings.setValue("defaultColorMapRange", Post::CGLColorMap::m_defaultRngType);

		// Selection
		settings.setValue("respectPartitions", vs.m_bpart);

		// UI
		settings.setValue("emulateApply", vs.m_apply);
		settings.setValue("autoSaveInterval", ui->m_settings.autoSaveInterval);

		// Units
		settings.setValue("defaultUnits", ui->m_settings.defaultUnits);
	}
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	{
		settings.setValue("currentPath", ui->m_settings.m_currentPath);

		settings.setValue("defaultProjectFolder", ui->m_settings.m_defaultProjectParent);
		settings.setValue("repositoryFolder", ui->databasePanel->GetRepositoryFolder());
		settings.setValue("repoMessageTime", ui->databasePanel->GetLastMessageTime());

		settings.setValue("recentFiles", fbs.m_recentFiles);
		settings.setValue("recentGeomFiles", fbs.m_recentGeomFiles);
		settings.setValue("recentProjects", fbs.m_recentProjects);
		settings.setValue("recentPlugins", fbs.m_recentPlugins);
		settings.setValue("recentImages", fbs.m_recentImages);
	}
	settings.endGroup();

	settings.beginGroup("LaunchConfigurations");
	{
		// Create and save a list of launch config names
		// NOTE: We do not save the first config, which should be the DEFAULT one
		assert((fbs.m_launch_configs.size() > 0) && (fbs.m_launch_configs[0]->type() == CLaunchConfig::DEFAULT));
		QStringList launch_config_names;
		for (int i = 1; i < fbs.m_launch_configs.size(); ++i)
		{
			CLaunchConfig& confi = *fbs.m_launch_configs[i];
			launch_config_names.append(QString::fromStdString(confi.name()));
		}
		settings.setValue("launchConfigNames", launch_config_names);

		// Save launch configs
		for (int i = 0; i < launch_config_names.count(); i++)
		{
			CLaunchConfig& confi = *fbs.m_launch_configs[i + 1];
			QString configName = "launchConfigs/" + launch_config_names[i];

			settings.setValue(configName + "/type", confi.type());
			for (int n = 0; n < confi.Parameters(); ++n)
			{
				Param& p = confi.GetParam(n);
				QString name = configName + "/" + QString(p.GetShortName());
				switch (p.GetParamType())
				{
				case Param_URL   : settings.setValue(name, p.GetURLValue().c_str()); break;
				case Param_STRING: settings.setValue(name, p.GetStringValue().c_str()); break;
				case Param_INT   : settings.setValue(name, p.GetIntValue()); break;
				case Param_BOOL  : settings.setValue(name, p.GetBoolValue()); break;
				default:
					assert(false);
				}
			}
		}
	}
	settings.endGroup();

	// store user colormaps
	int n = ColorMapManager::UserColorMaps();
	settings.beginGroup("UserColorMaps");
	{
		settings.remove("");
		for (int i = 0; i < n; ++i)
		{
			const CColorMap& c = ColorMapManager::GetColorMap(ColorMapManager::USER + i);
			string sname = ColorMapManager::GetColorMapName(ColorMapManager::USER + i);
			settings.beginGroup(QString::fromStdString(sname));
			{
				int m = c.Colors();
				settings.setValue("colors", m);
				for (int j = 0; j < m; ++j)
				{
					QString sj = QString::number(j);
					float v = c.GetColorPos(j);
					QColor col = toQColor(c.GetColor(j));
					settings.setValue(sj + "/pos", v);
					settings.setValue(sj + "/col", col);
				}
			}
			settings.endGroup();
		}
	}
	settings.endGroup();

	if (!ui->m_updateWidget.UUID.isEmpty())
	{
		settings.setValue("UUID", ui->m_updateWidget.UUID);
	}
}

void CMainWindow::readSettings()
{
	GLViewSettings& vs = GetGLView()->GetViewSettings();
	FBS_SETTINGS& fbs = ui->m_settings;

	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MRLSoftware", "FEBioStudio");
	QString versionString = settings.value("version", "").toString();
	settings.beginGroup("MainWindow");
	{
		restoreGeometry(settings.value("geometry").toByteArray());
		restoreState(settings.value("state").toByteArray());

		QRect rt;
		QRect defaultRect(geometry().center().x() - 400, geometry().center().y() - 300, 800, 600);
		rt = settings.value("curveEditorSize", defaultRect).toRect();
		if (rt.isValid()) CCurveEditor::setPreferredSize(rt);
		rt = settings.value("graphWindowSize", defaultRect).toRect();
		if (rt.isValid()) CGraphWindow::setPreferredSize(rt);
	}
	settings.endGroup();

	settings.beginGroup("Settings");
	{
		// background
		vs.m_col1 = GLColor(settings.value("bgColor1", (int)vs.m_col1.to_uint()).toInt());
		vs.m_col2 = GLColor(settings.value("bgColor2", (int)vs.m_col2.to_uint()).toInt());
		vs.m_fgcol = GLColor(settings.value("fgColor", (int)vs.m_fgcol.to_uint()).toInt());
		vs.m_nbgstyle = settings.value("bgStyle", vs.m_nbgstyle).toInt();

		// display
		vs.m_node_size = settings.value("nodeSize", vs.m_node_size).toInt();
		vs.m_line_size = settings.value("lineWidth", vs.m_line_size).toInt();
		vs.m_meshColor = GLColor(settings.value("meshColor", vs.m_meshColor.to_uint()).toUInt());
		// alpha component used to not be stored so set it to default if zero
		if (vs.m_meshColor.a == 0) vs.m_meshColor.a = 64;
		vs.m_scaleNormals = settings.value("normalsScaleFactor", vs.m_scaleNormals).toDouble();
		vs.m_nconv = settings.value("multiViewProjection", 0).toInt();
		vs.m_bzsorting = settings.value("improvedTransparency", vs.m_bzsorting).toBool();
		vs.m_bgrid = settings.value("showGrid", vs.m_bgrid).toBool();
		vs.m_defaultFGColorOption = settings.value("defaultFGColorOption", vs.m_defaultFGColorOption).toInt();
		vs.m_defaultFGColor = GLColor(settings.value("defaultFGColor", (int)vs.m_defaultFGColor.to_uint()).toInt());
		vs.m_tagFontSize = settings.value("tagFontSize", vs.m_tagFontSize).toInt();
		QFont font = settings.value("defaultWidgetFont", GLWidget::get_default_font()).value<QFont>();
		GLWidget::set_default_font(font);

		// FEBio
		ui->m_settings.loadFEBioConfigFile = settings.value("loadFEBioConfigFile", true).toBool();
		ui->m_settings.febioConfigFileName = settings.value("febioConfigFileName", ui->m_settings.febioConfigFileName).toString();
		QString defaultSDK = QFileInfo(QApplication::applicationDirPath() + QString(REL_ROOT) + "sdk/").absoluteFilePath();
		ui->m_settings.FEBioSDKInc = settings.value("FEBioSDKInclude", defaultSDK + "include").toString();
		ui->m_settings.FEBioSDKLib = settings.value("FEBioSDKLibrary", defaultSDK + "lib").toString();
		ui->m_settings.createPluginPath = settings.value("createPluginPath", "").toString();

		// Lighting
		vs.m_bLighting = settings.value("lighting", vs.m_bLighting).toBool();
		vs.m_bShadows = settings.value("shadows", vs.m_bShadows).toBool();
		vs.m_shadow_intensity = settings.value("shadowIntensity", vs.m_shadow_intensity).toFloat();
		vs.m_light = StringToVec3f(settings.value("lightDirection", "{0.5,0.5,1}").toString());
		QString envmap = settings.value("environmentMap").toString();
		fbs.m_envMapFile = envmap;
		vs.m_use_environment_map = settings.value("useEnvironmentMap", false).toBool();

		if (envmap.isEmpty()) vs.m_use_environment_map = false;

		// Physics
		vs.m_fiber_scale = settings.value("fiberScaleFactor", vs.m_fiber_scale).toDouble();
		vs.m_showHiddenFibers = settings.value("showFibersOnHiddenParts", vs.m_showHiddenFibers).toBool();

		// Post options
		ColorMapManager::SetDefaultMap(settings.value("defaultMap", ColorMapManager::JET).toInt());
		Post::CGLColorMap::m_defaultRngType = settings.value("defaultColorMapRange").toInt();

		// Selection
		vs.m_bpart = settings.value("respectPartitions", vs.m_bpart).toBool();

		// UI
		vs.m_apply = settings.value("emulateApply", vs.m_apply).toBool();
		ui->m_settings.autoSaveInterval = settings.value("autoSaveInterval", 600).toInt();

		// Units
		ui->m_settings.defaultUnits = settings.value("defaultUnits", 0).toInt();


		if (vs.m_defaultFGColorOption != 0)
		{
			GLWidget::set_base_color(vs.m_defaultFGColor);
		}

		Units::SetUnitSystem(ui->m_settings.defaultUnits);
	}
	settings.endGroup();

	settings.beginGroup("FolderSettings");
	{
		fbs.m_currentPath = settings.value("currentPath", QDir::homePath()).toString();

		fbs.m_defaultProjectParent = settings.value("defaultProjectFolder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
		QString repositoryFolder = settings.value("repositoryFolder").toString();
		ui->databasePanel->SetRepositoryFolder(repositoryFolder);
		qint64 lastMessageTime = settings.value("repoMessageTime", -1).toLongLong();
		ui->databasePanel->SetLastMessageTime(lastMessageTime);

		QStringList recentFiles = settings.value("recentFiles").toStringList(); ui->setRecentFiles(recentFiles);
		QStringList recentGeomFiles = settings.value("recentGeomFiles").toStringList(); ui->setRecentGeomFiles(recentGeomFiles);
		QStringList recentProjects = settings.value("recentProjects").toStringList(); ui->setRecentProjects(recentProjects);
		QStringList recentPlugins = settings.value("recentPlugins").toStringList(); ui->setRecentPlugins(recentPlugins);
		QStringList recentImages = settings.value("recentImages").toStringList(); ui->setRecentImageFiles(recentImages);
	}
	settings.endGroup();

	settings.beginGroup("LaunchConfigurations");
	{
		QStringList launch_config_names;
		launch_config_names = settings.value("launchConfigNames", launch_config_names).toStringList();

		// clear launch configurations
		fbs.m_launch_configs.clear();

		// create the default launch configuration
		fbs.m_launch_configs.push_back(new CDefaultLaunchConfig("Default"));

		for (QString conf : launch_config_names)
		{
			QString configName = "launchConfigs/" + conf;

			CLaunchConfig* lc = nullptr;
			int ntype = settings.value(configName + "/type").toInt();
			switch (ntype)
			{
			case CLaunchConfig::LOCAL : lc = new CLocalLaunchConfig(conf.toStdString()); break;
			case CLaunchConfig::REMOTE: lc = new CRemoteLaunchConfig(conf.toStdString()); break;
			case CLaunchConfig::PBS   : lc = new CPBSLaunchConfig(conf.toStdString()); break;
			case CLaunchConfig::SLURM : lc = new CSLURMLaunchConfig(conf.toStdString()); break;
			case CLaunchConfig::CUSTOM: lc = new CCustomLaunchConfig(conf.toStdString()); break;
			default:
				assert(false);
			}

			if (lc)
			{
				fbs.m_launch_configs.push_back(lc);

				for (int n = 0; n < lc->Parameters(); ++n)
				{
					Param& p = lc->GetParam(n);
					QString name = configName + "/" + QString(p.GetShortName());
					switch (p.GetParamType())
					{
					case Param_URL   : p.SetURLValue   (settings.value(name).toString().toStdString()); break;
					case Param_STRING: p.SetStringValue(settings.value(name).toString().toStdString()); break;
					case Param_INT   : p.SetIntValue   (settings.value(name).toInt()); break;
					case Param_BOOL  : p.SetBoolValue  (settings.value(name).toBool()); break;
					default:
						assert(false);
					}
				}
			}
		}
	}
	settings.endGroup();

	settings.beginGroup("UserColorMaps");
	{
		QStringList l = settings.childGroups();
		for (int i = 0; i < l.size(); ++i)
		{
			QString name = l.at(i);
			CColorMap c; c.SetColors(0);
			settings.beginGroup(name);
			{
				int m = settings.value("colors", 0).toInt();
				c.SetColors(m);
				for (int j = 0; j < m; ++j)
				{
					QString sj = QString::number(j);
					float    v = settings.value(sj + "/pos").toFloat();
					QColor col = settings.value(sj + "/col").value<QColor>();
					c.SetColorPos(j, v);
					c.SetColor(j, toGLColor(col));
				}
			}
			settings.endGroup();
			if (c.Colors() > 0) ColorMapManager::AddColormap(name.toStdString(), c);
		}
	}
	settings.endGroup();

	// Read UUID. Generate if not present. 
	QString UUID = settings.value("UUID").toString();
	if(UUID.isEmpty())
	{
		UUID = QUuid::createUuid().toString();
	}

	ui->m_updateWidget.UUID = UUID;
}


//-----------------------------------------------------------------------------
void CMainWindow::UpdateUI()
{
	/*	m_pToolbar->Update();
	m_pCmdWnd->Update();
	if (m_pCurveEdit->visible()) m_pCurveEdit->Update();
	 */
	ui->centralWidget->glw->Update();
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateToolbar()
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	CMainMenu* menu = ui->mainMenu;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	if (view.m_blma   != menu->actionShowMatAxes->isChecked()) menu->actionShowMatAxes->trigger();
	if (view.m_bmesh  != menu->actionShowMeshLines->isChecked()) menu->actionShowMeshLines->trigger();
	if (view.m_bgrid  != menu->actionShowGrid->isChecked()) menu->actionShowGrid->trigger();

	GLScene* scene = doc->GetScene();
	if (scene)
	{
		GLCamera& cam = scene->GetCamera();
		if (cam.IsOrtho() != menu->actionOrtho->isChecked()) menu->actionOrtho->trigger();
	}

	if (ui->buildToolBar->isVisible())
	{
		ui->SetSelectionMode(doc->GetSelectionMode());
	}
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
//! set item selection mode
void CMainWindow::SetItemSelectionMode(int nselect, int nitem)
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;

	doc->SetSelectionMode(nselect);
	if (nselect == SELECT_OBJECT) doc->SetItemMode(nitem);

	UpdateToolbar();
	UpdateGLControlBar();
	RedrawGL();
}

//-----------------------------------------------------------------------------
//! Updates the model editor and selects object po.
//! \param po pointer to object that will be selected in the model editor
void CMainWindow::UpdateModel(FSObject* po, bool bupdate)
{
	if (ui->modelViewer && GetModelDocument())
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
	else if (ui->postPanel && GetPostDocument())
	{
		ui->postPanel->Update(bupdate);
		if (po) ui->postPanel->SelectObject(po);
	}
}

//-----------------------------------------------------------------------------
//! Updates the GLView control bar
void CMainWindow::UpdateGLControlBar()
{
	ui->centralWidget->glw->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::UpdateUIConfig()
{
	Update(0, true);

	CDocument* doc = GetDocument();

	if (dynamic_cast<CModelDocument*>(doc))
	{
		ui->setUIConfig(Ui::Config::MODEL_CONFIG);
	}
	else if (dynamic_cast<CPostDocument*>(doc))
	{
		ui->setUIConfig(Ui::Config::POST_CONFIG);
	}
	else if (dynamic_cast<CTextDocument*>(doc))
	{
		ui->setUIConfig(Ui::Config::TEXT_CONFIG);
	}
	else if (dynamic_cast<CHTMLDocument*>(doc))
	{
		ui->setUIConfig(Ui::Config::HTML_CONFIG);
	}
	else if (dynamic_cast<CXMLDocument*>(doc))
	{
		ui->setUIConfig(Ui::Config::XML_CONFIG);
	}
	else if (dynamic_cast<FEBioMonitorDoc*>(doc))
	{
		ui->setUIConfig(Ui::Config::MONITOR_CONFIG);
	}
	else if (dynamic_cast<CFEBioReportDoc*>(doc))
	{
		ui->setUIConfig(Ui::Config::FEBREPORT_CONFIG);
	}
	else if (dynamic_cast<FEBioBatchDoc*>(doc))
	{
		ui->setUIConfig(Ui::Config::BATCHRUN_CONFIG);
	}
	else
	{
		ui->setUIConfig(Ui::Config::EMPTY_CONFIG);
	}

	Update(0, true);
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
	int view = ui->centralWidget->tab->findView(doc);
	if (view == -1)
	{
		AddView(doc->GetDocTitle(), doc);
	}
	else
	{
		SetActiveView(view);
	}
}

//-----------------------------------------------------------------
int CMainWindow::Views()
{
	return ui->centralWidget->tab->views();
}

//-----------------------------------------------------------------
void CMainWindow::SetActiveView(int n)
{
	ui->centralWidget->tab->setActiveView(n);
}

//-----------------------------------------------------------------
int CMainWindow::FindView(CDocument* doc)
{
	return ui->centralWidget->tab->findView(doc);
}

//-----------------------------------------------------------------------------
GObject* CMainWindow::GetActiveObject()
{
	CGLDocument* doc = GetGLDocument();
	if (doc) return doc->GetActiveObject();
	return nullptr;
}

//-----------------------------------------------------------------
void CMainWindow::AddView(const std::string& viewName, CDocument* doc, bool makeActive)
{
	string docIcon = doc->GetIcon();
	ui->centralWidget->tab->addView(viewName, doc, makeActive, docIcon);
	CGLView* glview = GetGLView();
	on_actionZoomExtents_triggered();
	glview->UpdateWidgets();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_currentChanged(int n)
{
	CDocument* newDoc = GetDocument();
	CDocument::SetActiveDocument(newDoc);

	if (ui->planeCutTool && ui->planeCutTool->isVisible()) ui->planeCutTool->close();
	GetGLView()->ClearCommandStack();

	UpdateUIConfig();
	UpdateGLControlBar();
	UpdateToolbar();
	ui->updateMeshInspector();
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_tab_tabCloseRequested(int n)
{
	CloseView(n);
	ui->projectViewer->Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::OnPostObjectStateChanged()
{
	Post::CGLModel* mdl = GetCurrentModel();
	if (mdl == nullptr) return;
	bool b = mdl->GetColorMap()->IsActive();
	ui->postToolBar->CheckColorMap(b);
	mdl->Update(false);
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
void CMainWindow::CloseView(int n, bool forceClose)
{
	CDocument* doc = ui->centralWidget->tab->getDocument(n);

	// make sure this doc has no active jobs running.
	CFEBioJob* activeJob = CFEBioJob::GetActiveJob();
	if (activeJob && (activeJob->GetDocument() == doc))
	{
		QMessageBox::warning(this, "FEBio Studio", "This model has an active job running and cannot be closed.\n");
		return;
	}

	if (doc->IsModified() && (forceClose == false))
	{
		if (maybeSave(doc) == false) return;
	}

	// close any graph windows that use this document
	QList<::CGraphWindow*>::iterator it;
	for (it = ui->graphList.begin(); it != ui->graphList.end();)
	{
		CGraphWindow* w = *it;
		CModelGraphWindow* mgw = dynamic_cast<CModelGraphWindow*>(w);
		if (mgw)
		{
			CDocument* pd = mgw->GetDocument();
			if (pd == doc)
			{
				RemoveGraph(w);
				delete w;
				it = ui->graphList.begin();
			}
			else it++;
		}
		else it++;
	}

	// clear highlights, just to be safe
	GLHighlighter::ClearHighlights();

	ui->ShowDefaultBackground();
	ui->centralWidget->txtEdit->setDocument(nullptr);

	if (dynamic_cast<CModelDocument*>(doc))
	{
		ui->modelViewer->Clear();
	}

	// now, remove from the doc manager
	m_DocManager->RemoveDocument(n);

	// close the view and update UI
	ui->centralWidget->tab->closeView(n);
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
	if(QThread::currentThread() == this->thread())
	{
		CGLView* view = GetGLView();
		view->repaint();
	}
	else
	{
		QMetaObject::invokeMethod(GetGLView(), "repaint");
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectObjects_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_OBJECT);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectParts_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_PART);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectSurfaces_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_SURF);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectCurves_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_EDGE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectNodes_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_NODE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSelectDiscrete_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionMode(SELECT_DISCRETE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectRect_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_BOX);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectCircle_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	if (b) doc->SetSelectionStyle(REGION_SELECT_CIRCLE);
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectFree_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
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
// show the log panel
void CMainWindow::ShowLogPanel()
{
	ui->logPanel->parentWidget()->raise();
	ui->logPanel->parentWidget()->show();
}

//-----------------------------------------------------------------------------
// add to the log 
void CMainWindow::AddLogEntry(const QString& txt)
{
	QMetaObject::invokeMethod(this, &CMainWindow::AddLogEntrySlot, txt);
}

void CMainWindow::AddLogEntrySlot(const QString& txt)
{
	ui->logPanel->AddText(txt);
}

//-----------------------------------------------------------------------------
// add to the output log
void CMainWindow::AddOutputEntry(const QString& txt)
{
	QMetaObject::invokeMethod(this, &CMainWindow::AddOutputEntrySlot, txt);
}

void CMainWindow::AddOutputEntrySlot(const QString& txt)
{
	ui->logPanel->AddText(txt, CLogPanel::FEBIO_LOG);
}

//-----------------------------------------------------------------------------
// add to the build log
void CMainWindow::AddBuildLogEntry(const QString& txt)
{
	QMetaObject::invokeMethod(this, &CMainWindow::AddBuildLogEntrySlot, txt);
}

void CMainWindow::AddBuildLogEntrySlot(const QString& txt)
{
	ui->logPanel->AddText(txt, CLogPanel::BUILD_LOG);
}

//-----------------------------------------------------------------------------
// add to the python log
void CMainWindow::AddPythonLogEntry(const QString& txt)
{
	QMetaObject::invokeMethod(this, &CMainWindow::AddPythonLogEntrySlot, txt);
}

void CMainWindow::AddPythonLogEntrySlot(const QString& txt)
{
	ui->logPanel->AddText(txt, CLogPanel::PYTHON_LOG);
}

//-----------------------------------------------------------------------------
// clear the log
void CMainWindow::ClearLog()
{
	ui->logPanel->Clear(CLogPanel::FBS_LOG);
}

//-----------------------------------------------------------------------------
// clear the output window
void CMainWindow::ClearOutput()
{
	ui->logPanel->Clear(CLogPanel::FEBIO_LOG);
}

void CMainWindow::ClearBuildLog()
{
	ui->logPanel->ShowLog(CLogPanel::BUILD_LOG);
	ui->logPanel->Clear(CLogPanel::BUILD_LOG);
}

void CMainWindow::ClearPythonLog()
{
	ui->logPanel->ShowLog(CLogPanel::PYTHON_LOG);
	ui->logPanel->Clear(CLogPanel::PYTHON_LOG);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_glview_pointPicked(const vec3d& r)
{
	GetCreatePanel()->setInput(r);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_glview_selectionChanged()
{
	ReportSelection();
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
	CMainMenu* mainMenu = ui->mainMenu;

	menu.addAction(mainMenu->actionZoomSelect);
	menu.addAction(mainMenu->actionShowGrid);
	menu.addAction(mainMenu->actionShowMeshLines);
	menu.addAction(mainMenu->actionShowEdgeLines);
	menu.addAction(mainMenu->actionOrtho);
	menu.addSeparator();

	QMenu* view = new QMenu("Standard views");
	view->addAction(mainMenu->actionFront);
	view->addAction(mainMenu->actionBack);
	view->addAction(mainMenu->actionLeft);
	view->addAction(mainMenu->actionRight);
	view->addAction(mainMenu->actionTop);
	view->addAction(mainMenu->actionBottom);
    view->addAction(mainMenu->actionIsometric);
	menu.addAction(view->menuAction());
	menu.addSeparator();

	menu.addAction(mainMenu->actionRenderMode);

	CModelDocument* doc = GetModelDocument();
	if (doc)
	{
		menu.addAction(mainMenu->actionShowNormals);

		QMenu* physicsMenu = new QMenu("Physics");

		physicsMenu->addAction(mainMenu->actionShowFibers);
		physicsMenu->addAction(mainMenu->actionShowMatAxes);
		physicsMenu->addAction(mainMenu->actionShowDiscrete);
		physicsMenu->addAction(mainMenu->actionShowRigidBodies);
		physicsMenu->addAction(mainMenu->actionShowRigidJoints);
		physicsMenu->addAction(mainMenu->actionShowRigidLabels);
		menu.addMenu(physicsMenu);

		menu.addSeparator();

		// NOTE: Make sure the texts match the texts in OnSelectObjectTransparencyMode
		GLViewSettings& vs = GetGLView()->GetViewSettings();
		QMenu* display = new QMenu("Object transparency mode");
		QAction* a;
		a = display->addAction("None"); a->setCheckable(true); if (vs.m_transparencyMode == 0) a->setChecked(true);
		a = display->addAction("Selected only"); a->setCheckable(true); if (vs.m_transparencyMode == 1) a->setChecked(true);
		a = display->addAction("Unselected only"); a->setCheckable(true); if (vs.m_transparencyMode == 2) a->setChecked(true);
		QObject::connect(display, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectObjectTransparencyMode(QAction*)));
		menu.addAction(display->menuAction());

		CGLModelScene* scene = dynamic_cast<CGLModelScene*>(doc->GetScene());
		if (scene)
		{
			OBJECT_COLOR_MODE mode = scene->ObjectColorMode();
			QMenu* colorMode = new QMenu("Color mode");
			a = colorMode->addAction("Default"         ); a->setCheckable(true); a->setData((int) OBJECT_COLOR_MODE::DEFAULT_COLOR ); if (mode == OBJECT_COLOR_MODE::DEFAULT_COLOR ) a->setChecked(true);
			a = colorMode->addAction("By object"       ); a->setCheckable(true); a->setData((int) OBJECT_COLOR_MODE::OBJECT_COLOR  ); if (mode == OBJECT_COLOR_MODE::OBJECT_COLOR  ) a->setChecked(true);
			a = colorMode->addAction("By material type"); a->setCheckable(true); a->setData((int) OBJECT_COLOR_MODE::MATERIAL_TYPE ); if (mode == OBJECT_COLOR_MODE::MATERIAL_TYPE ) a->setChecked(true);
			a = colorMode->addAction("By element type" ); a->setCheckable(true); a->setData((int) OBJECT_COLOR_MODE::FSELEMENT_TYPE); if (mode == OBJECT_COLOR_MODE::FSELEMENT_TYPE) a->setChecked(true);
			a = colorMode->addAction("By physics"      ); a->setCheckable(true); a->setData((int) OBJECT_COLOR_MODE::PHYSICS_TYPE  ); if (mode == OBJECT_COLOR_MODE::PHYSICS_TYPE  ) a->setChecked(true);
			colorMode->addSeparator();
			a = colorMode->addAction("Identify backfacing surfaces"); a->setCheckable(true); if (vs.m_identifyBackfacing) a->setChecked(true);
			a->setData(0xFF);
			QObject::connect(colorMode, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectObjectColorMode(QAction*)));
			menu.addAction(colorMode->menuAction());
		}
	}

	CPostDocument* pdoc = GetPostDocument();
	if (pdoc)
	{
		GLViewSettings& vs = GetGLView()->GetViewSettings();
		QMenu* colorMode = new QMenu("Color mode");
		QAction* a;
		a = colorMode->addAction("Identify backfacing surfaces"); a->setCheckable(true); if (vs.m_identifyBackfacing) a->setChecked(true);
		a->setData(0xFF);
		QObject::connect(colorMode, SIGNAL(triggered(QAction*)), this, SLOT(OnSelectObjectColorMode(QAction*)));
		menu.addAction(colorMode->menuAction());
	}

	menu.addAction(mainMenu->actionOptions);
}

//-----------------------------------------------------------------------------
void CMainWindow::OnSelectObjectTransparencyMode(QAction* ac)
{
	GLViewSettings& vs = GetGLView()->GetViewSettings();

	if      (ac->text() == "None"           ) vs.m_transparencyMode = 0;
	else if (ac->text() == "Selected only"  ) vs.m_transparencyMode = 1;
	else if (ac->text() == "Unselected only") vs.m_transparencyMode = 2;

	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::OnSelectObjectColorMode(QAction* ac)
{
	int data = ac->data().toInt();

	CModelDocument* doc = GetModelDocument();
	if (doc)
	{
		CGLModelScene* scene = dynamic_cast<CGLModelScene*>(doc->GetScene());
		if (scene == nullptr) return;

		if      (data == OBJECT_COLOR_MODE::DEFAULT_COLOR ) scene->SetObjectColorMode(OBJECT_COLOR_MODE::DEFAULT_COLOR);
		else if (data == OBJECT_COLOR_MODE::OBJECT_COLOR  ) scene->SetObjectColorMode(OBJECT_COLOR_MODE::OBJECT_COLOR);
		else if (data == OBJECT_COLOR_MODE::MATERIAL_TYPE ) scene->SetObjectColorMode(OBJECT_COLOR_MODE::MATERIAL_TYPE);
		else if (data == OBJECT_COLOR_MODE::FSELEMENT_TYPE) scene->SetObjectColorMode(OBJECT_COLOR_MODE::FSELEMENT_TYPE);
		else if (data == OBJECT_COLOR_MODE::PHYSICS_TYPE  ) scene->SetObjectColorMode(OBJECT_COLOR_MODE::PHYSICS_TYPE);
	}

	if (data == 0xFF)
	{
		GLViewSettings& vs = GetGLView()->GetViewSettings();
		vs.m_identifyBackfacing = !vs.m_identifyBackfacing;
	}

	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::onExportAllMaterials()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSModel& fem = *doc->GetFSModel();

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

	QString fileName = QFileDialog::getSaveFileName(this, "Export Materials", "", "FEBio Studio Materials (*.pvm)");
	if (fileName.isEmpty() == false)
	{
		CModelDocument* doc = GetModelDocument();
		if (doc && (doc->ExportMaterials(fileName.toStdString(), matList) == false))
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

	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Import Materials", "", "FEBio files (*.feb)");
	if (fileNames.isEmpty() == false)
	{
		for (int i=0; i<fileNames.size(); ++i)
		{
			QString file = fileNames.at(i);
			if (doc->ImportFEBioMaterials(file.toStdString()) == false)
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
void CMainWindow::onImportMaterialsFromModel(CModelDocument* srcDoc)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if ((doc == nullptr) || (doc == srcDoc) || (srcDoc == nullptr)) return;

	FSModel* srcfem = srcDoc->GetFSModel();
	if (srcfem->Materials() == 0)
	{
		QMessageBox::information(this, "Import Materials", "The selected source file does not contain any materials.");
		return;
	}

	std::vector<GMaterial*> items;
	for (int i = 0; i < srcfem->Materials(); ++i)
	{
		GMaterial* gm = srcfem->GetMaterial(i);
		items.push_back(gm);
	}

	FSModel* dstfem = doc->GetFSModel();

	CDlgListMaterials input(this);
	input.SetMaterials(items);
	if (input.exec())
	{
		std::vector<GMaterial*> matList = input.GetSelectedMaterials();

		GMaterial* newMat = nullptr;
		for (GMaterial* gm : matList)
		{
			FSMaterial* pmsrc = gm->GetMaterialProperties();
			FSMaterial* pmnew = dynamic_cast<FSMaterial*>(FEBio::CloneModelComponent(pmsrc, dstfem));
			newMat = new GMaterial(pmnew);
			newMat->SetName(gm->GetName());
			newMat->SetColor(gm->GetColor());
			doc->DoCommand(new CCmdAddMaterial(dstfem, newMat), newMat->GetNameAndType());
		}
		UpdateModel(newMat);
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::ClearRecentFilesList()
{
	ui->m_settings.m_recentFiles.clear();
	ui->m_settings.m_recentProjects.clear();
	ui->m_settings.m_recentImages.clear();
}

void CMainWindow::OnCameraChanged()
{
	if (ui->postPanel->isVisible())
	{
		ui->postPanel->OnViewChanged();
	}
}

void CMainWindow::OnSelectionTransformed()
{
	UpdateGLControlBar();
	if (ui->buildPanel->IsEditPanelVisible())
	{
		ui->buildPanel->Update(false);
	}
	RedrawGL();
}

bool CMainWindow::IsColorPickerActive() const
{
	if (ui->pickColorTool && ui->pickColorTool->isVisible()) return true;
	return false;
}

CDlgPickColor* CMainWindow::GetPickColorDialog()
{
	if (IsColorPickerActive()) return ui->pickColorTool;
	return nullptr;
}

// remove a graph from the list
void CMainWindow::RemoveGraph(::CGraphWindow* graph)
{
	ui->graphList.removeOne(graph);
}

// Add a graph to the list of managed graph windows
void CMainWindow::AddGraph(CGraphWindow* graph)
{
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
	if (dynamic_cast<GLLabel*>(pglw))
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
//		QMessageBox::information(this, "Properties", "No properties available");
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

bool CMainWindow::DoModelCheck(CModelDocument* doc, bool askRunQuestion)
{
	if (doc == nullptr) return false;

	vector<MODEL_ERROR> warnings;
	checkModel(doc->GetProject(), warnings);

	if (!askRunQuestion && warnings.empty())
	{
		QMessageBox::information(this, "Model Check", "Model check completed. No issues found!");
		return true;
	}

	if (warnings.empty() == false)
	{
		CDlgCheck dlg(this, askRunQuestion);
		dlg.SetWarnings(warnings);
		if (dlg.exec() == 0)
		{
			return false;
		}
	}

	return true;
}

bool CMainWindow::ExportFEBioFile(CModelDocument* doc, const std::string& febFile, int febioFileVersion, bool allowHybridMesh, ProgressTracker* prg)
{
	// try to save the file first
	AddLogEntry(QString("Saving to %1 ...").arg(QString::fromStdString(febFile)));

	bool ret = false;
	string err;

	// pass the units to the model project
	doc->GetProject().SetUnits(doc->GetUnitSystem());

	FEBioExport* writer = nullptr;
	if (febioFileVersion == 0x0205)
	{
		FEBioExport25* feb = new FEBioExport25(doc->GetProject());
		feb->SetExportSelectionsFlag(true);
		writer = feb;
	}
	else if (febioFileVersion == 0x0300)
	{
		FEBioExport3* feb = new FEBioExport3(doc->GetProject());
		feb->SetExportSelectionsFlag(true);
		writer = feb;
	}
	else if (febioFileVersion == 0x0400)
	{
		FEBioExport4* feb = new FEBioExport4(doc->GetProject());
		feb->SetMixedMeshFlag(allowHybridMesh);
		feb->SetProgressTracker(prg);
		writer = feb;
	}
	else
	{
		assert(false);
	}
	if (writer == nullptr) return false;


	try {
		ret = writer->Write(febFile.c_str());
		if (ret == false) err = writer->GetErrorMessage();
	}
	catch (...)
	{
		err = "Unknown exception detected.";
		ret = false;
	}
	delete writer;

	if ((ret == false) && (prg == nullptr))
	{
		QString msg = QString("Failed saving FEBio file:\n%1").arg(QString::fromStdString(err));
		QMessageBox::critical(this, "Run FEBio", msg);
		AddLogEntry("FAILED\n");
	}
	else AddLogEntry("SUCCESS!\n");

	return ret;
}

void CMainWindow::onShowPartViewer()
{
	CModelDocument* doc = GetModelDocument();
	ui->showPartViewer(doc);
}

void CMainWindow::checkJobProgress()
{
	UpdateTitle();

	if (ui->m_jobManager->IsJobRunning())
	{
		QTimer::singleShot(100, this, SLOT(checkJobProgress()));
	}
}

void CMainWindow::ShowProgress(bool show, QString message)
{
	if(show)
	{
		ui->statusBar->showMessage(message);
		ui->statusBar->showProgress();
	}
	else
	{
		ui->statusBar->clearMessage();
		ui->statusBar->hideProgress();
	}
}

void CMainWindow::ShowIndeterminateProgress(bool show, QString message)
{
	if(show)
	{
		ui->statusBar->showMessage(message);
		ui->statusBar->showIndeterminateProgress();
	}
	else
	{
		ui->statusBar->clearMessage();
		ui->statusBar->hideProgress();
	}
}

void CMainWindow::UpdateProgress(int n)
{
	ui->statusBar->setProgress(n);
}

void CMainWindow::on_modelViewer_currentObjectChanged(FSObject* po)
{
	ui->infoPanel->SetObject(po);

	GLHighlighter::ClearHighlights();
	if (ui->modelViewer->IsHighlightSelectionEnabled())
	{
		IHasItemLists* il = dynamic_cast<IHasItemLists*>(po);
		if (il)
		{
			int itemLists = il->ItemLists();
			for (int i = 0; i < itemLists; ++i)
			{
				int colorMode = (i == 0 ? 0 : 1);
				FSItemListBuilder* pg = il->GetItemList(i);
				GPartList* partList = dynamic_cast<GPartList*>(pg);
				if (partList)
				{
					vector<GPart*> parts = partList->GetPartList();
					for (GPart* part : parts)
						GLHighlighter::PickItem(part, colorMode);
				}

				GFaceList* faceList = dynamic_cast<GFaceList*>(pg);
				if (faceList)
				{
					vector<GFace*> faces = faceList->GetFaceList();
					for (GFace* face : faces)
						GLHighlighter::PickItem(face, colorMode);
				}

				GEdgeList* edgeList = dynamic_cast<GEdgeList*>(pg);
				if (edgeList)
				{
					vector<GEdge*> edges = edgeList->GetEdgeList();
					for (GEdge* edge : edges)
						GLHighlighter::PickItem(edge, colorMode);
				}

				GNodeList* nodeList = dynamic_cast<GNodeList*>(pg);
				if (nodeList)
				{
					vector<GNode*> nodes = nodeList->GetNodeList();
					for (GNode* node : nodes)
						GLHighlighter::PickItem(node, colorMode);
				}

				FSGroup* meshSelection = dynamic_cast<FSGroup*>(pg);
				if (meshSelection)
				{
					GLHighlighter::PickItem(meshSelection, colorMode);
				}
			}
		}
		else if (dynamic_cast<FSItemListBuilder*>(po))
		{
			FSItemListBuilder* pg = dynamic_cast<FSItemListBuilder*>(po);
			GPartList* partList = dynamic_cast<GPartList*>(pg);
			if (partList)
			{
				vector<GPart*> parts = partList->GetPartList();
				for (GPart* part : parts)
					GLHighlighter::PickItem(part);
			}

			GFaceList* faceList = dynamic_cast<GFaceList*>(pg);
			if (faceList)
			{
				vector<GFace*> faces = faceList->GetFaceList();
				for (GFace* face : faces)
					GLHighlighter::PickItem(face);
			}

			GEdgeList* edgeList = dynamic_cast<GEdgeList*>(pg);
			if (edgeList)
			{
				vector<GEdge*> edges = edgeList->GetEdgeList();
				for (GEdge* edge : edges)
					GLHighlighter::PickItem(edge);
			}

			GNodeList* nodeList = dynamic_cast<GNodeList*>(pg);
			if (nodeList)
			{
				vector<GNode*> nodes = nodeList->GetNodeList();
				for (GNode* node : nodes)
					GLHighlighter::PickItem(node);
			}

			FSGroup* meshSelection = dynamic_cast<FSGroup*>(pg);
			if (meshSelection)
			{
				GLHighlighter::PickItem(meshSelection);
			}
		}
		else if (dynamic_cast<GItem*>(po))
		{
			GLHighlighter::PickItem(dynamic_cast<GItem*>(po));
		}
		else if (dynamic_cast<GMaterial*>(po))
		{
			GMaterial* mat = dynamic_cast<GMaterial*>(po);
			FSModel* fem = mat->GetModel();
			if (fem)
			{
				std::vector<GPart*> parts = fem->GetMaterialPartList(mat);
				for (GPart* pg : parts) GLHighlighter::PickItem(pg);
			}
		}
	}
	RedrawGL();
}

void CMainWindow::toggleOrtho()
{
	ui->mainMenu->actionOrtho->trigger();
}

QStringList CMainWindow::GetRecentFileList()
{
	return ui->m_settings.m_recentFiles;
}

QString CMainWindow::GetEnvironmentMap() const
{
	return ui->m_settings.m_envMapFile;
}

void CMainWindow::SetEnvironmentMap(const QString& filename)
{
	ui->m_settings.m_envMapFile = filename;
}

bool CMainWindow::IsEnvironmentMapEnabled()
{
	CGLView* glv = GetGLView();
	if (glv) return glv->GetViewSettings().m_use_environment_map;
	else return false;
}

QStringList CMainWindow::GetRecentProjectsList()
{
	return ui->m_settings.m_recentProjects;
}

QStringList CMainWindow::GetRecentPluginsList()
{
	return ui->m_settings.m_recentPlugins;
}

void CMainWindow::AddRecentPlugin(const QString& fileName)
{
	ui->addToRecentPlugins(fileName);
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

void CMainWindow::ShowWelcomePage()
{
	int n = ui->centralWidget->tab->findView("Welcome");
	if (n == -1)
	{
		AddDocument(new CWelcomePage(this));
	}
	else SetActiveView(n);
}

void CMainWindow::CloseWelcomePage()
{
	int n = ui->centralWidget->tab->findView("Welcome");
	if (n >= 0)
	{
		ui->ShowDefaultBackground();
		ui->centralWidget->tab->tabCloseRequested(n);
	}
}

bool CMainWindow::ImportImage(CImageModel* imgModel)
{
	static int n = 1;
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return false;

	CDlgStartThread dlg(this, new CImageReadThread(imgModel));

	if (dlg.exec())
	{
		std::string name = imgModel->GetImageSource()->GetName();
		if (name.empty())
		{
			std::stringstream ss;
			ss << "ImageModel" << n++;
			name = ss.str();
		}
		imgModel->SetName(name);

		// add it to the project
		doc->AddImageModel(imgModel);

        // We don't handle image models on the command stack so that 
        // image deletion actually clears up ram
        doc->ClearCommandStack();

		Update(0, true);
		// only for model docs
		if (dynamic_cast<CModelDocument*>(doc))
		{
			ShowInModelViewer(imgModel);
		}

		GLScene* scene = doc->GetScene();
		if (scene)
		{
			scene->GetCamera().ZoomToBox(imgModel->GetBoundingBox());
			RedrawGL();
		}
		return true;
	}
	return false;
}

#ifdef HAS_ITK
	bool CMainWindow::ProcessITKImage(const QString& fileName, int type)
	{
		CGLDocument* doc = GetGLDocument();

		CImageModel* imageModel = new CImageModel(nullptr);
        imageModel->SetImageSource(new CITKImageSource(imageModel, fileName.toStdString(), type));

        if(!ImportImage(imageModel))
        {
			delete imageModel;
			return false;
        }
		else ui->addToRecentImageFiles(fileName);
		return true;
	}
#else
	bool CMainWindow::ProcessITKImage(const QString& fileName, int type) { return false; }
#endif

QSize CMainWindow::GetEditorSize()
{
	return ui->centralWidget->stack->size();
}

void CMainWindow::on_doCommand(QString msg)
{
	AddLogEntry(msg);
}

void CMainWindow::on_selectionChanged()
{
//	ReportSelection();
}

CPluginManager* CMainWindow::GetPluginManager() { return &ui->m_pluginManager; }

QString CMainWindow::GetSDKIncludePath() const { return ui->m_settings.FEBioSDKInc; }
QString CMainWindow::GetSDKLibraryPath() const { return ui->m_settings.FEBioSDKLib; }
QString CMainWindow::GetCreatePluginPath() const { return ui->m_settings.createPluginPath; }

void CMainWindow::SetSDKIncludePath(const QString& s) { ui->m_settings.FEBioSDKInc = s; }
void CMainWindow::SetSDKLibraryPath(const QString& s) { ui->m_settings.FEBioSDKLib = s; }
void CMainWindow::SetCreatePluginPath(const QString& s) { ui->m_settings.createPluginPath = s; }

void CMainWindow::on_planecut_dataChanged()
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;
	
	CGLView* glview = nullptr;
	if (doc->GetUIViewMode() == CGLDocument::MODEL_VIEW)
	{
		glview = GetGLView();
	}
	else
	{
		CImageSliceView* v = GetImageSliceView();
		if (v) glview = v->GetGLView();
	}
	if (glview) glview->UpdateScene();
}
