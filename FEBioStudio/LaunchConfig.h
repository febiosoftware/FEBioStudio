/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include <string>
#include <FSCore/FSObject.h>

enum launchTypes{LOCAL=0, REMOTE, PBS, SLURM, CUSTOM, DEFAULT};

class CLaunchConfig : public FSObject
{

public:
	CLaunchConfig();
	CLaunchConfig(launchTypes launchType, const std::string& configName);

	~CLaunchConfig();
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
