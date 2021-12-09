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
class FEBioExport12 : public FEBioExport
{
public:
	// section flags
	enum { MAX_SECTIONS = 12 };

	// export sections
	enum ExportSections
	{
		FEBIO_MODULE,
		FEBIO_CONTROL,
		FEBIO_GLOBAL,
		FEBIO_MATERIAL,
		FEBIO_GEOMETRY,
		FEBIO_BOUNDARY,
		FEBIO_LOADS,
		FEBIO_INITIAL,
		FEBIO_CONSTRAINTS,
		FEBIO_LOADDATA,
		FEBIO_OUTPUT,
		FEBIO_STEPS
	};

public:
	FEBioExport12(FEProject& prj);
	virtual ~FEBioExport12();

	void Clear();

	bool Write(const char* szfile);

public: // set export attributes
	void SetSectionFlag(int n, bool bwrite) { m_section[n] = bwrite; }

protected:
	bool PrepareExport(FEProject& prj);

	void WriteModuleSection(FEAnalysisStep* pstep);
	void WriteControlSection(FEAnalysisStep* pstep);
	void WriteMaterialSection();
	void WriteGeometrySection();
	void WriteGeometryNodes();
	void WriteGeometryElements();
	void WriteGeometryElementData();
	void WriteBoundarySection(FEStep& s);
	void WriteLoadsSection(FEStep& s);
	void WriteContactSection(FEStep& s);
	void WriteInitialSection();
	void WriteGlobalsSection();
	void WriteLoadDataSection();
	void WriteOutputSection();
	void WriteStepSection();
	void WriteConstraintSection(FEStep& s);

	void WriteSolidControlParams(FEAnalysisStep* pstep);
	void WriteBiphasicControlParams(FEAnalysisStep* pstep);
	void WriteBiphasicSoluteControlParams(FEAnalysisStep* pstep);
	void WriteHeatTransferControlParams(FEAnalysisStep* pstep);

	void WriteBCFixed(FEStep& s);
	void WriteBCFixedDisplacement(FEFixedDisplacement&  rbc, FEStep& s);
	void WriteBCFixedRotation(FEFixedRotation&      rbc, FEStep& s);
	void WriteBCFixedFluidPressure(FEFixedFluidPressure& rbc, FEStep& s);
	void WriteBCFixedTemperature(FEFixedTemperature&   rbc, FEStep& s);
	void WriteBCFixedConcentration(FEFixedConcentration& rbc, FEStep& s);

	void WriteBCPrescribed(FEStep& s);
	void WriteBCPrescribedDisplacement(FEPrescribedDisplacement  &rbc, FEStep& s);
	void WriteBCPrescribedRotation(FEPrescribedRotation      &rbc, FEStep& s);
	void WriteBCPrescribedFluidPressure(FEPrescribedFluidPressure &rbc, FEStep& s);
	void WriteBCPrescribedTemperature(FEPrescribedTemperature   &rbc, FEStep& s);
	void WriteBCPrescribedConcentration(FEPrescribedConcentration &rbc, FEStep& s);

	void WriteLoadNodal(FEStep& s);
	void WriteLoadPressure(FEStep& s);
	void WriteLoadTraction(FEStep& s);
	void WriteFluidFlux(FEStep& s);
	void WriteHeatFlux(FEStep& s);
	void WriteConvectiveHeatFlux(FEStep& s);
	void WriteSoluteFlux(FEStep& s);
	void WriteBPNormalTraction(FEStep& s);
	void WriteBodyForces(FEStep& s);
	void WriteHeatSources(FEStep& s);

	void WriteContactSliding(FEStep& s);
	void WriteContactTied(FEStep& s);
	void WriteContactSticky(FEStep& s);
	void WriteContactPeriodic(FEStep& s);
	void WriteContactRigid(FEStep& s);
	void WriteContactPoro(FEStep& s);
	void WriteContactPoroSolute(FEStep& s);
	void WriteContactMultiphasic(FEStep& s);
	void WriteContactWall(FEStep& s);
	void WriteContactJoint(FEStep& s);
	void WriteContactTC(FEStep& s);
	void WriteContactTiedPoro(FEStep& s);
	void WriteSpringTied(FEStep& s);

	void WriteDiscrete();
	void WriteMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMultiMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMaterialParams(FSMaterial* pm);
	void WriteFiberMaterial(FEOldFiberMaterial& f);

	void WriteSurfaceSection(FEFaceList& s);
	void WriteSurface(XMLElement& el, FEItemListBuilder* pl);

protected:
	FSModel*		m_pfem;

protected:
	bool HasSurface(FEItemListBuilder* pl);

protected:
	std::vector<FEItemListBuilder*>	m_pSurf;	//!< list of named surfaces

	bool	m_section[MAX_SECTIONS];	//!< write section flags

	int m_nodes;	// number of nodes
	int	m_nsteps;	// number of steps
	int	m_nrc;		// number of rigid constraints
	int	m_ntotelem;	// total element counter
};
