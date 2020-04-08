#pragma once

#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEBodyLoad.h>
#include "FEBioExport.h"

class FEDataMap;
class FESurfaceLoad;

//-----------------------------------------------------------------------------
typedef std::pair<std::string, FEItemListBuilder*> NamedList;

//-----------------------------------------------------------------------------
//! Exporter for FEBio format specification version 2.5
class FEBioExport3 : public FEBioExport
{
private:
	class NodeSet
	{
	public:
		string			m_name;
		FENodeList*		m_nodeList;

	public:
		NodeSet(const string& name, FENodeList* nodeList) : m_name(name), m_nodeList(nodeList) {}
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

	class ElementSet
	{
	public:
		FECoreMesh*	mesh;
		int			matID;
		string		name;
		vector<int>	elem;

	public:
		ElementSet(){ mesh = 0; }
		ElementSet(const ElementSet& es)
		{
			mesh = es.mesh;
			name = es.name;
			elem = es.elem;
			matID = es.matID;
		}

		void operator = (const ElementSet& es)
		{
			mesh = es.mesh;
			name = es.name;
			elem = es.elem;
			matID = es.matID;
		}
	};

	class Part
	{
	public:
		GObject*			m_obj;
		vector<NodeSet*>	m_NSet;
		vector<Surface*>	m_Surf;

	public:
		Part(GObject* po) : m_obj(po){}
		~Part()
		{
			for (size_t i = 0; i<m_NSet.size(); ++i) delete m_NSet[i];
			for (size_t i = 0; i<m_Surf.size(); ++i) delete m_Surf[i];
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
	};

public:
	FEBioExport3();
	virtual ~FEBioExport3();

	void Clear();

	bool Export(FEProject& prj, const char* szfile);

public: // set export attributes
	void SetSectionFlag(int n, bool bwrite) { m_section[n] = bwrite; }

	void SetExportPartsFlag(bool b) { m_exportParts = b; }

	void SetWriteNotesFlag(bool b) { m_writeNotes = b; }

protected:
	bool PrepareExport(FEProject& prj);
	void BuildItemLists(FEProject& prj);

	GPartList* BuildPartList(GMaterial* mat);

	void WriteModuleSection(FEAnalysisStep* pstep);
	void WriteControlSection(FEAnalysisStep* pstep);
	void WriteMaterialSection();
	void WriteGeometrySection();
	void WriteGeometrySectionOld();	// old, global node and element list
	void WriteGeometrySectionNew();	// new, grouped by parts
	void WriteGeometryNodes();
	void WriteGeometryElements();
	void WriteGeometryPart(GPart* pg, bool writeMats = true, bool useMatNames = false);
	void WriteGeometrySurfaces();
	void WriteGeometryElementSets();
	void WriteGeometrySurfacePairs();
	void WriteGeometryNodeSets();
	void WriteGeometryDiscreteSets();
	void WriteMeshDataSection();
	void WriteBoundarySection(FEStep& s);
	void WriteLoadsSection(FEStep& s);
	void WriteContactSection(FEStep& s);
	void WriteDiscreteSection(FEStep& s);
	void WriteInitialSection();
	void WriteGlobalsSection();
	void WriteLoadDataSection();
	void WriteOutputSection();
	void WriteStepSection();
	void WriteConstraintSection(FEStep& s);
	
	void WriteRigidSection(FEStep& s);
	void WriteRigidConstraints(FEStep& s);

	void WriteBodyLoads(FEStep& s);
	void WriteBodyLoad(FEBodyLoad* pbl, GPart* pg);
	void WriteBodyForce(FEBodyForce* pbf, GPart* pg);
	void WriteHeatSource(FEHeatSource* phs, GPart* pg);

	// Used by new Part export feature
	void WriteGeometryObject(Part* po);
	void WriteGeometryNodeSetsNew();
	void WriteGeometrySurfacesNew();

	void WriteElementDataSection();
	void WriteSurfaceDataSection();
	void WriteEdgeDataSection();
	void WriteNodeDataSection();

