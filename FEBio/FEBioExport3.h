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

#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEBodyLoad.h>
#include "FEBioExport.h"

class FSSurfaceLoad;
class GPartList;
class GMaterial;
class GPart;

//-----------------------------------------------------------------------------
class NamedItemList
{
public:
	std::string			m_name;
	FEItemListBuilder*	m_list;
	FSObject*			m_parent;
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

//-----------------------------------------------------------------------------
//! Exporter for FEBio format specification version 2.5
class FEBioExport3 : public FEBioExport
{
private:
	class NodeSet
	{
	public:
		string			m_name;
		FSNodeList*		m_nodeList;

	public:
		NodeSet(const string& name, FSNodeList* nodeList) : m_name(name), m_nodeList(nodeList) {}
		~NodeSet() { delete m_nodeList; }
	};

	class Surface
	{
	public:
		string			m_name;
		FEFaceList*		m_faceList;

	public:
		Surface(const string& name, FEFaceList* faceList) : m_name(name), m_faceList(faceList) {}
		~Surface() { delete m_faceList; }
	};

	class ElementList
	{
	public:
		string		m_name;
		FEElemList*	m_elemList;

	public:
		ElementList(const string& name, FEElemList* elemList) : m_name(name), m_elemList(elemList) {}
		~ElementList() { delete m_elemList; }
	};

	class Domain
	{
	public:
		Domain() { m_pg = nullptr; }

	public:
		string	m_name;
		string	m_matName;
		int		m_elemClass;
		GPart*	m_pg;
	};

	class Part
	{
	public:
		GObject*				m_obj;
		vector<NodeSet*>		m_NSet;
		vector<Surface*>		m_Surf;
		vector<ElementList*>	m_ELst;
		vector<Domain*>			m_Dom;

	public:
		Part(GObject* po) : m_obj(po){}
		~Part()
		{
			for (size_t i = 0; i<m_NSet.size(); ++i) delete m_NSet[i];
			for (size_t i = 0; i<m_Surf.size(); ++i) delete m_Surf[i];
			for (size_t i = 0; i<m_ELst.size(); ++i) delete m_ELst[i];
			for (size_t i = 0; i < m_Dom.size(); ++i) delete m_Dom[i];
		}
		NodeSet* FindNodeSet(const string& name)
		{
			for (size_t i = 0; i<m_NSet.size(); ++i)
				if (m_NSet[i]->m_name == name) return m_NSet[i];
			return 0;
		}
		Surface* FindSurface(const string& name)
		{
			for (size_t i = 0; i<m_Surf.size(); ++i)
				if (m_Surf[i]->m_name == name) return m_Surf[i];
			return 0;
		}
		ElementList* FindElementSet(const string& name)
		{
			for (size_t i = 0; i<m_ELst.size(); ++i)
				if (m_ELst[i]->m_name == name) return m_ELst[i];
			return 0;
		}
	};

	class ElementSet
	{
	public:
		FSCoreMesh*	m_mesh;
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
	FEBioExport3(FSProject& prj);
	virtual ~FEBioExport3();

	void Clear();

	bool Write(const char* szfile);

public: // set export attributes

	void SetExportPartsFlag(bool b) { m_exportParts = b; }

	void SetWriteNotesFlag(bool b) { m_writeNotes = b; }

protected:
	bool PrepareExport(FSProject& prj);
	void BuildItemLists(FSProject& prj);

public:
	void WriteModuleSection(FSStep* pstep);
	void WriteControlSection(FSStep& s);
	void WriteMaterialSection();
	void WriteGeometrySection();
	void WriteGeometrySectionOld();	// old, global node and element list
	void WriteGeometrySectionNew();	// new, grouped by parts
	void WriteMeshSection();
	void WriteMeshElements();
	void WriteMeshDomainsSection();
	void WriteGeometryNodes();
	void WriteGeometryElements(bool writeMats = true, bool useMatNames = false);
	void WriteGeometryPart(Part* part, GPart* pg, bool writeMats = true, bool useMatNames = false);
	void WriteGeometrySurfaces();
	void WriteGeometryElementSets();
	void WriteGeometrySurfacePairs();
	void WriteGeometryNodeSets();
	void WriteGeometryDiscreteSets();
	void WriteMeshDataSection();
	void WriteBoundarySection(FSStep& s);
	void WriteLoadsSection(FSStep& s);
	void WriteContactSection(FSStep& s);
	void WriteDiscreteSection(FSStep& s);
	void WriteInitialSection();
	void WriteGlobalsSection();
	void WriteLoadDataSection();
	void WriteOutputSection();
	void WriteStepSection();
	void WriteConstraintSection(FSStep& s);
	
