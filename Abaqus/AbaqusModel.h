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
#include <FEMLib/FSProject.h>
#include <list>
#include <vector>
#include <map>

using std::vector;
using std::pair;
using std::list;
using std::map;
using std::string;

class AbaqusModel
{
public:
	enum { Max_Nodes = 20 };	// max nodes per element
	enum { C3D8 } ElementTypes;

	// surface types
	enum { 
		ST_ELEMENT,
		ST_NODE
	};

public:

	// Node
	struct NODE
	{
		int	id;				// nodal ID (as read from file)
		int lid;			// local index into FSMesh
		double x, y, z;		// nodal coordinates
	};
	typedef vector<NODE>::iterator Tnode_itr;

	// Element
	struct ELEMENT
	{
		int id;		// element ID (as read from file)
		int lid;	// local index into FSMesh
		int type;
		int n[Max_Nodes];
	};
	typedef vector<ELEMENT>::iterator Telem_itr;

	// Spring element
	struct SPRING_ELEMENT
	{
		int	id;
		string	n[2];
	};
	typedef list<SPRING_ELEMENT>::iterator Tspring_itr;

	class PART;

	// node set
	struct NODE_SET
	{
		string		name;
		string		instance;
		bool		binternal;
		PART*		part = nullptr;
		vector<int>	node;
	};

	// Element set
	struct ELEMENT_SET
	{
		string		name;
		string		instance;
		bool		binternal;
		PART* part = nullptr;
		vector<int>	elem;
	};

	// Surface
	struct SURFACE
	{
		string		name;
		string		instance;
		int			type = -1;
		PART* part = nullptr;
		vector<pair<string, int> >  set;
	};

	// Solid section
	struct SOLID_SECTION
	{
		string  elset;
		string	mat;
		string	orient;
		PART*	part = nullptr;
	};

	// Shell section
	struct SHELL_SECTION
	{
		string	elset;
		string	mat;
		string	orient;
		PART*	part = nullptr;
		double	m_shellThickness = 0.0;
	};

	struct Orientation
	{
		string	name;
		string	dist;
	};

	class Distribution
	{
	public:
		struct ENTRY
		{
			int		elem = -1;
			double	val[6] = { 0 };
		};

	public:
		string			name;
		vector<ENTRY>	m_data;
	};

	class Amplitude
	{
	public:
		enum AmplitudeType {
			AMP_TABULAR,
			AMP_SMOOTH_STEP
		};

	public:
		Amplitude() { m_type = 0; }

	public:
		string	m_name;
		int		m_type;
		vector<vec2d>	m_points;
	};

	class SpringSet
	{
	public:
		string name;
		vector<SPRING_ELEMENT> m_Elem;
	};

	class SPRING
	{
	public:
		string elset;
		double k;	// spring stiffness for linear spring
		LoadCurve m_lc; // for nonlinear springs
		bool nonlinear = false;
	};

	class INSTANCE;

	// part
	class PART
	{
	public:
		// constructor
		PART();

		~PART();

		PART* Clone();

	public:
		// find a node
		Tnode_itr FindNode(int nid);

		// add a node
		Tnode_itr AddNode(NODE& n);

		// add an element
		void AddElement(ELEMENT& n);

		// add a spring
		void AddSpring(SPRING& spring);

		void AddSpringSet(SpringSet& springs);

		SpringSet* FindSpringSet(const std::string& s);

		// finds a element with a particular id
		Telem_itr FindElement(int id);

		// find an element set with a particular name
		ELEMENT_SET* FindElementSet(const string& name);

		// find all element sets with a particular name
		vector<ELEMENT_SET*> FindElementSets(const string& name);

		// adds an element set
		ELEMENT_SET* AddElementSet(const string& name);

		// find a node set with a particular name
		NODE_SET* FindNodeSet(const string& name);

		// adds a node set
		NODE_SET* AddNodeSet(const string& name);

