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
#include "Serializable.h"
#include "color.h"
#include <MathLib/mat3d.h>
#include "LoadCurve.h"
#include <vector>
#include <string.h>
//using namespace std;

using std::vector;

// parameter types
enum Param_Type {
	Param_UNDEF,
	Param_INT,
	Param_FLOAT,
	Param_BOOL,
	Param_VEC3D,
	Param_STRING,
	Param_MATH,
	Param_COLOR,
	Param_MAT3D,
	Param_MAT3DS,
	Param_VEC2I,
	Param_CHOICE = 0x0020		// like INT but imported/exported as one-based numbers
};

// obsolete parameter types
#define Param_CURVE_OBSOLETE		0x0010		//-> obsolete

// parameter states
// (Note that the paramete state is not serialized, so it can be modified without affecting compatibility)
// (Comments about import and export of parameters only relate to the FEBio IO classes)
enum Param_State {
	Param_HIDDEN    = 0,

	Param_EDITABLE   = 0x01,	// Parameter will be shown and edited in standard GUI components
	Param_READWRITE  = 0x02,	// Parameter will be imported/exported from FEBio files
	Param_VISIBLE    = 0x04,	// Parameter will be displayed in model tree
	Param_PERSISTENT = 0x08,	// if false, parameter will only be shown in top-level (only for materials)

	Param_ALLFLAGS = 0x0F
};

class ParamBlock;

//-----------------------------------------------------------------------------
// This class implements a parameter. Parameters are used by classes that
// require user interaction. The parameter interface facilitates the interaction
// with the user.
//
class Param
{
public:
	Param();
	~Param();

	//! clear allocated data
	void clear();

	//! copy constructor
	Param(const Param& p);

	//! assignment operator
	Param& operator = (const Param& p);

	// constructors
	explicit Param(int n, Param_Type ntype, const char* szb, const char* szn);
	explicit Param(int n, const char* szb, const char* szn = 0);
	explicit Param(double d, const char* szb, const char* szn = 0);
	explicit Param(double d, const char* szunit = 0, const char* szb = 0, const char* szn = 0);
	explicit Param(bool b, const char* szb, const char* szn = 0);
	explicit Param(vec2i v, const char* szb, const char* szn = 0);
	explicit Param(vec3d v, const char* szb, const char* szn = 0);
	explicit Param(mat3d v, const char* szb, const char* szn = 0);
	explicit Param(mat3ds v, const char* szb, const char* szn = 0);
	explicit Param(int n, const char* szi, int idx, const char* szb, const char* szn = 0);
	explicit Param(double d, const char* szi, int idx, const char* szb, const char* szn = 0);
	explicit Param(double d, const char* szi, int idx, const char* szunit = 0, const char* szb = 0, const char* szn = 0);
	explicit Param(GLColor c, const char* szb, const char* szn = 0);
	Param(const std::string& val, const char* szb, const char* szn = 0);

	void SetParamType(Param_Type t);
	Param_Type GetParamType() const { return m_ntype; }
	int GetParamID() const { return m_nID; }

	void SetParamID(int nid) { m_nID = nid; }

	void SetLoadCurve();
	void SetLoadCurve(const FELoadCurve& lc);
	FELoadCurve* GetLoadCurve() const { return m_plc; }
	void DeleteLoadCurve();
	FELoadCurve* RemoveLoadCurve();

	const char* GetShortName() const { return m_szbrev; }
	const char* GetLongName () const { return m_szname; }
	const char* GetEnumNames() const { return m_szenum; }
	const char* GetIndexName() const { return m_szindx; }

	const char* GetEnumName(int n) const;

	Param* SetEnumNames(const char* sz);
	Param* CopyEnumNames(const char* sz);
	
	int GetIndexValue() const { return m_nindx; }

	template <typename T> T& val() { return *((T*) m_pd); }
	template <typename T> T val() const { return *((T*) m_pd); }