	void WriteMeshDataShellThickness();
	void WriteMeshDataMaterialFibers();
	void WriteMeshDataMaterialAxes();
	void WriteMeshDataFields();
	void WriteMeshData(FEDataMap* map);

	void WriteSolidControlParams(FEAnalysisStep* pstep);
	void WriteBiphasicControlParams(FEAnalysisStep* pstep);
	void WriteBiphasicSoluteControlParams(FEAnalysisStep* pstep);
	void WriteHeatTransferControlParams(FEAnalysisStep* pstep);
	void WriteFluidControlParams(FEAnalysisStep* pstep);
	void WriteFluidFSIControlParams(FEAnalysisStep* pstep);
	void WriteReactionDiffusionControlParams(FEAnalysisStep* pstep);

	void WriteBCFixed(FEStep& s);
	void WriteBCPrescribed(FEStep& s);
	void WriteBCRigid(FEStep& s);

	void WriteInitVelocity(FENodalVelocities&        iv);
	void WriteInitShellVelocity(FENodalShellVelocities&   iv);
	void WriteInitConcentration(FEInitConcentration&      ic);
	void WriteInitShellConcentration(FEInitShellConcentration& ic);
	void WriteInitFluidPressure(FEInitFluidPressure&      ip);
	void WriteInitShellFluidPressure(FEInitShellFluidPressure& iq);
	void WriteInitTemperature(FEInitTemperature&        it);

	void WriteLoadNodal(FEStep& s);

	void WriteSurfaceLoads(FEStep& s);
	void WriteSurfaceLoad(FEStep& s, FESurfaceLoad* psl, const char* sztype);

	void WriteContactInterface(FEStep& s, const char* sztype, FEPairedInterface* pi);
	void WriteContactWall(FEStep& s);
	void WriteContactSphere(FEStep& s);
	void WriteLinearConstraints(FEStep& s);
	void WriteConnectors(FEStep& s);
	void WriteRigidJoint(FEStep& s);
	void WriteConstraints(FEStep& s);

	void WriteMaterial(FEMaterial* pmat, XMLElement& el);
	void WriteRigidMaterial(FEMaterial* pmat, XMLElement& el);
	void WriteMaterialParams(FEMaterial* pm, bool isTopLevel = false);
	void WriteFiberMaterial(FEOldFiberMaterial& f);
	void WriteReactionMaterial(FEMaterial* pmat, XMLElement& el);
	void WriteReactionMaterial2(FEMaterial* pmat, XMLElement& el);

	void WriteSurfaceSection(FEFaceList& s);
	void WriteElementList(FEElemList& el);

protected:
	FEModel*		m_pfem;
	FEProject*		m_pprj;

	bool	m_useReactionMaterial2;

protected:
	const char* GetSurfaceName(FEItemListBuilder* pl);
	string GetNodeSetName(FEItemListBuilder* pl);

	void AddNodeSet(const std::string& name, FEItemListBuilder* pl);
	void AddSurface(const std::string& name, FEItemListBuilder* pl);
	void AddElemSet(const std::string& name, FEItemListBuilder* pl);

	bool WriteNodeSet(const string& name, FENodeList* pl);

protected:
	Part* FindPart(GObject* po);

protected:
	std::vector<NamedList>		m_pSurf;	//!< list of named surfaces
	std::vector<NamedList>		m_pNSet;	//!< list of named node sets
	std::vector<NamedList>		m_pESet;	//!< list of named element sets

	std::vector<ElementSet>		m_ElSet;	//!< the element sets

	// used by the new export Part feature
	std::vector<Part*>		m_Part;	//!< list of parts

	int	m_numbl;	// number of body loads
	int	m_nsteps;	// number of steps
	int	m_ntotelem;	// total element counter
	int m_ntotnodes;	// total node counter
	bool	m_bdata;	// write MeshData section flag
	bool	m_exportParts;	// write geometry using parts
	bool	m_writeNotes;	// write notes as comments
};
