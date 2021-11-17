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
			
			QString fileName;
			// Handle custom URL scheme
			if (!openEvent->url().isEmpty())
        	{
				fileName =openEvent->url().toString();
			}
			else
			{	
				fileName = openEvent->file();
			}

			m_pWnd->OpenFile(fileName);
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

	QDir febioDir(QApplication::applicationDirPath());
	febioDir.cd("../febio");
	string febdir = QDir::toNativeSeparators(febioDir.absolutePath()).toStdString();
	FSDir::setMacro("FEBioDir", febdir);

	// show the splash screen
	QPixmap pixmap(":/icons/splash.png");
    qreal pixelRatio = app.devicePixelRatio();
    pixmap.setDevicePixelRatio(pixelRatio);
	QSplashScreen splash(pixmap);
	splash.show();

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

    // set the display name (this will be displayed on all windows and dialogs)
    app.setApplicationVersion("1.0.0");
    app.setApplicationName("FEBio Studio");
    app.setApplicationDisplayName("FEBio Studio");

	string appdir = QApplication::applicationDirPath().toStdString();
	
    qDebug() << appdir.c_str();
    
	FSDir::setMacro("FEBioStudioDir", appdir);

	// show the splash screen
	QPixmap pixmap(":/icons/splash_hires.png");
    qreal pixelRatio = app.devicePixelRatio();
    pixmap.setDevicePixelRatio(pixelRatio);
	QSplashScreen splash(pixmap);
	splash.show();
	
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
	app.SetMainWindow(&wnd);
	wnd.show();	

	splash.finish(&wnd);

    if ((argc == 2) && (breset == false))
    {
        wnd.OpenFile(argv[1], false);
    }
    
	return app.exec();

#endif
}

CMainWindow* PRV::getMainWindow()
{
	CMainWindow* wnd = CMainWindow::GetInstance(); assert(wnd);
	return wnd;
}

CDocument* PRV::getDocument()
{
	CMainWindow* wnd = PRV::getMainWindow();
	return wnd->GetDocument();
}
