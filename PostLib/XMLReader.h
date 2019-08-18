#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "math3d.h"
#include "ColorMap.h"

namespace Post {

class XMLAtt
{
public:
	enum {MAX_TAG = 128};

public:
	char	m_szval[MAX_TAG];
	char	m_sztag[MAX_TAG];

public:
	template <typename T> T value() { return T(0); }

	const char* cvalue() { return m_szval; }

	void value(bool& v);
	void value(int& v);
	void value(float& v);
	void value(double& v);
	void value(GLColor& v);
	void value(std::string& s);
	int value(double* v, int n);
	int value(int* v, int n);

	bool operator == (const char* szval) { return (strcmp(szval, m_sztag) == 0); }
	bool operator != (const char* szval) { return (strcmp(szval, m_sztag) != 0); }
};

template <> inline int XMLAtt::value<int>() { return atoi(m_szval); }
template <> inline double XMLAtt::value<double>() { return atof(m_szval); }

class XMLReader;

class XMLTag
{
public:
	enum {MAX_TAG   = 256};
	enum {MAX_ATT   =  32};
	enum {MAX_LEVEL =  16};

public:
	char	m_sztag[MAX_TAG];			// tag name
	char	m_szval[MAX_TAG];			// tag value

	XMLAtt	m_att[MAX_ATT];		// attributes

	int		m_nlevel;	// depth level
	char	m_szroot[MAX_LEVEL][MAX_TAG];	// name tag of parent's

	int		m_natt;	// nr of attributes

	XMLReader*	m_preader;		// pointer to reader
	fpos_t	m_fpos;				// file position of next tag
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

public:
	bool operator == (const char* sztag) { return (strcmp(sztag, m_sztag) == 0); }
	bool operator != (const char* sztag) { return (strcmp(sztag, m_sztag) != 0); }

	bool isend() { return m_bend; }
	bool isleaf() { return m_bleaf; }
	bool isempty() { return m_bempty; }

	void operator ++ ();

public:
	const char* AttributeValue(const char* szat, bool bopt = false);
	XMLAtt* AttributePtr(const char* szat);
	XMLAtt& Attribute(const char* szat);

	template <typename T> T AttributeValue(const char* szatt, T def_val) { return def_val; }

	const char* Name() { return m_sztag; }

	void value(char* szstr) { strcpy(szstr, m_szval); }
	void value(double& val) { val = atof(m_szval); } 
	void value(float& val)  { val = (float) atof(m_szval); }
	void value(int& val) { val = atoi(m_szval); }
	void value(double* pf, int n);
	void value(float* pf, int n);
	void value(int* pi, int n);
	void value(vec3f& v);
	void value(bool& b) { b = (atoi(m_szval) == 1); }
	void value(GLColor& c) 
	{
		int n[4] = {0,0,0,255};
		value(n, 4);
		c = GLColor((byte) n[0], (byte) n[1], (byte) n[2], (byte) n[3]);
	}

	const char* szvalue() { return m_szval; }
};

template <> inline int XMLTag::AttributeValue<int>(const char* szatt, int def_val)
{
	XMLAtt* pa = AttributePtr(szatt);
	if (pa) return pa->value<int>();
	else return def_val;
}

template <> inline double XMLTag::AttributeValue<double >(const char* szatt, double def_val)
{
	XMLAtt* pa = AttributePtr(szatt);
	if (pa) return pa->value<double>();
	else return def_val;
}

class XMLReader  
{
public:
	// exceptions -----------

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

	//------------------------


public:
	XMLReader();
	virtual ~XMLReader();

	FILE* GetFilePtr() { return m_fp; } 

	bool Attach(FILE* fp);

	bool FindTag(const char* sztag, XMLTag& tag);

	void NextTag(XMLTag& tag);

	int GetCurrentLine() { return m_nline; }

	void SkipTag(XMLTag& tag);

protected:
	char GetChar() 
	{
		char ch;
		while ((ch=fgetc(m_fp))=='\n') ++m_nline;
		if (feof(m_fp)) throw UnexpectedEOF();
		return ch;
	}

	void ReadTag(XMLTag& tag);
	void ReadValue(XMLTag& tag);
	void ReadEndTag(XMLTag& tag);

protected:
	FILE*	m_fp;		// the file pointer
	int		m_nline;	// current line (used only as temp storage)

	friend class XMLTag;
};

inline void XMLTag::operator ++ () { m_preader->NextTag(*this); }

}

