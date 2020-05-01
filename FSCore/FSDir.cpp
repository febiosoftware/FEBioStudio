#include "stdafx.h"
#include "FSDir.h"
#include <QtCore/QFileInfo>
using namespace std;
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
	polish(m_path);
}

void FSDir::setMacro(const string& def, const string& val)
{
	string v(val);
	polish(v);
	m_defs[def] = val;
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

	polish(s);
	return s;
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
