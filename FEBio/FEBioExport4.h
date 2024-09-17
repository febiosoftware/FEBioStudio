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
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEBodyLoad.h>
#include "FEBioExport.h"

class FSSurfaceLoad;
class GPartList;
class GMaterial;
class GPart;

struct ProgressTracker
{
	double pct = 0;
	const char* sztask = nullptr;
	bool cancel = false;
};

//-----------------------------------------------------------------------------
//! Exporter for FEBio format specification version 2.5
class FEBioExport4 : public FEBioExport
{
private:
	class NamedItemList
	{
	public:
		std::string			m_name;
		std::string			m_extName;
		FEItemListBuilder* m_list;
		FSObject* m_parent;
		bool				m_duplicate;	// this list is defined more than once, and should not be written to Mesh section.

	public:
		NamedItemList()
		{
			m_list = nullptr;
			m_duplicate = false;
		}

		NamedItemList(const std::string& name, FEItemListBuilder* itemList, bool duplicate = false)
		{
			m_name = name;
			m_parent = nullptr;
			m_list = itemList;
			m_duplicate = duplicate;
		}

		NamedItemList(const std::string& name, FSObject* parent, FEItemListBuilder* itemList, bool duplicate = false)
		{
			m_name = name;
			m_parent = parent;
			m_list = itemList;
			m_duplicate = duplicate;
		}

		NamedItemList(const NamedItemList& n)
		{
			m_name = n.m_name;
			m_list = n.m_list;
			m_duplicate = n.m_duplicate;
		}

		void operator = (const NamedItemList& n)
		{
			m_name = n.m_name;
			m_list = n.m_list;
			m_duplicate = n.m_duplicate;
		}
	};

	class NodeSet
	{
	public:
		string			m_name;
		FSNodeList* m_nodeList;

	public:
		NodeSet(const string& name, FSNodeList* nodeList) : m_name(name), m_nodeList(nodeList) {}
		~NodeSet() { delete m_nodeList; }
	};

	class Surface
	{
	public:
		string			m_name;
		FEFaceList* m_faceList;

	public:
		Surface(const string& name, FEFaceList* faceList) : m_name(name), m_faceList(faceList) {}
		~Surface() { delete m_faceList; }
	};

	class ElementList
	{
	public:
		string		m_name;
		FEElemList* m_elemList;

	public:
		ElementList(const string& name, FEElemList* elemList) : m_name(name), m_elemList(elemList) {}
		~ElementList() { delete m_elemList; }
	};

	class Domain
	{
	public:
		Domain() {}

	public:
		string	m_name;
		string	m_matName;
		int		m_elemClass = 0;
		int		m_elemType = 0;
		GPart* m_pg = nullptr;
	};

	class Part
	{
	public:
		GObject* m_obj;
		vector<NodeSet*>		m_NSet;
		vector<Surface*>		m_Surf;
		vector<ElementList*>	m_ELst;
		vector<Domain*>			m_Dom;

	public:
		Part(GObject* po) : m_obj(po) {}
		~Part()
		{
			for (size_t i = 0; i < m_NSet.size(); ++i) delete m_NSet[i];
			for (size_t i = 0; i < m_Surf.size(); ++i) delete m_Surf[i];
			for (size_t i = 0; i < m_ELst.size(); ++i) delete m_ELst[i];
			for (size_t i = 0; i < m_Dom.size(); ++i) delete m_Dom[i];
		}
		NodeSet* FindNodeSet(const string& name)
		{
			for (size_t i = 0; i < m_NSet.size(); ++i)
				if (m_NSet[i]->m_name == name) return m_NSet[i];
			return 0;
		}
		Surface* FindSurface(const string& name)
		{
			for (size_t i = 0; i < m_Surf.size(); ++i)
				if (m_Surf[i]->m_name == name) return m_Surf[i];
			return 0;
		}
		ElementList* FindElementSet(const string& name)
		{
			for (size_t i = 0; i < m_ELst.size(); ++i)
				if (m_ELst[i]->m_name == name) return m_ELst[i];
			return 0;
		}

		std::vector<FEBioExport4::Domain*> GetDomainsFromGPart(GPart* pg);
	};

	class ElementSet
	{
	public:
		FSCoreMesh* m_mesh;
		int			m_matID;
		string		m_name;
		vector<int>	m_elem;

	public:
		ElementSet() { m_mesh = 0; }
		ElementSet(const ElementSet& es)
		{
			m_mesh = es.m_mesh;
			m_name = es.m_name;
			m_elem = es.m_elem;
			m_matID = es.m_matID;
		}

		void operator = (const ElementSet& es)
		{
			m_mesh = es.m_mesh;
			m_name = es.m_name;
			m_elem = es.m_elem;
			m_matID = es.m_matID;
		}
	};

public:
	FEBioExport4(FSProject& prj);
	virtual ~FEBioExport4();

	void SetProgressTracker(ProgressTracker* prg);

	void Clear();

	bool Write(const char* szfile);

public: // set export attributes
	void SetSectionFlag(int n, bool bwrite) { m_section[n] = bwrite; }

	void SetWriteNotesFlag(bool b) { m_writeNotes = b; }

