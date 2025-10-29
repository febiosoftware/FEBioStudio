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
#include "ui_mainwindow.h"
#include <QMenuBar>
#include <QThread>

#include "IconProvider.h"

Ui::CMainWindow::CMainWindow()
{
	m_settings.defaultUnits = 0;
	m_settings.clearUndoOnSave = true;
	m_settings.autoSaveInterval = 600;
}

void Ui::CMainWindow::setupUi(::CMainWindow* wnd)
{
	m_wnd = wnd;

#ifdef WIN32
	m_settings.m_defaultProjectParent = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
	m_jobManager = new CFEBioJobManager(wnd);

	m_isAnimating = false;

	// Check if updater is present 
	m_updaterPresent = QFile::exists(m_updateWidget.getUpdaterPath());
	m_updateAvailable = false;
	m_updateOnClose = false;
	m_updateDevChannel = false;

	// initialize current path
	m_settings.m_currentPath = QDir::currentPath();

	// set the initial window size
//        QRect screenSize = QDesktopWidget().availableGeometry(wnd);
		//wnd->resize(QSize(screenSize.width() * 1.0f, screenSize.height() * 1.0f));
//		wnd->resize(800, 600);

		// build the central widget
	centralWidget = new CMainCentralWidget(wnd);
	wnd->setCentralWidget(centralWidget);

	// init Python stuff
#ifdef HAS_PYTHON
	m_pyRunner = new CPythonRunner;
	m_pyRunner->moveToThread(&m_pyThread);
	QObject::connect(&m_pyThread, &QThread::finished, m_pyRunner, &QObject::deleteLater);
	m_pyThread.start();
#endif

	// build the menu
	mainMenu = new CMainMenu(wnd);

	// build the toolbars
	buildToolbars(wnd);

	// build the dockable windows
	// (must be done after menu is created)
	buildDockWidgets(wnd);

	// build status bar
	statusBar = new CMainStatusBar();
	m_wnd->setStatusBar(statusBar);

	BuildConfigs();

	QMetaObject::connectSlotsByName(wnd);

	QObject::connect(modelViewer, &::CModelViewer::currentObjectChanged, imageSettingsPanel, &::CImageSettingsPanel::ModelTreeSelectionChanged);
	QObject::connect(modelViewer, &::CModelViewer::currentObjectChanged, centralWidget->sliceView, &::CImageSliceView::ModelTreeSelectionChanged);
	QObject::connect(modelViewer, &::CModelViewer::currentObjectChanged, centralWidget->timeView2D, &::C2DImageTimeView::ModelTreeSelectionChanged);

    QObject::connect(postPanel, &::CPostPanel::postTree_currentObjectChanged, imageSettingsPanel, &::CImageSettingsPanel::ModelTreeSelectionChanged);
    QObject::connect(postPanel, &::CPostPanel::postTree_currentObjectChanged, centralWidget->sliceView, &::CImageSliceView::ModelTreeSelectionChanged);
    QObject::connect(postPanel, &::CPostPanel::postTree_currentObjectChanged, centralWidget->timeView2D, &::C2DImageTimeView::ModelTreeSelectionChanged);
}

QAction* Ui::CMainWindow::addAction(const QString& title, const QString& name, const QString& iconFile, bool bcheckable)
{
	QAction* pa = new QAction(title, m_wnd);
	pa->setObjectName(name);
	if (iconFile.isEmpty() == false) pa->setIcon(CIconProvider::GetIcon(iconFile));
	if (bcheckable) pa->setCheckable(true);
	return pa;
}

