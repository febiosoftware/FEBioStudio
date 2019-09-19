#include "LaunchConfig.h"


CLaunchConfig::CLaunchConfig(const CLaunchConfig &old)
{
	type = old.type;
	name = old.name;
	path = old.path;
	server = old.server;
	port = old.port;
	userName = old.userName;
	remoteDir = old.remoteDir;
	jobName = old.jobName;
	walltime = old.walltime;
	procNum = old.procNum;
	ram = old.ram;
}

void CLaunchConfig::operator=(const CLaunchConfig &old)
{
	type = old.type;
	name = old.name;
	path = old.path;
	server = old.server;
	port = old.port;
	userName = old.userName;
	remoteDir = old.remoteDir;
	jobName = old.jobName;
	walltime = old.walltime;
	procNum = old.procNum;
	ram = old.ram;
}

bool CLaunchConfig::operator!=(const CLaunchConfig &b)
{
	return !operator==(b);
}

bool CLaunchConfig::operator==(const CLaunchConfig &b)
{
	if(type != b.type) return false;
	if(name.compare(b.name) != 0) return false;
	if(path.compare(b.path) != 0) return false;
	if(server.compare(b.server) != 0) return false;
	if(port != b.port) return false;
	if(userName.compare(b.userName) != 0) return false;
	if(remoteDir.compare(b.remoteDir) != 0) return false;
	if(jobName.compare(b.jobName) != 0) return false;
	if(walltime.compare(b.walltime) != 0) return false;
	if(ram != b.ram) return false;

	return true;
}


bool CLaunchConfig::SameServer(const CLaunchConfig &b)
{
	if(server.compare(b.server) != 0) return false;
	if(port != b.port) return false;
	if(userName.compare(b.userName) != 0) return false;

	return true;
}

void CLaunchConfig::Save(OArchive& ar)
{
	ar.WriteChunk(CID_LCONFIG_TYPE, type);
	ar.WriteChunk(CID_LCONFIG_PATH, path);
	ar.WriteChunk(CID_LCONFIG_SERVER, server);
	ar.WriteChunk(CID_LCONFIG_PORT, port);
	ar.WriteChunk(CID_LCONFIG_USERNAME, userName);
	ar.WriteChunk(CID_LCONFIG_REMOTEDIR, remoteDir);
	ar.WriteChunk(CID_LCONFIG_JOBNAME, jobName);
	ar.WriteChunk(CID_LCONFIG_WALLTIME, walltime);
	ar.WriteChunk(CID_LCONFIG_PROCNUM, procNum);
	ar.WriteChunk(CID_LCONFIG_RAM, ram);
}

void CLaunchConfig::Load(IArchive& ar)
{
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();
		switch(nid)
		{
		case CID_LCONFIG_TYPE: ar.read(type); break;
		case CID_LCONFIG_PATH: ar.read(path); break;
		case CID_LCONFIG_SERVER: ar.read(server); break;
		case CID_LCONFIG_PORT: ar.read(port); break;
		case CID_LCONFIG_USERNAME: ar.read(userName); break;
		case CID_LCONFIG_REMOTEDIR: ar.read(remoteDir); break;
		case CID_LCONFIG_JOBNAME: ar.read(jobName); break;
		case CID_LCONFIG_WALLTIME: ar.read(walltime); break;
		case CID_LCONFIG_PROCNUM: ar.read(procNum); break;
		case CID_LCONFIG_RAM: ar.read(ram); break;
		}
		ar.CloseChunk();
	}
}








