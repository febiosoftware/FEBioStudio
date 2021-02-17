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
#include <map>

using std::map;
using std::string;

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
