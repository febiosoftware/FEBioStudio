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

// XMLWriter.cpp: implementation of the XMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "XMLWriter.h"

const char* XMLElement::intFormat = "%6d";

void XMLElement::setDefautlFormats()
{
	intFormat = "%6d";
}

void XMLElement::value(int* pi, int n)
{
	m_szval[0] = 0;
	if (n==0) return;

	sprintf(m_szval, intFormat, pi[0]);
	int l = (int)strlen(m_szval);
	for (int i=1; i<n; ++i)
	{
		sprintf(m_szval+l, ",");
		sprintf(m_szval + l + 1, intFormat, pi[i]);
		l = (int)strlen(m_szval);
	}
}

void XMLElement::value(double* pg, int n)
{
	m_szval[0] = 0;
	if (n==0) return;

	sprintf(m_szval, "%lg", pg[0]);
	int l = (int)strlen(m_szval);
	for (int i=1; i<n; ++i)
	{
		sprintf(m_szval+l, ",%lg", pg[i]);
		l = (int)strlen(m_szval);
	}
}

void XMLElement::value(const vec3d& r)
{ 
	if (XMLWriter::GetFloatFormat() == XMLWriter::ScientificFormat)
		sprintf(m_szval, "%15.7e,%15.7e,%15.7e", r.x, r.y, r.z); 
	else
		sprintf(m_szval, "%.9lg,%.9lg,%.9lg", r.x, r.y, r.z);
}

void XMLElement::value(const vec2i& r)
{
	sprintf(m_szval, "%d,%d", r.x, r.y);
}

void XMLElement::value(const mat3d& a)
{
	char* s = m_szval;
	if (XMLWriter::GetFloatFormat() == XMLWriter::ScientificFormat)
	{
		sprintf(s, "%15.7e,%15.7e,%15.7e,", a(0, 0), a(0, 1), a(0, 2)); s += strlen(s);
		sprintf(s, "%15.7e,%15.7e,%15.7e,", a(1, 0), a(1, 1), a(1, 2)); s += strlen(s);
		sprintf(s, "%15.7e,%15.7e,%15.7e" , a(2, 0), a(2, 1), a(2, 2));
	}
	else
	{
		sprintf(s, "%lg,%lg,%lg,", a(0,0), a(0,1), a(0,2)); s += strlen(s);
		sprintf(s, "%lg,%lg,%lg,", a(1,0), a(1,1), a(1,2)); s += strlen(s);
		sprintf(s, "%lg,%lg,%lg,", a(2,0), a(2,1), a(2,2));
	}
}

void XMLElement::value(const std::vector<int>& v)
{
	m_szval[0] = 0;
	if (v.empty()) return;

	sprintf(m_szval, intFormat, v[0]);
	int l = (int)strlen(m_szval);
	for (int i = 1; i < v.size(); ++i)
	{
		sprintf(m_szval + l, ",");
		sprintf(m_szval + l + 1, intFormat, v[i]);
		l = (int)strlen(m_szval);
	}
}

void XMLElement::value(const std::vector<double>& v)
{
	m_szval[0] = 0;
	if (v.empty()) return;

	sprintf(m_szval, "%lg", v[0]);
	int l = (int)strlen(m_szval);
	for (int i = 1; i < v.size(); ++i)
	{
		sprintf(m_szval + l, ",%lg", v[i]);
		l = (int)strlen(m_szval);
	}
}

int XMLElement::add_attribute(const char* szn, const char* szv)
{
	strcpy(m_attn[m_natt], szn);
	strcpy(m_attv[m_natt], szv);
	m_natt++;
	return m_natt-1;
}

int XMLElement::add_attribute(const char* szn, int n)
{
	strcpy(m_attn[m_natt], szn);
	sprintf(m_attv[m_natt], "%d", n);
	m_natt++;
	return m_natt-1;
}

int XMLElement::add_attribute(const char* szn, bool b)
{
	strcpy(m_attn[m_natt], szn);
	sprintf(m_attv[m_natt], "%d", (int) b);
	m_natt++;
	return m_natt-1;
}

int XMLElement::add_attribute(const char* szn, double g)
{
	strcpy(m_attn[m_natt], szn);
	sprintf(m_attv[m_natt], "%lg", g);
	m_natt++;
	return m_natt-1;
}

int XMLElement::add_attribute(const char* szn, const std::string& s)
{
	strcpy(m_attn[m_natt], szn);
	strcpy(m_attv[m_natt], s.c_str());
	m_natt++;
	return m_natt - 1;
}

void XMLElement::set_attribute(int nid, const char* szv)
{
	strcpy(m_attv[nid], szv);
}

void XMLElement::set_attribute(int nid, int n)
{
	sprintf(m_attv[nid], "%d", n);
}

void XMLElement::set_attribute(int nid, bool b)
{
	sprintf(m_attv[nid], "%d", (int) b);
}

