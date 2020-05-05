#include "stdafx.h"
#include "FEBioJob.h"
#include "ModelDocument.h"
#include <PostLib/FEPostModel.h>
#include <sstream>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <FSCore/FSDir.h>
#include <QtCore/QThread>
#ifdef HAS_SSH
#include "SSHHandler.h"
#endif

#include <iostream>

//-----------------------------------------------------------------------------
int CFEBioJob::m_count = 0;
CFEBioJob*	CFEBioJob::m_activeJob = nullptr;

void CFEBioJob::SetActiveJob(CFEBioJob* activeJob) { m_activeJob = activeJob; }
CFEBioJob* CFEBioJob::GetActiveJob() { return m_activeJob; }


CFEBioJob::CFEBioJob(CModelDocument* doc) : m_doc(doc)
{
	m_count++;
	std::stringstream ss;
	ss << "job" << m_count;
	SetName(ss.str());

	m_status = NONE;
}

CFEBioJob::~CFEBioJob()
{
#ifdef HAS_SSH
	delete m_sshHandler;
#endif
}

CFEBioJob::CFEBioJob(CModelDocument* doc, const std::string& jobName, const std::string& workingDirectory, CLaunchConfig launchConfig)
	: m_doc(doc), m_launchConfig(launchConfig)
{
	// set the job's name
	SetName(jobName);

	// build the feb file name
	m_febFile = workingDirectory + "/" + jobName + ".feb";

	m_status = NONE;

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

#ifdef HAS_SSH
	if(launchConfig.type == LOCAL)
	{
		m_sshHandler = nullptr;
	}
	else
	{
		m_sshHandler = NewHandler();
	}
#endif

}

CModelDocument* CFEBioJob::GetDocument()
{
	return m_doc;
}

void CFEBioJob::UpdateWorkingDirectory(const std::string& dir)
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

CLaunchConfig* CFEBioJob::GetLaunchConfig()
{
	return &m_launchConfig;
}

void CFEBioJob::UpdateLaunchConfig(CLaunchConfig launchConfig)
{
	CLaunchConfig oldConfig = m_launchConfig;

	m_launchConfig = launchConfig;

#ifdef HAS_SSH
	if(launchConfig.type == LOCAL)
	{
		if(m_sshHandler)
		{
			delete m_sshHandler;
		}

		m_sshHandler = nullptr;
	}
	else
	{
		if(m_sshHandler)
		{
			m_sshHandler->Update(oldConfig);
		}
		else
		{
			m_sshHandler = NewHandler();
		}
	}
#endif

}

#ifdef HAS_SSH
CSSHHandler* CFEBioJob::GetSSHHandler()
{
	return m_sshHandler;
}

CSSHHandler* CFEBioJob::NewHandler()
{
	CSSHHandler* handler = new CSSHHandler(this);

	QObject::connect(handler, &CSSHHandler::ShowProgress, m_doc->GetMainWindow(), &CMainWindow::ShowProgress);
	QObject::connect(handler, &CSSHHandler::UpdateProgress, m_doc->GetMainWindow(), &CMainWindow::UpdateProgress);

	return handler;
}

#endif

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

std::string CFEBioJob::GetFEBFileName() const
{
	return m_febFile;
}

void CFEBioJob::SetPlotFileName(const std::string& plotFile)
{
	m_plotFile = plotFile;
}

std::string CFEBioJob::GetPlotFileName() const
{
	return m_plotFile;
}

void CFEBioJob::SetConfigFileName(const std::string& configFile)
{
	m_cnfFile = configFile;
}

std::string CFEBioJob::GetConfigFileName() const
{
	return m_cnfFile;
}

void CFEBioJob::Save(OArchive& ar)
{
	ar.WriteChunk(CID_FEOBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	ar.WriteChunk(CID_FEBIOJOB_FILENAME, m_febFile);
	ar.WriteChunk(CID_FEBIOJOB_PLOTFILE, m_plotFile);

	ar.BeginChunk(CID_FEBIOJOB_LCONFIG);
	m_launchConfig.Save(ar);
	ar.EndChunk();

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
		case CID_FEBIOJOB_LCONFIG: {CLaunchConfig lConfig; lConfig.Load(ar); m_sshHandler = nullptr; UpdateLaunchConfig(lConfig); } break;
		}
		ar.CloseChunk();
	}
}
