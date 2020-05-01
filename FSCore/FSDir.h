#pragma once
#include <string>
#include <map>

// This class helps in resolving file paths and converting between 
// relative (to project dir) and absolute.
// It also manages some macros that can be used to reference files in a path independent manner
class FSDir
{
public:
	FSDir(const std::string& path);

	std::string expandMacros(bool expandSymbolicLinks = true);
	std::string makeRelative(const std::string& path);
	std::string fileBase();
	std::string fileName();
	std::string fileDir();
	std::string fileExt();

	static std::string expandMacros(const std::string& file, bool expandSymbolicLinks = true);
	static std::string makeRelative(const std::string& file, const std::string& path);
	static std::string fileBase(const std::string& file);
	static std::string fileName(const std::string& file);
	static std::string fileDir(const std::string& file);
	static std::string fileExt(const std::string& file);
	static std::string filePath(const std::string& file);

	static void setMacro(const std::string& def, const std::string& val);

private:
	static std::string polish(const std::string& s);

private:
	std::string	m_path;
	static std::map<std::string, std::string>	m_defs;
	static char m_separator;
};
