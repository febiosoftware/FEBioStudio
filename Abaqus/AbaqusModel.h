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
#include <MeshTools/FEProject.h>
#include <list>
#include <vector>
#include <map>
//using namespace std;

using std::vector;
using std::list;
using std::map;

class AbaqusModel
{
public:
	enum { Max_Nodes = 20 };	// max nodes per element
	enum { Max_Name = 256 };	// 
	enum { Max_Title = 256 };
	enum { C3D8 } ElementTypes;

	// surface types
	enum { ST_ELEMENT };

public:

	// Node
	struct NODE
	{
		int	id;				// nodal ID
		int n;				// node index
		double x, y, z;		// nodal coordinates
	};

	typedef vector<NODE>::iterator Tnode_itr;

	// Element
	struct ELEMENT
	{
		int id;		// Global ID (as read from file)
		int lid;	// local ID (index into part's element list)
		int type;
		int n[Max_Nodes];
	};
	typedef vector<ELEMENT>::iterator Telem_itr;

	// Spring element
	struct SPRING
	{
		int	id;
		int	n[2];
	};
	typedef list<SPRING>::iterator Tspring_itr;

	// Face
	struct FACE
	{
		int	eid;		// element ID pointer
		int nf;			// face number
	};

	class PART;

	// node set
	struct NODE_SET
	{
		char		szname[Max_Name + 1];
		PART*		part;
		list<Tnode_itr>	node;
	};

	// Element set
	struct ELEMENT_SET
	{
		char szname[Max_Name + 1];	// element set name
		PART*			part;
		vector<int>		elem;
	};

	// Surface
	struct SURFACE
	{
		char szname[Max_Name + 1];	// surface name
		list<FACE> face;			// face list
		PART*		part;
	};

	// Solid section
	struct SOLID_SECTION
	{
		char	szelset[Max_Name + 1];
		char	szmat[Max_Name + 1];
		char	szorient[Max_Name + 1];
		PART*	part;
	};

	struct Orientation
	{
		char	szname[Max_Name + 1];
		char	szdist[Max_Name + 1];
	};

	class Distribution
	{
	public:
		struct ENTRY
		{
			int		elem;
			double	val[6];
		};

		Distribution() {}
		Distribution(const Distribution& d) { strcpy(m_szname, d.m_szname); m_data = d.m_data; }
		void operator = (const Distribution& d) { strcpy(m_szname, d.m_szname); m_data = d.m_data; }

	public:
		char			m_szname[Max_Name + 1];
		vector<ENTRY>	m_data;
	};

	// part
	class PART
	{
	public:
		// constructor
		PART() { m_po = 0; }

		// return the part's name
		const char* GetName() { return m_szname; }

		// set the part's name
		void SetName(const char* sz);

	public:
		// find a node
		Tnode_itr FindNode(int nid);

		// add a node
		Tnode_itr AddNode(NODE& n);

		// add an element
		void AddElement(ELEMENT& n);

		// add a spring
		Tspring_itr AddSpring(SPRING& n);

		// finds a element with a particular id
		Telem_itr FindElement(int id);

		// find an element set with a particular name
		list<ELEMENT_SET>::iterator FindElementSet(const char* szname);

		// adds an element set
		list<ELEMENT_SET>::iterator AddElementSet(const char* szname);

		// find a node set with a particular name
		map<string, NODE_SET>::iterator FindNodeSet(const char* szname);

		// adds a node set
		map<string, NODE_SET>::iterator AddNodeSet(const char* szname);

		// find a surface with a particular name
		list<SURFACE>::iterator FindSurface(const char* szname);

		// add a surface
		list<SURFACE>::iterator AddSurface(const char* szname);

		// add a solid section
		list<SOLID_SECTION>::iterator AddSolidSection(const char* szset, const char* szmat, const char* szorient);

		// number of nodes
		int Nodes() { return (int)m_Node.size(); }

		// number of elements
		int Elements() { return (int)m_Elem.size(); }

		// number of springs
		int Springs() { return (int)m_Spring.size(); }

		// build the node-look-up table
		bool BuildNLT();

		void AddOrientation(const char* szname, const char* szdist);

		Orientation* FindOrientation(const char* szname);

		Distribution* FindDistribution(const char* szname);

	public:
		char m_szname[256];
		vector<NODE>			m_Node;		// list of nodes
		vector<ELEMENT>			m_Elem;		// list of elements
		list<SPRING>			m_Spring;	// list of springs
		map<string, NODE_SET>	m_NSet;		// node sets
		list<ELEMENT_SET>		m_ElSet;	// element sets
		list<SURFACE>			m_Surf;		// surfaces
		list<SOLID_SECTION>		m_Solid;	// solid section
		list<Orientation>		m_Orient;
		list<Distribution>		m_Distr;

