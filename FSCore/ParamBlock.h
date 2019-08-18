#pragma once
#include "Serializable.h"
#include <MeshTools/LoadCurve.h>
#include <vector>
#include <string.h>
using namespace std;

// parameter types
enum Param_Type {
	Param_UNDEF,
	Param_INT,
	Param_FLOAT,
	Param_BOOL,
	Param_VEC3D,
	Param_STRING,
	Param_MATH,
	Param_CHOICE = 0x0020		// like INT but imported/exported as one-based numbers
};

// obsolete parameter types
#define Param_CURVE_OBSOLETE		0x0010		//-> obsolete

// unit types for double params
enum Param_Unit
{
	Param_NONE,
	Param_LENGTH,
	Param_MASS,
	Param_TIME,
	Param_STRESS,
	Param_DENSITY,
    Param_ANGLE,
	Param_PERMEABILITY,
	Param_DIFFUSIVITY,
	Param_CONDUCTIVITY,
	Param_CAPACITY,
	Param_NOUNIT = 0xffff
};

// parameter states
// (Note that the paramete state is not serialized, so it can be modified without affecting compatibility)
// (Comments about import and export of parameters only relate to the FEBio IO classes)
enum Param_State {
	Param_HIDDEN    = 0,

	Param_EDITABLE  = 1,	// Parameter will be shown and edited in standard GUI components
	Param_READWRITE = 2,	// Parameter will be imported/exported from FEBio files

	Param_ALLFLAGS  = 0xF
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
	explicit Param(double d, Param_Unit nunit = Param_NOUNIT, const char* szb = 0, const char* szn = 0);
	explicit Param(bool b, const char* szb, const char* szn = 0);
	explicit Param(vec3d v, const char* szb, const char* szn = 0);
	explicit Param(int n, const char* szi, int idx, const char* szb, const char* szn = 0);
	explicit Param(double d, const char* szi, int idx, const char* szb, const char* szn = 0);
	explicit Param(double d, const char* szi, int idx, Param_Unit nunit = Param_NOUNIT, const char* szb = 0, const char* szn = 0);
	Param(const std::string& val, const char* szb, const char* szn = 0);

	void SetParamType(Param_Type t);
	Param_Type GetParamType() const { return m_ntype; }
	int GetParamID() const { return m_nID; }

	void SetParamID(int nid) { m_nID = nid; }

	void SetLoadCurve();
	void SetLoadCurve(const FELoadCurve& lc);
	FELoadCurve* GetLoadCurve() const { return m_plc; }

	const char* GetShortName() const { return m_szbrev; }
	const char* GetLongName () const { return m_szname; }
	const char* GetEnumNames() const { return m_szenum; }
	const char* GetIndexName() const { return m_szindx; }

	Param* SetEnumNames(const char* sz);
	Param* CopyEnumNames(const char* sz);
	
	int GetIndexValue() const { return m_nindx; }

	template <typename T> T& val() { return *((T*) m_pd); }
	template <typename T> T val() const { return *((T*) m_pd); }

	void SetFloatValue (double g) {assert(m_ntype == Param_FLOAT ); val<double>() = g; }
	void SetIntValue   (int    a) {assert((m_ntype == Param_INT)||(m_ntype == Param_CHOICE)); val<int>  () = a; }
	void SetBoolValue  (bool   b) {assert(m_ntype == Param_BOOL  ); val<bool> () = b; }
	void SetVecValue   (const vec3d& v) {assert(m_ntype == Param_VEC3D ); val<vec3d>() = v; }
	void SetStringValue(const std::string& v) {assert(m_ntype == Param_STRING); val<std::string>() = v; }
	void SetMathString (const std::string& v) { assert(m_ntype == Param_MATH); val<std::string>() = v; }

	double GetFloatValue () const {assert(m_ntype == Param_FLOAT ); return val<double>(); }
	int    GetIntValue   () const {assert((m_ntype == Param_INT)||(m_ntype == Param_CHOICE)); return val<int>  (); }
	bool   GetBoolValue  () const {assert(m_ntype == Param_BOOL  ); return val<bool> (); }
	vec3d  GetVecValue   () const {assert(m_ntype == Param_VEC3D ); return val<vec3d>(); }
	std::string GetStringValue() const { assert(m_ntype == Param_STRING); return val<std::string>(); }
	std::string GetMathString() const { assert(m_ntype == Param_MATH); return val<std::string>(); }

