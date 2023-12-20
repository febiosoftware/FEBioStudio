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

#include "stdafx.h"
#include "FSDir.h"
#include <QtCore/QFileInfo>
//using namespace std;
map<string, string>	FSDir::m_defs;

#ifdef WIN32
char FSDir::m_separator = '\\';
#else
char FSDir::m_separator = '/';
#endif

string FSDir::polish(const string& path)
{
	string s(path);
	size_t l = s.length();
	for (int i=0; i<l; ++i)
	{
		char ch = s[i];
		if ((ch == '\\') || (ch == '/'))
		{
			if (ch != m_separator) s[i] = m_separator;
		}
	}
	return s;
}

// replaces in s all occurrences of sub with rep
void replace(string& s, const string& sub, const string& rep)
{
	size_t n, l = sub.size();
	while ((n = s.find(sub)) != string::npos)
	{
		s.replace(n, l, rep);
	}
}

FSDir::FSDir(const string& path) : m_path(path)
{
	m_path = polish(path);
}

void FSDir::setMacro(const string& def, const string& val)
{
	string v = polish(val);
	m_defs[def] = v;
}

std::string FSDir::expandMacros(bool expandSymbolicLinks) { return expandMacros(m_path); }

std::string FSDir::expandMacros(const std::string& path, bool expandSymbolicLinks)
{
	string s = polish(path);

	// replace all the macros
	map<string, string>::iterator it;
	for (it = m_defs.begin(); it != m_defs.end(); ++it)
	{
		string def_i = "$(" + it->first + ")";
		replace(s, def_i, it->second);
	}

	// expand symbolic links
	if (expandSymbolicLinks)
	{
		// see if this is a symbolic link
		QFileInfo fileInfo(QString::fromStdString(s));
		if (fileInfo.isSymLink())
		{
			// it is, so get the target
			QString absPath = fileInfo.symLinkTarget();
			s = absPath.toStdString();
		}
	}

	return polish(s);
}

std::string FSDir::makeRelative(const std::string& path) { return makeRelative(m_path, path); }

std::string FSDir::makeRelative(const std::string& file, const std::string& path)
{
	string s = polish(file);

	// TODO: The goal is that this function figures out the relative path 
	// from path to the project folder, but for now we do a simple substitution
	string dir = m_defs[path];
	if (dir.empty() == false)
	{
		size_t n = s.find(dir);
		if (n != std::string::npos) s.replace(n, dir.size(), path);
	}

	return s;
}

// return just the base of the file (no dir, no ext)
std::string FSDir::fileBase() { return fileBase(m_path); }

std::string FSDir::fileBase(const std::string& path)
{
	string s = polish(path);
	
	// strip the file path off
	size_t n = s.rfind(m_separator);
	if (n != string::npos)
	{
		s.erase(0, n + 1);
	}

	// strip the extension
	n = s.rfind('.');
	if (n != string::npos)
	{
		s.erase(n, string::npos);
	}

	return s;
}

std::string FSDir::fileExt() { return fileExt(m_path); }

std::string FSDir::fileExt(const std::string& path)
{
	string s = polish(path);

	// strip the file path off
	size_t n = s.rfind(m_separator);
	if (n != string::npos)
	{
		s.erase(0, n + 1);
	}

	// strip the extension
	n = s.rfind('.');
	if (n != string::npos)
	{
		s.erase(0, n+1);
		return s;
	}
	else return "";
}

std::string FSDir::fileName() { return fileName(m_path); }

std::string FSDir::fileName(const std::string& path)
{
	string s = polish(path);

	size_t n = s.rfind(m_separator);
	if (n != string::npos)
	{
		return s.substr(n + 1);
	}
	else return s;
}


std::string FSDir::fileDir() { return fileDir(m_path); }

std::string FSDir::fileDir(const std::string& path)
{
	string s = polish(path);

	size_t n = s.rfind(m_separator);
	if (n != string::npos)
	{
		s.erase(n);
	}
	return s;
}

std::string FSDir::filePath(const std::string& file)
{
	string s = polish(file);
	return s;
}