void Ui::CMainWindow::buildToolbars(::CMainWindow* mainWindow)
{
	// Create the toolbar
	QToolBar* mainToolBar = m_wnd->addToolBar("Main toolbar");
	mainToolBar->setObjectName(QStringLiteral("mainToolBar"));

	mainToolBar->addAction(mainMenu->actionNewModel);
	mainToolBar->addAction(mainMenu->actionOpen);
	mainToolBar->addAction(mainMenu->actionSave);
	mainToolBar->addAction(mainMenu->actionSnapShot);
	mainToolBar->addAction(mainMenu->actionRayTrace);

	// Build tool bar
	coord = new QComboBox;
	coord->setObjectName("selectCoord");
	coord->addItem("Global");
	coord->addItem("Local");

	buildToolBar = new QToolBar(mainWindow);
	buildToolBar->setObjectName(QStringLiteral("buildToolBar"));
	buildToolBar->setWindowTitle("Build Toolbar");

	buildToolBar->addAction(mainMenu->actionUndo);
	buildToolBar->addAction(mainMenu->actionRedo);
	buildToolBar->addSeparator();
	buildToolBar->addAction(mainMenu->selectRect);
	buildToolBar->addAction(mainMenu->selectCircle);
	buildToolBar->addAction(mainMenu->selectFree);
	buildToolBar->addSeparator();
	buildToolBar->addAction(mainMenu->actionMeasureTool);
	buildToolBar->addAction(mainMenu->actionPlaneCutTool);
	buildToolBar->addAction(mainMenu->actionPickColor);
	buildToolBar->addAction(mainMenu->actionExplodedView);
	buildToolBar->addSeparator();
	buildToolBar->addAction(mainMenu->actionSelect);
	buildToolBar->addAction(mainMenu->actionTranslate);
	buildToolBar->addAction(mainMenu->actionRotate);
	buildToolBar->addAction(mainMenu->actionScale);
	buildToolBar->addWidget(coord);
	buildToolBar->addSeparator();
	buildToolBar->addAction(mainMenu->actionSelectObjects);
	buildToolBar->addAction(mainMenu->actionSelectParts);
	buildToolBar->addAction(mainMenu->actionSelectSurfaces);
	buildToolBar->addAction(mainMenu->actionSelectCurves);
	buildToolBar->addAction(mainMenu->actionSelectNodes);
	buildToolBar->addAction(mainMenu->actionSelectDiscrete);
	buildToolBar->addSeparator();
	buildToolBar->addAction(mainMenu->actionCurveEditor);
	buildToolBar->addAction(mainMenu->actionMeshInspector);
	buildToolBar->addAction(mainMenu->actionFEBioRun);
	buildToolBar->addSeparator();
	buildToolBar->addAction(mainMenu->actionMerge);
	buildToolBar->addAction(mainMenu->actionDetach);
	buildToolBar->addAction(mainMenu->actionExtract);
	buildToolBar->addAction(mainMenu->actionClone);
	buildToolBar->addAction(mainMenu->actionCloneGrid);
	buildToolBar->addAction(mainMenu->actionCloneRevolve);

	mainWindow->addToolBar(Qt::TopToolBarArea, buildToolBar);

	// Post tool bar
	postToolBar = new CPostToolBar(mainWindow);
	postToolBar->setObjectName(QStringLiteral("postToolBar"));
	postToolBar->setWindowTitle("Post Toolbar");
	mainWindow->addToolBar(Qt::TopToolBarArea, postToolBar);
	postToolBar->addAction(mainMenu->actionGraph);
	postToolBar->addAction(mainMenu->actionPlaneCut);
	postToolBar->addAction(mainMenu->actionMirrorPlane);
	postToolBar->addAction(mainMenu->actionVectorPlot);
	postToolBar->addAction(mainMenu->actionTensorPlot);
	postToolBar->addAction(mainMenu->actionIsosurfacePlot);
	postToolBar->addAction(mainMenu->actionSlicePlot);
	postToolBar->addAction(mainMenu->actionStreamLinePlot);
	postToolBar->addAction(mainMenu->actionParticleFlowPlot);
	postToolBar->addAction(mainMenu->actionVolumeFlowPlot);

	postToolBar->setDisabled(true);
	postToolBar->hide();

	// Image tool bar
	imageToolBar = new CImageToolBar(mainWindow);
	imageToolBar->setObjectName("imageToolBar");
	imageToolBar->setWindowTitle("Image Toolbar");
	mainWindow->addToolBar(Qt::TopToolBarArea, imageToolBar);
	imageToolBar->hide();

	// Font tool bar
	pFontToolBar = new QToolBar(mainWindow);
	pFontToolBar->setObjectName("FontToolBar");
	pFontToolBar->setWindowTitle("Font Toolbar");
	mainWindow->addToolBarBreak();
	mainWindow->addToolBar(Qt::TopToolBarArea, pFontToolBar);

	QAction* actionProperties = addAction("Properties ...", "actionProperties", "properties");

	pFontToolBar->addWidget(pFontStyle = new QFontComboBox); pFontStyle->setObjectName("fontStyle");
	pFontToolBar->addWidget(pFontSize = new QSpinBox); pFontSize->setObjectName("fontSize");
	pFontToolBar->addAction(actionFontBold = addAction("Bold", "fontBold", "font_bold")); actionFontBold->setCheckable(true);
	pFontToolBar->addAction(actionFontItalic = addAction("Italic", "fontItalic", "font_italic")); actionFontItalic->setCheckable(true);
	pFontToolBar->addAction(actionProperties);
	pFontToolBar->setEnabled(false);

	// XML toolbar
	xmlToolbar = new QToolBar(mainWindow);
	xmlToolbar->setObjectName("xmlToolbar");
	xmlToolbar->setWindowTitle("XML Toolbar");
	mainWindow->addToolBar(Qt::TopToolBarArea, xmlToolbar);

	actionEditXmlAsText = addAction("Edit as Text", "actionEditXmlAsText", "txt", true);

	xmlToolbar->addAction(actionEditXmlAsText);
	xmlToolbar->addSeparator();
	xmlToolbar->addAction(mainMenu->actionAddAttribute);
	xmlToolbar->addAction(mainMenu->actionAddElement);
	xmlToolbar->addAction(mainMenu->actionRemoveRow);
	xmlToolbar->addSeparator();
	xmlToolbar->addAction(mainMenu->actionUndo);
	xmlToolbar->addAction(mainMenu->actionRedo);

	// FEBio Monitor toolbar
	monitorToolBar = new QToolBar(mainWindow);
	monitorToolBar->setObjectName("monitorToolbar");
	monitorToolBar->setWindowTitle("Monitor Toolbar");
	monitorToolBar->addAction(mainMenu->actionFEBioMonitorSettings);
	monitorToolBar->addAction(mainMenu->actionFEBioContinue);
	monitorToolBar->addAction(mainMenu->actionFEBioPause);
	monitorToolBar->addAction(mainMenu->actionFEBioNext);
	monitorToolBar->addAction(mainMenu->actionFEBioStop);
	mainWindow->addToolBar(Qt::TopToolBarArea, monitorToolBar);

}

