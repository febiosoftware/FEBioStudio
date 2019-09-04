#pragma once

#include <string>

enum launchTypes{LOCAL=0, REMOTE, PBS, SLURM, CUSTOM};

class CLaunchConfig
{

public:
	CLaunchConfig(){}
	~CLaunchConfig(){}

	CLaunchConfig(const CLaunchConfig &old)
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

	int type = 0;
	std::string name;
	std::string path;
	std::string server;
	int port = 0;
	std::string userName;
	std::string remoteDir;
	std::string jobName;
	std::string walltime;
	int procNum = 0;
	int ram = 0;

};
