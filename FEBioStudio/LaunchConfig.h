#pragma once

#include <string>
#include <FSCore/FSObject.h>

enum launchTypes{LOCAL=0, REMOTE, PBS, SLURM, CUSTOM};

class CLaunchConfig : public FSObject
{

public:
	CLaunchConfig(){}
	~CLaunchConfig(){}
	CLaunchConfig(const CLaunchConfig &old);
	void operator=(const CLaunchConfig &old);

	bool operator!=(const CLaunchConfig &b);
	bool operator==(const CLaunchConfig &b);

	bool SameServer(const CLaunchConfig &b);

	void Load(IArchive& ar) override;
	void Save(OArchive& ar) override;

	int type = 0;
	std::string name;
	std::string path;
	std::string server;
	int port = 22;
	std::string userName;
	std::string remoteDir;
	std::string jobName;
	std::string walltime;
	std::string customFile;
	int procNum = 0;
	int ram = 0;

};
