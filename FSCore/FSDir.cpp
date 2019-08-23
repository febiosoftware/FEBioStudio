#include "stdafx.h"
#include "FSDir.h"
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

std::string FSDir::toAbsolutePath(const std::string& path)
{
	string s(path);
#ifdef WIN32
	windowsify(s);
#endif

	// replace all the macros
	map<string, string>::iterator it;
	for (it = m_defs.begin(); it != m_defs.end(); ++it)
	{
		string def_i = "$(" + it->first + ")";
		replace(s, def_i, it->second);
	}
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
