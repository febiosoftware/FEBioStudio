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
#include <string>
#include <vector>
#include <FEMLib/FSProject.h>
#include <FEMLib/FEElementFormulation.h>

class GMeshObject;
class GDiscreteElementSet;
class GPart;

//-----------------------------------------------------------------------------
// Helper class for processing and finding mesh data 
class FEBioMesh
{
public:
	FEBioMesh();

	FSMesh* operator -> () { return &m_mesh; }

	FSMesh* GetFEMesh() { return &m_mesh; }

	void UpdateMeshData();

	int FindFace(const int* n, int nn, int noff = 0);
	int FindEdge(const int* n, int nn, int noff = 0);

	int FindFace(const std::vector<int>& n, int noff = 0);
	int FindEdge(const std::vector<int>& n, int noff = 0);

	FSElement* FindElementFromID(int nid);

private:
	void BuildFaceTable();
	void BuildEdgeTable();
	void BuildElementTable();

private:
	FSMesh				m_mesh;

	// edge table
	std::vector<int>	m_iEdge;
	std::vector<int*>	m_pEdge;
	std::vector<int>	m_nEdge;

	// face table
	std::vector<int>	m_iFace;
	std::vector<int*>	m_pFace;
	std::vector<int>	m_nFace;

	// element table
	std::vector<FSElement*>		m_elem;
	int							m_maxID, m_minID;
};

//-----------------------------------------------------------------------------
// Helper class that stores the components read from the FEBio file
class FEBioInputModel
{
public:
	struct NODE
	{
		int		id;
		vec3d	r;
	};

	struct ELEM
	{
		int		id;
		int		n[FSElement::MAX_NODES];
	};

	// Class for storing node sets
	class NodeSet
	{
	public:
		NodeSet();
		NodeSet(const NodeSet& nodeSet);
		NodeSet(const std::string& name, const std::vector<int>& node);
		void operator = (const NodeSet& nodeSet);

		const std::string& name() const { return m_name; }

		int Nodes() const { return (int) m_node.size(); }
		int Node(int n) const { return m_node[n]; }

		const std::vector<int>& nodeList() const { return m_node; }
		std::vector<int>& nodeList() { return m_node; }

	private:
		std::string			m_name;
		std::vector<int>	m_node;
	};

	// class for storing edges
	class EdgeSet
	{
	public:
		EdgeSet() { m_refs = 0; }

		const std::string& name() const { return m_name; }

		int edges() const { return (int)m_edge.size(); }

		const std::vector<int>& edge(int i) const { return m_edge[i]; }

		void addEdge(const std::vector<int>& node) { m_edge.push_back(node); }

		void clear() { m_edge.clear(); }

	public:
		std::string						m_name;
		std::vector< std::vector<int> > m_edge;
		int		m_refs;
	};

	// class for storing surfaces
	class Surface
	{
	public:
		Surface();
		Surface(const Surface& s);
		void operator = (const Surface& s);

		const std::string& name() const { return m_name; }

		int faces() const { return (int) m_face.size(); }

		const std::vector<int>& face(int i) const { return m_face[i]; }

		void addFace(const std::vector<int>& node) { m_face.push_back(node); }

		void clear() { m_face.clear(); }

	public:
		std::string						m_name;
		std::vector< std::vector<int> > m_face;
		int		m_refs;
	};

	// class for storing element sets
	class ElementSet
	{
	public:
		ElementSet();
		ElementSet(const std::string& name, const std::vector<int>& elem);
		ElementSet(const ElementSet& s);
		void operator = (const ElementSet& s);

		const std::string& name() const { return m_name; }

		int elements() const { return (int)m_elem.size(); }
		int element(int i) const { return m_elem[i]; }
		const std::vector<int>& elemList() const { return m_elem; }

		void clear() { m_elem.clear(); }

	public:
		std::string			m_name;
		std::vector<int>	m_elem;
	};

	class Part;

	// Class for storing domains (i.e. element sets with materials assigned)
	class Domain
	{
	public:
		enum DOMAIN_TYPE {
			UNKNOWN,
			SOLID,
			SHELL,
			BEAM
		};

	public:
		Domain(Part* part);
		Domain(Part* part, const std::string& name, int matID);
		Domain(const Domain& part);
		void operator = (const Domain& part);

