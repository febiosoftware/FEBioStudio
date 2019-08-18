#pragma once
#include <MeshTools/FEObject.h>

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
	~CFEBioJob();
	CFEBioJob(const std::string& fileName, JOB_STATUS status);

	void SetStatus(JOB_STATUS status);
	int GetStatus();

	void SetFileName(const std::string& fileName);
	std::string GetFileName() const;

	void SetPlotFileName(const std::string& plotFile);
	std::string GetPlotFileName() const;

	bool OpenPlotFile();

	bool HasPostDoc();

	CPostDoc* GetPostDoc();

private:
	std::string		m_fileName;	// the .feb file name
	std::string		m_plotFile;	// the .xplt file name
	int				m_status;	// return status

	CPostDoc*	m_postDoc;

	static int	m_count;
};