		// find a surface with a particular name
		SURFACE* FindSurface(const string& name);

		// add a surface
		SURFACE* AddSurface(const string& name);

		// add a solid section
		void AddSolidSection(const string& set, const string& mat, const string& orient);

		// add a solid section
		SHELL_SECTION& AddShellSection(const string& set, const string& mat, const string& orient);

		// number of nodes
		int Nodes() { return (int)m_Node.size(); }

		// number of elements
		int Elements() { return (int)m_Elem.size(); }

		int CountElements();

		// number of springs
		int Springs() { return (int)m_Spring.size(); }

		// build the node-look-up table
		bool BuildNLT();

		void AddOrientation(const string& name, const string& szdist);

		Orientation* FindOrientation(const string& name);

		Distribution* FindDistribution(const string& name);

	public:
		string						m_name;
		vector<NODE>				m_Node;		// list of nodes
		vector<ELEMENT>				m_Elem;		// list of elements
		list<SPRING>				m_Spring;	// spring data
		list<NODE_SET>				m_NSet;		// node sets
		list<ELEMENT_SET>			m_ESet;		// element sets
		list<SURFACE>				m_Surf;		// surfaces
		list<SpringSet>				m_SpringSet; // set of springs
		list<SOLID_SECTION>			m_Solid;	// solid sections
		list<SHELL_SECTION>			m_Shell;	// shell sections
		list<Orientation>			m_Orient;
		list<Distribution>			m_Distr;

		vector<Tnode_itr>	m_NLT;	// Node look-up table
		int					m_ioff;	// node id offset (min node id)

		INSTANCE* m_instance = nullptr;
	};

	// an instance of a part
	class INSTANCE
	{
	public:
		INSTANCE();

		void SetName(const string& name) { m_name = name; }
		const string& GetName() { return m_name; }

		void SetPartName(const string& name) { m_partName = name; }
		const string& GetPartName() { return m_partName; }

		void SetPart(PART* pg);
		PART* GetPart() { return m_part; }

		void GetTranslation(double t[3]);
		void SetTranslation(double t[3]);

		void GetRotation(double t[7]);
		void SetRotation(double t[7]);

	private:
		string	m_name;			// name of instance
		string	m_partName;		// the part this instances
		PART*	m_part = nullptr;
		double	m_trans[3] = { 0 };		// translation
		double	m_rot[7] = { 0 };		// rotation
	};

	// material types
	enum { ELASTIC, HYPERELASTIC, ANI_HYPERELASTIC };

	// material
	struct MATERIAL
	{
		string  name;
		int		mattype = -1;
		int		ntype = -1;
		int		nparam = -1;
		double	dens = 0;
		double	d[10] = { 0 };
	};

	class CLOAD
	{
	public:
		struct NSET
		{
			int ndof = -1;
			double val = 0;
			string nset;
		};

		string ampl;
		vector<NSET> nset;
	};

	// surface loads
	class DSLOAD
	{
	public:
		struct SURF
		{
			double		load;
			string		surf;
		};

	public:
		DSLOAD(){}

		void add(const string& surf, double p)
		{
			SURF s = {p, surf};
			m_surf.push_back(s);
		}

	public:
		vector<SURF>	m_surf;
		int				m_ampl = -1;
	};

	// Boundary conditions
	class BOUNDARY
	{
	public:
		struct NSET
		{
			int		ndof[2] = { -1, -1 }; // min, max dof numbers
			double	val = 0.0;
			string	nset;
		};

	public:
		BOUNDARY(){}

		void add(NSET& ns)
		{
			m_nodeSet.push_back(ns);
		}

	public:
		vector<NSET>	m_nodeSet;
		string			m_ampl;
	};

	struct CONTACT_PAIR
	{
		string	name;
		string surf1;
		string surf2;

		double friction = 0.0;
	};

	struct TIE
	{
		string	name;
		bool	adjust = false;
		string surf1;
		string surf2;
	};


