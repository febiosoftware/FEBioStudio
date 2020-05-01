#ifdef WIN32
#include <glew.h>
#endif
#include "stdafx.h"
#include <QApplication>
#include <QFileDialog>
#include "MainWindow.h"
#include "FEBioStudio.h"
#include <stdio.h>
#include <PostLib/PostView.h>
#include <FSCore/FSDir.h>
#include <MeshLib/FEElementLibrary.h>
#include <QSplashScreen>
#include <QDebug>

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
            // create a file reader
            FileReader* fileReader = m_pWnd->CreateFileReader(fileName);
            // make sure we have one
            if (fileReader)
            {
                m_pWnd->ReadFile(fileName, fileReader, false);
            }
            else {
                QString ext = QFileInfo(fileName).suffix();
                if ((ext.compare("prv", Qt::CaseInsensitive) == 0) ||
                    (ext.compare("fsprj", Qt::CaseInsensitive) == 0))
                    m_pWnd->OpenDocument(fileName);
            }
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
	// Initialize the libraries
	FEElementLibrary::InitLibrary();
	Post::Initialize();

#ifndef __APPLE__

	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	// create the application object
	QApplication app(argc, argv);

	// set the display name (this will be displayed on all windows and dialogs)
	app.setApplicationVersion("1.0.0");
	app.setApplicationName("FEBio Studio");
	app.setApplicationDisplayName("FEBio Studio");
	app.setWindowIcon(QIcon(":/icons/FEBioStudio.png"));

	string appdir = QApplication::applicationDirPath().toStdString();
	qDebug() << appdir.c_str();
	
	FSDir::setMacro("FEBioStudioDir", appdir);

	// show the splash screen
	QPixmap pixmap(":/icons/splash.png");
    qreal pixelRatio = app.devicePixelRatio();
    pixmap.setDevicePixelRatio(pixelRatio);
	QSplashScreen splash(pixmap);
	splash.show();

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

	splash.finish(&wnd);

	if ((argc == 2) && (breset == false))
	{
		wnd.OpenFile(argv[1], false);
	}

	return app.exec();

#else
	MyApplication app(argc, argv);
	
	string appdir = QApplication::applicationDirPath().toStdString();
	
	FSDir::setMacro("FEBioStudioDir", appdir);

	// show the splash screen
	QPixmap pixmap(":/icons/splash_hires.png");
    qreal pixelRatio = app.devicePixelRatio();
    pixmap.setDevicePixelRatio(pixelRatio);
	QSplashScreen splash(pixmap);
	splash.show();
	
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
	CMainWindow wnd;
	app.SetMainWindow(&wnd);
	wnd.show();	

	splash.finish(&wnd);

	app.setApplicationVersion("1.0.0");
	app.setApplicationName("FEBio Studio");
	app.setApplicationDisplayName("FEBio Studio");
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
