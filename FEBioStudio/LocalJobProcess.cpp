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
#include "stdafx.h"
#include "LocalJobProcess.h"
#include "MainWindow.h"
#include "FEBioJob.h"
#include <FSCore/FSDir.h>
#include <QFileInfo>

CLocalJobProcess::CLocalJobProcess(CMainWindow* wnd, CFEBioJob* job, const QString& program, QObject* parent) : m_wnd(wnd), m_job(job), m_program(program)
{
	setProcessChannelMode(QProcess::MergedChannels);

	QObject::connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), parent, SLOT(onRunFinished(int, QProcess::ExitStatus)));
	QObject::connect(this, SIGNAL(readyRead()), parent, SLOT(onReadyRead()));
	QObject::connect(this, SIGNAL(errorOccurred(QProcess::ProcessError)), parent, SLOT(onErrorOccurred(QProcess::ProcessError)));
}

void CLocalJobProcess::run()
{
	// get the FEBio job file path
	string febFile = m_job->GetFEBFileName();

	// extract the working directory and file title from the file path
	QFileInfo fileInfo(QString::fromStdString(febFile));
	QString workingDir = fileInfo.absolutePath();
	QString fileName = fileInfo.fileName();

	if (workingDir.isEmpty() == false)
	{
		m_wnd->AddLogEntry(QString("Setting current working directory to: %1\n").arg(workingDir));
		setWorkingDirectory(workingDir);
	}

	// do string substitution
	QString program = m_program;
	string sprogram = program.toStdString();
	sprogram = FSDir::expandMacros(sprogram);
	program = QString::fromStdString(sprogram);

	// extract the arguments
	QString cmd = QString::fromStdString(m_job->GetCommand());
	QStringList args = cmd.split(" ", Qt::SkipEmptyParts);

	std::string configFile = m_job->GetConfigFileName();

	args.replaceInStrings("$(Filename)", fileName);
	args.replaceInStrings("$(ConfigFile)", QString::fromStdString(configFile));

	// If this job does not name a config file of its own, fall back to the one
	// the application itself is configured with.
	//
	// Without this, febio4 falls back to its own default: FEBioApp.cpp does
	// get_app_path() + "febio.xml", i.e. it looks for the config NEXT TO ITS OWN
	// BINARY. Inside the .app that is Contents/MacOS, and a non-Mach-O file
	// cannot live there -- codesign seals that directory as code and signing
	// fails with "code object is not signed at all". The config therefore ships
	// in Contents/Resources (see ui_mainwindow.cpp), which febio4 will never
	// find on its own, so it has to be passed explicitly.
	//
	// Only applied when the command does not already specify one, so a custom
	// command or a per-job config still wins.
	if ((args.contains("-config") == false) && (args.contains("-cnf") == false))
	{
		QString cnf = m_wnd->GetConfigFileName();
		if (cnf.isEmpty() == false)
		{
			QString cnfPath = QString::fromStdString(FSDir::expandMacros(cnf.toStdString()));
			if (QFileInfo::exists(cnfPath))
			{
				args << "-config" << cnfPath;
				m_wnd->AddLogEntry(QString("Using FEBio configuration file: %1\n").arg(cnfPath));
			}
			else
			{
				m_wnd->AddLogEntry(QString("Warning: FEBio configuration file not found: %1\n").arg(cnfPath));
			}
		}
	}

	// get ready ...
	m_wnd->AddLogEntry(QString("Starting FEBio: %1\n").arg(args.join(" ")));

	// set ...
	m_job->SetStatus(CFEBioJob::RUNNING);

	// make rocket go now!
	start(program, args);
}
