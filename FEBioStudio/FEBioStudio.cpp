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
#include "version.h"
#include <stdio.h>
#include <PostLib/PostView.h>
#include <FSCore/FSDir.h>
#include <MeshLib/FEElementLibrary.h>
#include <QSplashScreen>
#include <QDebug>
#include <QPainter>
#include <FEBioLink/FEBioInit.h>
#include "FBSApplication.h"

class FBSSplashScreen : public QSplashScreen
{
public:
	FBSSplashScreen(QPixmap& pixmap)
	{
		QPainter painter(&pixmap);

        qreal pr = pixmap.devicePixelRatio();
		QRect rtdi = pixmap.rect();
        QRect rt = QRect(rtdi.left()/pr,rtdi.top()/pr,rtdi.width()/pr,rtdi.height()/pr);
        qreal border = 20;
		rt.adjust(border, border, -border, -border);
		painter.setPen(Qt::white);
		QFont font = painter.font();
		font.setPointSizeF(25);

		// This causes the font to clip on macOS, so I've disabled it for now
#ifndef __APPLE__
		font.setStretch(175);
#endif

		font.setBold(true);
		painter.setFont(font);
		QString t1 = QString("FEBIO STUDIO");
		painter.drawText(rt, Qt::AlignRight, t1);

		QFontInfo fi(font);
		rt.setTop(rt.top() + fi.pixelSize() + 10);
		font.setPointSize(14);
		font.setStretch(100);
		font.setBold(false);
		painter.setFont(font);
		QString t2 = QString("version %1.%2.%3").arg(FBS_VERSION).arg(FBS_SUBVERSION).arg(FBS_SUBSUBVERSION);
		painter.drawText(rt, Qt::AlignRight, t2);

		QString t3 = QString("Weiss Lab, University of Utah\nAteshian Lab, Columbia University\n\nCopyright (c) 2007-2024, All rights reserved");
		painter.drawText(rt, Qt::AlignLeft | Qt::AlignBottom, t3);

		setPixmap(pixmap);
	}
};

// starting point of application
int main(int argc, char* argv[])
{
	// Initialize the libraries
	FSElementLibrary::InitLibrary();
	Post::Initialize();

	// initialize the FEBio library
	FEBio::InitFEBioLibrary();

	// create the application object
	FBSApplication app(argc, argv);

#ifdef WIN32
	app.setStyle("fusion");
#endif

	// set the display name (this will be displayed on all windows and dialogs)
	QString version = QString("%1.%2.%3").arg(FBS_VERSION).arg(FBS_SUBVERSION).arg(FBS_SUBSUBVERSION);
	app.setApplicationVersion(version);
	app.setApplicationName("FEBio Studio");
	app.setApplicationDisplayName("FEBio Studio " + version);
	app.setWindowIcon(QIcon(":/icons/FEBioStudio.png"));

	string appdir = QApplication::applicationDirPath().toStdString();
	qDebug() << appdir.c_str();
	
	FSDir::setMacro("FEBioStudioDir", appdir);

	// show the splash screen
    QPixmap pixmap;
	qreal pixelRatio = app.devicePixelRatio();
    if (pixelRatio == 1)
        pixmap = QPixmap(":/icons/splash.png");
    else
        pixmap = QPixmap(":/icons/splash_hires.png");
    pixmap.setDevicePixelRatio(pixelRatio);
    FBSSplashScreen splash(pixmap);
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

#ifdef __APPLE__
	splash.close();
#else
	splash.finish(&wnd);
#endif


	if ((argc == 2) && (breset == false))
	{
		wnd.OpenFile(argv[1], false);
	}

	return app.exec();
}

CMainWindow* FBS::getMainWindow()
{
	return FBSApplication::Instance()->GetMainWindow();
}

CDocument* FBS::getActiveDocument()
{
	CMainWindow* wnd = FBS::getMainWindow();
	return wnd->GetDocument();
}
