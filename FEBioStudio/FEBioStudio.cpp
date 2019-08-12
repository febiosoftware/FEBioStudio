#ifdef WIN32
#include <glew.h>
#endif
#include "stdafx.h"
#include <QApplication>
#include <QFileDialog>
#include "MainWindow.h"
#include "FEBioStudio.h"
#include <stdio.h>
#include <PostViewLib/PostView.h>

#ifdef __APPLE__
#include <QFileOpenEvent>
class MyApplication : public QApplication
{
public:
	MyApplication(int &argc, char **argv)
		: QApplication(argc, argv)
	{
	}
	void SetMainWindow(CMainWindow* wnd) { m_pWnd = wnd; }

	bool event(QEvent *event)
	{
		if (event->type() == QEvent::FileOpen) {
			QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
			QString fileName = openEvent->file();
			QString ext = QFileInfo(fileName).suffix();
			if (ext.compare("feb", Qt::CaseInsensitive) == 0)
				m_pWnd->OpenFEModel(fileName);
			else if (ext.compare("prv", Qt::CaseInsensitive) == 0)
				m_pWnd->OpenDocument(fileName);
		}

		return QApplication::event(event);
	}

public:
	CMainWindow* m_pWnd;
};

#endif

// starting point of application
int main(int argc, char* argv[])
{
	// Initialize the PostView libraries
	Post::Initialize();

#ifndef __APPLE__
	// create the application object
	QApplication app(argc, argv);

	// set the display name (this will be displayed on all windows and dialogs)
	app.setApplicationVersion("1.0.0");
	app.setApplicationName("FEBio Studio");
	app.setApplicationDisplayName("FEBio Studio");
	app.setWindowIcon(QIcon(":/icons/FEBioStudio.png"));


	// initialize glew
#ifdef WIN32
	glewInit();
#endif

	// see if the reset flag was defined
	// the reset flag can be used to restore the UI, i.e.
	// when reset is true, the CMainWindow class will not read the settings.
	bool breset = false;
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-reset") == 0)
		{
			breset = true;
			break;
		}
	}

	// create the main window
	CMainWindow wnd(breset);
	//	wnd.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	wnd.show();

	if ((argc == 2) && (breset == false))
	{
		wnd.OpenDocument(argv[1]);
	}
	else
	{
		wnd.on_actionNew_triggered();
	}

	return app.exec();

#else
	MyApplication app(argc, argv);

	// create the main window
	CMainWindow wnd;
	wnd.show();

	app.setApplicationVersion("1.0.0");
	app.setApplicationName("FEBio Studio");
	app.setApplicationDisplayName("FEBio Studio");
	app.SetMainWindow(&wnd);
	return app.exec();

#endif
}

CMainWindow* PRV::getMainWindow()
{
	CMainWindow* wnd = dynamic_cast<CMainWindow*>(QApplication::activeWindow()); assert(wnd);
	return wnd;
}

CDocument* PRV::getDocument()
{
	CMainWindow* wnd = PRV::getMainWindow();
	return wnd->GetDocument();
}
