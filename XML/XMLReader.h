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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <MathLib/math3d.h>
#include <MathLib/mat3d.h>
#include <FSCore/color.h>
#include <stdexcept>

#ifndef WIN32
	#include <string>
#endif

class XMLAtt
{
public:
	enum {MAX_TAG = 128};

public:
	char	m_szval[MAX_TAG];
	char	m_sztag[MAX_TAG];

public:
	void value(int& val) { val = atoi(m_szval); }

	template <typename T> T value() { return T(0); }

	int value(double* v, int n);

	const char* cvalue() { return m_szval; }

	bool operator == (const char* szval) { return (strcmp(szval, m_szval) == 0); }
	bool operator != (const char* szval) { return (strcmp(szval, m_szval) != 0); }
};

template <> inline int XMLAtt::value<int>() { return atoi(m_szval); }
template <> inline double XMLAtt::value<double>() { return atof(m_szval); }
template <> inline std::string XMLAtt::value<std::string>() { return m_szval; }

class XMLReader;

class XMLTag
{
public:
	enum {MAX_TAG   = 512};
	enum {MAX_ATT   =   8};
	enum {MAX_LEVEL =  16};

public:
	char	m_sztag[MAX_TAG];			// tag name
	char	m_szval[MAX_TAG];			// tag value

	XMLAtt	m_att[MAX_ATT];		// attributes

	int		m_nlevel;	// depth level
	char	m_szroot[MAX_LEVEL][MAX_TAG];	// name tag of parent's

	int		m_natt;	// nr of attributes

	XMLReader*	m_preader;		// pointer to reader
#ifdef WIN32
	fpos_t	m_fpos;				// file position of next tag
#else
	long int	m_fpos;				// file position of next tag
#endif
	int		m_nstart_line;		// line number at beginning of tag
	int		m_ncurrent_line;	// current line number

	bool	m_bend;		// end tag flag
	bool	m_bleaf;	// this is a leaf (i.e. has no child elements)
	bool	m_bempty;	// empty tag (i.e. no value)


	XMLTag();

	void clear()
	{
		m_sztag[0] = 0;
		m_szval[0] = 0;
		m_natt = 0;
		m_bend = false;
		m_bleaf = true;
		m_bempty = false;
	}

	int currentLine() const { return m_ncurrent_line; }

public:
	bool operator == (const char* sztag) { return (strcmp(sztag, m_sztag) == 0); }
	bool operator != (const char* sztag) { return (strcmp(sztag, m_sztag) != 0); }

	bool isend() { return m_bend; }
	bool isleaf() { return m_bleaf; }
	bool isempty() { return m_bempty; }

	void operator ++ ();

	int children();

public:
	const char* AttributeValue(const char* szat, bool bopt = false);
	XMLAtt* AttributePtr(const char* szat);
	XMLAtt& Attribute(const char* szat);

	template <typename T> T AttributeValue(const char* szatt, const T& def_val) { return def_val; }

	const char* Name() { return m_sztag; }

	void value(char* szstr) { strcpy(szstr, m_szval); }
	void value(double& val) { val = atof(m_szval); } 
	void value(float& val)  { val = (float) atof(m_szval); }
	void value(int& val) { val = atoi(m_szval); }
	int value(double* pf, int n);
	int value(float* pf, int n);
	int value(int* pi, int n);
	void value(vec3d& v);
	void value(vec2i& v);
	void value(mat3d& v);
	void value(vec3f& v);
	void value(bool& b) { b = (atoi(m_szval) == 1); }
	void value(vector<int>& l);
	void value(GLColor& c);
	void value(string& s);

	const char* szvalue() { return m_szval; }

	const std::string& comment();
};

template <> inline int XMLTag::AttributeValue<int>(const char* szatt, const int& def_val)
{
	XMLAtt* pa = AttributePtr(szatt);
	if (pa) return pa->value<int>();
	else return def_val;
}

template <> inline double XMLTag::AttributeValue<double >(const char* szatt, const double& def_val)
{
	XMLAtt* pa = AttributePtr(szatt);
	if (pa) return pa->value<double>();
	else return def_val;
}

template <> inline std::string XMLTag::AttributeValue<std::string>(const char* szatt, const std::string& def_val)
{
	XMLAtt* pa = AttributePtr(szatt);
	if (pa) return pa->value<std::string>();
	else return def_val;
}

