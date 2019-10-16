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

	const std::string& getCustomFile() const;
	void setCustomFile(const std::string &customFile);
	const std::string& getName() const;
	void setName(const std::string &name);
	const std::string& getPath() const;
	void setPath(const std::string &path);
	int getPort() const;
	void setPort(int port = 22);
	const std::string& getRemoteDir() const;
	void setRemoteDir(const std::string &remoteDir);
	const std::string& getServer() const;
	void setServer(const std::string &server);
	int getType() const;
	void setType(int type = 0);
	const std::string& getUserName() const;
	void setUserName(const std::string &userName);
	void setText(const std::string &text);
	const std::string& getText() const;

	int type = 0;
	std::string name;
	std::string path;
	std::string server;
	int port = 22;
	std::string userName;
	std::string remoteDir;
//	std::string jobName;
//	std::string walltime;
	std::string customFile;
//	int procNum = 0;
//	int ram = 0;

private:
	std::string text;

};
