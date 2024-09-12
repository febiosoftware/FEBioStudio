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

#pragma once
#include <FSCore/FSObject.h>
#include <PostLib/FEFileReader.h>
#include "LaunchConfig.h"
#include <FEBioLib/febiolib_types.h>

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

	void SetFEBFileName(const std::string& fileName);
	std::string GetFEBFileName(bool relative = false) const;

	void SetPlotFileName(const std::string& plotFile);
	std::string GetPlotFileName(bool relative = false) const;

	void SetLogFileName(const std::string& logFile);
	std::string GetLogFileName(bool relative = false) const;

	void SetConfigFileName(const std::string& configFile);
	std::string GetConfigFileName() const;

	void UpdateWorkingDirectory(const std::string& dir);

	CLaunchConfig* GetLaunchConfig();
	void UpdateLaunchConfig(CLaunchConfig launchConfig);

#ifdef HAS_SSH
	CSSHHandler* GetSSHHandler();
#endif

	void Load(IArchive& ar) override;
	void Save(OArchive& ar) override;

	CDocument* GetDocument();

	void StartTimer();
	void StopTimer();

public:
	void SetProgress(double pct);
	double GetProgress() const;
	bool HasProgress() const;
	void ClearProgress();

private:
	std::string		m_febFile;	// the .feb file name
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

	// progress management
	bool	m_bhasProgress;
	double	m_pct;

	// FEBio output
	std::string	m_jobReport;
	TimingInfo m_timingInfo;

	double	m_tic, m_toc;

public:
	CDocument*	m_doc;

	static CFEBioJob*	m_activeJob;
	static void SetActiveJob(CFEBioJob* activeJob);
	static CFEBioJob* GetActiveJob();

	static int	m_count;
};
