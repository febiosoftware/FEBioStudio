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
#include "FEBioJob.h"
#include "ModelDocument.h"
#include <sstream>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <FSCore/FSDir.h>
#include <iostream>
#include <chrono>

using namespace std::chrono;

int CFEBioJob::m_count = 0;
CFEBioJob*	CFEBioJob::m_activeJob = nullptr;

void CFEBioJob::SetActiveJob(CFEBioJob* activeJob) { m_activeJob = activeJob; }
CFEBioJob* CFEBioJob::GetActiveJob() { return m_activeJob; }

CFEBioJob::CFEBioJob(CDocument* doc) : m_doc(doc)
{
	m_count++;
	std::stringstream ss;
	ss << "job" << m_count;
	SetName(ss.str());

	m_status = NONE;
	m_writeNotes = true;
	m_allowMixedMesh = false;

	m_bhasProgress = false;
	m_pct = 0.0;
	m_tic = m_toc = 0.0;
}

CFEBioJob::~CFEBioJob()
{
	if (m_activeJob == this)
	{
		m_activeJob = nullptr;
	}
}

CFEBioJob::CFEBioJob(CDocument* doc, const std::string& jobName, const std::string& workingDirectory) : m_doc(doc)
{
	// set the job's name
	SetName(jobName);

	// build the feb file name
	m_febFile = workingDirectory + "/" + jobName;

	// add extension
	const char* sz = strrchr(jobName.c_str(), '.');
	if ((sz == nullptr) || (strcmp(sz, ".feb") != 0))
	{
		m_febFile += ".feb";
	}

	m_status = NONE;

	m_writeNotes = true;
	m_allowMixedMesh = false;

	// set default plot file name
	m_plotFile = m_febFile;
	size_t pos = m_plotFile.rfind(".");
	if (pos != std::string::npos)
	{
		// remove extension
		m_plotFile.erase(pos + 1);
	}

	// add the xplt extension
	m_plotFile += "xplt";

	// set default log file name
	m_logFile = m_febFile;
	pos = m_logFile.rfind(".");
	if (pos != std::string::npos)
	{
		// remove extension
		m_logFile.erase(pos + 1);
	}

	// add the log extension
	m_logFile += "log";

	m_bhasProgress = false;
	m_pct = 0.0;
	m_tic = m_toc = 0.0;
}

CDocument* CFEBioJob::GetDocument()
{
	return m_doc;
}

void CFEBioJob::StartTimer()
{
	time_point<steady_clock> tp = steady_clock::now();
	m_tic = m_toc = duration_cast< duration<double> >(tp.time_since_epoch()).count();
}

void CFEBioJob::StopTimer()
{
	time_point<steady_clock> tp = steady_clock::now();
	m_toc = duration_cast<duration<double>>(tp.time_since_epoch()).count();
}

void CFEBioJob::SetProgress(double pct)
{
	m_bhasProgress = true;
	m_pct = pct;
}

bool CFEBioJob::HasProgress() const
{
	return m_bhasProgress;
}

double CFEBioJob::GetProgress() const
{
	return m_pct;
}

void CFEBioJob::ClearProgress()
{
	m_bhasProgress = false;
	m_pct = 0.0;
}

void CFEBioJob::SetCommand(const std::string& cmd)
{
	m_cmd = cmd;
}

const std::string& CFEBioJob::GetCommand() const
{
	return m_cmd;
}

double CFEBioJob::ElapsedTime() const
{
	return m_toc - m_tic;
}

std::string CFEBioJob::GetWorkingDirectory()
{
    QFileInfo info(m_febFile.c_str());
    QString dir = info.absoluteDir().absolutePath();

	// see if this directory exists
	if (!QDir(dir).exists())
		return "";

    return dir.toStdString();
}

void CFEBioJob::SetWorkingDirectory(const std::string& dir)
{
	QString dirName = QString::fromStdString(dir);

	// update feb file name
	QString febName = QFileInfo(QString::fromStdString(m_febFile)).fileName();
	m_febFile = (dirName + "/" + febName).toStdString();

	// update xplt file name
	QString xpltName = QFileInfo(QString::fromStdString(m_plotFile)).fileName();
	m_plotFile = (dirName + "/" + xpltName).toStdString();

	// update log file name
	QString logName = QFileInfo(QString::fromStdString(m_logFile)).fileName();
	m_logFile = (dirName + "/" + logName).toStdString();
}

