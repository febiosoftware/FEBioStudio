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
#include <QProcess>
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
#include "XMLReader.h"

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
		app.setApplicationName("FEBio Studio");
		app.setApplicationDisplayName("FEBio Studio Updater");
		app.setWindowIcon(QIcon(":/icons/FEBioStudio.png"));

		// Check if --devChannel flag is present
		bool devChannel = false;
		bool updaterUpdateCheck = true;
        QString branch = "";
		for(int index = 1; index < argc; index++)
		{
			if(QString(argv[index]) == QString("--devChannel") || QString(argv[index]) == QString("-d"))
			{
				devChannel = true;
			}
			else if(QString(argv[index]) == QString("--noUpdaterCheck") || QString(argv[index]) == QString("-n"))
			{
				updaterUpdateCheck = false;
			}
            else if((QString(argv[index]) == QString("--branch") || QString(argv[index]) == QString("-b")) && index + 1 < argc)
            {
                devChannel = true;
                branch = QString(argv[index + 1]);
            }
		}

		// create the main window
		CMainWindow wnd(devChannel, updaterUpdateCheck, branch);
		wnd.show();

		return app.exec();
	}
}

void readXML(QStringList& files, QStringList& dirs, QStringList& uninstallCmds)
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
        else if(tag == "uninstallCmds")
        {
            ++tag;
            do
            {
                if(tag == "cmd")
                {
                    uninstallCmds.append(tag.m_szval.c_str());
                }

                ++tag; 
            }
            while(!tag.isend());

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

    QStringList files;
    QStringList dirs;
    QStringList cmds;

	if(QFileInfo::exists(updaterPath))
	{
		readXML(files, dirs, cmds);

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

        for(auto cmd : cmds)
        {
            std::system(cmd.toStdString().c_str());
        }
	}

// Ugly fix for deleting updater dependencies on Windows
#ifdef WIN32
    QStringList args;
    args << "-rm";

    int runMvUtil = false;
    for(auto file : files)
    {
        if(QFileInfo::exists(file))
        {
            args.push_back(file);

            runMvUtil = true;
        }
    }

    if(runMvUtil)
    {
        int zero = 0;
        QApplication app(zero, nullptr);

        QProcess* mvUtil = new QProcess;
        mvUtil->startDetached(QApplication::applicationDirPath() + MVUTIL, args);
    }
#endif
}

CMainWindow* getMainWindow()
{
	CMainWindow* wnd = dynamic_cast<CMainWindow*>(QApplication::activeWindow()); assert(wnd);
	return wnd;
}
