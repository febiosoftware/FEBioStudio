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
#include <FEMLib/FSProject.h>
#include "FEBioExport.h"

//-----------------------------------------------------------------------------
typedef std::pair<std::string, FSItemListBuilder*> NamedList;

//-----------------------------------------------------------------------------
class FS1DPointFunction;
class GPart;

//-----------------------------------------------------------------------------
//! Exporter for FEBio format specification version 2.5
class FEBioExport25 : public FEBioExport
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
		FSFaceList*		m_faceList;

	public:
		Surface(const string& name, FSFaceList* faceList) : m_name(name), m_faceList(faceList) {}
		~Surface() { delete m_faceList; }
	};

	class ElementSet
	{
	public:
		FSCoreMesh*	mesh;
		int			matID;
		std::string		name;
		std::vector<int>	elem;

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
		std::vector<NodeSet*>	m_NSet;
		std::vector<Surface*>	m_Surf;

	public:
		Part(GObject* po) : m_obj(po){}
		~Part()
		{
			for (size_t i=0; i<m_NSet.size(); ++i) delete m_NSet[i];
			for (size_t i=0; i<m_Surf.size(); ++i) delete m_Surf[i];
		}
		NodeSet* FindNodeSet(const string& name)
		{
			for (size_t i=0; i<m_NSet.size(); ++i) 
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
	FEBioExport25(FSProject& prj);
	virtual ~FEBioExport25();

	void Clear() override;

	bool Write(const char* szfile) override;

public: // set export attributes
	void SetExportPartsFlag(bool b) { m_exportParts = b; }

	void SetWriteNotesFlag(bool b) { m_writeNotes = b; }

protected:
	bool PrepareExport(FSProject& prj) override;
	void BuildNodeSetList(FSProject& prj);
	void BuildSurfaceList(FSProject& prj);
	void BuildElemSetList(FSProject& prj);

	void WriteModuleSection    (FSAnalysisStep* pstep);
	void WriteControlSection   (FSAnalysisStep* pstep);
	void WriteMaterialSection  ();
	void WriteGeometrySection();
	void WriteGeometrySectionOld();	// old, global node and element list
	void WriteGeometrySectionNew();	// new, grouped by parts
	void WriteGeometryNodes();
	void WriteGeometryElements ();
	void WriteGeometryPart(GPart* pg, bool useMatNames = false);
	void WriteGeometrySurfaces ();
	void WriteGeometryElementSets();
	void WriteGeometrySurfacePairs();
	void WriteGeometryNodeSets ();
	void WriteGeometryDiscreteSets();
	void WriteMeshDataSection  ();
	void WriteBoundarySection  (FSStep& s);
	void WriteLoadsSection     (FSStep& s);
	void WriteContactSection   (FSStep& s);
	void WriteDiscreteSection  (FSStep& s);
	void WriteInitialSection   (FSStep& s);
	void WriteGlobalsSection   ();
	void WriteLoadDataSection  ();
	void WriteOutputSection    ();
	void WriteStepSection      ();
	void WriteConstraintSection(FSStep& s);
	void WriteRigidConstraints (FSStep& s);

	void WriteBodyLoads(FSStep& s);
	void WriteBodyLoad(FSBodyLoad* pbl, GPart* pg);
	void WriteBodyForce(FSConstBodyForce* pbf, GPart* pg);
	void WriteHeatSource(FSHeatSource* phs, GPart* pg);
    void WriteCentrifugalBodyForce(FSCentrifugalBodyForce* pbf, GPart* pg);

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
	void WriteMeshDataMaterialAxes  ();
	void WriteMeshDataFields        ();

	void WriteSolidControlParams            (FSAnalysisStep* pstep);
	void WriteBiphasicControlParams         (FSAnalysisStep* pstep);
	void WriteBiphasicSoluteControlParams   (FSAnalysisStep* pstep);
	void WriteHeatTransferControlParams     (FSAnalysisStep* pstep);
    void WriteFluidControlParams            (FSAnalysisStep* pstep);
    void WriteFluidFSIControlParams         (FSAnalysisStep* pstep);
	void WriteReactionDiffusionControlParams(FSAnalysisStep* pstep);

	void WriteBCFixed     (FSStep& s);
	void WriteBCPrescribed(FSStep& s);
	void WriteBCRigid     (FSStep& s);

	void WriteInitVelocity          (FSNodalVelocities&        iv);
	void WriteInitShellVelocity     (FSNodalShellVelocities&   iv);
	void WriteInitConcentration     (FSInitConcentration&      ic);
    void WriteInitShellConcentration(FSInitShellConcentration& ic);
	void WriteInitFluidPressure     (FSInitFluidPressure&      ip);
    void WriteInitShellFluidPressure(FSInitShellFluidPressure& iq);
	void WriteInitTemperature       (FSInitTemperature&        it);
    void WriteInitFluidDilatation   (FSInitFluidDilatation&    it);
	void WriteInitPrestrain         (FSInitPrestrain&          ip);

	void WriteLoadNodal         (FSStep& s);
	void WriteLoadPressure      (FSStep& s);
	void WriteLoadTraction      (FSStep& s);
    void WriteFluidTraction     (FSStep& s);
    void WriteFluidPressureLoad           (FSStep& s);
    void WriteFluidVelocity               (FSStep& s);
    void WriteFluidNormalVelocity         (FSStep& s);
    void WriteFluidRotationalVelocity     (FSStep& s);
    void WriteFluidFlowResistance         (FSStep& s);
    void WriteFluidFlowRCR                (FSStep& s);
    void WriteFluidBackflowStabilization  (FSStep& s);
    void WriteFluidTangentialStabilization(FSStep& s);
    void WriteFSITraction                 (FSStep& s);
    void WriteBFSITraction                (FSStep& s);
	void WriteFluidFlux         (FSStep& s);
	void WriteHeatFlux          (FSStep& s);
	void WriteConvectiveHeatFlux(FSStep& s);
	void WriteSoluteFlux        (FSStep& s);
	void WriteBPNormalTraction  (FSStep& s);
	void WriteConcentrationFlux (FSStep& s);
    void WriteMatchingOsmoticCoefficient  (FSStep& s);

	void WriteContactInterface  (FSStep& s, const char* sztype, FSPairedInterface* pi);
	void WriteContactWall       (FSStep& s);
	void WriteContactSphere     (FSStep& s);
	void WriteLinearConstraints (FSStep& s);
    void WriteConnectors        (FSStep& s);
	void WriteRigidJoint        (FSStep& s);
	void WriteConstraints       (FSStep& s);

	void WriteMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteRigidMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMaterialParams(FSMaterial* pm, bool topLevel = true);
	void WriteFiberMaterial(FSOldFiberMaterial& f);
	void WriteReactionMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteReactionMaterial2(FSMaterial* pmat, XMLElement& el);

	void WriteSurfaceSection(FSFaceList& s);
	void WriteElementList(FSElemList& el);

	void WritePointCurve(FS1DPointFunction* f1d, XMLElement& el);

protected:
	FSModel*		m_pfem;

	bool	m_useReactionMaterial2;
	bool	m_writeNotes;

protected:
	const char* GetSurfaceName(FSItemListBuilder* pl);
	const char* GetNodeSetName(FSItemListBuilder* pl);

	void AddNodeSet(const std::string& name, FSItemListBuilder* pl);
	void AddSurface(const std::string& name, FSItemListBuilder* pl);
	void AddElemSet(const std::string& name, FSItemListBuilder* pl);

	bool WriteNodeSet(const string& name, FSNodeList* pl);

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
};