	Param_Unit GetUnit() const { return m_nunit; }

	int GetState() const { return m_nstate; }
	Param* SetState(Param_State s) { m_nstate = s; return this; }

	bool IsEditable () const { return (m_nstate & Param_EDITABLE) != 0; }
	bool IsReadWrite() const { return (m_nstate & Param_READWRITE) != 0; }

	int GetOffset() const { return m_offset; }
	Param* SetOffset(int n) { m_offset = n; return this; }

	void MakeVariable(bool b) { m_isVariable = b; }
	bool IsVariable() const { return m_isVariable; }

protected:
	int				m_nID;		// parameter ID
	Param_Type		m_ntype;	// parameter type
	Param_Unit		m_nunit;	// scientific unit
	int				m_nstate;	// parameter state

	void*			m_pd;		// pointer to actual value
	FELoadCurve*	m_plc;		// load curve for parameter
	int				m_offset;	// offset for output (choice parameters only)

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

	ParamBlock(ParamBlock& b);
	ParamBlock& operator = (ParamBlock& b);

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
		Param p(d, szb, szn);
		p.m_nID = ns;
		m_Param.push_back(p);
		return LastParam();
	}

	Param* AddScienceParam(double d, Param_Unit nunit, const char* szb, const char* szn = 0)
	{
		int ns = (int)m_Param.size();
		Param p(d, nunit, szb, szn);
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
		Param p(d, szi, idx, szb, szn);
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
    
	const Param& operator [] (int n) const { return m_Param[n]; }
	Param& operator [] (int n)	{ return m_Param[n]; }
	int Size() { return (int)m_Param.size(); }

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
	int Parameters() { return m_Param.Size(); }

	// get a particular parameter
	Param& GetParam(int n)
	{
		assert((n >= 0) && ( n < m_Param.Size()));
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
	Param* AddScienceParam(double d, Param_Unit nunit, const char* szb, const char* szn = 0) { return m_Param.AddScienceParam(d, nunit, szb, szn); }
	Param* AddBoolParam(bool   b, const char* szb = 0, const char* szn = 0) { return m_Param.AddBoolParam(b, szb, szn); }
	Param* AddVecParam(vec3d  v, const char* szb = 0, const char* szn = 0) { return m_Param.AddVecParam(v, szb, szn); }
	Param* AddIndxIntParam(int n, const char* szi, int idx, const char* szb = 0, const char* szn = 0) { return m_Param.AddIndxIntParam(n, szi, idx, szb, szn); }
	Param* AddIndxDoubleParam(double d, const char* szi, int idx, const char* szb = 0, const char* szn = 0) { return m_Param.AddIndxDoubleParam(d, szi, idx, szb, szn); }
	Param* AddStringParam(const std::string& s, const char* szb = 0, const char* szn = 0) { return m_Param.AddStringParam(s, szb, szn); }

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
	vec3d GetVecValue(int n) const { return m_Param[n].GetVecValue(); }
	FELoadCurve* GetParamLC(int n) { return m_Param[n].GetLoadCurve(); }
	int GetIndexValue(int n) const { return m_Param[n].GetIndexValue(); }
	std::string GetStringValue(int n) const { return m_Param[n].GetStringValue(); }

	void SetIntValue   (int n, int    v) { m_Param[n].SetIntValue   (v); }
	void SetFloatValue (int n, double v) { m_Param[n].SetFloatValue (v); }
	void SetBoolValue  (int n, bool   v) { m_Param[n].SetBoolValue  (v); }
	void SetVecValue   (int n, const vec3d& v) { m_Param[n].SetVecValue(v); }
	void SetStringValue(int n, const std::string& s) { m_Param[n].SetStringValue(s); }
	void Clear() { m_Param.clear(); }

public:
	ParamBlock& GetParamBlock() { return m_Param; }

private:
	ParamBlock	m_Param;
};
