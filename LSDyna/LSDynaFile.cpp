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
#include "LSDynaFile.h"
#include <FSCore/FileReader.h>
#include <assert.h>

//-----------------------------------------------------------------------------
LSDynaFile::CARD::CARD(int field)
{
	m_szline[0] = 0;
	m_szkey[0] = 0;
	m_ch = 0;
	m_bfree = false;
	m_nfield = field;
}

bool LSDynaFile::CARD::nextd(double& d, int nwidth)
{
	d = 0;
	if (m_ch == 0) return false;

	if (nwidth == -1) nwidth = m_nfield;

	if (m_bfree)
	{
		char* ch = strchr(m_ch, ',');
		if (ch) *ch = 0;
		d = atof(m_ch);
		if (ch) m_ch = ch + 1; else m_ch = 0;
	}
	else
	{
		char sz[256] = { 0 };
		strncpy(sz, m_ch, nwidth);
		d = atof(sz);
		m_ch += nwidth;
		if ((*m_ch == 0) || (m_ch - m_szline >= m_l)) m_ch = 0;
	}

	return true;
}

bool LSDynaFile::CARD::nextf(float& d, int nwidth)
{
	d = 0.f;
	if (m_ch == 0) return false;

	if (nwidth == -1) nwidth = m_nfield;

	if (m_bfree)
	{
		char* ch = strchr(m_ch, ',');
		if (ch) *ch = 0;
		d = (float)atof(m_ch);
		if (ch) m_ch = ch + 1; else m_ch = 0;
	}
	else
	{
		char sz[256] = { 0 };
		strncpy(sz, m_ch, nwidth);
		d = (float) atof(sz);
		m_ch += nwidth;
		if ((*m_ch == 0) || (m_ch - m_szline >= m_l)) m_ch = 0;
	}

	return true;
}


bool LSDynaFile::CARD::nexts(char* sz, int maxLength)
{
	assert(m_bfree == false);
	if (m_ch == 0) return false;

	int nwidth = (m_nfield > maxLength ? maxLength : m_nfield);
	strncpy(sz, m_ch, nwidth);
	sz[nwidth] = 0;
	m_ch += m_nfield;
	if ((*m_ch == 0) || (m_ch - m_szline >= m_l)) m_ch = 0;

	return true;
}

bool LSDynaFile::CARD::nexti(int& n, int nwidth)
{
	n = 0;
	if (m_ch == 0) return false;

	if (nwidth == -1) nwidth = m_nfield;

	if (m_bfree)
	{
		char* ch = strchr(m_ch, ',');
		if (ch) *ch = 0;
		n = atoi(m_ch);
		if (ch) m_ch = ch + 1; else m_ch = 0;
	}
	else
	{
		char sz[256] = { 0 };
		strncpy(sz, m_ch, nwidth);
		n = atoi(sz);
		m_ch += nwidth;
		if ((*m_ch == 0) || (m_ch - m_szline >= m_l)) m_ch = 0;
	}

	return true;
}

bool LSDynaFile::CARD::operator == (const char* sz)
{
	if (sz == nullptr) return false;
	if (m_szkey[0] == 0) return false; // only check keywords
	size_t l = strlen(sz);
	if (l == 0) return false;
	return (strncmp(m_szkey, sz, l) == 0);
}

bool LSDynaFile::CARD::contains(const char* sz)
{
	return strstr(m_szline, sz);
}

//===================================================================
LSDynaFile::LSDynaFile() : m_fp(nullptr)
{
	m_szline[0] = 0;
	m_lineno = 0;
	m_bmyfp = false;
}

bool LSDynaFile::Open(const char* szfile)
{
	if (m_fp) return false;
	m_fp = fopen(szfile, "rt");
	m_lineno = 0;
	m_bmyfp = true;

	m_fileName = szfile;

	// make sure the first line is *KEYWORD
	if (m_fp)
	{
		if (get_line(m_szline) == 0) return false;
		if (szcmp(m_szline, "*KEYWORD") != 0) return false;
	}

	return (m_fp != nullptr);
}

bool LSDynaFile::Open(FILE* fp)
{
	if (m_fp) return false;
	m_lineno = 0;

	m_bmyfp = false;
	m_fp = fp;

	// make sure the first line is *KEYWORD
	if (m_fp)
	{
		if (get_line(m_szline) == 0) return false;
		if (szcmp(m_szline, "*KEYWORD") != 0) return false;
	}

	return (m_fp != nullptr);
}

void LSDynaFile::Close()
{
	if (m_bmyfp) fclose(m_fp);
	m_fp = nullptr;
}

//---------------------------------------------------------------------------------------
char* LSDynaFile::get_line(char* szline)
{
	if (m_fp == nullptr) return nullptr;
	if (feof(m_fp)) return nullptr;

	do {
		if (fgets(szline, 255, m_fp) == nullptr) return nullptr;
		++m_lineno;
	} while (szline[0] == '$');

	// remove line endings
	char* ch = strrchr(szline, '\n'); if (ch) *ch = 0;
	ch = strrchr(szline, '\r'); if (ch) *ch = 0;

	return szline;
}

void LSDynaFile::GetCard(LSDynaFile::CARD& c)
{
	strcpy(c.m_szline, m_szline);

	// figure out which format is used
	if (strchr(c.m_szline, ',')) c.m_bfree = true; else c.m_bfree = false;

	// set the intitial pointer
	c.m_ch = c.m_szline;

	c.m_l = (int)strlen(c.m_szline);

	c.m_szkey[0] = 0;
	if (c.m_szline[0] == '*')
	{
		size_t l = 0;
		const char* ch = m_szline;
		while (ch && (*ch))
		{
			if (!isspace(*ch) && ((*ch) != ','))
				c.m_szkey[l++] = *ch;
			else
				break;
			ch++;
		}
		c.m_szkey[l] = 0;
	}
}

bool LSDynaFile::NextCard(LSDynaFile::CARD& c)
{
	// get the next line
	if (get_line(m_szline) == 0) return false;

	// process the line
	GetCard(c);

	return true;
}