	void SetFloatValue (double g) {assert(m_ntype == Param_FLOAT ); val<double>() = g; }
	void SetIntValue   (int    a) {assert((m_ntype == Param_INT)||(m_ntype == Param_CHOICE)); val<int>  () = a; }
	void SetBoolValue  (bool   b) {assert(m_ntype == Param_BOOL  ); val<bool> () = b; }
	void SetVec3dValue (const vec3d& v) {assert(m_ntype == Param_VEC3D ); val<vec3d>() = v; }
	void SetVec2iValue (const vec2i& v) { assert(m_ntype == Param_VEC2I); val<vec2i>() = v; }
	void SetMat3dValue (const mat3d& v) { assert(m_ntype == Param_MAT3D); val<mat3d>() = v; }
	void SetMat3dsValue(const mat3ds& v){ assert(m_ntype == Param_MAT3DS); val<mat3ds>() = v; }
	void SetStringValue(const std::string& v) {assert(m_ntype == Param_STRING); val<std::string>() = v; }
	void SetMathString (const std::string& v) { assert(m_ntype == Param_MATH); val<std::string>() = v; }
	void SetColorValue(const GLColor& c) { assert(m_ntype == Param_COLOR); val<GLColor>() = c; }

	double GetFloatValue () const {assert(m_ntype == Param_FLOAT ); return val<double>(); }
	int    GetIntValue   () const {assert((m_ntype == Param_INT)||(m_ntype == Param_CHOICE)); return val<int>  (); }
	bool   GetBoolValue  () const {assert(m_ntype == Param_BOOL  ); return val<bool> (); }
	vec3d  GetVec3dValue () const {assert(m_ntype == Param_VEC3D ); return val<vec3d>(); }
	vec2i  GetVec2iValue () const { assert(m_ntype == Param_VEC2I); return val<vec2i>(); }
	mat3d  GetMat3dValue () const {assert(m_ntype == Param_MAT3D); return val<mat3d>(); }
	mat3ds GetMat3dsValue () const {assert(m_ntype == Param_MAT3DS); return val<mat3ds>(); }
	std::string GetStringValue() const { assert(m_ntype == Param_STRING); return val<std::string>(); }
	std::string GetMathString() const { assert(m_ntype == Param_MATH); return val<std::string>(); }
	GLColor GetColorValue() const { assert(m_ntype == Param_COLOR); return val<GLColor>(); }

	const char* GetUnit() const { return m_szunit; }
	void SetUnit(const char* szunit) { m_szunit = szunit; }

	int GetState() const { return m_nstate; }
	Param* SetState(int s) { m_nstate = s; return this; }

	bool IsEditable () const { return (m_nstate & Param_EDITABLE) != 0; }
	bool IsReadWrite() const { return (m_nstate & Param_READWRITE) != 0; }
	bool IsVisible  () const { return (m_nstate & Param_VISIBLE) != 0; }
	bool IsPersistent() const { return (m_nstate & Param_PERSISTENT) != 0; }

	Param* SetPersistent(bool b) { if (b) m_nstate |= Param_PERSISTENT; else m_nstate &= ~Param_PERSISTENT; return this; }

	int GetOffset() const { return m_offset; }
	Param* SetOffset(int n) { m_offset = n; return this; }

	Param* MakeVariable(bool b) { m_isVariable = b; return this; }
	bool IsVariable() const { return m_isVariable; }

	bool UseRange() const { return m_floatRange; }

	double GetFloatMax() const { return m_fmax; }
	double GetFloatMin() const { return m_fmin; }
	double GetFloatStep() const { return m_fstep; }

	int GetIntMax() const { return (int) m_fmax; }
	int GetIntMin() const { return (int) m_fmin; }
	int GetIntStep() const { return (int) m_fstep; }

	void SetFloatRange(double fmin, double fmax, double fstep = 0.0);
	void SetIntRange(int imin, int imax, int istep = 1);

	void SetCheckable(bool b);
	bool IsCheckable() const;
	void SetChecked(bool b);
	bool IsChecked() const;

	unsigned int GetFlags() const { return m_flags; }
	void SetFlags(unsigned int flags) { m_flags = flags; }

protected:
	int				m_nID;		// parameter ID
	Param_Type		m_ntype;	// parameter type
	const char*		m_szunit;	// scientific unit (see paramunit.h)
	int				m_nstate;	// parameter state

	unsigned int	m_flags;	// FEBio parameter flags
	
	bool			m_checkable;
	bool			m_checked;

	void*			m_pd;		// pointer to actual value
	FELoadCurve*	m_plc;		// load curve for parameter
	int				m_offset;	// offset for output (choice parameters only)

	bool		m_floatRange;
	double		m_fmin, m_fmax, m_fstep;

	const char*	m_szbrev;	// short name of parameter
	const char*	m_szname;	// long name of parameter
	char*		m_szenum;	// enum values for Choice params (zero escaped)
    
    const char* m_szindx;   // name of index
    int			m_nindx;    // index value
	bool		m_bcopy;	// copy enum values

