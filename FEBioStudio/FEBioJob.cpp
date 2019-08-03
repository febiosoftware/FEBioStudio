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

std::string CFEBioJob::GetFileName()
{
	return m_fileName;
}

bool CFEBioJob::HasPostDoc()
{
	return (m_postDoc != nullptr);
}

CPostDoc* CFEBioJob::GetPostDoc()
{
	return m_postDoc;
}

bool CFEBioJob::OpenPlotFile(const std::string& fileName)
{
	if (m_postDoc) delete m_postDoc;
	m_postDoc = new CPostDoc;

	if (m_postDoc->Load(fileName) == false)
	{
		delete m_postDoc;
		m_postDoc = nullptr;
		return false;
	}

	return true;
}