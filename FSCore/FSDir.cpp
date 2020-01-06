#include "stdafx.h"
#include "FSDir.h"
#include <QtCore/QFileInfo>
using namespace std;
map<string, string>	FSDir::m_defs;

void windowsify(string& s)
{
	size_t n;
	while ((n = s.find('/')) != string::npos)
	{
		s.replace(n, 1, "\\");
	}
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
#ifdef WIN32
	windowsify(m_path);
#endif
}

void FSDir::setMacro(const string& def, const string& val)
{
	string v(val);
#ifdef WIN32
	windowsify(v);
#endif
	m_defs[def] = val;
}

std::string FSDir::toAbsolutePath() const
{
	return toAbsolutePath(m_path);
}

std::string FSDir::toAbsolutePath(const std::string& path, bool expandSymbolicLinks)
{
	string s(path);

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

#ifdef WIN32
	windowsify(s);
#endif

	return s;
}

std::string FSDir::toRelativePath(const std::string& path)
{
	string s(path);
#ifdef WIN32
	windowsify(s);
#endif

	// TODO: The goal is that this function figures out the relative path 
	// from path to the project folder, but for now we do a simple substitution
	string projectDir = m_defs["ProjectDir"];
	if (projectDir.empty() == false)
	{
		size_t n = s.find(projectDir);
		if (n != std::string::npos) s.replace(n, projectDir.size(), "$(ProjectDir)");
	}

	return s;
}

// return just the base of the file (no dir, no ext)
std::string FSDir::fileBase(const std::string& path)
{
	string s(path);
#ifdef WIN32
	windowsify(s);
#endif

	// strip the file path off
	size_t n1 = s.rfind('\\');
	size_t n2 = s.rfind('/');

	size_t n = string::npos;
	if      ((n1 == string::npos) && (n2 != string::npos)) n = n2;
	else if ((n1 != string::npos) && (n2 == string::npos)) n = n1;
	else if ((n1 != string::npos) && (n2 != string::npos)) n = (n1 > n2 ? n1 : n2);
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

std::string FSDir::fileName(const std::string& path)
{
	string s(path);
#ifdef WIN32
	windowsify(s);
#endif

	size_t n1 = s.rfind('\\');
	size_t n2 = s.rfind('/');

	size_t n = string::npos;
	if ((n1 == string::npos) && (n2 != string::npos)) n = n2;
	else if ((n1 != string::npos) && (n2 == string::npos)) n = n1;
	else if ((n1 != string::npos) && (n2 != string::npos)) n = (n1 > n2 ? n1 : n2);

	if (n != string::npos)
	{
		return s.substr(n + 1);
	}
	else return s;
}

std::string FSDir::fileDir(const std::string& path)
{
	string s(path);
#ifdef WIN32
	windowsify(s);
#endif

	size_t n1 = s.rfind('\\');
	size_t n2 = s.rfind('/');

	size_t n = string::npos;
	if ((n1 == string::npos) && (n2 != string::npos)) n = n2;
	else if ((n1 != string::npos) && (n2 == string::npos)) n = n1;
	else if ((n1 != string::npos) && (n2 != string::npos)) n = (n1 > n2 ? n1 : n2);
	if (n != string::npos)
	{
		s.erase(n);
	}
	return s;
}

std::string FSDir::filePath(const std::string& path)
{
	string s(path);
#ifdef WIN32
	windowsify(s);
#endif

	return s;
}