void Ui::CMainWindow::buildDockWidgets(::CMainWindow* wnd)
{
	wnd->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

	QDockWidget* dock1 = new QDockWidget("Project", m_wnd); dock1->setObjectName("dockFiles");
	dock1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	projectViewer = new ::CProjectViewer(m_wnd, dock1);
	projectViewer->setObjectName(QStringLiteral("projectViewer"));
	dock1->setWidget(projectViewer);
	m_wnd->addDockWidget(Qt::LeftDockWidgetArea, dock1);
	mainMenu->menuWindows->addAction(dock1->toggleViewAction());

	QDockWidget* dock2 = new QDockWidget("Model", m_wnd); dock2->setObjectName("dockModel");
	dock2->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	modelViewer = new ::CModelViewer(m_wnd, dock2);
	modelViewer->setObjectName("modelViewer");
	dock2->setWidget(modelViewer);
	mainMenu->menuWindows->addAction(dock2->toggleViewAction());
	m_wnd->tabifyDockWidget(dock1, dock2);

	QDockWidget* dock3 = new QDockWidget("Build", m_wnd); dock3->setObjectName("dockBuild");
	dock3->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	buildPanel = new ::CBuildPanel(m_wnd, dock3);
	dock3->setWidget(buildPanel);
	mainMenu->menuWindows->addAction(dock3->toggleViewAction());
	m_wnd->addDockWidget(Qt::RightDockWidgetArea, dock3);
	//		m_wnd->tabifyDockWidget(dock2, dock3);

	QDockWidget* dock4 = new QDockWidget("Output", m_wnd); dock4->setObjectName("dockLog");
	dock4->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
	logPanel = new ::CLogPanel(dock4);
	dock4->setWidget(logPanel);
	mainMenu->menuWindows->addAction(dock4->toggleViewAction());
	m_wnd->addDockWidget(Qt::BottomDockWidgetArea, dock4);

	QDockWidget* dock5 = new QDockWidget("Post", m_wnd); dock5->setObjectName("dockPost");
	dock5->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	postPanel = new ::CPostPanel(m_wnd, dock5);
	dock5->setWidget(postPanel);
	mainMenu->menuWindows->addAction(dock5->toggleViewAction());
	m_wnd->tabifyDockWidget(dock1, dock5);

	QDockWidget* dock6 = new QDockWidget("Notes", m_wnd); dock6->setObjectName("dockInfo");
	infoPanel = new ::CInfoPanel(wnd, dock6);
	dock6->setWidget(infoPanel);
	mainMenu->menuWindows->addAction(dock6->toggleViewAction());
	m_wnd->tabifyDockWidget(dock4, dock6);

#ifdef MODEL_REPO
	QDockWidget* dock7 = new QDockWidget("Repository", m_wnd); dock7->setObjectName("dockDatabase");
	databasePanel = new ::CRepositoryPanel(wnd, dock7);
	dock7->setWidget(databasePanel);
	mainMenu->menuWindows->addAction(dock7->toggleViewAction());
	m_wnd->tabifyDockWidget(dock1, dock7);
#endif

	QDockWidget* dock8 = new QDockWidget("Timeline", m_wnd); dock8->setObjectName("dockTime");
	timePanel = new ::CTimelinePanel(wnd, dock8);
	dock8->setWidget(timePanel);
	mainMenu->menuWindows->addAction(dock8->toggleViewAction());
	m_wnd->tabifyDockWidget(dock4, dock8);

	QDockWidget* dock9 = new QDockWidget("3D Image Settings", m_wnd); dock9->setObjectName("dockImageSettings");
	imageSettingsPanel = new ::CImageSettingsPanel(wnd, dock9);
	dock9->setWidget(imageSettingsPanel);
	mainMenu->menuWindows->addAction(dock9->toggleViewAction());
	m_wnd->tabifyDockWidget(dock4, dock9);

	QDockWidget* dock10 = new QDockWidget("FEBio Monitor", m_wnd); dock10->setObjectName("dockFEBioMonitor");
	febioMonitor = new CFEBioMonitorPanel(wnd, dock10);
	dock10->setWidget(febioMonitor);
	mainMenu->menuWindows->addAction(dock10->toggleViewAction());
	m_wnd->tabifyDockWidget(dock1, dock10);

	QDockWidget* dock11 = new QDockWidget("FEBio Monitor Graphs", m_wnd); dock11->setObjectName("dockFEBioMonitorView");
	febioMonitorView = new CFEBioMonitorView(wnd, dock11);
	dock11->setWidget(febioMonitorView);
	mainMenu->menuWindows->addAction(dock11->toggleViewAction());
	m_wnd->tabifyDockWidget(dock4, dock11);

	QDockWidget* dock12 = new QDockWidget("Properties", m_wnd); 
	dock12->setObjectName("docProps");
	docProps = new ::CDocPropsPanel(wnd, dock12);
	dock12->setWidget(docProps);
	mainMenu->menuWindows->addAction(dock12->toggleViewAction());
	m_wnd->tabifyDockWidget(dock2, dock12);

#ifdef HAS_PYTHON
//	QDockWidget* dock13 = new QDockWidget("Python", m_wnd); dock12->setObjectName("dockPython");
//	pythonToolsPanel = new ::CPythonToolsPanel(wnd, dock13);
//	dock13->setWidget(pythonToolsPanel);
//	mainMenu->menuWindows->addAction(dock13->toggleViewAction());
//	m_wnd->tabifyDockWidget(dock3, dock13);
#endif

	// make sure the file viewer is the visible tab
	dock1->raise();
}

