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

#include "RepositoryPanel.h"
#include "GLView.h"
#include "ImageSliceView.h"
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QFile>
#include <QDockWidget>
#include <QToolBar>
#include <QProgressBar>
#include <QComboBox>
#include <QBoxLayout>
#include <QSpinBox>
#include <QStackedWidget>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QFontComboBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include "XMLTreeView.h"
#include "ProjectViewer.h"
#include "ModelViewer.h"
#include "ModelTree.h"
#include "CurveEditor.h"
#include "MeshInspector.h"
#include "LogPanel.h"
#include "BuildPanel.h"
#include "ImageSettingsPanel.h"
#include "2DImageTimeView.h"
#include "GLControlBar.h"
#include "Document.h"
#include "PostPanel.h"
#include "InfoPanel.h"
#include "LaunchConfig.h"
#include "MainTabBar.h"
#include "MainStatusBar.h"
#include "DlgMeasure.h"
#include "DlgPlaneCut.h"
#include "DlgPickColor.h"
#include "DlgExplodedView.h"
#include "PostToolBar.h"
#include "ImageToolBar.h"
#include "FEBioStudioProject.h"
#include "TimelinePanel.h"
#include "UpdateChecker.h"
#include "TextEditor.h"
#include "FEBioJobManager.h"
#include "XMLDocument.h"
#include "PostDocument.h"
#include "ui_config.h"
#include "DlgFiberViz.h"
#include "GLViewer.h"
#include "DlgPartViewer.h"
#include "DlgScreenCapture.h"
#include <PyLib/PythonToolsPanel.h>
#include "DlgPartViewer.h"
#include <FEBioMonitor/FEBioMonitorDoc.h>
#include <FEBioMonitor/FEBioMonitorPanel.h>
#include <FEBioMonitor/FEBioMonitorView.h>
#include <FEBioMonitor/FEBioReportView.h>
#include <vector>
#include "HTMLBrowser.h"
#include "PythonEditor.h"
#include "MainMenu.h"
#include "PluginManager.h"
#include <PyLib/PythonRunner.h>

class QProcess;

class CentralStackedWidget : public QStackedWidget
{
public:
	CentralStackedWidget(CMainWindow* wnd) : m_wnd(wnd)
	{
		setAcceptDrops(true);
	}

	void dragEnterEvent(QDragEnterEvent* e) override
	{
		if (e->mimeData()->hasUrls()) {
			e->acceptProposedAction();
		}
	}

	void dropEvent(QDropEvent* e) override
	{
		foreach(const QUrl & url, e->mimeData()->urls()) {
			QString fileName = url.toLocalFile();

			FileReader* fileReader = nullptr;

			QFileInfo file(fileName);

			// Create a file reader
			// NOTE: For FEB files I prefer to open the file as a separate model,
			// so I need this hack. 
			if (file.suffix() != "feb") fileReader = m_wnd->CreateFileReader(fileName);

			CDocument* doc = m_wnd->GetDocument();

			// make sure we have one
			if (fileReader && doc)
			{
				m_wnd->ReadFile(doc, fileName, fileReader, 0);
			}
			else {
				m_wnd->OpenFile(fileName, false, false);
			}
		}
	}

private:
	CMainWindow* m_wnd;

};

// The central widget of the main window
// This class manages the tab widget and all the view classes. 
class CMainCentralWidget : public QWidget
{
public:
	// This is the order in which the view widgets are added to the stack widget.
	// Do not change order and add new views to the bottom.
	enum Viewer {
		HTML_VIEWER,
		TEXT_VIEWER,
		XML_VIEWER,
		IMG_SLICE,
		TIME_VIEW_2D,
		GL_VIEWER,
		FEBREPORT_VIEW
	};

public:
	CMainTabBar* tab;

