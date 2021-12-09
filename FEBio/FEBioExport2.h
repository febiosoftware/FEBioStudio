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
#include "FEBioExport.h"
#include <MeshTools/FEProject.h>

//-----------------------------------------------------------------------------
class GPart;

//-----------------------------------------------------------------------------
//! Exporter for FEBio format specification version 2.x
class FEBioExport2 : public FEBioExport
{
public:
	FEBioExport2(FEProject& prj);
	virtual ~FEBioExport2();

	void Clear() override;

	bool Write(const char* szfile) override;

public: // set export attributes
	void SetSectionFlag(int n, bool bwrite) { m_section[n] = bwrite; }

protected:
	bool PrepareExport(FEProject& prj) override;

	void WriteModuleSection    (FSAnalysisStep* pstep);
	void WriteControlSection   (FSAnalysisStep* pstep);
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

	void WriteSolidControlParams            (FSAnalysisStep* pstep);
	void WriteBiphasicControlParams         (FSAnalysisStep* pstep);
	void WriteBiphasicSoluteControlParams   (FSAnalysisStep* pstep);
	void WriteHeatTransferControlParams     (FSAnalysisStep* pstep);
    void WriteFluidControlParams            (FSAnalysisStep* pstep);
    void WriteFluidFSIControlParams         (FSAnalysisStep* pstep);
	void WriteReactionDiffusionControlParams(FSAnalysisStep* pstep);

	void WriteBCFixed(FEStep& s);
	void WriteBCFixedDisplacement     (FSFixedDisplacement&      rbc, FEStep& s);
	void WriteBCFixedShellDisplacement(FSFixedShellDisplacement& rbc, FEStep& s);
	void WriteBCFixedRotation         (FSFixedRotation&          rbc, FEStep& s);
	void WriteBCFixedFluidPressure    (FSFixedFluidPressure&     rbc, FEStep& s);
	void WriteBCFixedTemperature      (FSFixedTemperature&       rbc, FEStep& s);
	void WriteBCFixedConcentration    (FSFixedConcentration&     rbc, FEStep& s);
    void WriteBCFixedFluidVelocity    (FSFixedFluidVelocity&     rbc, FEStep& s);
    void WriteBCFixedFluidDilatation  (FSFixedFluidDilatation&   rbc, FEStep& s);

	void WriteBCPrescribed(FEStep& s);
	void WriteBCPrescribedDisplacement   (FSPrescribedDisplacement       &rbc, FEStep& s);
	void WriteBCPrescribedRotation       (FSPrescribedRotation           &rbc, FEStep& s);
	void WriteBCPrescribedFluidPressure  (FSPrescribedFluidPressure      &rbc, FEStep& s);
	void WriteBCPrescribedTemperature    (FSPrescribedTemperature        &rbc, FEStep& s);
	void WriteBCPrescribedConcentration  (FSPrescribedConcentration      &rbc, FEStep& s);
    void WriteBCPrescribedFluidVelocity  (FSPrescribedFluidVelocity      &rbc, FEStep& s);
    void WriteBCPrescribedFluidDilatation(FSPrescribedFluidDilatation    &rbc, FEStep& s);

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

	void WriteMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMultiMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMaterialParams(FSMaterial* pm);
	void WriteFiberMaterial(FSOldFiberMaterial& f);
	void WriteRigidMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteTCNLOrthoMaterial(FSMaterial* pmat, XMLElement& el);

	void WriteSurfaceSection(FEFaceList& s);
	void WriteSurface(XMLElement& el, FEItemListBuilder* pl);

protected:
	FSModel*		m_pfem;

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
