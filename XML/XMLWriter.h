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
#include <string.h>
#include <MathLib/math3d.h>
#include <MathLib/mat3d.h>
#include <FSCore/color.h>
#include <vector>
#include <string>

#define MAX_TAGS	32
#define MAX_ATTR	32

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
	const char* name() const { return m_sztag; }

	void value(const char* sz) { strcpy(m_szval, sz); }
	void value(int    n) { sprintf(m_szval, "%d" , n); }
	void value(int* pi, int n);
	void value(bool   b) { sprintf(m_szval, "%d" , (int) b); }
	void value(double g) { sprintf(m_szval, "%.9lg", g); }
	void value(double* pg, int n);
	void value(const vec3d& r);
	void value(const mat3d& a);
	void value(const vec2i& r);

	int add_attribute(const char* szn, const char* szv);
	int add_attribute(const char* szn, int n);
	int add_attribute(const char* szn, bool b);
	int add_attribute(const char* szn, double g);
	int add_attribute(const char* szn, const std::string& s);

	void set_attribute(int nid, const char* szv);
	void set_attribute(int nid, int n);
	void set_attribute(int nid, bool b);
	void set_attribute(int nid, double g);

protected:
	char	m_sztag[128];	// element name
	char	m_szval[512];	// element value

	int	m_natt;	// number of attributes
	char m_attn[MAX_ATTR][128];	// attribute name
	char m_attv[MAX_ATTR][128]; // attribute value

public:
	static void setDefautlFormats();
	static const char* intFormat;

	friend class XMLWriter;
};

class XMLWriter  
{
public:
	enum XMLFloatFormat {
		ScientificFormat,
		FixedFormat
	};

public:
	XMLWriter();
	virtual ~XMLWriter();
	
	bool open(const char* szfile);

	void close();

	void add_branch(XMLElement& el, bool bclear = true);
	void add_branch(const char* szname);

	void add_empty(XMLElement& el, bool bclear = true);

	void add_leaf  (XMLElement& el, bool bclear = true);

	void add_leaf(const char* szn, const char* szv);
	void add_leaf(const char* szn, const std::string& s);

	void add_leaf(const char* szn, int    n){ char szv[256]; sprintf(szv, "%d" , n); add_leaf(szn, szv); }
	void add_leaf(const char* szn, bool   b){ char szv[256]; sprintf(szv, "%d" , b); add_leaf(szn, szv); }
	void add_leaf(const char* szn, double g){ char szv[256]; sprintf(szv, "%lg", g); add_leaf(szn, szv); }
	void add_leaf(const char* szn, int *pi, int n);
	void add_leaf(const char* szn, float* pg, int n);
	void add_leaf(const char* szn, double* pg, int n);
	void add_leaf(const char* szn, const vec3d& r){ char szv[256]; sprintf(szv, "%g,%g,%g", r.x, r.y, r.z); add_leaf(szn, szv); }
	void add_leaf(const char* szn, const quatd& q){ char szv[256]; sprintf(szv, "%g,%g,%g,%g", q.x, q.y, q.z, q.w); add_leaf(szn, szv); }
	void add_leaf(const char* szn, const GLColor& c) { char szv[256]; sprintf(szv, "%d,%d,%d", c.r, c.g, c.b); }
	void add_leaf(XMLElement& el, const std::vector<int>& A);

	void close_branch();

	void add_comment(const std::string& s, bool singleLine = false);

public:
	static void SetFloatFormat(XMLFloatFormat fmt);
	static XMLFloatFormat GetFloatFormat();

protected:
	void inc_level();
	void dec_level();

protected:
	FILE*	m_fp;
	int		m_level;

	char	m_tag[MAX_TAGS][256];
	char	m_sztab[256];

	static XMLFloatFormat	m_floatFormat;
};
