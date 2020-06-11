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

#include "stdafx.h"
#include "ParamBlock.h"
#include <assert.h>

//-----------------------------------------------------------------------------
//! parameter constructor. Set all default values.
Param::Param()
{ 
	m_nID = -1; 
	m_ntype = Param_UNDEF; 
	m_pd = 0;
	m_plc = 0; 
	m_szbrev = m_szname = m_szenum = 0; 
	m_szunit = 0; 
	m_nstate = Param_ALLFLAGS; 
	m_szindx = 0;
	m_nindx = -1;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
//! parameter destructor.
Param::~Param()
{ 
	clear(); 
	if (m_plc) delete m_plc;
	m_plc = 0;
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
		int l = (int) strlen(sz);
		m_szenum = new char[l+1];
		strncpy(m_szenum, sz, l);
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
		case Param_MAT3D : delete ((mat3d*)m_pd); break;
		case Param_STRING: delete ((std::string*) m_pd); break;
		case Param_MATH  : delete ((std::string*) m_pd); break;
		case Param_COLOR : delete ((GLColor*)m_pd); break;
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
	case Param_MAT3D : m_pd = new mat3d; break;
	case Param_STRING: m_pd = new std::string; break;
	case Param_MATH  : m_pd = new std::string; break;
	case Param_COLOR : m_pd = new GLColor; break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void Param::SetLoadCurve()
{
	if (m_plc == 0) m_plc = new FELoadCurve;
}

//-----------------------------------------------------------------------------
void Param::SetLoadCurve(const FELoadCurve& lc)
{
	if (m_plc) delete m_plc;
	m_plc = new FELoadCurve(lc);
}

//-----------------------------------------------------------------------------
void Param::DeleteLoadCurve()
{
	if (m_plc) delete m_plc;
	m_plc = nullptr;
}

//-----------------------------------------------------------------------------
FELoadCurve* Param::RemoveLoadCurve()
{
	FELoadCurve* plc = m_plc;
	m_plc = nullptr;
	return plc;
}

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
Param::Param(const Param& p)
{
	m_nID = p.m_nID;
	m_ntype = p.m_ntype;
	m_szbrev = p.m_szbrev;
	m_szname = p.m_szname;
	m_szenum = p.m_szenum;
	m_szunit = p.m_szunit;
	m_nstate = p.m_nstate;
    m_szindx = p.m_szindx;
    m_nindx = p.m_nindx;
	m_bcopy = false;
	m_offset = p.m_offset;
	m_isVariable = p.m_isVariable;
	m_floatRange = p.m_floatRange;
	m_fmin = p.m_fmin;
	m_fmax = p.m_fmax;
	m_fstep = p.m_fstep;
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
	case Param_MAT3D : { mat3d*	 pv = new mat3d ; m_pd = pv; *pv = *((mat3d* )p.m_pd); } break;
	case Param_STRING: { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_MATH  : { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_COLOR : { GLColor* pc = new GLColor; m_pd = pc; *pc = *((GLColor*)p.m_pd); } break;
	default:
		assert(false);
	}
	if (p.m_plc) m_plc = new FELoadCurve(*p.m_plc); else m_plc = 0;
}

//-----------------------------------------------------------------------------
Param& Param::operator = (const Param& p)
{
	clear();
	m_nID = p.m_nID;
	m_ntype = p.m_ntype;
//	m_szbrev = p.m_szbrev;
//	m_szname = p.m_szname;
//	m_szenum = p.m_szenum;
//	m_szunit = p.m_szunit;
//	m_nstate = p.m_nstate;
//  m_szindx = p.m_szindx;
//  m_nindx = p.m_nindx;
	m_offset = p.m_offset;
	m_isVariable = p.m_isVariable;
//	m_checkable = p.m_checkable;
	m_checked = p.m_checked;

	switch (m_ntype)
	{
	case Param_INT   : { int*    pi = new int   ; m_pd = pi; *pi = *((int*   )p.m_pd); } break;
	case Param_CHOICE: { int*    pi = new int   ; m_pd = pi; *pi = *((int*   )p.m_pd); } break;
	case Param_FLOAT : { double* pd = new double; m_pd = pd; *pd = *((double*)p.m_pd); } break;
	case Param_BOOL  : { bool*   pb = new bool  ; m_pd = pb; *pb = *((bool*  )p.m_pd); } break;
	case Param_VEC3D : { vec3d*	 pv = new vec3d ; m_pd = pv; *pv = *((vec3d* )p.m_pd); } break;
	case Param_VEC2I : { vec2i*	 pv = new vec2i ; m_pd = pv; *pv = *((vec2i* )p.m_pd); } break;
	case Param_MAT3D : { mat3d*	 pv = new mat3d ; m_pd = pv; *pv = *((mat3d* )p.m_pd); } break;
	case Param_STRING: { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_MATH  : { std::string* ps = new std::string; m_pd = ps; *ps = *((std::string*)p.m_pd); } break;
	case Param_COLOR : { GLColor* pc = new GLColor; m_pd = pc; *pc = *((GLColor*)p.m_pd); } break;
	default:
		assert(false);
	}
	if (m_plc) delete m_plc;
	m_plc = 0;
	if (p.m_plc) m_plc = new FELoadCurve(*p.m_plc);

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
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(int n, const char* szb, const char* szn)
{
	int* pi = new int;
	*pi = n;
	m_pd = pi;
	m_ntype = Param_INT;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(double d, const char* szb, const char* szn)
{
	double* pd = new double;
	*pd = d;
	m_pd = pd;
	m_ntype = Param_FLOAT;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(double d, const char* szunit, const char* szb, const char* szn)
{
	double* pd = new double;
	*pd = d;
	m_pd = pd;
	m_ntype = Param_FLOAT;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = szunit;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(bool b, const char* szb, const char* szn)
{
	bool* pb = new bool;
	*pb = b;
	m_pd = pb;
	m_ntype = Param_BOOL;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(vec3d v, const char* szb, const char* szn)
{
	vec3d* pv = new vec3d;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_VEC3D;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}


//-----------------------------------------------------------------------------
Param::Param(vec2i v, const char* szb, const char* szn)
{
	vec2i* pv = new vec2i;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_VEC2I;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(mat3d v, const char* szb, const char* szn)
{
	mat3d* pv = new mat3d;
	*pv = v;
	m_pd = pv;
	m_ntype = Param_MAT3D;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(GLColor c, const char* szb, const char* szn)
{
	GLColor* pc = new GLColor(c);
	m_pd = pc;
	m_ntype = Param_COLOR;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

Param::Param(const std::string& val, const char* szb, const char* szn)
{
	std::string* pv = new std::string;
	*pv = val;
	m_pd = pv;
	m_ntype = Param_STRING;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_szindx = 0;
	m_nindx = -1;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//-----------------------------------------------------------------------------
Param::Param(int n, const char* szi, int idx, const char* szb, const char* szn)
{
	int* pi = new int;
	*pi = n;
	m_pd = pi;
	m_ntype = Param_INT;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szindx = szi;
    m_nindx = idx;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}
    
//-----------------------------------------------------------------------------
Param::Param(double d, const char* szi, int idx, const char* szb, const char* szn)
{
	double* pd = new double;
	*pd = d;
	m_pd = pd;
	m_ntype = Param_FLOAT;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szindx = szi;
    m_nindx = idx;
	m_szunit = 0;
	m_nstate = Param_ALLFLAGS;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}
    
//-----------------------------------------------------------------------------
Param::Param(double d, const char* szi, int idx, const char* szunit, const char* szb, const char* szn)
{
	double* pd = new double;
	*pd = d;
	m_ntype = Param_FLOAT;
	m_szbrev = szb;
	m_szname = (szn == 0 ? szb : szn);
	m_szenum = 0;
	m_szindx = szi;
    m_nindx = idx;
	m_szunit = szunit;
	m_nstate = Param_ALLFLAGS;
	m_plc = 0;
	m_bcopy = false;
	m_offset = 0;
	m_isVariable = false;
	m_floatRange = false;
	m_fmin = m_fmax = m_fstep = 0.0;
	m_checkable = false;
	m_checked = false;
}

//=============================================================================
ParamBlock::ParamBlock(void)
{
}

//-----------------------------------------------------------------------------
ParamBlock::~ParamBlock(void)
{
}

//-----------------------------------------------------------------------------
ParamBlock::ParamBlock(ParamBlock &b)
{
	m_Param = b.m_Param;
}

ParamBlock& ParamBlock::operator =(ParamBlock &b)
{
	m_Param = b.m_Param;

	return *this;
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

	switch (ntype)
	{
	case Param_FLOAT : { double g = p.GetFloatValue(); ar.WriteChunk(CID_PARAM_VALUE, g); } break;
	case Param_INT   : { int    n = p.GetIntValue  (); ar.WriteChunk(CID_PARAM_VALUE, n); } break;
	case Param_CHOICE: { int    n = p.GetIntValue  (); ar.WriteChunk(CID_PARAM_VALUE, n); } break;
	case Param_BOOL  : { bool   b = p.GetBoolValue (); ar.WriteChunk(CID_PARAM_VALUE, b); } break;
	case Param_VEC3D : { vec3d  v = p.GetVec3dValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_VEC2I : { vec2i  v = p.GetVec2iValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_MAT3D : { mat3d  v = p.GetMat3dValue(); ar.WriteChunk(CID_PARAM_VALUE, v); } break;
	case Param_STRING: { std::string s = p.GetStringValue(); ar.WriteChunk(CID_PARAM_VALUE, s); } break;
	case Param_MATH  : { std::string s = p.GetMathString(); ar.WriteChunk(CID_PARAM_VALUE, s); } break;
	case Param_COLOR : { GLColor c = p.GetColorValue(); ar.WriteChunk(CID_PARAM_VALUE, c); } break;
	default:
		assert(false);
	}

	// store the load curve if there is one
	FELoadCurve* plc = p.GetLoadCurve();
	if (plc)
	{
		ar.BeginChunk(CID_LOAD_CURVE);
		{
			plc->Save(ar);
		}
		ar.EndChunk();
	}
}


//-----------------------------------------------------------------------------
void ParamContainer::Load(IArchive &ar)
{
	TRACE("ParamContainer::Load");

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
	int npid;
	bool b;
	Param p;
	int ntype = -1;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_PARAM_ID: ar.read(npid); p.SetParamID(npid); break;
		case CID_PARAM_CHECKED: ar.read(b); p.SetChecked(b); break;
		case CID_PARAM_TYPE: 
			ar.read(ntype); 
			switch (ntype)
			{
			case Param_INT   : p.SetParamType(Param_INT  ); break;
			case Param_FLOAT : p.SetParamType(Param_FLOAT); break;
			case Param_BOOL  : p.SetParamType(Param_BOOL ); break;
			case Param_VEC3D : p.SetParamType(Param_VEC3D); break;
			case Param_VEC2I : p.SetParamType(Param_VEC2I); break;
			case Param_MAT3D : p.SetParamType(Param_MAT3D); break;
			case Param_CHOICE: p.SetParamType(Param_CHOICE); break;
			case Param_STRING: p.SetParamType(Param_STRING); break;
			case Param_MATH  : p.SetParamType(Param_MATH); break;
			case Param_COLOR : p.SetParamType(Param_COLOR); break;
			case Param_CURVE_OBSOLETE: p.SetParamType(Param_FLOAT); break;
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
			case Param_MAT3D : { mat3d  m; ar.read(m); p.SetMat3dValue (m); } break;
			case Param_STRING: { std::string s; ar.read(s); p.SetStringValue(s); } break;
			case Param_MATH  : { std::string s; ar.read(s); p.SetMathString(s); } break;
			case Param_COLOR : { GLColor c; ar.read(c); p.SetColorValue(c); break; }
			case Param_CURVE_OBSOLETE:
				{
					// This is obsolete but remains for backward compatibility.
					FELoadCurve lc;
					lc.Load(ar);
					if (lc.Size() > 0) p.SetLoadCurve(lc);
				}
				break;
			default:
				assert(false);
			}
			break;
		case CID_LOAD_CURVE:
			{
				FELoadCurve lc;
				lc.Load(ar);

				// Old versions (<2.0) defined load curves for all float parameters,
				// although initially these load curves did not have points assigned yet.
				// Since 2.0 load curves are only assigned to parameters that are time dependant
				// so we have to add this check to prevent all these load curves from being read in.
				if (lc.Size() > 0) p.SetLoadCurve(lc);
			}
			break;
		}
		ar.CloseChunk();
	}

	// For old versions, call LoadParam which could be overriden by classes if the order of parameters has changed
	if (ar.Version() < 0x00020000)
	{
		LoadParam(p);
	}
	else
	{
		Param& param = GetParam(p.GetParamID());

		// some boolean parameters are replaced with int parameters (e.g. laugon contact parameters)
		if ((p.GetParamType() == Param_BOOL) && (param.GetParamType() == Param_INT))
		{
			bool b = p.GetBoolValue();
			p.SetParamType(Param_INT);
			p.SetIntValue(b ? 1 : 0);
		}

		bool var = param.IsVariable();
		if (param.GetParamType() == p.GetParamType())
		{
			param = p;
		}
		else
		{
			if (param.IsVariable())
			{
				param.SetParamType(p.GetParamType());
				param = p;
			}
			else
			{
				// TODO: print some type of error message that parameters are mismatched	
			}
		}
		if (var) param.MakeVariable(true);
	}
}

void ParamContainer::LoadParam(const Param& p)
{
	Param* param = GetParamPtr(p.GetParamID());
	if (param) *param = p;
}
