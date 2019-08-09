#include "stdafx.h"
#include "FEBioJob.h"
#include "PostDoc.h"
#include <sstream>

//-----------------------------------------------------------------------------
int CFEBioJob::m_count = 0;

CFEBioJob::CFEBioJob()
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

CFEBioJob::CFEBioJob(const std::string& fileName, JOB_STATUS status)
{
	m_count++;
	std::stringstream ss;
	ss << "job" << m_count;
	SetName(ss.str());

	m_fileName = fileName;

	m_status = status;

	// set default plot file name
	m_plotFile = fileName;
	size_t pos = m_plotFile.rfind(".");
	if (pos != std::string::npos)
	{
		// remove extension
		m_plotFile.erase(pos + 1);
	}

	// add the xplt extension
	m_plotFile += "xplt";
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

bool CFEBioJob::OpenPlotFile()
{
	if (m_postDoc) delete m_postDoc;
	m_postDoc = new CPostDoc;

	if (m_postDoc->Load(m_plotFile) == false)
	{
		delete m_postDoc;
		m_postDoc = nullptr;
		return false;
	}

	return true;
}