		int MatID() const { return m_matID; }
		void SetMatID(int i) { m_matID = i; }

		const std::string& name() const { return m_name; }

		Part* GetPart() { return m_part; }

		void AddElement(int n) { m_elem.push_back(n); }

		int Elements() const { return (int) m_elem.size(); }

		int ElementID(int n) const { return m_elem[n]; }

		std::vector<int> GetElementIDList() const { return m_elem; }

		DOMAIN_TYPE Type() const { return m_type; }
		void SetType(DOMAIN_TYPE type) { m_type = type; }

		void SetElementFormulation(FEElementFormulation* eform) { m_form = eform; }
		FEElementFormulation* GetElementFormulation() { return m_form; }

		void SetDefaultShellThickness(double h) { m_defaultShellThickness = h; }
		double GetDefaultShellThickness() { return m_defaultShellThickness; }

	private:
		int				m_matID;	// zero-based material index
		DOMAIN_TYPE		m_type;		// domain type

		FEElementFormulation* m_form;
		double	m_defaultShellThickness;

		std::string			m_name;	
		Part*				m_part;
		std::vector<int>	m_elem;		// list of (local) element IDs that belong to this domain
	};

	// class for storing discrete element sets
	class DiscreteSet
	{
	public:
		struct DISCRETE_ELEMENT
		{
			int n0, n1;
		};

	public:
		DiscreteSet();
		DiscreteSet(const DiscreteSet& s);
		void operator = (const DiscreteSet& s);
		void SetName(const char* sz);
		void Add(int n0, int n1);

		int size() const { return (int)m_elem.size(); }
		const DISCRETE_ELEMENT& Element(int i) const { return m_elem[i]; }

		const std::string& name() const { return m_name; }

		void SetPart(Part* part) { m_part = part; }
		Part* GetPart() { return m_part; }

	private:
		std::string m_name;
		std::vector<DISCRETE_ELEMENT>	m_elem;
		Part*	m_part;
	};

	class SurfacePair
	{
	public:
		SurfacePair();
		SurfacePair(const SurfacePair& sp);
		SurfacePair(const std::string& name, int surf1ID, int surf2ID);
		void operator = (const SurfacePair& sp);

		const std::string& name() const { return m_name; }

		int PrimarySurfaceID() const { return m_surf1_ID; }
		int SecondarySurfaceID() const { return m_surf2_ID; }

		void SetPart(Part* part) { m_part = part; }
		Part* GetPart() { return m_part; }

	private:
		std::string m_name;
		int	m_surf1_ID, m_surf2_ID;
		Part*	m_part;
	};

	class DomainList
	{
	public:
		DomainList() { m_part = nullptr; }
		DomainList(const std::string& name, const std::vector<Domain*>& domList) : m_name(name), m_domList(domList) { m_part = nullptr; }
		void Add(Domain* pg) { m_domList.push_back(pg); }

	public:
		string	m_name;
		std::vector<Domain*> m_domList;
		Part* m_part;
	};

	class Part
	{
	public:
		Part();
		~Part();

		void SetName(const char* szname);
		const char* GetName();

	public:
		void AddNodeSet(const NodeSet& nset) { m_nset.push_back(nset); }
		int NodeSets() const { return (int)m_nset.size(); }
		NodeSet& GetNodeSet(int i) { return m_nset[i]; }
		NodeSet* FindNodeSet(const std::string& name);

	public:
		void AddEdgeSet(const EdgeSet& eset) { m_edge.push_back(eset); }
		int EdgeSets() const { return (int)m_edge.size(); }
		EdgeSet& GetEdgeSet(int i) { return m_edge[i]; }
		EdgeSet* FindEdgeSet(const std::string& name);

	public:
		Domain* AddDomain(const string& name, int matID);
		int Domains() const { return (int)m_dom.size(); }
		Domain& GetDomain(int i) { return m_dom[i]; }
		Domain* FindDomain(const std::string& name);

	public:
		void AddSurface(const Surface& s) { m_surf.push_back(s); }
		int Surfaces() const { return (int)m_surf.size(); }
		Surface& GetSurface(int i) { return m_surf[i]; }
		Surface* FindSurface(const std::string& name);
		int FindSurfaceIndex(const std::string& name);