	bool		m_isVariable;	// allow the type of the parameter to change

	friend class ParamBlock;
};

//-----------------------------------------------------------------------------
// The param block class manages a list of parameters. 
//
class ParamBlock
{
public:
	ParamBlock(void);
	~ParamBlock(void);

	ParamBlock(const ParamBlock& b);
	ParamBlock& operator = (const ParamBlock& b);

	Param* AddIntParam(int n, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(n, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddChoiceParam(int n, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(n, Param_CHOICE, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddDoubleParam(double d, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(d, 0, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddScienceParam(double d, const char* szunit, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(d, szunit, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddBoolParam(bool b, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(b, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddVecParam(vec3d v, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(v, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddVec2iParam(vec2i v, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(v, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddMat3dParam(mat3d v, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(v, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddMat3dsParam(mat3ds v, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(v, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddIndxIntParam(int n, const char* szi, int idx, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(n, szi, idx, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}
    
	Param* AddIndxDoubleParam(double d, const char* szi, int idx, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(d, szi, idx, 0, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddStringParam(const std::string& s, const char* szb, const char* szn = 0)
	{
		int np = (int)m_Param.size();
		Param p(s, szb, szn);
		p.m_nID = np;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddMathParam(const std::string& s, const char* szb, const char* szn = 0)
	{
		int np = (int)m_Param.size();
		Param p(s, szb, szn);
		p.m_ntype = Param_MATH;
		p.m_nID = np;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddColorParam(GLColor c, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(c, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	const Param& operator [] (int n) const { return m_Param[n]; }
	Param& operator [] (int n)	{ return m_Param[n]; }
	int Size() const { return (int)m_Param.size(); }

	// returns last parameter in list
	// This pointer is valid until the parameter list changes (e.g. when a new parameter is added)
	Param* LastParam() { return (m_Param.empty() ? 0 : &m_Param[m_Param.size()-1]); } 

	void clear() { m_Param.clear(); }

	Param* Find(int nid)
	{
		int N = (int)m_Param.size();
		for (int i=0; i<N; ++i)
		{
			if (m_Param[i].m_nID == nid) return &m_Param[i];
		}
		return 0;
	}

	Param* Find(const char* sz)
	{
		int N = (int)m_Param.size();
		for (int i=0; i<N; ++i)
		{
			Param& p = m_Param[i];
			const char* szname = p.GetShortName();
			if ((szname) && (strcmp(szname, sz) == 0)) return &p;
		}
		return 0;
	}

	Param* Find(const char* sz, const char*szi, int idx)
	{
		int N = (int)m_Param.size();
		for (int i=0; i<N; ++i)
		{
			Param& p = m_Param[i];
			const char* szname = p.GetShortName();
            const char* szidx = p.GetIndexName();
			if (szidx)
			{
				int pidx = p.GetIndexValue();
				if ((szname) && (strcmp(szname, sz) == 0) && (strcmp(szidx, szi) == 0) && (pidx == idx)) return &p;
			}
		}
		return 0;
	}
    
	int         GetIntValue   (int n) const { return m_Param[n].GetIntValue(); }
	double      GetFloatValue (int n) const { return m_Param[n].GetFloatValue(); }
	bool        GetBoolValue  (int n) const { return m_Param[n].GetBoolValue(); }
	std::string GetStringValue(int n) const { return m_Param[n].GetStringValue(); }
	GLColor     GetColorValue (int n) const { return m_Param[n].GetColorValue(); }
	
	int GetIndexValue(int n) const { return m_Param[n].GetIndexValue(); }
    const char* GetIndexName(int n) { return m_Param[n].GetIndexName(); }


protected:
	vector<Param>	m_Param;
};

//-----------------------------------------------------------------------------
// classes that use parameter blocks need to derive from this class
//
class ParamContainer : public CSerializable
{
public:
	// get the number of parameters
	int Parameters() const { return m_Param.Size(); }

	// get a particular parameter
	Param& GetParam(int n)
	{
		assert((n >= 0) && ( n < m_Param.Size()));
		return m_Param[n];
	}

	// get a particular parameter
	const Param& GetParam(int n) const
	{
		assert((n >= 0) && (n < m_Param.Size()));
		return m_Param[n];
	}

	Param* GetParamPtr(int n)
	{
		if ((n >= 0) && (n < m_Param.Size())) return &m_Param[n];
		return 0;
	}


	// add a parameter to the parameter list
	Param* AddIntParam   (int    n, const char* szb = 0, const char* szn = 0) { return m_Param.AddIntParam   (n, szb, szn); }
	Param* AddChoiceParam(int    n, const char* szb = 0, const char* szn = 0) { return m_Param.AddChoiceParam(n, szb, szn); }
	Param* AddDoubleParam(double d, const char* szb = 0, const char* szn = 0) { return m_Param.AddDoubleParam(d, szb, szn); }
	Param* AddScienceParam(double d, const char* szunit, const char* szb, const char* szn = 0) { return m_Param.AddScienceParam(d, szunit, szb, szn); }
	Param* AddBoolParam(bool   b, const char* szb = 0, const char* szn = 0) { return m_Param.AddBoolParam(b, szb, szn); }
	Param* AddVecParam(vec3d  v, const char* szb = 0, const char* szn = 0) { return m_Param.AddVecParam(v, szb, szn); }
	Param* AddVec2iParam(vec2i  v, const char* szb = 0, const char* szn = 0) { return m_Param.AddVec2iParam(v, szb, szn); }
	Param* AddIndxIntParam(int n, const char* szi, int idx, const char* szb = 0, const char* szn = 0) { return m_Param.AddIndxIntParam(n, szi, idx, szb, szn); }
	Param* AddIndxDoubleParam(double d, const char* szi, int idx, const char* szb = 0, const char* szn = 0) { return m_Param.AddIndxDoubleParam(d, szi, idx, szb, szn); }
	Param* AddStringParam(const std::string& s, const char* szb = 0, const char* szn = 0) { return m_Param.AddStringParam(s, szb, szn); }
	Param* AddColorParam(GLColor c, const char* szb = 0, const char* szn = 0) { return m_Param.AddColorParam(c, szb, szn); }
	Param* AddMat3dParam(mat3d v, const char* szb = 0, const char* szn = 0) { return m_Param.AddMat3dParam(v, szb, szn); }
	Param* AddMat3dsParam(mat3ds v, const char* szb = 0, const char* szn = 0) { return m_Param.AddMat3dsParam(v, szb, szn); }
	Param* AddMathParam(const std::string& s, const char* szb = 0, const char* szn = 0) { return m_Param.AddMathParam(s, szb, szn); }

	// get a parameter from its name
	Param* GetParam(const char* sz) { return m_Param.Find(sz); }
	Param* GetParam(const char* sz, const char* szi, int idx) { return m_Param.Find(sz, szi, idx); }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void SaveParam(Param& p, OArchive& ar);
	void LoadParam(IArchive& ar);

	// In 2.0, the parameter lists for some classes
	// were redefined. This function is a mechanism for reading older files
	// Classes can read the old parameters by overriding this function and mapping the old parameter
	// to the new parameter
	virtual void LoadParam(const Param& p);

	int GetIntValue(int n) const { return m_Param[n].GetIntValue(); }
	double GetFloatValue(int n) const { return m_Param[n].GetFloatValue(); }
	bool GetBoolValue(int n)const  { return m_Param[n].GetBoolValue(); }
	vec3d GetVecValue(int n) const { return m_Param[n].GetVec3dValue(); }
	vec2i GetVec2iValue(int n) const { return m_Param[n].GetVec2iValue(); }
	FELoadCurve* GetParamLC(int n) { return m_Param[n].GetLoadCurve(); }
	int GetIndexValue(int n) const { return m_Param[n].GetIndexValue(); }
	std::string GetStringValue(int n) const { return m_Param[n].GetStringValue(); }
	GLColor GetColorValue(int n) const { return m_Param[n].GetColorValue(); }

	void SetIntValue   (int n, int    v) { m_Param[n].SetIntValue   (v); }
	void SetFloatValue (int n, double v) { m_Param[n].SetFloatValue (v); }
	void SetBoolValue  (int n, bool   v) { m_Param[n].SetBoolValue  (v); }
	void SetVecValue   (int n, const vec3d& v) { m_Param[n].SetVec3dValue(v); }
	void SetStringValue(int n, const std::string& s) { m_Param[n].SetStringValue(s); }
	void SetColorValue (int n, const GLColor& c) { m_Param[n].SetColorValue(c); }
	void Clear() { m_Param.clear(); }

public:
	ParamBlock& GetParamBlock() { return m_Param; }
	const ParamBlock& GetParamBlock() const { return m_Param; }

	void CopyParams(const ParamContainer& pc);

private:
	ParamBlock	m_Param;
};
