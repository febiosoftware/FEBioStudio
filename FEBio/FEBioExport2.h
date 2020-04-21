#pragma once
#include <FEMLib/FEMultiMaterial.h>
#include "FEBioExport.h"

//-----------------------------------------------------------------------------
class GPart;

//-----------------------------------------------------------------------------
//! Exporter for FEBio format specification version 2.x
class FEBioExport2 : public FEBioExport
{
public:
	FEBioExport2();
	virtual ~FEBioExport2();

	void Clear();

	bool Export(FEProject& prj, const char* szfile);

public: // set export attributes
	void SetSectionFlag(int n, bool bwrite) { m_section[n] = bwrite; }

protected:
	bool PrepareExport(FEProject& prj);

	void WriteModuleSection    (FEAnalysisStep* pstep);
	void WriteControlSection   (FEAnalysisStep* pstep);
	void WriteMaterialSection  ();
	void WriteGeometrySection  ();
	void WriteGeometryNodes    ();
	void WriteGeometryElements ();
	void WriteGeometryPart(GPart* pg);
	void WriteGeometryElementData();
	void WriteGeometrySurfaces ();
	void WriteGeometryNodeSets ();
	void WriteBoundarySection  (FEStep& s);
	void WriteLoadsSection     (FEStep& s);
	void WriteContactSection   (FEStep& s);
	void WriteDiscreteSection  (FEStep& s);
	void WriteInitialSection   ();
	void WriteGlobalsSection   ();
	void WriteLoadDataSection  ();
	void WriteOutputSection    ();
	void WriteStepSection      ();
	void WriteConstraintSection(FEStep& s);

	void WriteSolidControlParams            (FEAnalysisStep* pstep);
	void WriteBiphasicControlParams         (FEAnalysisStep* pstep);
	void WriteBiphasicSoluteControlParams   (FEAnalysisStep* pstep);
	void WriteHeatTransferControlParams     (FEAnalysisStep* pstep);
    void WriteFluidControlParams            (FEAnalysisStep* pstep);
    void WriteFluidFSIControlParams         (FEAnalysisStep* pstep);
	void WriteReactionDiffusionControlParams(FEAnalysisStep* pstep);

	void WriteBCFixed(FEStep& s);
	void WriteBCFixedDisplacement     (FEFixedDisplacement&      rbc, FEStep& s);
	void WriteBCFixedShellDisplacement(FEFixedShellDisplacement& rbc, FEStep& s);
	void WriteBCFixedRotation         (FEFixedRotation&          rbc, FEStep& s);
	void WriteBCFixedFluidPressure    (FEFixedFluidPressure&     rbc, FEStep& s);
	void WriteBCFixedTemperature      (FEFixedTemperature&       rbc, FEStep& s);
	void WriteBCFixedConcentration    (FEFixedConcentration&     rbc, FEStep& s);
    void WriteBCFixedFluidVelocity    (FEFixedFluidVelocity&     rbc, FEStep& s);
    void WriteBCFixedFluidDilatation  (FEFixedFluidDilatation&   rbc, FEStep& s);

	void WriteBCPrescribed(FEStep& s);
	void WriteBCPrescribedDisplacement   (FEPrescribedDisplacement       &rbc, FEStep& s);
	void WriteBCPrescribedRotation       (FEPrescribedRotation           &rbc, FEStep& s);
	void WriteBCPrescribedFluidPressure  (FEPrescribedFluidPressure      &rbc, FEStep& s);
	void WriteBCPrescribedTemperature    (FEPrescribedTemperature        &rbc, FEStep& s);
	void WriteBCPrescribedConcentration  (FEPrescribedConcentration      &rbc, FEStep& s);
    void WriteBCPrescribedFluidVelocity  (FEPrescribedFluidVelocity      &rbc, FEStep& s);
    void WriteBCPrescribedFluidDilatation(FEPrescribedFluidDilatation    &rbc, FEStep& s);

	void WriteLoadNodal         (FEStep& s);
	void WriteLoadPressure      (FEStep& s);
	void WriteLoadTraction      (FEStep& s);
    void WriteFluidTraction     (FEStep& s);
    void WriteFluidVelocity     (FEStep& s);
    void WriteFluidNormalVelocity         (FEStep& s);
    void WriteFluidRotationalVelocity     (FEStep& s);
    void WriteFluidFlowResistance         (FEStep& s);
    void WriteFluidBackflowStabilization  (FEStep& s);
    void WriteFluidTangentialStabilization(FEStep& s);
    void WriteFSITraction       (FEStep& s);
	void WriteFluidFlux         (FEStep& s);
	void WriteHeatFlux          (FEStep& s);
	void WriteConvectiveHeatFlux(FEStep& s);
	void WriteSoluteFlux        (FEStep& s);
	void WriteBPNormalTraction  (FEStep& s);
	void WriteBodyForces        (FEStep& s);
	void WriteHeatSources       (FEStep& s);
	void WriteConcentrationFlux (FEStep& s);

	void WriteContactSliding    (FEStep& s);
	void WriteContactTied       (FEStep& s);
	void WriteContactSticky     (FEStep& s);
	void WriteContactPeriodic   (FEStep& s);
	void WriteContactRigid      (FEStep& s);
	void WriteContactPoro       (FEStep& s);
	void WriteContactPoroSolute (FEStep& s);
	void WriteContactMultiphasic(FEStep& s);
	void WriteContactWall       (FEStep& s);
	void WriteContactJoint      (FEStep& s);
	void WriteContactTC         (FEStep& s);
	void WriteContactTiedPoro   (FEStep& s);
	void WriteSpringTied        (FEStep& s);
	void WriteLinearConstraints (FEStep& s);
	void WriteVolumeConstraint  (FEStep& s);
    void WriteConnectors        (FEStep& s);
	void WriteSymmetryPlane     (FEStep& s);
    void WriteNormalFlow        (FEStep& s);
    void WriteFrictionlessFluidWall(FEStep& s);

	void WriteMaterial(FEMaterial* pmat, XMLElement& el);
	void WriteMultiMaterial(FEMaterial* pmat, XMLElement& el);
	void WriteMaterialParams(FEMaterial* pm);
	void WriteFiberMaterial(FEOldFiberMaterial& f);
	void WriteRigidMaterial(FEMaterial* pmat, XMLElement& el);
	void WriteTCNLOrthoMaterial(FEMaterial* pmat, XMLElement& el);

	void WriteSurfaceSection(FEFaceList& s);
	void WriteSurface(XMLElement& el, FEItemListBuilder* pl);

protected:
	FEModel*		m_pfem;
	FEProject*		m_pprj;

protected:
	bool HasSurface(FEItemListBuilder* pl);
	bool HasNodeSet(FEItemListBuilder* pl);

	bool WriteNodeSet(const string& name, FENodeList* pl);

protected:
	std::vector<FEItemListBuilder*>	m_pSurf;	//!< list of named surfaces
	std::vector<FEItemListBuilder*>	m_pNSet;	//!< list of named node sets

	int m_nodes;	// number of nodes
	int	m_nsteps;	// number of steps
	int	m_nrc;		// number of rigid constraints
	int	m_ntotelem;	// total element counter
};
