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
#include <stdio.h>
#include <string>

// Helper class for processing LSDyna keyword files
class LSDynaFile
{
public:
	class CARD
	{
	public:
		CARD(int field = 10);

		bool nexti(int& n, int nwidth = -1);		// return the next integer parameter
		bool nextd(double& d, int nwidth = -1);		// return the next double parameter
		bool nextf(float& d, int nwidth = -1);		// return the next float parameter
		bool nexts(char* s, int maxLength);

		const char* szvalue() { return m_szline; }

		bool IsKeyword() { return m_szline[0] == '*'; }

		bool operator == (const char* sz);

		bool contains(const char* sz);

	public:
		enum { MAX_LINE = 256 };		// max characters per line
		char	m_szline[MAX_LINE];		// line read in from file
		bool	m_bfree;				// free format flag
		char* m_ch;					// current position in line
		int		m_nfield;				// field width
		int		m_l;					// length of line
	};

public:
	LSDynaFile();

	bool Open(const char* szfile);
	bool Open(FILE* fp);

	void Close();

	size_t CurrentLineNumber() const { return m_lineno; }

	// get the card at the current line
	void GetCard(LSDynaFile::CARD& c);

	// move to next line, and read card
	bool NextCard(LSDynaFile::CARD& c);

	std::string FileName() const { return m_fileName; }

	void SetFileName(const std::string& fileName) { m_fileName = fileName; }

private:
	char* get_line(char* szline);

private:
	std::string	m_fileName;
	FILE* m_fp;
	bool	m_bmyfp;
	char   m_szline[256];
	size_t m_lineno;
};