	public:
		void AddElementSet(const ElementSet& s) { m_eset.push_back(s); }
		int ElementSets() const { return (int)m_eset.size(); }
		ElementSet& GetElementSet(int i) { return m_eset[i]; }
		ElementSet* FindElementSet(const std::string& name);
		int FindElementSetIndex(const std::string& name);

	public:
		void AddDiscreteSet(const DiscreteSet& s) { m_des.push_back(s); }
		int DiscreteSets() const { return (int)m_des.size(); }
		DiscreteSet& GetDiscreteSet(int i) { return m_des[i]; }
		DiscreteSet* FindDiscreteSet(const std::string& name);

	public:
		void AddSurfacePair(SurfacePair s) { s.SetPart(this); m_spr.push_back(s); }
		int SurfacePairs() const { return (int)m_spr.size(); }
		SurfacePair& GetSurfacePair(int i) { return m_spr[i]; }
		SurfacePair* FindSurfacePair(const std::string& name);

	public:
		void AddDomainList(DomainList s) { s.m_part = this; m_domlist.push_back(s); }
		size_t DomainLists() { return m_domlist.size(); }
		DomainList& GetDomainList(int i) { return m_domlist[i]; }
		DomainList* FindDomainList(const std::string& name);

	public:
		FSMesh* GetFEMesh() { return m_mesh.GetFEMesh(); }

		FEBioMesh& GetFEBioMesh() { return m_mesh; }

		void Update();

		int GlobalToLocalNodeIndex(int globalID);

	private:
		void BuildNLT();

	public:
		FEBioMesh				m_mesh;
		string					m_name;

	private:
		std::vector<NodeSet>		m_nset;
		std::vector<EdgeSet>		m_edge;
		std::vector<Surface>		m_surf;
		std::vector<ElementSet>		m_eset;
		std::vector<Domain>			m_dom;
		std::vector<DiscreteSet>	m_des;
		std::vector<SurfacePair>	m_spr;
		std::vector<DomainList>		m_domlist;

		std::vector<int> m_NLT;
		int m_nltoff = 0;

	private: // don't allow copying
		Part(const Part& p){}
		void operator = (const Part& p) {}
	};

	class PartInstance
	{
	public:
		PartInstance(Part* part);
		~PartInstance();

		Part* GetPart() { return m_part; }

		void SetName(const char* szname) { m_name = szname; }
		const char* GetName() { return m_name.c_str(); }

		void SetGObject(GMeshObject* po)
		{
			assert(m_po == 0);
			m_po = po;
		}

		GMeshObject* GetGObject() { return m_po; }

		GMeshObject* TakeGObject()
		{
			GMeshObject* po = m_po;
			m_po = 0;
			return po;
		}

		// return the mesh of the mesh object (not the part)
		FSMesh* GetMesh();

	public:
		FSNodeSet* BuildFENodeSet(const NodeSet& nset);
		FSEdgeSet* BuildFEEdgeSet(EdgeSet& surf);
		FSSurface* BuildFESurface(const Surface& surf);

		FSNodeSet* BuildFENodeSet(const char* szname);
		FSEdgeSet* BuildFEEdgeSet(const char* szname);
		FSSurface* BuildFESurface(const char* szname);
		FSElemSet* BuildFEElemSet(const char* szname);

	public:
		vec3d	m_pos;
		quatd	m_rot;
		vec3d	m_scl;

	private:
		string			m_name;
		Part*			m_part;
		GMeshObject*	m_po;

	private: // don't allow copying
		PartInstance(const PartInstance& p){}
		void operator = (const PartInstance& p){}
	};

	class PlotVariable
	{
	public:
		PlotVariable();
		PlotVariable(const PlotVariable& p);
		PlotVariable(const std::string& name);
		PlotVariable(const std::string& name, const std::string& domain, DOMAIN_TYPE domType);
		void operator = (const PlotVariable& p);

		const std::string& name() const { return m_name; }
        const std::string& domain() const { return m_domain; }

		DOMAIN_TYPE domainType();

	private:
		std::string	m_name;
        std::string m_domain;
		DOMAIN_TYPE	m_type;
	};

	class LogVariable
	{
	public:
		LogVariable();
		LogVariable(const LogVariable& log);
		LogVariable(int ntype, const std::string& data);
		void operator = (const LogVariable& log);

		int type() { return m_type; }
		const std::string& data() { return m_data; }
		
