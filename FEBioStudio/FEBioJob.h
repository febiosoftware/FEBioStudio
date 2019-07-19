#pragma once
#include <PreViewLib/FEObject.h>

class CPostDoc;


//-----------------------------------------------------------------------------
class CFEBioJob : public FEObject
{
public:
	enum JOB_STATUS {
		NONE,
		COMPLETED,
		FAILED,
		CANCELLED,
		RUNNING
	};

public:
	CFEBioJob();
	CFEBioJob(const std::string& fileName, JOB_STATUS status);

	void SetStatus(JOB_STATUS status);
	int GetStatus();

	void SetFileName(const std::string& fileName);
	std::string GetFileName();

	bool OpenPlotFile(const std::string& fileName);

	bool HasPostDoc();

	CPostDoc* GetPostDoc();

private:
	std::string		m_fileName;	// the .feb file name
	int				m_status;	// return status

	CPostDoc*	m_post;

	static int	m_count;
};