	void WriteRigidSection(FSStep& s);
	void WriteRigidConstraints(FSStep& s);

	void WriteBodyLoads(FSStep& s);

	// Used by new Part export feature
	void WriteGeometryObject(Part* po);
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

	void WriteSolidControlParams(FSAnalysisStep* pstep);
	void WriteBiphasicControlParams(FSAnalysisStep* pstep);
	void WriteBiphasicSoluteControlParams(FSAnalysisStep* pstep);
	void WriteHeatTransferControlParams(FSAnalysisStep* pstep);
	void WriteFluidControlParams(FSAnalysisStep* pstep);
	void WriteFluidFSIControlParams(FSAnalysisStep* pstep);
	void WriteReactionDiffusionControlParams(FSAnalysisStep* pstep);
    void WritePolarFluidControlParams(FSAnalysisStep* pstep);

	void WriteBC(FSStep& s, FSBoundaryCondition* pbc);
	void WriteBCFixed(FSStep& s, FSBoundaryCondition* pbc);
	void WriteBCPrescribed(FSStep& s, FSBoundaryCondition* pbc);
	void WriteBCRigid(FSStep& s);

	void WriteInitVelocity(FSNodalVelocities&        iv);
	void WriteInitShellVelocity(FSNodalShellVelocities&   iv);
	void WriteInitConcentration(FSInitConcentration&      ic);
	void WriteInitShellConcentration(FSInitShellConcentration& ic);
	void WriteInitFluidPressure(FSInitFluidPressure&      ip);
	void WriteInitShellFluidPressure(FSInitShellFluidPressure& iq);
	void WriteInitTemperature(FSInitTemperature&        it);
    void WriteInitFluidDilatation(FSInitFluidDilatation&  it);
	void WriteInitPrestrain(FSInitPrestrain&          ip);

	void WriteDOFNodalLoad(FSStep& s, FSNodalLoad* pbc);

	void WriteNodalLoads(FSStep& s);
	void WriteSurfaceLoads(FSStep& s);
	void WriteSurfaceLoad(FSStep& s, FSSurfaceLoad* psl, const char* sztype);

	void WriteContactInterface(FSStep& s, const char* sztype, FSPairedInterface* pi);
	void WriteContactWall(FSStep& s);
	void WriteContactSphere(FSStep& s);
	void WriteLinearConstraints(FSStep& s);
	void WriteConnectors(FSStep& s);
	void WriteRigidJoint(FSStep& s);
	void WriteConstraints(FSStep& s);

	void WriteMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteRigidMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMaterialParams(FSMaterial* pm, bool isTopLevel = false);
	void WriteFiberMaterial(FSOldFiberMaterial& f);
	void WriteReactionMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteReactionMaterial2(FSMaterial* pmat, XMLElement& el);
    void WriteMembraneReactionMaterial(FSMaterial* pmat, XMLElement& el);

	void WriteSurfaceSection(FEFaceList& s);
	void WriteSurfaceSection(NamedItemList& l);
	void WriteElementList(FEElemList& el);

protected:
	FSModel*		m_pfem;

	bool	m_useReactionMaterial2;
	bool	m_writeControlSection;	// write Control section for single step analysis

protected:
	const char* GetSurfaceName(FEItemListBuilder* pl);
	string GetNodeSetName(FEItemListBuilder* pl);
	string GetElementSetName(FEItemListBuilder* pl);

	void AddNodeSet(const std::string& name, FEItemListBuilder* pl);
	void AddSurface(const std::string& name, FEItemListBuilder* pl);
	void AddElemSet(const std::string& name, FEItemListBuilder* pl);

	bool WriteNodeSet(const string& name, FSNodeList* pl);

protected:
	Part* FindPart(GObject* po);

protected:
	std::vector<NamedItemList>		m_pSurf;	//!< list of named surfaces
	std::vector<NamedItemList>		m_pNSet;	//!< list of named node sets
	std::vector<NamedItemList>		m_pESet;	//!< list of named element sets

	std::vector<ElementSet>		m_ElSet;	//!< the element sets

	// used by the new export Part feature
	std::vector<Part*>		m_Part;	//!< list of parts

	int	m_numbl;	// number of body loads
	int	m_nsteps;	// number of steps
	int	m_ntotelem;	// total element counter
	int m_ntotnodes;	// total node counter
	bool	m_bdata;	// write MeshData section flag
	bool	m_exportParts;	// write geometry using parts
	bool	m_exportMesh;	// write new mesh/meshdomains sections
	bool	m_writeNotes;	// write notes as comments
};