		void setFile(const std::string& file) { m_file = file; }
		const std::string& file() { return m_file;}

		void SetGroupID(int n) { m_groupID = n; }
		int GroupID() const { return m_groupID; }

	private:
		int			m_type;
		std::string	m_data;
		int			m_groupID;
		std::string	m_file;
	};

	struct PARAM_CURVE
	{
		Param*			m_p;
		LoadCurve*	m_plc;	// TODO: We need this for the must-point curve. Try to remove this.
		int				m_lc;
	};

public:
	FEBioInputModel(FSModel& fem);
	~FEBioInputModel();

	void UpdateGeometry();

	void CopyMeshSelections();

	FSModel& GetFSModel() { return m_fem; }

public:
	int Parts() { return (int) m_part.size(); }
	Part& GetPart(int i) { return *m_part[i]; }
	Part* AddPart(const char* szname);
	Part* FindPart(const char* szname);

public:
	void AddParamCurve(LoadCurve* plc, int lc);
	void AddParamCurve(Param* p, int lc);

	int ParamCurves() const { return (int) m_PC.size(); }
	PARAM_CURVE GetParamCurve(int i) { return m_PC[i]; }

	void AddLoadCurve(const LoadCurve& lc) { m_LC.push_back(lc); }
	int LoadCurves() const { return (int) m_LC.size(); }
	LoadCurve& GetLoadCurve(int i) { return m_LC[i]; }

public:
	void AddMaterial(GMaterial* pmat) { m_mat.push_back(pmat); }
	int Materials() const { return (int) m_mat.size(); }
	GMaterial* GetMaterial(int i) { if ((i<0) || (i>=(int)m_mat.size())) return 0; else return m_mat[i]; }

public:
	void AddPlotVariable(const PlotVariable& s) { m_plot.push_back(s); }
	int PlotVariables() const { return (int)m_plot.size(); }
	PlotVariable& GetPlotVariable(int i) { return m_plot[i]; }

public:
	void AddLogVariable(const LogVariable& s) { m_log.push_back(s); }
	int LogVariables() const { return (int)m_log.size(); }
	LogVariable& GetLogVariable(int i) { return m_log[i]; }

public:
	void AddInstance(PartInstance* instance) { m_Inst.push_back(instance); }
	int Instances() { return (int) m_Inst.size(); }
	PartInstance* GetInstance(int i) { return m_Inst[i]; }
	PartInstance* FindInstance(const char* szname);

public:
	int GetMaterialIndex(const char* szname);
	FSNodeSet* FindNodeSet(const char* szname);
	Surface* FindSurface(const char* szname);
	FSNodeSet* BuildFENodeSet(const char* szname);
	FSEdgeSet* BuildFEEdgeSet(const char* szname);
	FSSurface* BuildFESurface(const char* szname);
	FSElemSet* BuildFEElemSet(const char* szname);
	FSElemSet* BuildFEElemSet(Domain* dom);
	GPart* FindGPart(const char* szname);
	FEItemListBuilder* BuildItemList(const char* szname);
	SurfacePair* FindSurfacePair(const char* szname);
	Domain* FindDomain(const char* szname);
	ElementSet* FindElementSet(const char* szname);
	bool BuildDiscreteSet(GDiscreteElementSet& set, const char* szset);

	FEItemListBuilder* FindNamedSelection(const std::string& name, unsigned filter = MESH_ITEM_FLAGS::FE_ALL_FLAGS);
	FSNodeSet* FindNamedNodeSet(const std::string& name);
	FSSurface* FindNamedSurface(const std::string& name);
	FSElemSet* FindNamedElementSet(const std::string& name);
	FSPartSet* FindNamedPartSet(const std::string& name);

public:
	bool	m_shellNodalNormals;

private:
	FSModel&					m_fem;
	std::vector<GMaterial*>		m_mat;
	std::vector<Part*>			m_part;
	std::vector<PartInstance*>	m_Inst;
	std::vector<PARAM_CURVE> 	m_PC;
	std::vector<LoadCurve>		m_LC;
	std::vector<PlotVariable>	m_plot;
	std::vector<LogVariable>	m_log;
};

class FEBioFileImport;
void AddLogEntry(const char* sz, ...);
void InitLog(FEBioFileImport* im);
