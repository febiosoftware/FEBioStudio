#pragma once
#include <FSCore/FSObject.h>

class CPostDoc;
class CDocument;

//-----------------------------------------------------------------------------
class CFEBioJob : public FSObject
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
	CFEBioJob(CDocument* doc);
	~CFEBioJob();
	CFEBioJob(CDocument* doc, const std::string& jobName, const std::string& workingDirectory);

	void SetStatus(JOB_STATUS status);
	int GetStatus();

	void SetFileName(const std::string& fileName);
	std::string GetFileName() const;

	void SetPlotFileName(const std::string& plotFile);
	std::string GetPlotFileName() const;

	void UpdateWorkingDirectory(const std::string& dir);

	bool OpenPlotFile();

	bool HasPostDoc();

	CPostDoc* GetPostDoc();

	void Load(IArchive& ar) override;
	void Save(OArchive& ar) override;

protected:
	size_t RemoveChild(FSObject* po) override;

private:
	std::string		m_fileName;	// the .feb file name
	std::string		m_plotFile;	// the .xplt file name
	int				m_status;	// return status

	CDocument*	m_doc;
	CPostDoc*	m_postDoc;

	static int	m_count;
};