	void SetMixedMeshFlag(bool b) { m_allowMixedParts = b; }

protected:
	bool PrepareExport(FSProject& prj);
	void BuildItemLists(FSProject& prj);

	void WriteModuleSection(FSProject& prj);
	void WriteControlSection(FSStep& s);
	void WriteMaterialSection();
	void WriteMeshSection();
	void WriteMeshElements();
	void WriteMeshDomainsSection();
	void WriteGeometryNodes();
	void WriteMixedElementsPart(Part* part, GPart* pg, std::vector<int>& elemList, bool writeMats = true, bool useMatNames = false);
	void WriteGeometryPart(Part* part, GPart* pg, std::vector<int>& elemList, bool writeMats = true, bool useMatNames = false);
	void WriteGeometryEdges();
	void WriteGeometrySurfaces();
	void WriteGeometryElementSets();
	void WriteGeometryPartLists();
	void WriteGeometrySurfacePairs();
	void WriteGeometryNodeSets();
	void WriteGeometryDiscreteSets();
	void WriteMeshDataSection();
	void WriteMeshAdaptorSection(FSStep& s);
	void WriteBoundarySection(FSStep& s);
	void WriteLoadsSection(FSStep& s);
	void WriteContactSection(FSStep& s);
	void WriteDiscreteSection(FSStep& s);
	void WriteInitialSection(FSStep& s);
	void WriteGlobalsSection();
	void WriteLoadDataSection();
	void WriteOutputSection();
	void WriteStepSection();
	void WriteConstraintSection(FSStep& s);

	void WriteRigidSection(FSStep& s);
	void WriteRigidBCs(FSStep& s);
	void WriteRigidICs(FSStep& s);
	void WriteRigidLoads(FSStep& s);

	void WriteBodyLoads(FSStep& s);

	// Used by new Part export feature
	void WriteGeometryNodeSetsNew();
	void WriteGeometrySurfacesNew();
	void WriteGeometryElementSetsNew();

	void WriteElementDataSection();
	void WriteSurfaceDataSection();
	void WriteEdgeDataSection();
	void WriteNodeDataSection();

	void WriteMeshDataShellThickness();
	void WriteMeshDataMaterialFibers();
	void WriteMeshDataMaterialAxes();
	void WriteElementDataFields();
	void WriteNodeDataGenerator(FSNodeDataGenerator* map);
	void WriteEdgeDataGenerator(FSEdgeDataGenerator* map);
	void WriteFaceDataGenerator(FSFaceDataGenerator* map);
	void WriteElemDataGenerator(FSElemDataGenerator* map);

	void WriteBC(FSStep& s, FSBoundaryCondition* pbc);

	void WriteNodalLoads(FSStep& s);
	void WriteSurfaceLoads(FSStep& s);

	void WriteRigidConnectors(FSStep& s);
	void WriteConstraints(FSStep& s);

	void WriteModelComponent(FSModelComponent* pmat, XMLElement& el);

	void WriteSurfaceSection(FEFaceList& s);
	void WriteSurfaceSection(NamedItemList& l);
	void WriteEdgeSection(NamedItemList& l);
	void WriteElementList(FEElemList& el);

protected:
	FSModel* m_pfem;

	bool	m_writeControlSection;	// write Control section for single step analysis
	ProgressTracker* m_prg;

	void setProgress(double v);
	void setProgress(double v, const char* sztask);
	void setProgressTask(const char* sztask);

protected:
	const char* GetSurfaceName(FEItemListBuilder* pl, bool allowPartLists = false);
	string GetNodeSetName(FEItemListBuilder* pl);
	const char* GetEdgeSetName(FEItemListBuilder* pl);
	string GetElementSetName(FEItemListBuilder* pl);

	void AddNodeSet(const std::string& name, FEItemListBuilder* pl);
	void AddEdgeSet(const std::string& name, FEItemListBuilder* pl);
	void AddSurface(const std::string& name, FEItemListBuilder* pl);
	void AddElemSet(const std::string& name, FEItemListBuilder* pl);
	void AddPartList(const std::string& name, FEItemListBuilder* pl);

	bool WriteNodeSet(const string& name, FSNodeList* pl);

	void ProcessParts();

protected:
	Part* FindPart(GObject* po);

protected:
	std::vector<NamedItemList>		m_pSurf;	//!< list of named surfaces
	std::vector<NamedItemList>		m_pNSet;	//!< list of named node sets
	std::vector<NamedItemList>		m_pESet;	//!< list of named element sets
	std::vector<NamedItemList>		m_pEdge;	//!< list of named edges
	std::vector<NamedItemList>		m_pPSet;	//!< list of named part lists

	std::vector<ElementSet>		m_ElSet;	//!< the element sets

	// used by the new export Part feature
	std::vector<Part*>		m_Part;	//!< list of parts

	int	m_numbl;	// number of body loads
	int	m_nsteps;	// number of steps
	int	m_ntotelem;	// total element counter
	int m_ntotnodes;	// total node counter
	bool	m_bdata;	// write MeshData section flag
	bool	m_writeNotes;	// write notes as comments
	bool	m_allowMixedParts; // if true, parts will not be split by elements. Instead, degenerate elements are written.
};

