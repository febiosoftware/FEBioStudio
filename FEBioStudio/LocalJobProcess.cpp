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

	// get ready ...
	m_wnd->AddLogEntry(QString("Starting FEBio: %1\n").arg(args.join(" ")));

	// set ...
	m_job->SetStatus(CFEBioJob::RUNNING);

	// make rocket go now!
	start(program, args);
}
