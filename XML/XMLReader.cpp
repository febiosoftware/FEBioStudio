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

// XMLReader.cpp: implementation of the XMLReader class.
//
//////////////////////////////////////////////////////////////////////

#include "XMLReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int XMLAtt::value(double* pf, int n)
{
	char* sz = m_szval;
	int nr = 0;
	for (int i = 0; i<n; ++i)
	{
		char* sze = strchr(sz, ',');

		pf[i] = atof(sz);
		nr++;

		if (sze) sz = sze + 1;
		else break;
	}
	return nr;
}

//////////////////////////////////////////////////////////////////////
// XMLTag
//////////////////////////////////////////////////////////////////////

XMLTag::XMLTag()
{
	m_preader = 0;
	m_bend = false;

	m_sztag[0] = 0;
	m_szval[0] = 0;
	m_nlevel = 0;

	m_natt = 0;
	int i;
	for (i=0; i<MAX_ATT; ++i)
	{
		m_att[i].m_sztag[0] = 0;
		m_att[i].m_szval[0] = 0;
	}

	for (i=0; i<MAX_LEVEL; ++i) m_szroot[i][0] = 0;
}

//////////////////////////////////////////////////////////////////////

int XMLTag::value(double* pf, int n)
{
	char* sz = m_szval;
	int nr = 0;
	for (int i=0; i<n; ++i)
	{
		char* sze = strchr(sz, ',');

		pf[i] = atof(sz);
		nr++;

		if (sze) sz = sze + 1;
		else break;
	}
	return nr;
}
 
//////////////////////////////////////////////////////////////////////

int XMLTag::value(float* pf, int n)
{
	char* sz = m_szval;
	int nr = 0;
	for (int i=0; i<n; ++i)
	{
		char* sze = strchr(sz, ',');

		pf[i] = (float) atof(sz);
		nr++;

		if (sze) sz = sze + 1;
		else break;
	}
	return nr;
}

//////////////////////////////////////////////////////////////////////

int XMLTag::value(int* pi, int n)
{
	char* sz = m_szval;
	int nr = 0;
	for (int i=0; i<n; ++i)
	{
		char* sze = strchr(sz, ',');

		pi[i] = atoi(sz);
		nr++;

		if (sze) sz = sze + 1;
		else break;
	}
	return nr;
}

//////////////////////////////////////////////////////////////////////

void XMLTag::value(vec3d& v)
{
	sscanf(m_szval, "%lg,%lg,%lg", &v.x, &v.y, &v.z);
}

void XMLTag::value(vec2i& v)
{
	sscanf(m_szval, "%d,%d", &v.x, &v.y);
}

void XMLTag::value(vec3f& v)
{
	sscanf(m_szval, "%g,%g,%g", &v.x, &v.y, &v.z);
}

