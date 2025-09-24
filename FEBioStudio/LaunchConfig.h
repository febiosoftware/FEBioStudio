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

#include <string>
#include <FSCore/FSObject.h>

class CFEBioJob;

class CLaunchConfig : public FSObject
{
public:
	enum LaunchType {
		LOCAL = 0,
		REMOTE,
		PBS,
		SLURM,
		CUSTOM,
		DEFAULT
	};

protected:
	CLaunchConfig();
	CLaunchConfig(LaunchType launchType, const std::string& configName);
	~CLaunchConfig();

public:
	std::string path() const;
	std::string server() const;
	std::string userName() const;
	std::string remoteDir() const;
	std::string text() const;
	int port() const;

private:
	CLaunchConfig(const CLaunchConfig& old) = delete;
	void operator=(const CLaunchConfig& old) = delete;

public:
	int type() const { return m_type; }
	std::string typeString() const;

	void setName(const std::string& name) { m_name = name; }
	std::string name() const { return m_name; }

private:
	int m_type = 0;
	std::string m_name;
};

class CLocalLaunchConfig : public CLaunchConfig
{
public:
	CLocalLaunchConfig(const std::string& configname);
};

class CRemoteLaunchConfig : public CLaunchConfig
{
public:
	CRemoteLaunchConfig(const std::string& configname);
};

class CPBSLaunchConfig : public CLaunchConfig
{
public:
	CPBSLaunchConfig(const std::string& configname);
};

class CSLURMLaunchConfig : public CLaunchConfig
{
public:
	CSLURMLaunchConfig(const std::string& configname);
};

class CCustomLaunchConfig : public CLaunchConfig
{
public:
	CCustomLaunchConfig(const std::string& configname);
};

class CDefaultLaunchConfig : public CLaunchConfig
{
public:
	CDefaultLaunchConfig(const std::string& configname);
};
