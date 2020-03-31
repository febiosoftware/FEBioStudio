#pragma once
#include <FSCore/FSObject.h>
#include <PostLib/FEFileReader.h>
#include "LaunchConfig.h"

class CPostDoc;
class CDocument;
#ifdef HAS_SSH
class CSSHHandler;
#endif
class xpltFileReader;

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
	CFEBioJob(CDocument* doc, const std::string& jobName, const std::string& workingDirectory, CLaunchConfig launchConfig);

	void SetStatus(JOB_STATUS status);
	int GetStatus();

	void SetFileName(const std::string& fileName);
	std::string GetFileName() const;

	void SetPlotFileName(const std::string& plotFile);
	std::string GetPlotFileName() const;

	void SetConfigFileName(const std::string& configFile);
	std::string GetConfigFileName() const;

	void UpdateWorkingDirectory(const std::string& dir);

	CLaunchConfig* GetLaunchConfig();
	void UpdateLaunchConfig(CLaunchConfig launchConfig);

#ifdef HAS_SSH
	CSSHHandler* GetSSHHandler();
#endif

	bool OpenPlotFile(xpltFileReader* xplt);

	// load a project from file
	bool LoadFEModel(Post::FEFileReader* preader, const char* szfile);

	bool HasPostDoc();

	CPostDoc* GetPostDoc();

	void Load(IArchive& ar) override;
	void Save(OArchive& ar) override;

protected:
	size_t RemoveChild(FSObject* po) override;

private:
	std::string		m_fileName;	// the .feb file name
	std::string		m_plotFile;	// the .xplt file name
	std::string		m_logFile;	// the .log file name
	std::string		m_cnfFile;	// the config file
	int				m_status;	// return status

	CLaunchConfig 	m_launchConfig;
#ifdef HAS_SSH
	CSSHHandler*	m_sshHandler;
	CSSHHandler* 	NewHandler();
#endif

public:
	// additional run settings (TODO: Is this a good place, or should this go in the launch config?)
	int			m_febVersion;	// the .feb file version
	bool		m_writeNotes;	// write notes to .feb file
	std::string	m_cmd;			// command line options

public:
	CDocument*	m_doc;
	CPostDoc*	m_postDoc;

	static int	m_count;
};
