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

#pragma once
#include <vector>
#include <list>
#include <string>
#include <cstring>

//using namespace std;

using std::vector;
using std::list;

class FSModel;
class GMeshObject;
class FSMesh;

class LSDYNAModel
{
public:
	struct ELEMENT_SHELL
	{
		int		eid;
		int		pid;
		int		n[4];
		double	h[4];

		int		tag;

		ELEMENT_SHELL() { h[0] = h[1] = h[2] = h[3] = 0; }
	};

	struct ELEMENT_SOLID
	{
		int eid;	// element id
		int pid;	// part id
		int n[8];	// nodal numbers

		int		tag;
	};

    struct DOMAIN_SHELL
    {
        int       pid;
        double    h[4];
        
        DOMAIN_SHELL() { h[0] = h[1] = h[2] = h[3] = 0; }
    };

	struct ELEMENT_DISCRETE
	{
		int eid;
		int pid;
		int n1, n2;
	};

	struct NODE
	{
		NODE() { id = n = 0; x = y = z = 0; }
		int id;
		int n;
		double	x, y, z;
	};

	struct PART
	{
		int		pid;	// the part ID
		int		mid;	// the material ID
		int		sid;	// section ID
		char	szname[81];	// name of part

		int		lid;	// local ID of corresponding GPart
	};

	class MATERIAL
	{
    public:
        MATERIAL() { mid = 0; szname[0] = 0; }
        MATERIAL(const MATERIAL& s)
        { mid = s.mid; strcpy(szname, s.szname); }
        MATERIAL& operator = (const MATERIAL& s)
        { mid = s.mid; strcpy(szname, s.szname); return *this; }
    public:
		virtual void f() {};
		int       mid;
		char      szname[256];
	};

	class MAT_ELASTIC : public MATERIAL
	{
    public:
        MAT_ELASTIC() { ro = e = pr = da = db = 0; }
        MAT_ELASTIC(const MAT_ELASTIC& s)
        { ro = s.ro; e = s.e; pr = s.pr; da = s.da; db = s.db; }
        MAT_ELASTIC& operator = (const MAT_ELASTIC& s)
        { ro = s.ro; e = s.e; pr = s.pr; da = s.da; db = s.db; return *this; }
    public:
		double    ro;
		double    e;
		double    pr;
		double    da;
		double    db;
	};

	class MAT_VISCOELASTIC : public MATERIAL
	{
    public:
        MAT_VISCOELASTIC() { ro = bulk = g0 = gi = beta = 0; }
        MAT_VISCOELASTIC(const MAT_VISCOELASTIC& s)
        { ro = s.ro; bulk = s.bulk; g0 = s.g0; gi = s.gi; beta = s.beta; }
        MAT_VISCOELASTIC& operator = (const MAT_VISCOELASTIC& s)
        { ro = s.ro; bulk = s.bulk; g0 = s.g0; gi = s.gi; beta = s.beta; return *this; }
    public:
		double    ro;
		double    bulk;
		double    g0;
		double    gi;
		double    beta;
	};

	class MAT_KELVIN_MAXWELL_VISCOELASTIC : public MATERIAL
	{
    public:
        MAT_KELVIN_MAXWELL_VISCOELASTIC() { ro = bulk = g0 = gi = dc = fo = so = 0; }
        MAT_KELVIN_MAXWELL_VISCOELASTIC(const MAT_KELVIN_MAXWELL_VISCOELASTIC& s)
        { ro = s.ro; bulk = s.bulk; g0 = s.g0; gi = s.gi; dc = s.dc; fo = s.fo; so = s.so; }
        MAT_KELVIN_MAXWELL_VISCOELASTIC& operator = (const MAT_KELVIN_MAXWELL_VISCOELASTIC& s)
        { ro = s.ro; bulk = s.bulk; g0 = s.g0; gi = s.gi; dc = s.dc; fo = s.fo; so = s.so; return *this; }
    public:
		double    ro;
		double    bulk;
		double    g0;
		double    gi;
		double    dc;
		double    fo;
		double    so;
	};

	class MAT_RIGID : public MATERIAL
	{
    public:
        MAT_RIGID() {
            ro = e = pr = n = m = alias = cmo = a1 = a2 = a3 = v1 = v2 = v3 = 0;
            con1 = con2 = 0;
        }
        MAT_RIGID(const MAT_RIGID& s) {
            ro = s.ro; e = s.e; pr = s.pr; n = s.n; couple = s.couple; m = s.m; alias = s.alias; cmo = s.cmo;
            con1 = s.con1; con2 = s.con2; a1 = s.a1; a2 = s.a2; a3 = s.a3; v1 = s.v1; v2 = s.v2; v3 = s.v3;
        }
        MAT_RIGID& operator = (const MAT_RIGID& s)  {
            ro = s.ro; e = s.e; pr = s.pr; n = s.n; couple = s.couple; m = s.m; alias = s.alias; cmo = s.cmo;
            con1 = s.con1; con2 = s.con2; a1 = s.a1; a2 = s.a2; a3 = s.a3; v1 = s.v1; v2 = s.v2; v3 = s.v3;
            return *this;
        }
    public:
		double    ro;
		double    e;
		double    pr;
		double    n;
		double    couple;
		double    m;
		double    alias;
		double    cmo;
		int       con1;
		int       con2;
		double    a1;
		double    a2;
		double    a3;
		double    v1;
		double    v2;
		double    v3;
	};

