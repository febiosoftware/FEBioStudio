#pragma once
#include "math3d.h"
#include "ColorMap.h"
#include <stdio.h>
#include <string.h>

#define MAX_TAGS	32
#define MAX_ATTR	32

namespace Post {

class XMLWriter;

class XMLElement
{
public:
	XMLElement(const char* szname = 0)
	{
		clear();
		if (szname) strcpy(m_sztag, szname);
	}

	void clear()
	{
		m_natt = 0;
		m_sztag[0] = 0;
		m_szval[0] = 0;
	}

	void name(const char* sz) { strcpy(m_sztag, sz); }

	void value(const char* sz) { strcpy(m_szval, sz); }
	void value(int    n) { sprintf(m_szval, "%d" , n); }
	void value(int* pi, int n);
	void value(bool   b) { sprintf(m_szval, "%d" , (int) b); }
	void value(double g) { sprintf(m_szval, "%lg", g); }
	void value(double* pg, int n);
	void value(vec3f r) { sprintf(m_szval, "%15.7e,%15.7e,%15.7e", r.x, r.y, r.z); }

	int add_attribute(const char* szn, const std::string& s);
	int add_attribute(const char* szn, const char* szv);
	int add_attribute(const char* szn, int n);
	int add_attribute(const char* szn, bool b);
	int add_attribute(const char* szn, double g);
	int add_attribute(const char* szn, GLColor& c);
	int add_attribute(const char* szn, double* v, int n);
	int add_attribute(const char* szn, int* d, int n);

	void set_attribute(int nid, const char* szv);
	void set_attribute(int nid, int n);
	void set_attribute(int nid, bool b);
	void set_attribute(int nid, double g);

protected:
	char	m_sztag[256];	// element name
	char	m_szval[256];	// element value

	int	m_natt;	// number of attributes
	char m_attn[MAX_ATTR][256];	// attribute name
	char m_attv[MAX_ATTR][256]; // attribute value

	friend class XMLWriter;
};

class XMLWriter  
{
public:
	enum OUTPUT_STYLE {
		DEFAULT,
		ANDROID
	};

public:
	XMLWriter();
	virtual ~XMLWriter();
	
	bool open(const char* szfile);

	void close();

	void SetOutputStyle(OUTPUT_STYLE os);

	void add_branch(XMLElement& el, bool bclear = true);
	void add_branch(const char* szname);

	void add_empty(XMLElement& el, bool bclear = true);

	void add_leaf  (XMLElement& el, bool bclear = true);

	void add_leaf(const char* szn, const char* szv);
	void add_leaf(const char* szn, std::string& s);
	void add_leaf(const char* szn, int    n){ char szv[256]; sprintf(szv, "%d" , n); add_leaf(szn, szv); }
	void add_leaf(const char* szn, bool   b){ char szv[256]; sprintf(szv, "%d" , b); add_leaf(szn, szv); }
	void add_leaf(const char* szn, double g){ char szv[256]; sprintf(szv, "%lg", g); add_leaf(szn, szv); }
	void add_leaf(const char* szn, int *pi, int n);
	void add_leaf(const char* szn, float* pf, int n);
	void add_leaf(const char* szn, double* pg, int n);
	void add_leaf(const char* szn, const vec3f& r){ char szv[256]; sprintf(szv, "%g,%g,%g", r.x, r.y, r.z); add_leaf(szn, szv); }
	void add_leaf(const char* szn, GLColor& c) { char szv[256]; sprintf(szv, "%d,%d,%d,%d", (int) c.r, (int) c.g, (int) c.b, (int) c.a); add_leaf(szn, szv); }

	void close_branch();

protected:
	void inc_level();
	void dec_level();

protected:
	FILE*	m_fp;
	int		m_level;

	OUTPUT_STYLE	m_os;

	char	m_tag[MAX_TAGS][256];
	char	m_sztab[256];
};

}
