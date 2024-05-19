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

#include <QApplication>
#include <QIcon>
#include <QString>
#include <QFileInfo>
#include <QFileDialog>
#include "MainWindow.h"
#include <FEBioStudioUpdater.h>
#include <stdio.h>
#include <QSplashScreen>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <string.h>
#include <XML/XMLReader.h>

// starting point of application
int main(int argc, char* argv[])
{
	if(argc > 1 && QString(argv[1]) == QString("--uninstall"))
	{
		uninstall();
	}
	else
	{
		// create the application object
		QApplication app(argc, argv);

		// set the display name (this will be displayed on all windows and dialogs)
		app.setApplicationVersion("1.0.0");
		app.setApplicationName("FEBio Studio Updater");
		app.setApplicationDisplayName("FEBio Studio Updater");
		app.setWindowIcon(QIcon(":/icons/FEBioStudio.png"));

		// Check if --devChannel flag is present
		bool devChannel = false;
		bool updaterUpdateCheck = true;
		for(int index = 1; index < argc; index++)
		{
			if(QString(argv[index]) == QString("--devChannel"))
			{
				devChannel = true;
			}
			else if(QString(argv[index]) == QString("--noUpdaterCheck"))
			{
				updaterUpdateCheck = false;
			}
		}
		
		 argc > 1 && QString(argv[1]) == QString("--devChannel");

		// create the main window
		CMainWindow wnd(devChannel, updaterUpdateCheck);
		wnd.show();

		return app.exec();
	}
}

void readXML(QStringList& files, QStringList& dirs)
{
	char updaterPath[1050];
	char updaterDir[1024];
	get_app_path(updaterDir, 1023);
	sprintf(updaterPath, "%sautoUpdate.xml", updaterDir);

	XMLReader reader;
	if(reader.Open(updaterPath) == false) return;

	XMLTag tag;
	if(reader.FindTag("autoUpdate", tag) == false) return;

	++tag;
    do
    {
        if(tag == "file")
        {
            files.append(tag.m_szval.c_str());
        }
        else if(tag == "dir")
        {
            dirs.append(tag.m_szval.c_str());
        }

        ++tag;
    }
    while(!tag.isend());

	reader.Close();
}

void uninstall()
{
	char updaterPath[1050];
	char updaterDir[1024];
	get_app_path(updaterDir, 1023);
	sprintf(updaterPath, "%sautoUpdate.xml", updaterDir);

	if(QFileInfo::exists(updaterPath))
	{
		QStringList files;
		QStringList dirs;

		readXML(files, dirs);

		files.append("autoUpdate.xml");

		for(auto file : files)
		{
			QFile::remove(file);
		}


		for(auto dir : dirs)
		{
			QDir temp(dir);
			temp.removeRecursively();
		}
	}
}

CMainWindow* getMainWindow()
{
	CMainWindow* wnd = dynamic_cast<CMainWindow*>(QApplication::activeWindow()); assert(wnd);
	return wnd;
}