void XMLElement::set_attribute(int nid, double g)
{
	sprintf(m_attv[nid], "%lg", g);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XMLWriter::XMLFloatFormat	XMLWriter::m_floatFormat = XMLWriter::FixedFormat;

void XMLWriter::SetFloatFormat(XMLFloatFormat fmt)
{
	m_floatFormat = fmt;
}

XMLWriter::XMLFloatFormat XMLWriter::GetFloatFormat()
{
	return m_floatFormat;
}

XMLWriter::XMLWriter()
{
	m_fp = 0;
	m_level = 0;

	m_sztab[0] = 0;

	XMLElement::setDefautlFormats();
}

XMLWriter::~XMLWriter()
{
	close();
}

bool XMLWriter::open(const char* szfile)
{
	if (m_fp) return false;
	if (szfile == nullptr) return false;

	m_fp = fopen(szfile, "wt");

	// write the first line
	if (m_fp) fprintf(m_fp, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
	
	return (m_fp != nullptr);
}

void XMLWriter::close()
{
	if (m_fp) fclose(m_fp); m_fp = 0;
}

void XMLWriter::inc_level()
{
	++m_level;

	m_sztab[0] = 0;
	int l=0;
	for (int i=0; i<m_level; ++i) 
	{
		sprintf(m_sztab+l, "\t");
		++l;
	}
	m_sztab[l] = 0;
}

void XMLWriter::dec_level()
{
	if (m_level <= 0) return;

	--m_level;

	m_sztab[0] = 0;
	int l=0;
	for (int i=0; i<m_level; ++i) 
	{
		sprintf(m_sztab+l, "\t");
		++l;
	}
	m_sztab[l] = 0;
}


void XMLWriter::add_branch(XMLElement& el, bool bclear)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_sztag);

	for (int i=0; i<el.m_natt; ++i)
	{
		fprintf(m_fp, " %s=\"%s\"", el.m_attn[i], el.m_attv[i]);
	}

	fprintf(m_fp, ">%s\n", el.m_szval);

	strcpy(m_tag[m_level], el.m_sztag);

	inc_level();

	if (bclear) el.clear();
}

void XMLWriter::add_branch(const char* sz)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>\n", m_sztab);
	fprintf(m_fp, szformat, sz);

	strcpy(m_tag[m_level], sz);
	inc_level();
}

void XMLWriter::add_empty(XMLElement& el, bool bclear)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_sztag);

	for (int i=0; i<el.m_natt; ++i)
	{
		fprintf(m_fp, " %s=\"%s\"", el.m_attn[i], el.m_attv[i]);
	}

	fprintf(m_fp, "/>\n");

	if (bclear) el.clear();
}

void XMLWriter::add_leaf(XMLElement& el, bool bclear)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_sztag);

	for (int i=0; i<el.m_natt; ++i)
	{
		fprintf(m_fp, " %s=\"%s\"", el.m_attn[i], el.m_attv[i]);
	}

	fprintf(m_fp, ">%s</%s>\n", el.m_szval, el.m_sztag);

	if (bclear) el.clear();
}

void XMLWriter::add_leaf(const char* szn, const char* szv)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, szn);

	fprintf(m_fp, ">%s</%s>\n", szv, szn);
}

void XMLWriter::add_leaf(const char* szn, const std::string& s)
{
	add_leaf(szn, s.c_str());
}

void XMLWriter::add_leaf(const char* szn, double* pg, int n)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>", m_sztab);

	fprintf(m_fp, szformat, szn);

	if (n>0)
	{
		fprintf(m_fp, "%.12lg", pg[0]);
		for (int i=1; i<n; ++i) fprintf(m_fp, ",%.12lg", pg[i]);
	}

	fprintf(m_fp, "</%s>\n", szn);

}

void XMLWriter::add_leaf(const char* szn, float* pg, int n)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>", m_sztab);

	fprintf(m_fp, szformat, szn);

	if (n>0)
	{
		fprintf(m_fp, "%.12g", pg[0]);
		for (int i=1; i<n; ++i) fprintf(m_fp, ",%.12g", pg[i]);
	}

	fprintf(m_fp, "</%s>\n", szn);

}


void XMLWriter::add_leaf(const char* szn, int* pi, int n)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>", m_sztab);

	fprintf(m_fp, szformat, szn);

	if (n>0)
	{
		fprintf(m_fp, "%d", pi[0]);
		for (int i=1; i<n; ++i) fprintf(m_fp, ",%d", pi[i]);
	}

	fprintf(m_fp, "</%s>\n", szn);
}

void XMLWriter::add_leaf(XMLElement& el, const std::vector<int>& A)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_sztag);

	for (int i=0; i<el.m_natt; ++i)
	{
		fprintf(m_fp, " %s=\"%s\"", el.m_attn[i], el.m_attv[i]);
	}

	fprintf(m_fp, ">\n%s", m_sztab);

	int n = (int) A.size(), l = 0;
	for (int i=0; i<n; ++i)
	{
		l += fprintf(m_fp, "%5d", A[i]);
		if (i < n-1)
		{
			fprintf(m_fp, ",");
			if (l > 80) { fprintf(m_fp, "\n%s", m_sztab); l=0; }
		}
	}
	fprintf(m_fp,"\n%s</%s>\n", m_sztab, el.m_sztag);
}


void XMLWriter::close_branch()
{
	if (m_level > 0)
	{
		dec_level();

		char szformat[256] = {0};
		sprintf(szformat, "%s</%%s>\n", m_sztab);

		fprintf(m_fp, szformat, m_tag[m_level]);
	}
}

void XMLWriter::add_comment(const std::string& s, bool singleLine)
{
	if (s.empty()) return;

	if (singleLine)
	{
		fprintf(m_fp, "<!-- %s -->\n", s.c_str());
	}
	else
	{
		fprintf(m_fp, "<!--\n%s\n-->\n", s.c_str());
	}
}