	CentralStackedWidget* stack;
	CGLViewer* glw;
	CHTMLBrowser* htmlViewer;
	CTextEditView* txtEdit;
	::XMLTreeView* xmlTree;
	CImageSliceView* sliceView;
	::C2DImageTimeView* timeView2D;
	CFEBioReportView* febReportView;

public:
	CMainCentralWidget(CMainWindow* wnd) : m_wnd(wnd)
	{
		QVBoxLayout* centralLayout = new QVBoxLayout;
		centralLayout->setContentsMargins(0, 0, 0, 0);
		centralLayout->setSpacing(0);

		tab = new CMainTabBar(wnd);
		tab->setObjectName("tab");

		stack = new CentralStackedWidget(wnd);

		htmlViewer = new CHTMLBrowser(wnd);

		stack->addWidget(htmlViewer);

		txtEdit = new CTextEditView(wnd);
		stack->addWidget(txtEdit);

		xmlTree = new ::XMLTreeView(wnd);
		xmlTree->setObjectName("xmlTree");
		stack->addWidget(xmlTree);

		sliceView = new ::CImageSliceView(wnd);
		sliceView->setObjectName("sliceView");
		stack->addWidget(sliceView);

		timeView2D = new ::C2DImageTimeView(wnd);
		timeView2D->setObjectName("timeView2D");
		stack->addWidget(timeView2D);

		glw = new CGLViewer(wnd);
		stack->addWidget(glw);

		febReportView = new CFEBioReportView(wnd);
		febReportView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		stack->addWidget(febReportView);

		centralLayout->addWidget(tab);
		centralLayout->addWidget(stack);
		setLayout(centralLayout);
	}

	void setActiveView(Viewer viewer)
	{
		stack->setCurrentIndex(viewer);
	}

	QWidget* activeView()
	{
		return stack->currentWidget();
	}

	void SetDocumentTabText(CDocument* doc, QString text, QString tooltip)
	{
		int n = tab->findView(doc); assert(n >= 0);
		if (n >= 0)
		{
			tab->setTabText(n, text);
			tab->setTabToolTip(n, tooltip);
		}
	}

private:
	CMainWindow* m_wnd;
};

struct FBS_SETTINGS
{
	int		defaultUnits;	// default units used for new model and post documents
	int		autoSaveInterval; // interval (in seconds) between autosaves

	QString FEBioSDKInc;	// path to FEBio SDK includes
	QString FEBioSDKLib;	// path to FEBio SDK libraries
	QString createPluginPath;	// default path to FEBio plugins

	bool	clearUndoOnSave;	// clear the undo stack on save

	// folder settings
	QString		m_defaultProjectParent;
	QString		m_currentPath;
	QStringList	m_recentFiles;
	QStringList	m_recentGeomFiles;
	QStringList m_recentProjects;
	QStringList m_recentPlugins;
	QStringList m_recentImages;

	QString m_envMapFile;

	vector<CLaunchConfig*> m_launch_configs;
};

class Ui::CMainWindow
{
	enum
	{
		MAX_RECENT_FILES = 15		// max number of recent files
	};

public:
	::CMainWindow* m_wnd;

	CMainCentralWidget* centralWidget;

	// dockable widgets
	::CProjectViewer* projectViewer;
	::CModelViewer* modelViewer;
	::CBuildPanel* buildPanel;
	::CLogPanel* logPanel;
	::CPostPanel* postPanel;
	::CInfoPanel* infoPanel;
	::CRepositoryPanel* databasePanel;
//    ::CPythonToolsPanel*	pythonToolsPanel;
	::CTimelinePanel* timePanel;
	::CImageSettingsPanel* imageSettingsPanel;
	CFEBioMonitorPanel* febioMonitor;
	CFEBioMonitorView* febioMonitorView;

	// additional windows
	::CDlgFiberViz* fiberViz = nullptr;
	::CDlgMeasure* measureTool = nullptr;
	::CDlgPlaneCut* planeCutTool = nullptr;
	::CDlgPickColor* pickColorTool = nullptr;
	::CDlgExplodedView* explodeTool = nullptr;
	::CCurveEditor* curveWnd = nullptr;
	::CMeshInspector* meshWnd = nullptr;
	::CDlgScreenCapture* imageView = nullptr;
	::CPythonEditor* pythonEditor = nullptr;