void XMLTag::value(mat3d& m)
{
	double a[9] = { 0 };
	sscanf(m_szval, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", a, a+1, a+2,a+3,a+4,a+5,a+6,a+7,a+8);
	m = mat3d(a);
}

void XMLTag::value(GLColor& c)
{
	sscanf(m_szval, "%c,%c,%c", &c.r, &c.g, &c.b);
}

void XMLTag::value(std::string& s)
{
	s = m_szval;
}

//-----------------------------------------------------------------------------
void XMLTag::value(vector<int>& l)
{
	int i, n = 0, n0, n1, nn;
	char* szval = strdup(m_szval);
	char* ch;
	char* sz = szval;
	int nread;
	do
	{
		ch = strchr(sz, ',');
		if (ch) *ch = 0;
		nread = sscanf(sz, "%d:%d:%d", &n0, &n1, &nn);
		switch (nread)
		{
		case 1:
			n1 = n0;
			nn = 1;
			break;
		case 2:
			nn = 1;
			break;
		case 3:
			break;
		default:
			n0 = 0;
			n1 = -1;
			nn = 1;
		}

		for (i=n0; i<=n1; i += nn) ++n;

		if (ch) *ch = ',';
		sz = ch+1;
	}
	while (ch != 0);

	if (n != 0)
	{
		l.resize(n);

		sz = szval;
		n = 0;
		do
		{
			ch = strchr(sz, ',');
			if (ch) *ch = 0;
			nread = sscanf(sz, "%d:%d:%d", &n0, &n1, &nn);
			switch (nread)
			{
			case 1:
				n1 = n0;
				nn = 1;
				break;
			case 2:
				nn = 1;
			}

			for (i=n0; i<=n1; i += nn) l[n++] = i;
			assert(n <= (int) l.size());

			if (ch) *ch = ',';
			sz = ch+1;
		}
		while (ch != 0);
	}

	free(szval);
}

//////////////////////////////////////////////////////////////////////

const char* XMLTag::AttributeValue(const char* szat, bool bopt)
{
	// find the attribute
	for (int i=0; i<m_natt; ++i)
		if (strcmp(m_att[i].m_sztag, szat) == 0) return m_att[i].m_szval;

	// If the attribute was not optional, we throw a fit
	if (!bopt) throw XMLReader::MissingAttribute(*this, szat);

	// we didn't find it
	return 0;
}

//////////////////////////////////////////////////////////////////////

XMLAtt* XMLTag::AttributePtr(const char* szatt)
{
	// find the attribute
	for (int i=0; i<m_natt; ++i)
		if (strcmp(m_att[i].m_sztag, szatt) == 0) return &(m_att[i]);

	// return 0 if we did not find it
	return 0;
}

//////////////////////////////////////////////////////////////////////

XMLAtt& XMLTag::Attribute(const char* szatt)
{
	// find the attribute
	for (int i=0; i<m_natt; ++i)
		if (strcmp(m_att[i].m_sztag, szatt) == 0) return m_att[i];

	// throw a fit if we did not find it
	throw XMLReader::MissingAttribute(*this, szatt);
}

//-----------------------------------------------------------------------------
//! Return the number of children of a tag
int XMLTag::children()
{
	XMLTag tag(*this); ++tag;
	int ncount = 0;
	while (!tag.isend()) { ncount++; m_preader->SkipTag(tag); ++tag; }
	return ncount;
}

//////////////////////////////////////////////////////////////////////
// XMLReader
//////////////////////////////////////////////////////////////////////

XMLReader::XMLReader()
{
	m_fp = 0;
	m_ownFile = false;
	m_nline = 0;
	m_bufIndex = 0;
	m_bufSize = 0;
	m_eof = false;
	m_currentPos = 0;
}

XMLReader::~XMLReader()
{
	Close();
}

void XMLReader::Close()
{
	if (m_ownFile && (m_fp != 0))
	{
		fclose(m_fp);
	}
	m_fp = 0;

	m_bufIndex = 0;
	m_bufSize = 0;
	m_eof = false;
	m_currentPos = 0;
}

//////////////////////////////////////////////////////////////////////

// Open a file. 
bool XMLReader::Open(const char* szfile)
{
	// try to open the file
	m_fp = fopen(szfile, "rb");
	m_ownFile = true;

	// check if we were successful
	if (m_fp == 0) return false;

	// read the first line
	char szline[256] = { 0 };
	fgets(szline, 255, m_fp);

	// make sure it is correct
	if (strncmp(szline, "<?xml", 5) != 0)
	{
		// This file is not an XML file
		Close();
		return false;
	}

	// This file is ready to be processed
	return true;
}

bool XMLReader::Attach(FILE* fp)
{
	// keep a copy of the file pointer
	m_fp = fp;
	m_ownFile = false;

	// read the first line
	char szline[256] = {0};
	fgets(szline, 255, m_fp);

	// make sure it is correct
	if (strncmp(szline,"<?xml", 5) != 0)
	{
		// This file is not an XML file
		return false;
	}

	// This file is ready to be processed
	return true;
}

//////////////////////////////////////////////////////////////////////

bool XMLReader::FindTag(const char* sztag, XMLTag& tag)
{
	// go to the beginning of the file
	fseek(m_fp, 0, SEEK_SET);
	m_bufIndex = m_bufSize = 0;
	m_currentPos = 0;
	m_eof = false;

	// set the first tag
	tag.m_preader = this;
	tag.m_ncurrent_line = 1;
	tag.m_fpos = currentPos();

	// find the correct tag
	bool bfound = false;
	do
	{
		NextTag(tag);
		if (strcmp(sztag, tag.m_sztag) == 0) bfound = true;
	}
	while (!bfound);

	return true;
}

//////////////////////////////////////////////////////////////////////

void XMLReader::NextTag(XMLTag& tag)
{
	// set current line number
	m_nline = tag.m_ncurrent_line;

	// set the current file position
	if (m_currentPos != tag.m_fpos)
	{
		fseek(m_fp, tag.m_fpos, SEEK_SET);
		m_currentPos = tag.m_fpos;
		m_bufSize = m_bufIndex = 0;
		m_eof = false;
	}

	// clear tag's content
	tag.clear();

	// read the start tag
	ReadTag(tag);

	try
	{
		// read value and end tag if tag is not empty
		if (!tag.isempty())
		{
			// read the value
			ReadValue(tag);

			// read the end tag
			ReadEndTag(tag);
		}
	}
	catch (UnexpectedEOF)
	{
		if (!tag.isend()) throw;
	}

	// store current line number
	tag.m_ncurrent_line = m_nline;

	// store start file pos for next element
	tag.m_fpos = currentPos();
}

inline bool isvalid(char c)
{
	return (isalnum(c) || (c=='_') || (c=='.'));
}

const std::string& XMLReader::GetLastComment()
{
	return m_comment;
}

void XMLReader::ReadTag(XMLTag& tag)
{
	// find the start token
	char ch, *sz;
	while (true)
	{
		while ((ch=GetChar())!='<') if (!isspace(ch)) throw XMLSyntaxError();

		ch = GetChar();
		if (ch == '!')
		{
			// parse the comment
			ch = GetChar(); if (ch != '-') throw XMLSyntaxError();
			ch = GetChar(); if (ch != '-') throw XMLSyntaxError();

			m_comment.clear();

			// find the end of the comment
			do
			{
				ch = GetNextChar();
				if (ch == '-')
				{
					ch = GetNextChar();
					if (ch == '-')
					{
						ch = GetNextChar();
						if (ch == '>') break;
						else m_comment += "--";
					}
					else m_comment += '-';
				}
				m_comment += ch;
			}
			while (1);

			// remove the first and last end-of-line character
			while ((m_comment.empty() == false) && (m_comment[0] == '\n'))
			{
				m_comment.erase(m_comment.begin());
			}

			while ((m_comment.empty() == false) && (m_comment.back() == '\n'))
			{
				m_comment.erase(m_comment.begin() + m_comment.size() - 1);
			}

			int a = 0;
		}
		else if (ch == '?')
		{
			// parse the xml header tag
			while ((ch = GetChar()) != '?');
			ch = GetChar();
			if (ch != '>') throw XMLSyntaxError();
		}
		else break;
	}

	// record the startline
	tag.m_nstart_line = m_nline;

	if (ch == '/') 
	{
		tag.m_bend = true;
		m_comment.clear();
		ch = GetChar();
	}

	// skip whitespace
	while (isspace(ch)) ch = GetChar();

	// read the tag name
	if (!isvalid(ch)) throw XMLSyntaxError();
	sz = tag.m_sztag;
	*sz++ = ch;
	while (isvalid(ch=GetChar())) *sz++ = ch;
	*sz = 0;

	// read attributes
	tag.m_natt = 0;
	int n = 0;
	sz = 0;
	while (true)
	{
		// skip whitespace
		while (isspace(ch)) ch = GetChar();
		if (ch == '/')
		{
			tag.m_bempty = true;
			ch = GetChar();
			if (ch != '>') throw XMLSyntaxError();
			break;
		}
		else if (ch == '>') break;

		// read the attribute's name
		sz = tag.m_att[n].m_sztag;
		if (!isvalid(ch)) throw XMLSyntaxError();
		*sz++ = ch;
		while (isvalid(ch=GetChar())) *sz++ = ch;
		*sz=0; sz=0;

		// skip whitespace
		while (isspace(ch)) ch=GetChar();
		if (ch != '=') throw XMLSyntaxError();

		// skip whitespace
		while (isspace(ch=GetChar()));
		if ((ch != '"')&&(ch!='\'')) throw XMLSyntaxError();
		char quot = ch;

		sz = tag.m_att[n].m_szval;
		while ((ch=GetChar())!=quot) *sz++ = ch;
		*sz=0; sz=0;
		ch=GetChar();

		++n;
		++tag.m_natt;
	}

	if (!tag.isend() && !tag.isempty())
	{
		// keep a copy of the name
		strcpy(tag.m_szroot[tag.m_nlevel], tag.m_sztag);
		tag.m_nlevel++;
	}
}

void XMLReader::ReadValue(XMLTag& tag)
{
	char ch;
	if (!tag.isend())
	{
		char *sz = tag.m_szval;
		while ((ch=GetChar())!='<') *sz++ = ch;
		*sz=0;
	}
	else while ((ch=GetChar())!='<');
}

void XMLReader::ReadEndTag(XMLTag& tag)
{
	char ch, *sz = tag.m_sztag;
	if (!tag.isend())
	{
		ch = GetChar();
		if (ch == '/')
		{
			// this is the end tag
			// make sure it matches the tag name
			--tag.m_nlevel;

			// skip whitespace
			while (isspace(ch=GetChar()));

			int n = 0;
			do 
			{ 
				if (ch != *sz++) throw UnmatchedEndTag(tag);
				ch = GetChar();
				++n;
			}
			while (!isspace(ch) && (ch!='>'));
			if (n != (int) strlen(tag.m_sztag)) throw UnmatchedEndTag(tag);

			// skip whitespace
			while (isspace(ch)) ch=GetChar();
			if (ch != '>') throw XMLSyntaxError();
	
			// find the start of the next tag
			if (tag.m_nlevel)
			{
				while (isspace(ch=GetChar()));
				if (ch != '<') throw XMLSyntaxError();
				rewind(1);
			}
		}
		else
		{
			// this element has child elements
			// and therefor is not a leaf

			tag.m_bleaf = false;
			rewind(2);
		}
	}
	else 
	{
		rewind(1);

		--tag.m_nlevel;

		// make sure the name is the same as the root
		if (strcmp(tag.m_sztag, tag.m_szroot[tag.m_nlevel]) != 0) throw UnmatchedEndTag(tag);
	}
}

//////////////////////////////////////////////////////////////////////

void XMLReader::SkipTag(XMLTag& tag)
{
	// if this tag is a leaf we just return
	if (tag.isleaf()) return;

	// if it is not a leaf we have to loop over all 
	// the children, skipping each child in turn
	NextTag(tag);
	do
	{
		SkipTag(tag);
		++tag;
	}
	while (!tag.isend());
}
