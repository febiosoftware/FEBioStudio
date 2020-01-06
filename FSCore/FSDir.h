#pragma once
#include <string>
#include <map>

// This singleton class helps in resolving file paths and converting between 
// relative (to project dir) and absolute.
// It also manages some macros that can be used to reference files in a path independent manner
class FSDir
{
public:
	FSDir(const std::string& path);

	std::string toAbsolutePath() const;

public:
	static void setMacro(const std::string& def, const std::string& val);
	static std::string toAbsolutePath(const std::string& path, bool expandSymbolicLinks = true);
	static std::string toRelativePath(const std::string& path);
	static std::string fileBase(const std::string& path);
	static std::string fileName(const std::string& path);
	static std::string fileDir(const std::string& path);
	static std::string filePath(const std::string& path);

private:
	std::string	m_path;
	static std::map<std::string, std::string>	m_defs;
};