	class MAT_SPRING_NONLINEAR_ELASTIC : public MATERIAL
	{
	public:
		MAT_SPRING_NONLINEAR_ELASTIC() { lcd = -1; }

	public:
		int	lcd;
	};

	class SET_SEGMENT_TITLE
	{
	public:
		struct FACE
		{
			int	n[4];
		};

	public:
		SET_SEGMENT_TITLE() { m_szname[0] = 0; }
		SET_SEGMENT_TITLE(const SET_SEGMENT_TITLE& s) { m_nsid = s.m_nsid; m_face = s.m_face; strcpy(m_szname, s.m_szname); }
		SET_SEGMENT_TITLE& operator = (const SET_SEGMENT_TITLE& s) { m_nsid = s.m_nsid; m_face = s.m_face; strcpy(m_szname, s.m_szname); return *this; }

	public:
		int				m_nsid;	// segment ID
		vector<FACE>	m_face;	// face list
		char			m_szname[256];
	};

	class SET_NODE_LIST_TITLE
	{
	public:
		SET_NODE_LIST_TITLE() {}

	public:
		int				m_nid = -1;
		vector<int>		m_nodelist;
		std::string		m_name;
	};

	class LOAD_CURVE
	{
	public:
		LOAD_CURVE() {}

	public:
		int		m_lcid = 0;
		int		m_sidr = 0;
		float	m_sfa = 1.f;
		float	m_sfo = 1.f;
		float	m_offa = 0.f;
		float	m_offo = 0.f;
		int		m_dattyp = 0;
		std::string	m_name;
		std::vector< std::pair<float, float> >	m_pt;
	};

	class PARAMETER
	{
	public:
		PARAMETER(const std::string& name, double v) : m_name(name), m_val(v) {}
		std::string	m_name;
		double		m_val;
	};

public:
	LSDYNAModel();

	void clear();

	int nodes() const { return (int) m_node.size(); }
	void addNode(const NODE& node) { m_node.push_back(node); }

	void addSolidElement(const ELEMENT_SOLID& el) { m_solid.push_back(el); }

	void addShellElement(const ELEMENT_SHELL& el) { m_shell.push_back(el); }

    void addShellDomain(const DOMAIN_SHELL& ds) { m_dshell.push_back(ds); }

	void addDiscrete(const ELEMENT_DISCRETE& el) { m_discrete.push_back(el); }
    
	int parts() const { return (int) m_part.size(); }
	void addPart(const PART& p) { m_part.push_back(p); }

	void addMaterial(MATERIAL* m) { m_Mat.push_back(m); }

	void addSetSegmentTitle(const SET_SEGMENT_TITLE& s) { m_set.push_back(s); }

	void addNodeList(const SET_NODE_LIST_TITLE& nl) { m_nodelist.push_back(nl); }

	void addLoadCurve(const LOAD_CURVE& lc) { m_lc.push_back(lc); }

	void addParameter(const std::string& name, double v) { m_param.emplace_back(PARAMETER(name, v)); }

public:
	bool BuildModel(FSModel& fem);

	GMeshObject* TakeObject() { GMeshObject* po = m_po; m_po = 0; return po; }

	int FindNode(int id, list<NODE>::iterator& pn);

	int FindFace(int n[4]);

    int FindShellDomain(int pid);
    
	void UpdateMesh(FSMesh& mesh);

	void allocData(int N) { m_Data.assign(N, vector<double>(2)); }
	double& NodeData(int i, int j) { return m_Data[i][j]; }

protected:
	bool BuildFEMesh(FSModel& fem);
	bool BuildMaterials(FSModel& fem);
	bool BuildLoadCurves(FSModel& fem);
	bool BuildParameters(FSModel& fem);
	bool BuildDiscrete(FSModel& fem);
	void BuildNLT();

	int NodeIndex(int nodeId) { return m_NLT[nodeId - m_off]; }

public:
	vector<ELEMENT_SOLID>		m_solid;
	vector<ELEMENT_SHELL>		m_shell;
    vector<DOMAIN_SHELL>        m_dshell;
    vector<ELEMENT_DISCRETE>	m_discrete;
	vector<NODE>				m_node;
	vector<PART>				m_part;
	list<MATERIAL*>       		m_Mat;
	list<SET_SEGMENT_TITLE>		m_set;
	list<SET_NODE_LIST_TITLE>	m_nodelist;
	vector<LOAD_CURVE>	m_lc;
	vector<PARAMETER>	m_param;
	GMeshObject*	m_po; 
	vector<int>		m_iFace;
	vector<int*>	m_pFace;
	vector<int>		m_nFace;
	vector< vector<double> >	m_Data;	// nodal data

	// node lookup table
	vector<int> m_NLT;
	int m_off;

	// load curve lookup table
	vector<int> m_LCT;
	int m_lct_off;
};