	class ASSEMBLY
	{
	public:
		ASSEMBLY();

		~ASSEMBLY();

		// add an instance
		INSTANCE* AddInstance();

		// Get the current instance
		INSTANCE* CurrentInstance() { return m_currentInstance; }

		// clear the current instance
		void ClearCurrentInstance();

		// get instance list
		list<INSTANCE*>& InstanceList() { return m_Instance; }

	public:
		std::string	m_name;
		list<INSTANCE*>	m_Instance;		// list of instances
		INSTANCE*	m_currentInstance;	// current active instance
	};

	// Steps
	class STEP
	{
	public:
		string	name;

		double	dt0;
		double	time;

		// add a pressure load
		DSLOAD* AddPressureLoad(DSLOAD& p);

		// list of pressure loads
		list<DSLOAD>& SurfaceLoadList() { return m_SLoads; }

		// add a concentrated load
		void AddCLoad(CLOAD& l) { m_CLoads.push_back(l); }

		// list of concentrated loads
		list<CLOAD>& CLoadList() { return m_CLoads; }

		// add a boundary condition
		BOUNDARY* AddBoundaryCondition(BOUNDARY& p);

		// list of boundary conditions
		list<BOUNDARY>& BoundaryConditionList() { return m_Boundary; }

	public:
		void AddContactPair(CONTACT_PAIR& cp);
		int ContactPairs() const;
		const CONTACT_PAIR& GetContactPair(int n) const;

		void AddTie(TIE& tie) { m_Tie.push_back(tie); }
		int Ties() const { return (int)m_Tie.size(); }
		const TIE& GetTie(int n) const { return m_Tie[n]; }


	private:
		list<DSLOAD>		m_SLoads;		// surface loads
		list<CLOAD>			m_CLoads;		// concentrated loads
		list<BOUNDARY>		m_Boundary;		// boundary conditions
		vector<CONTACT_PAIR>	m_ContactPair;	// contact pairs
		vector<TIE>				m_Tie;			// ties
	};

public:
	// constructor
	AbaqusModel();

	// destructor
	~AbaqusModel();

	void SetName(const string& name) { m_name = name; }	

	const string& GetName() const { return m_name; }

	// create a part
	PART* CreatePart(const string& name = "");

	// find a part
	PART* FindPart(const string& name);

	// add a part
	void AddPart(PART* pg) { m_Part.push_back(pg); }

	// find a node set based on a name
	NODE_SET* FindNodeSet(const string& name);

	// find a part with a particular element set
	ELEMENT_SET* FindElementSet(const string& name);

	// get part list
	list<PART*>&	PartList() { return m_Part; }

	PART& GlobalPart() { return m_globalPart; }

	SURFACE* FindSurface(const string& name);

	SpringSet* FindSpringSet(const string& name);

public:
	// Add a material
	MATERIAL* AddMaterial(const string& name);

	// get material list
	list<MATERIAL>& MaterialList() { return m_Mat; }

	// add a step
	STEP* AddStep(const string& name);

	STEP& GetInitStep() { return m_initStep; }

	list<STEP>& StepList() { return m_Step; }

	// get the Assembly
	ASSEMBLY* GetAssembly() { return m_Assembly; }

	// add an assembly
	ASSEMBLY* CreateAssembly();

	// find the instance
	INSTANCE* FindInstance(const string& name);

public:
	void AddAmplitude(const Amplitude& a);
	int Amplitudes() const;
	const Amplitude& GetAmplitude(int n) const;
	int FindAmplitude(const string& name) const;

private:
	string m_name; // model name

	PART m_globalPart; // global definitions are added here
	list<PART*>	m_Part;		// list of parts

	ASSEMBLY*	m_Assembly = nullptr;	// the assembly

private:	// physics
	list<MATERIAL>		m_Mat;			// materials
	STEP				m_initStep;		// the initial step
	list<STEP>			m_Step;			// analysis steps
	std::vector<Amplitude>		m_Amp;
};