void CFEBioJob::SetStatus(JOB_STATUS status)
{
	m_status = status;
}

int CFEBioJob::GetStatus()
{
	return m_status;
}

void CFEBioJob::SetFEBFileName(const std::string& fileName)
{
	m_febFile = fileName;
}

std::string CFEBioJob::GetFEBFileName(bool relative) const
{
	if(relative || (m_doc == nullptr)) return m_febFile;
	else
	{
		// do string substitution
		string febFile = FSDir::expandMacros(m_febFile);
		return m_doc->ToAbsolutePath(febFile).toStdString();
	}
}

void CFEBioJob::SetPlotFileName(const std::string& plotFile)
{
	m_plotFile = plotFile;
}

std::string CFEBioJob::GetPlotFileName(bool relative) const
{
	if(relative || (m_doc == nullptr)) return m_plotFile;
	else return m_doc->ToAbsolutePath(m_plotFile).toStdString();
}

void CFEBioJob::SetLogFileName(const std::string& logFile)
{
	m_logFile = logFile;
}

std::string CFEBioJob::GetLogFileName(bool relative) const
{
	if(relative || (m_doc == nullptr)) return m_logFile;
	else return m_doc->ToAbsolutePath(m_logFile).toStdString();
}

void CFEBioJob::SetConfigFileName(const std::string& configFile)
{
	m_cnfFile = configFile;
}

std::string CFEBioJob::GetConfigFileName() const
{
	return m_cnfFile;
}

void CFEBioJob::SetTask(const std::string& task)
{
    m_task = task;
}

std::string CFEBioJob::GetTask() const
{
    return m_task;
}

void CFEBioJob::SetTaskFileName(const std::string& taskFile)
{
    m_taskFile = taskFile;
}

std::string CFEBioJob::GetTaskFileName() const
{
    return m_taskFile;
}

void CFEBioJob::Save(OArchive& ar)
{
	ar.WriteChunk(CID_FEOBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	ar.WriteChunk(CID_FEBIOJOB_FILENAME, m_febFile);
	ar.WriteChunk(CID_FEBIOJOB_PLOTFILE, m_plotFile);
	ar.WriteChunk(CID_FEBIOJOB_LOGFILE, m_logFile);
    
    ar.WriteChunk(CID_FEBIOJOB_WRITE_NOTES, m_writeNotes);
    ar.WriteChunk(CID_FEBIOJOB_MIXED_MESH, m_allowMixedMesh);
    ar.WriteChunk(CID_FEBIOJOB_DEBUG, m_debug);
    ar.WriteChunk(CID_FEBIOJOB_CONFIG_FILE, m_cnfFile);
    ar.WriteChunk(CID_FEBIOJOB_TASK, m_task);
    ar.WriteChunk(CID_FEBIOJOB_TASK_FILE, m_taskFile);
    ar.WriteChunk(CID_FEBIOJOB_COMMAND, m_cmd);
}

void CFEBioJob::Load(IArchive& ar)
{
	// TODO: get relative file name
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_FEBIOJOB_FILENAME:
			ar.read(m_febFile);
#ifndef WIN32
			m_febFile = QString::fromStdString(m_febFile).replace("\\","/").toStdString();
#endif
			break;
		case CID_FEBIOJOB_PLOTFILE:
			ar.read(m_plotFile);
#ifndef WIN32
			m_plotFile = QString::fromStdString(m_plotFile).replace("\\","/").toStdString();
#endif
			break;
		case CID_FEBIOJOB_LOGFILE:
			ar.read(m_logFile);
#ifndef WIN32
			m_logFile = QString::fromStdString(m_logFile).replace("\\","/").toStdString();
#endif
			break;
        case CID_FEBIOJOB_WRITE_NOTES: ar.read(m_writeNotes); break;
        case CID_FEBIOJOB_MIXED_MESH: ar.read(m_allowMixedMesh); break;
        case CID_FEBIOJOB_DEBUG: ar.read(m_debug); break;
        case CID_FEBIOJOB_CONFIG_FILE: ar.read(m_cnfFile); break;
        case CID_FEBIOJOB_TASK: ar.read(m_task); break;
        case CID_FEBIOJOB_TASK_FILE: ar.read(m_taskFile); break;
        case CID_FEBIOJOB_COMMAND: ar.read(m_cmd); break;
		}
		ar.CloseChunk();
	}
}