	CDlgPartViewer* partViewer = nullptr;

	CMainStatusBar* statusBar;

	// toolbars
	QToolBar* mainToolBar;
	QComboBox* coord;

	CPostToolBar* postToolBar;

	QToolBar* buildToolBar;
	
	CImageToolBar* imageToolBar;

	QToolBar* pFontToolBar;
	QFontComboBox* pFontStyle;
	QSpinBox* pFontSize;
	QAction* actionFontBold;
	QAction* actionFontItalic;

	QToolBar* xmlToolbar;
	QAction* actionEditXmlAsText;

	QToolBar* monitorToolBar;

	// Python stuff
	QThread m_pyThread;
	CPythonRunner* m_pyRunner = nullptr;

public:
	CMainMenu* mainMenu;

public:
	FBS_SETTINGS m_settings;

	CFEBioJobManager* m_jobManager;

	bool	m_isAnimating;

	QList<::CGraphWindow*>	graphList;

	FEBioStudioProject	m_project;

	QTimer* m_autoSaveTimer = nullptr;

	CUpdateWidget m_updateWidget; // TODO: Why is this a widget? It is not used as a widget.
	QString m_serverMessage;
	bool m_updaterPresent;
	bool m_updateAvailable;
	bool m_updateOnClose;
	bool m_updateDevChannel;

    CPluginManager m_pluginManager;

public:
	CGLDocument* m_copySrc = nullptr; // source for copy selection operation

public:
	vector<CUIConfig*>	m_configs;
	unsigned int m_activeConfig = Ui::Config::EMPTY_CONFIG;

public:
	CMainWindow();

	void setupUi(::CMainWindow* wnd);

	void setActiveCentralView(CMainCentralWidget::Viewer viewer);

	CLaunchConfig* findLaunchConfig(const std::string& name)
	{
		for (auto lc : m_settings.m_launch_configs)
		{
			if (lc->name() == name) return lc;
		}
		return nullptr;
	}

private:
	QAction* addAction(const QString& title, const QString& name, const QString& iconFile = QString(), bool bcheckable = false);

	// create toolbars
	void buildToolbars(::CMainWindow* mainWindow);

	// build the dockable windows
	// Note that this must be called after the menu is created.
	void buildDockWidgets(::CMainWindow* wnd);

	void BuildConfigs();

public:
	void SetSelectionMode(int nselect);

	void showCurveEditor();

	void showMeshInspector();

	void updateMeshInspector();

	void setRecentFiles(QStringList& recentFiles);

	void setRecentProjects(QStringList& recentFiles);

	void setRecentPlugins(QStringList& recentPlugins);

	void setRecentGeomFiles(QStringList& recentFiles);

	void setRecentImageFiles(QStringList& recentImages);

	void addToRecentFiles(const QString& file);

	void addToRecentProjects(const QString& file);

	void addToRecentPlugins(const QString& file);

	void addToRecentGeomFiles(const QString& file);

	void addToRecentImageFiles(const QString& file);

	void showProjectViewer();

	void showModelViewer();

	void showBuildPanel();

	void showPostPanel();

	void showDatabasePanel();

	void showTimeline();

	void stopAnimation();

	void ShowDefaultBackground();

	void setUIConfig(Ui::Config newConfig);

	void showPartViewer(CModelDocument* doc);

private:
	void setRecentFileList(QStringList& dstList, const QStringList& fileList, QMenu* menu, QActionGroup* actionGroup);

	void addToRecentFilesList(QStringList& dstList, const QString& file, QMenu* menu, QActionGroup* actionGroup);

	void addToRecentFilesList(QStringList& dstList, const QString& file);
};