		vector<Tnode_itr>	m_NLT;	// Node look-up table
		int					m_ioff;	// node id offset (min node id)

		GObject*			m_po;	// object created based on this part
	};

	// an instance of a part
	class INSTANCE
	{
	public:
		INSTANCE();

		void SetName(const char* sz);
		const char* GetName() { return m_szname; }

		void SetPart(PART* pg) { m_pPart = pg; }
		PART* GetPart() { return m_pPart; }

		void GetTranslation(double t[3]);
		void SetTranslation(double t[3]);

		void GetRotation(double t[7]);
		void SetRotation(double t[7]);

	private:
		char	m_szname[256];	// name of instance
		PART*	m_pPart;		// the part this instances
		double	m_trans[3];		// translation
		double	m_rot[7];		// rotation
	};

	// material types
	enum { ELASTIC, HYPERELASTIC, ANI_HYPERELASTIC };

	// material
	struct MATERIAL
	{
		char	szname[256];
		int		mattype;
		int		ntype;
		double	dens;
		double	d[5];
	};

	// surface loads
	class DSLOAD
	{
	public:
		struct SURF
		{
			double		load;
			SURFACE*	surf;
		};

	public:
		DSLOAD(){}
		DSLOAD(const DSLOAD& d)
		{
			m_surf = d.m_surf;
		}
		void operator = (const DSLOAD& d)
		{
			m_surf = d.m_surf;
		}

		void add(SURFACE* s, double p)
		{
			SURF surf = {p, s};
			m_surf.push_back(surf);
		}

	public:
		vector<SURF>	m_surf;
	};

	// Boundary conditions
	class BOUNDARY
	{
	public:
		struct NSET
		{
			double		load;
			int			ndof;
			NODE_SET*	nodeSet;
		};

	public:
		BOUNDARY(){}
		BOUNDARY(const BOUNDARY& d)
		{
			m_nodeSet = d.m_nodeSet;
		}
		void operator = (const BOUNDARY& d)
		{
			m_nodeSet = d.m_nodeSet;
		}

		void add(NODE_SET* ns, int ndof, double v)
		{
			NSET nset = { v, ndof, ns };
			m_nodeSet.push_back(nset);
		}

	public:
		vector<NSET>	m_nodeSet;
	};

	// Steps
	struct STEP
	{
		char	szname[Max_Name + 1];

		double	dt0;
		double	time;
	};

public:
	// constructor
	AbaqusModel();

	// destructor
	~AbaqusModel();

	// get the active part
	PART* GetActivePart(bool bcreate = false);

	// create a part
	PART* CreatePart(const char* sz = 0);

	// find a part
	PART* FindPart(const char* sz);

	// find a node set based on a name
	NODE_SET* FindNodeSet(const char* sznset);

	// find a part with a particular element set
	ELEMENT_SET* FindElementSet(const char* szelemset);

	// add an instance
	INSTANCE* AddInstance();

	// find the instance
	INSTANCE* FindInstance(const char* sz);

	// Get the current instance
	INSTANCE* CurrentInstance() { return m_pInst; }

	// clear the current instance
	void ClearCurrentInstance();

	// get the current part
	PART* CurrentPart() { return m_pPart; }

	// set the current part
	void SetCurrentPart(PART* part) { m_pPart = part; }

	// get instance list
	list<INSTANCE*>& InstanceList() { return m_Inst; }

	// get part list
	list<PART*>&	PartList() { return m_Part; }

	SURFACE* FindSurface(const char* szname);

public:
	// Add a material
	MATERIAL* AddMaterial(const char* szname);

	// get material list
	list<MATERIAL>& MaterialList() { return m_Mat; }

	// add a pressure load
	DSLOAD*	AddPressureLoad(DSLOAD& p);

	// list of pressure loads
	list<DSLOAD>& SurfaceLoadList() { return m_SLoads; }

	// add a boundary condition
	BOUNDARY* AddBoundaryCondition(BOUNDARY& p);

	// list of boundary conditions
	list<BOUNDARY>& BoundaryConditionList() { return m_Boundary; }

	// add a step
	STEP* AddStep(const char* szname);

	// set the current step
	void SetCurrentStep(STEP* p);

	// get the current step
	STEP* CurrentStep() { return m_pStep; }

	list<STEP>& StepList() { return m_Step; }

private:
	FEModel*	m_fem;		// the model

	list<PART*>	m_Part;		// list of parts
	PART*		m_pPart;	// current part

	list<INSTANCE*>	m_Inst;		// list of instances
	INSTANCE*		m_pInst;	// current active instance

private:	// physics
	list<MATERIAL>		m_Mat;			// materials
	list<DSLOAD>		m_SLoads;		// surface loads
	list<BOUNDARY>		m_Boundary;		// boundary conditions
	list<STEP>			m_Step;			// steps
	STEP*				m_pStep;		// current step
};
