#include "stdafx.h"
#include "FEBioJob.h"
#include "PostDoc.h"
#include "Document.h"
#include <sstream>
#include <QtCore/QString>

//-----------------------------------------------------------------------------
int CFEBioJob::m_count = 0;

CFEBioJob::CFEBioJob(CDocument* doc) : m_doc(doc)
{
	m_count++;
	std::stringstream ss;
	ss << "job" << m_count;
	SetName(ss.str());

	m_status = NONE;

	m_postDoc = nullptr;
}

CFEBioJob::~CFEBioJob()
{
	delete m_postDoc;
}

CFEBioJob::CFEBioJob(CDocument* doc, const std::string& jobName, const std::string& workingDirectory) : m_doc(doc)
{
	// set the job's name
	SetName(jobName);

	string dir = workingDirectory;
	char ch = dir.back();
	if (!((ch == '/') || (ch == '\\')))
	{
#ifdef WIN32
		dir += "\\";
#else
		dir += "/";
#endif
	}

	// build the feb file name
	m_fileName = dir + jobName + ".feb";

	m_status = NONE;

	// set default plot file name
	m_plotFile = m_fileName;
	size_t pos = m_plotFile.rfind(".");
	if (pos != std::string::npos)
	{
		// remove extension
		m_plotFile.erase(pos + 1);
	}

	// add the xplt extension
	m_plotFile += "xplt";

	m_postDoc = nullptr;
}

void CFEBioJob::SetStatus(JOB_STATUS status)
{
	m_status = status;
}

int CFEBioJob::GetStatus()
{
	return m_status;
}

void CFEBioJob::SetFileName(const std::string& fileName)
{
	m_fileName = fileName;
}

std::string CFEBioJob::GetFileName() const
{
	return m_fileName;
}

void CFEBioJob::SetPlotFileName(const std::string& plotFile)
{
	m_plotFile = plotFile;
}

std::string CFEBioJob::GetPlotFileName() const
{
	return m_plotFile;
}

bool CFEBioJob::HasPostDoc()
{
	return (m_postDoc != nullptr);
}

CPostDoc* CFEBioJob::GetPostDoc()
{
	return m_postDoc;
}

size_t CFEBioJob::RemoveChild(FSObject* po)
{
	assert(m_postDoc == po);
	m_postDoc = nullptr;
	return 0;
}

bool CFEBioJob::OpenPlotFile()
{
	QString plotFile = QString::fromStdString(m_plotFile);

	QString projectFolder = QString::fromStdString(m_doc->GetDocFolder());
	plotFile.replace("$(ProjectDir)", projectFolder);

	string splotfile = plotFile.toStdString();

	if (m_postDoc == nullptr)
	{
		m_postDoc = new CPostDoc;
		m_postDoc->SetParent(this);
		m_postDoc->SetName(GetName());
	}

	if (m_postDoc->LoadPlotfile(splotfile) == false)
	{
		return false;
	}

	return true;
}

void CFEBioJob::Save(OArchive& ar)
{
	ar.WriteChunk(CID_FEOBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	ar.WriteChunk(CID_FEBIOJOB_FILENAME, m_fileName);
	ar.WriteChunk(CID_FEBIOJOB_PLOTFILE, m_plotFile);
}

void CFEBioJob::Load(IArchive& ar)
{
	// TODO: get relative file name
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_FEBIOJOB_FILENAME: ar.read(m_fileName); break;
		case CID_FEBIOJOB_PLOTFILE: ar.read(m_plotFile); break;
		}
		ar.CloseChunk();
	}
}
