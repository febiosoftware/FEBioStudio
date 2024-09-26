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

#include "LaunchConfig.h"
#ifdef HAS_SSH
#include "SSHHandler.h"
#endif

CLaunchConfig::CLaunchConfig() {}
CLaunchConfig::~CLaunchConfig() {}

CLaunchConfig::CLaunchConfig(CLaunchConfig::LaunchType launchType, const std::string& configName)
{
	m_type = launchType;
	m_name = configName;
}

std::string CLaunchConfig::typeString() const
{
	std::string typestr;
	switch (type())
	{
	case LOCAL  : typestr = "local"; break;
	case REMOTE : typestr = "remote"; break;
	case PBS    : typestr = "PBS"; break;
	case SLURM  : typestr = "SLURM"; break;
	case CUSTOM : typestr = "custom"; break;
	case DEFAULT: typestr = "default"; break;
	default:
		assert(false);
	}
	return typestr;
}

void CLaunchConfig::GetRemoteFiles(CFEBioJob* job)
{
#ifdef HAS_SSH
/*
	CSSHHandler* ssh = new CSSHHandler(job);
	QObject::connect(ssh, &CSSHHandler::sessionFinished, ssh, &QObject::deleteLater);
	ssh->RequestRemoteFiles();
*/
#endif
}

void CLaunchConfig::GetQueueStatus(CFEBioJob* job)
{
#ifdef HAS_SSH
/*
	CSSHHandler* ssh = new CSSHHandler(job);
	QObject::connect(ssh, &CSSHHandler::sessionFinished, ssh, &QObject::deleteLater);
	ssh->RequestQueueStatus();
*/
#endif
}

std::string CLaunchConfig::path() const
{
	const Param* p = GetParam("path"); assert(p);
	if (p && (p->GetParamType() == Param_URL)) return p->GetURLValue();
	if (p && (p->GetParamType() == Param_STRING)) return p->GetStringValue();
	return "";
}

std::string CLaunchConfig::server() const
{
	const Param* p = GetParam("server"); assert(p);
	if (p) return p->GetStringValue(); else return "";
}

int CLaunchConfig::port() const
{
	const Param* p = GetParam("port"); assert(p);
	if (p) return p->GetIntValue(); else return -1;
}

std::string CLaunchConfig::userName() const
{
	const Param* p = GetParam("userName"); assert(p);
	if (p) return p->GetStringValue(); else return "";
}

std::string CLaunchConfig::remoteDir() const
{
	const Param* p = GetParam("remoteDir"); assert(p);
	if (p) return p->GetStringValue(); else return "";
}

std::string CLaunchConfig::text() const
{
	const Param* p = GetParam("text"); assert(p);
	if (p) return p->GetStringValue(); else return "";
}

CLocalLaunchConfig::CLocalLaunchConfig(const std::string& configname) : CLaunchConfig(LOCAL, configname) 
{
	AddURLParam("", "path", "FEBio executable");
}

CRemoteLaunchConfig::CRemoteLaunchConfig(const std::string& configname) : CLaunchConfig(REMOTE, configname) 
{
	AddStringParam("", "path", "Remote executable");
	AddStringParam("", "server", "Server");
	AddIntParam(22, "port", "Port")->SetIntRange(0, 65535);
	AddStringParam("", "userName", "Username");
	AddStringParam("", "remoteDir", "Remote Directory");
}

// PBS config widgets
static char defaultPBSText[] = "#!/bin/bash\n\n"
"#PBS -l nodes=1:ppn=1\n"
"#PBS -l walltime=1:00:00\n"
"#PBS -N ${JOB_NAME}\n"
"#PBS -o ${REMOTE_DIR}/${JOB_NAME}_stdout.log\n"
"#PBS -e ${REMOTE_DIR}/${JOB_NAME}_stderr.log\n\n"
"${FEBIO_PATH} ${REMOTE_DIR}/${JOB_NAME}.feb";

CPBSLaunchConfig::CPBSLaunchConfig(const std::string& configname) : CLaunchConfig(PBS, configname) 
{
	AddStringParam("", "path", "Remote executable");
	AddStringParam("", "server", "Server");
	AddIntParam(22, "port", "Port")->SetIntRange(0, 65535);
	AddStringParam("", "userName", "Username");
	AddStringParam("", "remoteDir", "Remote Directory");
	AddStringParam(defaultPBSText, "text", "text")->SetVisible(false);
}

// Slurm config widgets
static char defaultSlurmText[] = "#!/bin/bash\n\n"
"#SBATCH -N 1\n"
"#SBATCH -n 1\n"
"#SBATCH -t 1:00:00\n"
"#SBATCH -J ${JOB_NAME}\n"
"#SBATCH -o ${REMOTE_DIR}/${JOB_NAME}_stdout.log\n"
"#SBATCH -e ${REMOTE_DIR}/${JOB_NAME}_stderr.log\n\n"
"${FEBIO_PATH} ${REMOTE_DIR}/${JOB_NAME}.feb";

CSLURMLaunchConfig::CSLURMLaunchConfig(const std::string& configname) : CLaunchConfig(SLURM, configname)
{
	AddStringParam("", "path", "Remote executable");
	AddStringParam("", "server", "Server");
	AddIntParam(22, "port", "Port")->SetIntRange(0, 65535);
	AddStringParam("", "userName", "Username");
	AddStringParam("", "remoteDir", "Remote Directory");
	AddStringParam(defaultSlurmText, "text", "text")->SetVisible(false);
}

// Custom config widgets
static char defaultCustomText[] = "#Use this field to create a custom script.\n"
"#Lines starting with '#' are comments and will be ignored.\n"
"#Empty Lines will be ignored.\n"
"#Each line will be run as a separate command.\n\n"
"#You can use the following macros:\n"
"#\t${REMOTE_DIR}: the remote directory that you specify.\n"
"#\t${JOB_NAME}: your current job's name.\n";

CCustomLaunchConfig::CCustomLaunchConfig(const std::string& configname) : CLaunchConfig(CUSTOM, configname) 
{
	AddStringParam("", "server", "Server");
	AddIntParam(22, "port", "Port")->SetIntRange(0, 65535);
	AddStringParam("", "userName", "Username");
	AddStringParam("", "remoteDir", "Remote Directory");
	AddStringParam(defaultCustomText, "text", "text")->SetVisible(false);
}

CDefaultLaunchConfig::CDefaultLaunchConfig(const std::string& configname) : CLaunchConfig(DEFAULT, configname) 
{
}