void Ui::CMainWindow::BuildConfigs()
{
	// Insert in same order as Ui::Config !
	m_configs.push_back(new CEmptyConfig(this));
	m_configs.push_back(new CHTMLConfig(this));
	m_configs.push_back(new CModelConfig(this));
	m_configs.push_back(new CPostConfig(this));
	m_configs.push_back(new CTextConfig(this));
	m_configs.push_back(new CXMLConfig(this));
	m_configs.push_back(new CMonitorConfig(this));
	m_configs.push_back(new CFEBReportConfig(this));
	m_configs.push_back(new CBatchRunConfig(this));
	m_configs.push_back(new CRHIConfig(this));

	setUIConfig(Ui::Config::EMPTY_CONFIG);
}

void Ui::CMainWindow::SetSelectionMode(int nselect)
{
	switch (nselect)
	{
	case SELECT_OBJECT  : mainMenu->actionSelectObjects->trigger(); break;
	case SELECT_PART    : mainMenu->actionSelectParts->trigger(); break;
	case SELECT_FACE    : mainMenu->actionSelectSurfaces->trigger(); break;
	case SELECT_EDGE    : mainMenu->actionSelectCurves->trigger(); break;
	case SELECT_NODE    : mainMenu->actionSelectNodes->trigger(); break;
	case SELECT_DISCRETE: mainMenu->actionSelectDiscrete->trigger(); break;
	default:
		assert(false);
	}
}