class XMLReader  
{
public:
	enum { BUF_SIZE = 4096 };

public:
	// exceptions -----------

	// Base class for Exceptions
	class Error : public std::runtime_error
	{
	public:
		Error(const std::string& err) : std::runtime_error(err) {}
		Error(XMLTag& tag, const std::string& err);
	};

	// End of file was discovered 
	class EndOfFile : public Error {
	public:
		EndOfFile() : Error("End of file") {}
	};

	class UnexpectedEOF{};

	class XMLSyntaxError{};

	class UnmatchedEndTag
	{
	public:
		XMLTag tag;
		UnmatchedEndTag(XMLTag& t) : tag(t) {}
	};

	class InvalidTag
	{
	public:
		XMLTag tag;
		InvalidTag(XMLTag& t) : tag(t) {}
	};

	class InvalidValue
	{
	public:
		XMLTag tag;
		InvalidValue(XMLTag& t) : tag(t) {}
	};

	class InvalidAttributeValue
	{
	public:
		XMLTag tag;
		char szatt[XMLTag::MAX_TAG];
		char szval[XMLTag::MAX_TAG];
		InvalidAttributeValue(XMLTag& t, const char* sza, const char* szv) : tag(t)
		{ 
			strcpy(szatt, sza);
			strcpy(szval, szv);
		}
		InvalidAttributeValue(XMLTag& t, XMLAtt& a) : tag(t)
		{ 
			strcpy(szatt, a.m_sztag);
			strcpy(szval, a.m_szval);
		}
	};

	class MissingAttribute
	{
	public:
		XMLTag tag;
		char szatt[XMLAtt::MAX_TAG];
		MissingAttribute(XMLTag& t, const char* sza) : tag(t) { strcpy(szatt, sza); }
	};

	class MissingTag
	{
	public:
		XMLTag tag;
		MissingTag(XMLTag& t, const char* szt) : tag(t) {}
	};

	//------------------------


public:
	XMLReader();
	virtual ~XMLReader();

	FILE* GetFilePtr() { return m_fp; } 

	// Open a file. 
	bool Open(const char* szfile);

	void Close();

	// Attach a file to this reader. Reader does not take ownership of file pointer
	bool Attach(FILE* fp);

	bool FindTag(const char* sztag, XMLTag& tag);

	void NextTag(XMLTag& tag);

	int GetCurrentLine() { return m_nline; }

	void SkipTag(XMLTag& tag);

	const std::string& GetLastComment();

	int64_t currentPos()
	{
		return m_currentPos;
	}

protected:
	char GetChar() 
	{
		char ch;
		while ((ch = readNextChar()) == '\n') ++m_nline;
		return ch;
	}

	char readNextChar()
	{
		if (m_bufIndex >= m_bufSize)
		{
			if (m_eof) throw EndOfFile();

			m_bufSize = fread(m_buf, 1, BUF_SIZE, m_fp);
			m_bufIndex = 0;
			m_eof = (m_bufSize != BUF_SIZE);
		}
		m_currentPos++;
		return m_buf[m_bufIndex++];
	}

	void rewind(int64_t nstep)
	{
		m_bufIndex -= nstep;
		m_currentPos -= nstep;

		if (m_bufIndex < 0)
		{
			fseek(m_fp, m_bufIndex - m_bufSize, SEEK_CUR);
			m_bufIndex = m_bufSize = 0;
			m_eof = false;
		}
	}

	// only used for processing comments
	char GetNextChar()
	{
		char ch;
		do
		{
			ch = readNextChar();
			if (ch == '\n') ++m_nline;
		} 
		while (ch == '\r');
		return ch;
	}

	void ReadTag(XMLTag& tag);
	void ReadValue(XMLTag& tag);
	void ReadEndTag(XMLTag& tag);

protected:
	FILE*	m_fp;		// the file pointer
	bool	m_ownFile;	// flag that inidicates whether the reader owns the file pointer or not

	int		m_nline;	// current line (used only as temp storage)
	int64_t	m_currentPos;	//!< current file position

	string	m_comment;	// last comment that was read

	char		m_buf[BUF_SIZE];
	int64_t		m_bufIndex, m_bufSize;
	bool		m_eof;

	friend class XMLTag;
};

inline void XMLTag::operator ++ () { m_preader->NextTag(*this); }

inline const std::string& XMLTag::comment() { return m_preader->GetLastComment(); }
