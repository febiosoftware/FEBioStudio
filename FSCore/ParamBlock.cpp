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

#include "stdafx.h"
#include "ParamBlock.h"
#include "LoadCurve.h"
#include <assert.h>

//-----------------------------------------------------------------------------
//! parameter constructor. Set all default values.
Param::Param()
{ 
	m_nID = -1; 
	m_ntype = Param_UNDEF; 
	m_nsize = 0;
	m_pd = 0;
	m_lc = -1; 
	m_szbrev = m_szname = m_szenum = 0; 
	m_szunit = 0; 
	m_nstate = Param_ALLFLAGS; 
	m_szindx = 0;
	m_nindx = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0; // don't care
	m_checkable = false;
	m_checked = false;
	m_flags = 0;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
//! parameter destructor.
Param::~Param()
{ 
	clear(); 
	if (m_bcopy) delete [] m_szenum;
}

//-----------------------------------------------------------------------------
Param* Param::SetEnumNames(const char* sz)
{ 
	if (m_bcopy) delete [] m_szenum;
	m_szenum = 0;
	m_bcopy = false;
	if (sz) m_szenum = (char*) sz;
	return this; 
}

//-----------------------------------------------------------------------------
Param* Param::CopyEnumNames(const char* sz)
{
	if (m_bcopy) delete [] m_szenum;
	m_szenum = 0;
	m_bcopy = false;
	if (sz)
	{
		int l = 0;
		const char* ch = sz;
		while (*ch)
		{
			int l1 = (int)strlen(ch) + 1;
			ch += l1;
			l += l1;
		}
		m_szenum = new char[l + 1];
		for (int i=0; i<l; ++i) m_szenum[i] = sz[i];
		m_szenum[l] = 0;
		m_bcopy = true;
	}
	return this;
}

//-----------------------------------------------------------------------------
void Param::SetFloatRange(double fmin, double fmax, double fstep)
{
	assert(m_ntype == Param_FLOAT);
	m_floatRange = true;
	m_fmin = fmin;
	m_fmax = fmax;
	m_rngType = 6; // FE_CLOSED

	if (fstep == 0.0) fstep = (fmax - fmin) / 100.0;
	m_fstep = fstep;
}
//-----------------------------------------------------------------------------
void Param::SetIntRange(int imin, int imax, int istep)
{
	assert(m_ntype == Param_INT);
	m_floatRange = true;
	m_fmin = imin;
	m_fmax = imax;
	m_fstep = istep;
	m_rngType = 6; // FE_CLOSED
}

void Param::SetRangeType(int n)
{
	m_rngType = n;
}

bool is_value_in_range(int rng, double val, double dmin, double dmax)
{
	switch (rng)
	{
	case 0: return true; break;
	case 1: return (val > dmin); break;
	case 2: return (val >= dmin); break;
	case 3: return (val < dmin); break;
	case 4: return (val <= dmin); break;
	case 5: return ((val > dmin) && (val < dmax)); break;
	case 6: return ((val >= dmin) && (val <= dmax)); break;
	case 7: return ((val > dmin) && (val <= dmax)); break;
	case 8: return ((val >= dmin) && (val < dmax)); break;
	case 9: return (val != dmin); break;
	}
	return false;
}

bool Param::IsValueValid() const
{
	if ((m_ntype == Param_INT) && m_floatRange)
	{
		int v = GetIntValue();
		return is_value_in_range(m_rngType, (double)v, m_fmin, m_fmax);
	}
	if ((m_ntype == Param_FLOAT) && m_floatRange)
	{
		double v = GetFloatValue();
		return is_value_in_range(m_rngType, v, m_fmin, m_fmax);
	}

	return true;
}

//-----------------------------------------------------------------------------
void Param::SetCheckable(bool b)
{
	m_checkable = b;
}

//-----------------------------------------------------------------------------
bool Param::IsCheckable() const
{
	return m_checkable;
}

//-----------------------------------------------------------------------------
void Param::SetChecked(bool b)
{
	m_checked = b;
}

//-----------------------------------------------------------------------------
bool Param::IsChecked() const
{
	return (m_checkable ? m_checked : false);
}

//-----------------------------------------------------------------------------
size_t Param::size() const
{
	switch (m_ntype)
	{
	case Param_ARRAY_INT   : return ((std::vector<int>*)(m_pd))->size(); break;
	case Param_ARRAY_DOUBLE: return ((std::vector<double>*)(m_pd))->size(); break;
	default:
		assert(false);
	}
	return 0;
}

//-----------------------------------------------------------------------------
void Param::SetWatchVariable(Param* watchVar)
{
	assert(watchVar && (watchVar->GetParamType() == Param_BOOL));
	m_watch = watchVar;
}

bool Param::IsWatched() const
{
	return (m_watch != nullptr);
}

void Param::SetWatchFlag(bool b)
{
	assert(m_watch);
	if (m_watch) m_watch->SetBoolValue(b);
}

bool Param::GetWatchFlag() const
{
	assert(m_watch);
	if (m_watch) return m_watch->GetBoolValue();
	else return false;
}

//-----------------------------------------------------------------------------
void Param::SetParameterGroup(int n)
{
	m_paramGroup = n;
}

int Param::GetParameterGroup() const
{
	return m_paramGroup;
}

//-----------------------------------------------------------------------------
Param* Param::MakeVariable(bool b)
{
	if (b)
	{
		assert((m_ntype == Param_FLOAT) || (m_ntype == Param_ARRAY_DOUBLE) || (m_ntype == Param_VEC3D) || (m_ntype == Param_MAT3D) || (m_ntype == Param_MAT3DS) || (m_ntype == Param_MATH));
		m_varType = m_ntype;
	}
	else m_varType = Param_UNDEF;
	return this;
}

//-----------------------------------------------------------------------------
void Param::clear()
{
	if (m_pd)
	{
		switch (m_ntype)
		{
		case Param_INT   : delete ((int*) m_pd); break;
		case Param_CHOICE: delete ((int*)m_pd); break;
		case Param_FLOAT : delete ((double*)m_pd); break;
		case Param_BOOL  : delete ((bool*) m_pd); break;
		case Param_VEC3D : delete ((vec3d*) m_pd); break;
		case Param_VEC2I : delete ((vec2i*)m_pd); break;
		case Param_VEC2D : delete ((vec2d*)m_pd); break;
		case Param_MAT3D : delete ((mat3d*)m_pd); break;
		case Param_MAT3DS: delete ((mat3ds*)m_pd); break;
		case Param_STRING: delete ((std::string*) m_pd); break;
		case Param_URL   : delete ((std::string*) m_pd); break;
		case Param_MATH  : delete ((std::string*) m_pd); break;
		case Param_COLOR : delete ((GLColor*)m_pd); break;
		case Param_STD_VECTOR_INT   : delete ((std::vector<int>*)m_pd); break;
		case Param_STD_VECTOR_DOUBLE: delete ((std::vector<double>*)m_pd); break;
		case Param_STD_VECTOR_VEC2D : delete ((std::vector<vec2d>*)m_pd); break;
		case Param_ARRAY_INT   : delete ((std::vector<int>*)m_pd); break;
		case Param_ARRAY_DOUBLE: delete ((std::vector<double>*)m_pd); break;
		default:
			assert(false);
		}
	}
}

void Param::SetParamType(Param_Type t)
{
	clear();
	m_ntype = t;
	switch (t)
	{
	case Param_INT   : m_pd = new int; break;
	case Param_CHOICE: m_pd = new int; break;
	case Param_FLOAT : m_pd = new double; break;
	case Param_BOOL  : m_pd = new bool; break;
	case Param_VEC3D : m_pd = new vec3d; break;
	case Param_VEC2I : m_pd = new vec2i; break;
	case Param_VEC2D : m_pd = new vec2d; break;
	case Param_MAT3D : m_pd = new mat3d; break;
	case Param_MAT3DS: m_pd = new mat3ds; break;
	case Param_STRING: m_pd = new std::string; break;
	case Param_URL   : m_pd = new std::string; break;
	case Param_MATH  : m_pd = new std::string; break;
	case Param_COLOR : m_pd = new GLColor; break;
	case Param_STD_VECTOR_INT   : m_pd = new std::vector<int>(); break;
	case Param_STD_VECTOR_DOUBLE: m_pd = new std::vector<double>(); break;
	case Param_STD_VECTOR_VEC2D : m_pd = new std::vector<vec2d>(); break;
	case Param_ARRAY_INT   : m_pd = new std::vector<int   >(m_nsize); break;
	case Param_ARRAY_DOUBLE: m_pd = new std::vector<double>(m_nsize); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void Param::SetLoadCurveID(int lcid)
{
	m_lc = lcid;
}

//-----------------------------------------------------------------------------
int Param::GetLoadCurveID() const
{
	return m_lc;
}

//-----------------------------------------------------------------------------
int Param::GetArraySize() const
{
	if ((m_ntype == Param_ARRAY_DOUBLE) || (m_ntype == Param_ARRAY_INT)) return m_nsize;
	return 0;
}

bool Param::IsFixedSize() const { return m_fixedSize; }
void Param::SetFixedSize(bool b) { assert(m_ntype == Param_STD_VECTOR_DOUBLE); m_fixedSize = b; }

//-----------------------------------------------------------------------------
const char* Param::GetEnumName(int n) const
{
	const char* sz = m_szenum;
	for (int i = 0; i < n; ++i)
	{
		size_t l = strlen(sz);
		if (l == 0) 
		{
//			assert(false);
			return nullptr;
		}
		sz += l+1;
	}
	return sz;
}

//-----------------------------------------------------------------------------
int Param::FindEnum(const char* sz) const
{
	if (m_szenum == nullptr) return -1;
	const char* c = m_szenum;
	int n = 0;
	while (c && *c)
	{
		if (strcmp(c, sz) == 0) return n;
		n++;
		c += strlen(c)+1;
	}
	return n;
}

//-----------------------------------------------------------------------------
Param::Param(const Param& p)
{
	m_nID = p.m_nID;
	m_ntype = p.m_ntype;
	m_nsize = p.m_nsize;
	m_szbrev = p.m_szbrev;
	m_szname = p.m_szname;
	m_szunit = p.m_szunit;
	m_nstate = p.m_nstate;
    m_szindx = p.m_szindx;
    m_nindx = p.m_nindx;
	m_flags = p.m_flags;
	m_paramGroup = p.m_paramGroup;

	// we cannot copy the watch parameter!
	m_watch = nullptr;

	m_lc = p.m_lc;

	m_bcopy = false;
	if (p.m_bcopy) CopyEnumNames(p.m_szenum);
	else m_szenum = p.m_szenum;

	m_offset = p.m_offset;
	m_varType = p.m_varType;
	m_floatRange = p.m_floatRange;
	m_fmin = p.m_fmin;
	m_fmax = p.m_fmax;
	m_fstep = p.m_fstep;
	m_rngType = p.m_rngType;
	m_checkable = p.m_checkable;
	m_checked = p.m_checked;
	switch (m_ntype)
	{
	case Param_INT   : { int*    pi = new int   ; m_pd = pi; *pi = *((int*   )p.m_pd); } break;
	case Param_CHOICE: { int*    pi = new int   ; m_pd = pi; *pi = *((int*   )p.m_pd); } break;
	case Param_FLOAT : { double* pd = new double; m_pd = pd; *pd = *((double*)p.m_pd); } break;
	case Param_BOOL  : { bool*   pb = new bool  ; m_pd = pb; *pb = *((bool*  )p.m_pd); } break;
	case Param_VEC3D : { vec3d*	 pv = new vec3d ; m_pd = pv; *pv = *((vec3d* )p.m_pd); } break;
	case Param_VEC2I : { vec2i*	 pv = new vec2i ; m_pd = pv; *pv = *((vec2i* )p.m_pd); } break;
	case Param_VEC2D : { vec2d*	 pv = new vec2d ; m_pd = pv; *pv = *((vec2d* )p.m_pd); } break;
	case Param_MAT3D : { mat3d*	 pv = new mat3d ; m_pd = pv; *pv = *((mat3d* )p.m_pd); } break;
	case Param_MAT3DS: { mat3ds* pv = new mat3ds; m_pd = pv; *pv = *((mat3ds* )p.m_pd); } break;
	case Param_STRING: { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_URL   : { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_MATH  : { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_COLOR : { GLColor* pc = new GLColor; m_pd = pc; *pc = *((GLColor*)p.m_pd); } break;
	case Param_STD_VECTOR_INT   : { std::vector<int>* pv = new std::vector<int>(); m_pd = pv; *pv = *((std::vector<int>*)p.m_pd); } break;
	case Param_STD_VECTOR_DOUBLE: { std::vector<double>* pv = new std::vector<double>(); m_pd = pv; *pv = *((std::vector<double>*)p.m_pd); } break;
	case Param_STD_VECTOR_VEC2D : { std::vector<vec2d>* pv = new std::vector<vec2d>(); m_pd = pv; *pv = *((std::vector<vec2d>*)p.m_pd); } break;
	case Param_ARRAY_INT   : { std::vector<int   >* pv = new std::vector<int>   (); m_pd = pv; *pv = *((std::vector<int   >*)p.m_pd); } break;
	case Param_ARRAY_DOUBLE: { std::vector<double>* pv = new std::vector<double>(); m_pd = pv; *pv = *((std::vector<double>*)p.m_pd); } break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
Param& Param::operator = (const Param& p)
{
	clear();
//	m_nID = p.m_nID;
	m_ntype = p.m_ntype;
	m_nsize = p.m_nsize;
//	m_szbrev = p.m_szbrev;
//	m_szname = p.m_szname;
//	m_szenum = p.m_szenum;
//	m_szunit = p.m_szunit;
//	m_nstate = p.m_nstate;
//  m_szindx = p.m_szindx;
//  m_nindx = p.m_nindx;
//	m_offset = p.m_offset;
//	m_varType = p.m_varType;
//	m_checkable = p.m_checkable;
	m_checked = p.m_checked;
//	m_flags = p.m_flags;
	m_lc = p.m_lc;
//	m_paramGroup = p.m_paramGroup;

	switch (m_ntype)
	{
	case Param_INT   : { int*    pi = new int   ; m_pd = pi; *pi = *((int*   )p.m_pd); } break;
	case Param_CHOICE: { int*    pi = new int   ; m_pd = pi; *pi = *((int*   )p.m_pd); } break;
	case Param_FLOAT : { double* pd = new double; m_pd = pd; *pd = *((double*)p.m_pd); } break;
	case Param_BOOL  : { bool*   pb = new bool  ; m_pd = pb; *pb = *((bool*  )p.m_pd); } break;
	case Param_VEC3D : { vec3d*	 pv = new vec3d ; m_pd = pv; *pv = *((vec3d* )p.m_pd); } break;
	case Param_VEC2I : { vec2i*	 pv = new vec2i ; m_pd = pv; *pv = *((vec2i* )p.m_pd); } break;
	case Param_VEC2D : { vec2d*	 pv = new vec2d ; m_pd = pv; *pv = *((vec2d* )p.m_pd); } break;
	case Param_MAT3D : { mat3d*	 pv = new mat3d ; m_pd = pv; *pv = *((mat3d* )p.m_pd); } break;
	case Param_MAT3DS: { mat3ds* pv = new mat3ds; m_pd = pv; *pv = *((mat3ds*)p.m_pd); } break;
	case Param_STRING: { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_URL   : { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_MATH  : { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_COLOR : { GLColor* pc = new GLColor; m_pd = pc; *pc = *((GLColor*)p.m_pd); } break;
	case Param_STD_VECTOR_INT: { std::vector<int>* pv = new std::vector<int>(); m_pd = pv; *pv = *((std::vector<int>*)p.m_pd); } break;
	case Param_STD_VECTOR_DOUBLE: { std::vector<double>* pv = new std::vector<double>(); m_pd = pv; *pv = *((std::vector<double>*)p.m_pd); } break;
	case Param_STD_VECTOR_VEC2D : { std::vector<vec2d>* pv = new std::vector<vec2d>(); m_pd = pv; *pv = *((std::vector<vec2d>*)p.m_pd); } break;
	case Param_ARRAY_INT: { std::vector<int>* pv = new std::vector<int>(); m_pd = pv; *pv = *((std::vector<int>*)p.m_pd); } break;
	case Param_ARRAY_DOUBLE: { std::vector<double>* pv = new std::vector<double>(); m_pd = pv; *pv = *((std::vector<double>*)p.m_pd); } break;
	default:
		assert(false);
	}

	return (*this);
}

//-----------------------------------------------------------------------------
Param::Param(int n, Param_Type ntype, const char* szb, const char* szn)
{
	int* pi = new int;
	*pi = n;
	m_pd = pi;
	assert((ntype == Param_INT) || (ntype == Param_CHOICE));
	m_ntype = ntype;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(int n, const char* szb, const char* szn)
{
	int* pi = new int;
	*pi = n;
	m_pd = pi;
	m_ntype = Param_INT;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(double d, const char* szb, const char* szn)
{
	double* pd = new double;
	*pd = d;
	m_pd = pd;
	m_ntype = Param_FLOAT;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(double d, const char* szunit, const char* szb, const char* szn)
{
	double* pd = new double;
	*pd = d;
	m_pd = pd;
	m_ntype = Param_FLOAT;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = szunit;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(bool b, const char* szb, const char* szn)
{
	bool* pb = new bool;
	*pb = b;
	m_pd = pb;
	m_ntype = Param_BOOL;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(vec3d v, const char* szb, const char* szn)
{
	vec3d* pv = new vec3d;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_VEC3D;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}


//-----------------------------------------------------------------------------
Param::Param(vec2i v, const char* szb, const char* szn)
{
	vec2i* pv = new vec2i;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_VEC2I;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
	m_watch = nullptr;
}

Param::Param(vec2d v, const char* szb, const char* szn)
{
	vec2d* pv = new vec2d;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_VEC2D;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(mat3d v, const char* szb, const char* szn)
{
	mat3d* pv = new mat3d;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_MAT3D;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(mat3ds v, const char* szb, const char* szn)
{
	mat3ds* pv = new mat3ds;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_MAT3DS;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(GLColor c, const char* szb, const char* szn)
{
	GLColor* pc = new GLColor(c);
	m_pd = pc;
	m_ntype = Param_COLOR;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(const std::vector<int>& v, const char* szb, const char* szn)
{
	std::vector<int>* pc = new std::vector<int>(v);
	m_pd = pc;
	m_ntype = Param_STD_VECTOR_INT;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

Param::Param(const std::vector<double>& v, const char* szb, const char* szn)
{
	std::vector<double>* pc = new std::vector<double>(v);
	m_pd = pc;
	m_ntype = Param_STD_VECTOR_DOUBLE;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

Param::Param(const std::vector<vec2d>& v, const char* szb, const char* szn)
{
	std::vector<vec2d>* pc = new std::vector<vec2d>(v);
	m_pd = pc;
	m_ntype = Param_STD_VECTOR_VEC2D;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

Param::Param(const std::string& val, const char* szb, const char* szn)
{
	std::string* pv = new std::string;
	*pv = val;
	m_pd = pv;
	m_ntype = Param_STRING;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

Param::Param(const int* v, int nsize, const char* szb, const char* szn)
{
	assert(nsize > 1);
	std::vector<int>* pc = new std::vector<int>();
	pc->resize(nsize);
	for (int i = 0; i < nsize; ++i) (*pc)[i] = v[i];

	m_pd = pc;
	m_ntype = Param_ARRAY_INT;
	m_nsize = nsize;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

Param::Param(const double* v, int nsize, const char* szb, const char* szn)
{
	assert(nsize > 1);
	std::vector<double>* pc = new std::vector<double>();
	pc->resize(nsize);
	for (int i = 0; i < nsize; ++i) (*pc)[i] = v[i];

	m_pd = pc;
	m_ntype = Param_ARRAY_DOUBLE;
	m_nsize = nsize;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_szindx = 0;
	m_nindx = -1;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

//-----------------------------------------------------------------------------
Param::Param(int n, const char* szi, int idx, const char* szb, const char* szn)
{
	int* pi = new int;
	*pi = n;
	m_pd = pi;
	m_ntype = Param_INT;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szindx = szi;
    m_nindx = idx;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}
    
//-----------------------------------------------------------------------------
Param::Param(double d, const char* szi, int idx, const char* szb, const char* szn)
{
	double* pd = new double;
	*pd = d;
	m_pd = pd;
	m_ntype = Param_FLOAT;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szindx = szi;
    m_nindx = idx;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}
    
//-----------------------------------------------------------------------------
Param::Param(double d, const char* szi, int idx, const char* szunit, const char* szb, const char* szn)
{
	double* pd = new double;
	m_pd = pd;
	*pd = d;
	m_ntype = Param_FLOAT;
	m_nsize = 0;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szindx = szi;
    m_nindx = idx;
	m_szunit = szunit;
	m_nstate = Param_ALLFLAGS;
	m_flags = 0;
	m_lc = -1;
	m_bcopy = false;
	m_offset = 0;
	m_varType = Param_UNDEF;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_rngType = 0;
	m_checkable = false;
	m_checked = false;
	m_paramGroup = -1;
	m_watch = nullptr;
}

void Param::SetArrayIntValue(const std::vector<int>& v)
{
	assert(m_ntype == Param_ARRAY_INT);
	auto& d = val<std::vector<int> >();
	if (d.empty()) { d = v; return; }
	assert(d.size() == v.size());
	int n = MIN(d.size(), v.size());
	for (int i = 0; i < n; ++i) d[i] = v[i];
}

void Param::SetArrayIntValue(int* pd, int nsize)
{
	assert(m_ntype == Param_ARRAY_INT);
	auto& d = val<std::vector<int> >();
	assert(d.size() == nsize);
	int n = MIN(d.size(), nsize);
	for (int i = 0; i < n; ++i) d[i] = pd[i];
}

void Param::SetArrayDoubleValue(const std::vector<double>& v)
{
	assert(m_ntype == Param_ARRAY_DOUBLE);
	auto& d = val<std::vector<double> >();
	if (d.empty()) { d = v; m_nsize = v.size(); return; }
	assert(d.size() == v.size());
	int n = MIN(d.size(), v.size());
	for (int i = 0; i < n; ++i) d[i] = v[i];
}

//=============================================================================
ParamBlock::ParamBlock(void)
{
	m_currentGroup = -1;
}

//-----------------------------------------------------------------------------
ParamBlock::~ParamBlock(void)
{
	Clear();
}

//-----------------------------------------------------------------------------
void ParamBlock::Clear()
{
	for (int i = 0; i < m_Param.size(); ++i) delete m_Param[i];
	m_Param.clear();
	m_currentGroup = -1;
	m_pg.clear();
}

//-----------------------------------------------------------------------------
ParamBlock::ParamBlock(const ParamBlock &b)
{
	m_pg = b.m_pg;
	m_currentGroup = b.m_currentGroup;
	for (int i = 0; i < b.m_Param.size(); ++i)
	{
		const Param& s = b[i];
		Param* p = new Param(s);
		m_Param.push_back(p);
	}

	// restore watched parameters
	for (int i = 0; i < b.m_Param.size(); ++i)
	{
		const Param& s = b[i];
		Param& d = *m_Param[i];
		if (s.m_watch)
		{
			Param* w = Find(s.GetShortName()); assert(w);
			d.SetWatchVariable(w);
		}
	}
}

ParamBlock& ParamBlock::operator =(const ParamBlock &b)
{
	Clear();
	m_pg = b.m_pg;
	m_currentGroup = b.m_currentGroup;
	for (int i = 0; i < b.m_Param.size(); ++i)
	{
		const Param& s = b[i];
		Param* p = new Param(s);
		m_Param.push_back(p);
	}

	// restore watched parameters
	for (int i = 0; i < b.m_Param.size(); ++i)
	{
		const Param& s = b[i];
		Param& d = *m_Param[i];
		if (s.m_watch)
		{
			Param* w = Find(s.m_watch->GetShortName()); assert(w);
			d.SetWatchVariable(w);
		}
	}

	return *this;
}

void ParamBlock::Copy(const ParamBlock& b)
{
	assert(b.m_Param.size() == m_Param.size());
	for (int i = 0; i < b.m_Param.size(); ++i)
	{
		const Param& s = b[i];
		Param& p = *m_Param[i];
		p = s;
	}
}


//-----------------------------------------------------------------------------
void ParamBlock::ClearParamGroups()
{
	m_pg.clear();
	m_currentGroup = -1;
}

//-----------------------------------------------------------------------------
int ParamBlock::SetActiveGroup(const char* szgroup)
{
	if (szgroup == nullptr) m_currentGroup = -1;
	else
	{
		m_currentGroup = -1;
		for (size_t i = 0; i < m_pg.size(); ++i)
		{
			if (strcmp(m_pg[i], szgroup) == 0)
			{
				m_currentGroup = i;
			}
		}
		if (m_currentGroup == -1)
		{
			m_currentGroup = (int)m_pg.size();
			m_pg.push_back(szgroup);
		}
	}
	return m_currentGroup;

}

bool ParamBlock::SetActiveGroup(int n)
{
	if (n < 0) { m_currentGroup = -1; return true; }
	if (n >= m_pg.size()) return false;
	m_currentGroup = n;
	return true;
}

int ParamBlock::GetActiveGroup()
{
	return m_currentGroup;
}

int ParamBlock::ParameterGroups() const
{
	return (int)m_pg.size();
}

const char* ParamBlock::GetParameterGroupName(int i)
{
	if ((i < 0) || (i >= m_pg.size())) return nullptr;
	return m_pg[i];
}

//-----------------------------------------------------------------------------

void ParamContainer::Save(OArchive& ar)
{
	// write the parameters
	for (int i=0; i<m_Param.Size(); ++i)
	{
		Param& p = m_Param[i];
		ar.BeginChunk(CID_PARAM);
		{
			SaveParam(p, ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void ParamContainer::SaveParam(Param &p, OArchive& ar)
{
	int nid = p.GetParamID();
	int ntype = (int) p.GetParamType();

	ar.WriteChunk(CID_PARAM_ID, nid);
	ar.WriteChunk(CID_PARAM_TYPE, ntype);
	ar.WriteChunk(CID_PARAM_CHECKED, p.IsChecked());
	ar.WriteChunk(CID_PARAM_NAME, p.GetShortName());
	ar.WriteChunk(CID_PARAM_LC, p.GetLoadCurveID());
	ar.WriteChunk(CID_PARAM_STATE, p.GetState());

	switch (ntype)
	{
	case Param_FLOAT : { double g = p.GetFloatValue(); ar.WriteChunk(CID_PARAM_VALUE, g); } break;
	case Param_INT   : { int    n = p.GetIntValue  (); ar.WriteChunk(CID_PARAM_VALUE, n); } break;
	case Param_CHOICE: { int    n = p.GetIntValue  (); ar.WriteChunk(CID_PARAM_VALUE, n); } break;
	case Param_BOOL  : { bool   b = p.GetBoolValue (); ar.WriteChunk(CID_PARAM_VALUE, b); } break;
	case Param_VEC3D : { vec3d  v = p.GetVec3dValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_VEC2I : { vec2i  v = p.GetVec2iValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_VEC2D : { vec2d  v = p.GetVec2dValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_MAT3D : { mat3d  v = p.GetMat3dValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_MAT3DS: { mat3ds v = p.GetMat3dsValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_STRING: { std::string s = p.GetStringValue(); ar.WriteChunk(CID_PARAM_VALUE, s); } break;
	case Param_URL   : { std::string s = p.GetURLValue(); ar.WriteChunk(CID_PARAM_VALUE, s); } break;
	case Param_MATH  : { std::string s = p.GetMathString(); ar.WriteChunk(CID_PARAM_VALUE, s); } break;
	case Param_COLOR : { GLColor c = p.GetColorValue(); ar.WriteChunk(CID_PARAM_VALUE, c); } break;
	case Param_STD_VECTOR_INT: { std::vector<int> v = p.GetVectorIntValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_STD_VECTOR_DOUBLE: { std::vector<double> v = p.GetVectorDoubleValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_STD_VECTOR_VEC2D : { std::vector<vec2d> v = p.GetVectorVec2dValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_ARRAY_INT   : { std::vector<int   > v = p.GetArrayIntValue   (); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_ARRAY_DOUBLE: { std::vector<double> v = p.GetArrayDoubleValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void ParamContainer::Load(IArchive &ar)
{
//	TRACE("ParamContainer::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == CID_PARAM)
		{
			LoadParam(ar);
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
void ParamContainer::LoadParam(IArchive& ar)
{
	int npid = -1;
	bool b;
	Param p;
	int ntype = -1;
	int lcid = -1;
	int state = 0;
	string paramName;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_PARAM_ID: ar.read(npid); break;
		case CID_PARAM_CHECKED: ar.read(b); p.SetChecked(b); break;
		case CID_PARAM_NAME: ar.read(paramName); break;
		case CID_PARAM_LC: ar.read(lcid); p.SetLoadCurveID(lcid); break;
		case CID_PARAM_STATE: ar.read(state); break;
		case CID_PARAM_TYPE: 
			ar.read(ntype); 
			switch (ntype)
			{
			case Param_INT   : p.SetParamType(Param_INT  ); break;
			case Param_FLOAT : p.SetParamType(Param_FLOAT); break;
			case Param_BOOL  : p.SetParamType(Param_BOOL ); break;
			case Param_VEC3D : p.SetParamType(Param_VEC3D); break;
			case Param_VEC2I : p.SetParamType(Param_VEC2I); break;
			case Param_VEC2D : p.SetParamType(Param_VEC2D); break;
			case Param_MAT3D : p.SetParamType(Param_MAT3D); break;
			case Param_MAT3DS: p.SetParamType(Param_MAT3DS); break;
			case Param_CHOICE: p.SetParamType(Param_CHOICE); break;
			case Param_STRING: p.SetParamType(Param_STRING); break;
			case Param_URL   : p.SetParamType(Param_URL); break;
			case Param_MATH  : p.SetParamType(Param_MATH); break;
			case Param_COLOR : p.SetParamType(Param_COLOR); break;
			case Param_CURVE_OBSOLETE: p.SetParamType(Param_FLOAT); break;
			case Param_STD_VECTOR_INT: p.SetParamType(Param_STD_VECTOR_INT); break;
			case Param_STD_VECTOR_DOUBLE: p.SetParamType(Param_STD_VECTOR_DOUBLE); break;
			case Param_STD_VECTOR_VEC2D : p.SetParamType(Param_STD_VECTOR_VEC2D); break;
			case Param_ARRAY_INT   : p.SetParamType(Param_ARRAY_INT); break;
			case Param_ARRAY_DOUBLE: p.SetParamType(Param_ARRAY_DOUBLE); break;
			}
			break;
		case CID_PARAM_VALUE:
			switch (ntype)
			{
			case Param_CHOICE: { int    n; ar.read(n); p.SetIntValue(n); } break;
			case Param_INT   : { int    n; ar.read(n); p.SetIntValue(n); } break;
			case Param_FLOAT : { double g; ar.read(g); p.SetFloatValue (g); } break;
			case Param_BOOL  : { bool   b; ar.read(b); p.SetBoolValue  (b); } break;
			case Param_VEC3D : { vec3d  v; ar.read(v); p.SetVec3dValue (v); } break;
			case Param_VEC2I : { vec2i  v; ar.read(v); p.SetVec2iValue (v); } break;
			case Param_VEC2D : { vec2d  v; ar.read(v); p.SetVec2dValue (v); } break;
			case Param_MAT3D : { mat3d  m; ar.read(m); p.SetMat3dValue (m); } break;
			case Param_MAT3DS: { mat3ds m; ar.read(m); p.SetMat3dsValue (m); } break;
			case Param_STRING: { std::string s; ar.read(s); p.SetStringValue(s); } break;
			case Param_URL   : { std::string s; ar.read(s); p.SetURLValue(s); } break;
			case Param_MATH  : { std::string s; ar.read(s); p.SetMathString(s); } break;
			case Param_COLOR : { GLColor c; ar.read(c); p.SetColorValue(c); break; }
			case Param_STD_VECTOR_INT: { std::vector<int> v; ar.read(v); p.SetVectorIntValue(v); break; }
			case Param_STD_VECTOR_DOUBLE: { std::vector<double> v; ar.read(v); p.SetVectorDoubleValue(v); break; }
			case Param_STD_VECTOR_VEC2D : { std::vector<vec2d> v; ar.read(v); p.SetVectorVec2dValue(v); break; }
			case Param_ARRAY_INT   : { std::vector<int> v; ar.read(v); p.SetArrayIntValue(v); break; }
			case Param_ARRAY_DOUBLE: { std::vector<double> v; ar.read(v); p.SetArrayDoubleValue(v); break; }
			case Param_CURVE_OBSOLETE:
				{
					// This is obsolete but remains for backward compatibility.
					LoadCurve lc;
					lc.Load(ar);

					// TODO: Assign load curve to model
//					if (lc.Size() > 0) p.SetLoadCurve(lc);
				}
				break;
			default:
				assert(false);
			}
			break;
		case CID_LOAD_CURVE:
			{
				LoadCurve lc;
				lc.Load(ar);

				// In FBS2, load controllers are stored on the FSModel. Assign load curve to model
				AssignLoadCurve(p, lc);
			}
			break;
		}
		ar.CloseChunk();
	}

	// For old versions, call LoadParam which could be overriden by classes if the order of parameters has changed
	if (ar.Version() < 0x00020000)
	{
		p.SetParamID(npid);
		LoadParam(p);
	}
	else
	{
		// Now, we'll try to find the correct parameter in the class' parameter list. 
		Param* param = nullptr;

		// As of FEBio Studio 2, the parameter name is stored instead of its ID. To safeguard against
		// changes in the parameter definition of a class, we now use the parameter name
		// as the primary mechanism for finding the parameter. 
		if (paramName.empty() == false)
			param = GetParam(paramName.c_str());

		// Of course, older files may not have stored the parameter name, so let's try
		// the param ID, which should be an index into the container's parameter list. 
		if ((param == nullptr) && (npid >= 0))
			param = GetParamPtr(npid);

		// if we find the parameter, let's try to process it
		if (param)
		{
			// we only map the modified state
			if (state & Param_State::Param_MODIFIED) param->SetModified(true);

			// some boolean parameters are replaced with int parameters (e.g. laugon contact parameters)
			if ((p.GetParamType() == Param_BOOL) && (param->GetParamType() == Param_INT))
			{
				bool b = p.GetBoolValue();
				p.SetParamType(Param_INT);
				p.SetIntValue(b ? 1 : 0);
			}

			bool var = param->IsVariable();
			if (param->GetParamType() == p.GetParamType())
			{
				*param = p;
			}
			else
			{
				if (param->IsVariable())
				{
					// We used to store array parameters as vectors, but
					// now we can store array parameters correctly. So, we
					// need to check for that and convert. 
					if ((param->GetParamType() == Param_ARRAY_DOUBLE) && (p.GetParamType() == Param_VEC3D))
					{
						assert(param->GetArraySize() == 3);
						vec3d v = p.GetVec3dValue();
						std::vector<double> d{ v.x, v.y, v.z };
						param->SetArrayDoubleValue(d);
					}
					else {
						param->SetParamType(p.GetParamType());
						*param = p;
					}
				}
				else if ((param->GetParamType() == Param_STRING) && (p.GetParamType() == Param_MATH))
				{
					std::string smath = p.GetMathString();
					param->SetStringValue(smath);
				}
				else
				{
					// TODO: print some type of error message that parameters are mismatched	
					const char* szname = (paramName.empty() ? "(unknown)" : paramName.c_str());
					ar.log("Failed to map parameter \"%s\" due to type mismatch.", szname);
				}
			}
		}
		else
		{
			// TODO: print some type of error message that parameters are mismatched
			const char* szname = (paramName.empty() ? "(unknown)" : paramName.c_str());
			ar.log("Failed to map parameter: %s (%s)", szname, CCallStack::GetCurrentCall().c_str());
		}
	}
}

void ParamContainer::LoadParam(const Param& p)
{
	Param* param = GetParamPtr(p.GetParamID());
	if (param) *param = p;
}

void ParamContainer::CopyParams(const ParamContainer& pc)
{
	assert(pc.Parameters() == Parameters());
	for (int i = 0; i < Parameters(); ++i)
	{
		Param& pi = GetParam(i);
		const Param& pj = pc.GetParam(i);
		assert(pi.GetParamType() == pj.GetParamType());
		pi = pj;
	}
}

void ParamContainer::MapParams(const ParamContainer& pc)
{
	for (int i = 0; i < pc.Parameters(); ++i)
	{
		const Param& pi = pc.GetParam(i);
		Param* p = GetParam(pi.GetShortName());
		if (p && (p->GetParamType() == pi.GetParamType()))
		{
			*p = pi;
		}
	}
}

void ParamContainer::AssignLoadCurve(Param& p, LoadCurve& lc)
{
	assert(false);
}