void Ui::CMainWindow::showCurveEditor()
{
	if (curveWnd == 0) curveWnd = new CCurveEditor(m_wnd);

	curveWnd->Update();

	curveWnd->show();
	curveWnd->raise();
	curveWnd->activateWindow();
}

void Ui::CMainWindow::showMeshInspector()
{
	if (meshWnd == 0) meshWnd = new ::CMeshInspector(m_wnd);

	meshWnd->Update(true);

	meshWnd->show();
	meshWnd->raise();
	meshWnd->activateWindow();
}

void Ui::CMainWindow::updateMeshInspector()
{
	if (meshWnd && meshWnd->isVisible())
	{
		meshWnd->Update(true);
	}
}

void Ui::CMainWindow::setRecentFiles(QStringList& recentFiles)
{
	setRecentFileList(m_settings.m_recentFiles, recentFiles, mainMenu->menuRecentFiles, mainMenu->recentFilesActionGroup);
}

void Ui::CMainWindow::setRecentProjects(QStringList& recentFiles)
{
	setRecentFileList(m_settings.m_recentProjects, recentFiles, mainMenu->menuRecentProjects, mainMenu->recentProjectsActionGroup);
}

void Ui::CMainWindow::setRecentPlugins(QStringList& recentPlugins)
{
	setRecentFileList(m_settings.m_recentPlugins, recentPlugins, nullptr, nullptr);
}

void Ui::CMainWindow::setRecentGeomFiles(QStringList& recentFiles)
{
	setRecentFileList(m_settings.m_recentGeomFiles, recentFiles, mainMenu->menuRecentGeomFiles, mainMenu->recentGeomFilesActionGroup);
}

void Ui::CMainWindow::setRecentImageFiles(QStringList& recentImages)
{
	setRecentFileList(m_settings.m_recentImages, recentImages, mainMenu->menuRecentImages, mainMenu->recentImageFilesActionGroup);
}

void Ui::CMainWindow::addToRecentFiles(const QString& file)
{
	addToRecentFilesList(m_settings.m_recentFiles, file, mainMenu->menuRecentFiles, mainMenu->recentFilesActionGroup);
}

void Ui::CMainWindow::addToRecentProjects(const QString& file)
{
	addToRecentFilesList(m_settings.m_recentProjects, file, mainMenu->menuRecentProjects, mainMenu->recentProjectsActionGroup);
}

void Ui::CMainWindow::addToRecentPlugins(const QString& file)
{
	addToRecentFilesList(m_settings.m_recentPlugins, file);
}

void Ui::CMainWindow::addToRecentGeomFiles(const QString& file)
{
	addToRecentFilesList(m_settings.m_recentGeomFiles, file, mainMenu->menuRecentGeomFiles, mainMenu->recentGeomFilesActionGroup);
}

void Ui::CMainWindow::addToRecentImageFiles(const QString& file)
{
	addToRecentFilesList(m_settings.m_recentImages, file, mainMenu->menuRecentImages, mainMenu->recentImageFilesActionGroup);
}

void Ui::CMainWindow::showProjectViewer()
{
	projectViewer->parentWidget()->raise();
}

void Ui::CMainWindow::showModelViewer()
{
	modelViewer->parentWidget()->raise();
}

void Ui::CMainWindow::showBuildPanel()
{
	buildPanel->parentWidget()->raise();
}

