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

#include "LaunchConfig.h"

CLaunchConfig::CLaunchConfig() {}
CLaunchConfig::~CLaunchConfig() {}

CLaunchConfig::CLaunchConfig(launchTypes launchType, const std::string& configName)
{
	type = launchType;
	name = configName;
}

CLaunchConfig::CLaunchConfig(const CLaunchConfig &old)
{
	type = old.type;
	name = old.name;
	path = old.path;
	server = old.server;
	port = old.port;
	userName = old.userName;
	remoteDir = old.remoteDir;
//	jobName = old.jobName;
//	walltime = old.walltime;
//	procNum = old.procNum;
	customFile = old.customFile;
//	ram = old.ram;
	text = old.text;
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
//	jobName = old.jobName;
//	walltime = old.walltime;
//	procNum = old.procNum;
	customFile = old.customFile;
//	ram = old.ram;
	text = old.text;
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
//	if(jobName.compare(b.jobName) != 0) return false;
//	if(walltime.compare(b.walltime) != 0) return false;
	if(customFile.compare(b.customFile) != 0) return false;
//	if(ram != b.ram) return false;
	if(text.compare(b.text) != 0) return false;

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
//	ar.WriteChunk(CID_LCONFIG_JOBNAME, jobName);
//	ar.WriteChunk(CID_LCONFIG_WALLTIME, walltime);
//	ar.WriteChunk(CID_LCONFIG_PROCNUM, procNum);
//	ar.WriteChunk(CID_LCONFIG_RAM, ram);
	ar.WriteChunk(CID_LCONFIG_CUSTOMEFILE, customFile);
	ar.WriteChunk(CID_LCONFIG_TEXT, getText());
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
//		case CID_LCONFIG_JOBNAME: ar.read(jobName); break;
//		case CID_LCONFIG_WALLTIME: ar.read(walltime); break;
//		case CID_LCONFIG_PROCNUM: ar.read(procNum); break;
//		case CID_LCONFIG_RAM: ar.read(ram); break;
		case CID_LCONFIG_CUSTOMEFILE: ar.read(customFile); break;
		case CID_LCONFIG_TEXT: ar.read(text); break;
		}
		ar.CloseChunk();
	}
}

const std::string& CLaunchConfig::getCustomFile() const {
	return customFile;
}

void CLaunchConfig::setCustomFile(const std::string &customFile) {
	this->customFile = customFile;
}

const std::string& CLaunchConfig::getName() const {
	return name;
}

void CLaunchConfig::setName(const std::string &name) {
	this->name = name;
}

const std::string& CLaunchConfig::getPath() const {
	return path;
}

void CLaunchConfig::setPath(const std::string &path) {
	this->path = path;
}

int CLaunchConfig::getPort() const {
	return port;
}

void CLaunchConfig::setPort(int port) {
	this->port = port;
}

const std::string& CLaunchConfig::getRemoteDir() const {
	return remoteDir;
}

void CLaunchConfig::setRemoteDir(const std::string &remoteDir) {
	this->remoteDir = remoteDir;
}

const std::string& CLaunchConfig::getServer() const {
	return server;
}

void CLaunchConfig::setServer(const std::string &server) {
	this->server = server;
}

int CLaunchConfig::getType() const {
	return type;
}

void CLaunchConfig::setType(int type) {
	this->type = type;
}

const std::string& CLaunchConfig::getUserName() const {
	return userName;
}

void CLaunchConfig::setUserName(const std::string &userName) {
	this->userName = userName;
}

//const std::string& CLaunchConfig::getCustomText() const {
//	return CustomText;
//}
//
//void CLaunchConfig::setCustomText(const std::string &customText) {
//	CustomText = customText;
//}
//
//const std::string& CLaunchConfig::getPbsText() const {
//
//	if(PBSText.compare("${DEFAULT}") == 0)
//	{
//		return DefaultPBSText;
//	}
//
//	return PBSText;
//}
//
//void CLaunchConfig::setPbsText(const std::string &pbsText) {
//	PBSText = pbsText;
//}
//
//const std::string& CLaunchConfig::getSlurmText() const {
//	if(PBSText.compare("${DEFAULT}") == 0)
//	{
//		return DefaultPBSText;
//	}
//
//	return SlurmText;
//}
//
//void CLaunchConfig::setSlurmText(const std::string &slurmText) {
//	SlurmText = slurmText;
//}

void CLaunchConfig::setText(const std::string &text) {
	this->text = text;
}

const std::string& CLaunchConfig::getText() const {

//	switch(type)
//	{
//	case PBS:
//		return PBSText;
//	case SLURM:
//		return SlurmText;
//	case CUSTOM:
//		return CustomText;
//	}
//
//	return "";
	return text;
}