void Ui::CMainWindow::showPostPanel()
{
	postPanel->parentWidget()->raise();
}

void Ui::CMainWindow::showDatabasePanel()
{
	databasePanel->parentWidget()->raise();
}

void Ui::CMainWindow::showTimeline()
{
	timePanel->parentWidget()->raise();
}

void Ui::CMainWindow::stopAnimation()
{
	m_isAnimating = false;
	postToolBar->CheckPlayButton(false);
}

void Ui::CMainWindow::ShowDefaultBackground()
{
	centralWidget->htmlViewer->setDocument(nullptr);
}

void Ui::CMainWindow::setUIConfig(Ui::Config newConfig)
{
	m_activeConfig = newConfig;
	assert(m_configs[newConfig]->GetConfig() == newConfig);
	m_configs[newConfig]->Apply();
}

void Ui::CMainWindow::setRecentFileList(QStringList& dstList, const QStringList& fileList, QMenu* menu, QActionGroup* actionGroup)
{
	dstList = fileList;

	int N = dstList.count();
	if (N > MAX_RECENT_FILES) N = MAX_RECENT_FILES;

	for (int i = 0; i < N; ++i)
	{
		QString file = dstList.at(i);

		if (menu)
		{
			QAction* pa = menu->addAction(file);
			if (actionGroup) actionGroup->addAction(pa);
		}
	}
}

void Ui::CMainWindow::addToRecentFilesList(QStringList& dstList, const QString& file, QMenu* menu, QActionGroup* actionGroup)
{
	QString fileName = file;

#ifdef WIN32
	// on windows, make sure that all filenames use backslashes
	fileName.replace('/', '\\');
#endif

	QList<QAction*> actionList = menu->actions();
	if (actionList.isEmpty())
	{
		dstList.append(fileName);
		QAction* action = menu->addAction(fileName);
		actionGroup->addAction(action);
	}
	else
	{
		// we need the first action so that we can insert before it
		QAction* firstAction = actionList.at(0);

		// see if the file already exists or not
		int n = dstList.indexOf(fileName);
		if (n >= 0)
		{
			// if the file exists, we move it to the top
			if (n != 0)
			{
				QAction* action = actionList.at(n);
				menu->removeAction(action);
				menu->insertAction(firstAction, action);

				dstList.removeAt(n);
				dstList.push_front(fileName);
			}
		}
		else
		{
			int N = dstList.count();
			if (N >= MAX_RECENT_FILES)
			{
				// remove the last one
				dstList.removeLast();
				menu->removeAction(actionList.last());
			}

			// add a new file item
			dstList.push_front(fileName);
			QAction* pa = new QAction(fileName);
			menu->insertAction(firstAction, pa);
			actionGroup->addAction(pa);
		}
	}
}

void Ui::CMainWindow::addToRecentFilesList(QStringList& dstList, const QString& file)
{
	QString fileName = file;

#ifdef WIN32
	// on windows, make sure that all filenames use backslashes
	fileName.replace('/', '\\');
#endif

	if (dstList.isEmpty())
	{
		dstList.append(fileName);
	}
	else
	{
		// see if the file already exists or not
		int n = dstList.indexOf(fileName);
		if (n >= 0)
		{
			// if the file exists, we move it to the top
			if (n != 0)
			{
				dstList.removeAt(n);
				dstList.push_front(fileName);
			}
		}
		else
		{
			int N = dstList.count();
			if (N >= MAX_RECENT_FILES)
			{
				// remove the last one
				dstList.removeLast();
			}

			// add a new file item
			dstList.push_front(fileName);
		}
	}
}

void Ui::CMainWindow::showPartViewer(CModelDocument* doc)
{
	if (partViewer == nullptr) partViewer = new CDlgPartViewer(m_wnd);
	partViewer->SetDocument(doc);
	partViewer->show();
}

void Ui::CMainWindow::setActiveCentralView(CMainCentralWidget::Viewer viewer)
{
	centralWidget->setActiveView(viewer);
	QWidget* w = centralWidget->activeView();
	CDocumentView* v = dynamic_cast<CDocumentView*>(w);
	if (v) v->setDocument(m_wnd->GetDocument());
}
