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
#include <FEMLib/FSProject.h>

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
	FEBioExport12(FSProject& prj);
	virtual ~FEBioExport12();

	void Clear();

	bool Write(const char* szfile);

public: // set export attributes
	void SetSectionFlag(int n, bool bwrite) { m_section[n] = bwrite; }

protected:
	bool PrepareExport(FSProject& prj);

	void WriteModuleSection(FSAnalysisStep* pstep);
	void WriteControlSection(FSAnalysisStep* pstep);
	void WriteMaterialSection();
	void WriteGeometrySection();
	void WriteGeometryNodes();
	void WriteGeometryElements();
	void WriteGeometryElementData();
	void WriteBoundarySection(FSStep& s);
	void WriteLoadsSection(FSStep& s);
	void WriteContactSection(FSStep& s);
	void WriteInitialSection();
	void WriteGlobalsSection();
	void WriteLoadDataSection();
	void WriteOutputSection();
	void WriteStepSection();
	void WriteConstraintSection(FSStep& s);

	void WriteSolidControlParams(FSAnalysisStep* pstep);
	void WriteBiphasicControlParams(FSAnalysisStep* pstep);
	void WriteBiphasicSoluteControlParams(FSAnalysisStep* pstep);
	void WriteHeatTransferControlParams(FSAnalysisStep* pstep);

	void WriteBCFixed(FSStep& s);
	void WriteBCFixedDisplacement(FSFixedDisplacement&  rbc, FSStep& s);
	void WriteBCFixedRotation(FSFixedRotation&      rbc, FSStep& s);
	void WriteBCFixedFluidPressure(FSFixedFluidPressure& rbc, FSStep& s);
	void WriteBCFixedTemperature(FSFixedTemperature&   rbc, FSStep& s);
	void WriteBCFixedConcentration(FSFixedConcentration& rbc, FSStep& s);

	void WriteBCPrescribed(FSStep& s);
	void WriteBCPrescribedDisplacement(FSPrescribedDisplacement  &rbc, FSStep& s);
	void WriteBCPrescribedRotation(FSPrescribedRotation      &rbc, FSStep& s);
	void WriteBCPrescribedFluidPressure(FSPrescribedFluidPressure &rbc, FSStep& s);
	void WriteBCPrescribedTemperature(FSPrescribedTemperature   &rbc, FSStep& s);
	void WriteBCPrescribedConcentration(FSPrescribedConcentration &rbc, FSStep& s);

	void WriteLoadNodal(FSStep& s);
	void WriteLoadPressure(FSStep& s);
	void WriteLoadTraction(FSStep& s);
	void WriteFluidFlux(FSStep& s);
	void WriteHeatFlux(FSStep& s);
	void WriteConvectiveHeatFlux(FSStep& s);
	void WriteSoluteFlux(FSStep& s);
	void WriteBPNormalTraction(FSStep& s);
	void WriteBodyForces(FSStep& s);
	void WriteHeatSources(FSStep& s);

	void WriteContactSliding(FSStep& s);
	void WriteContactTied(FSStep& s);
	void WriteContactSticky(FSStep& s);
	void WriteContactPeriodic(FSStep& s);
	void WriteContactRigid(FSStep& s);
	void WriteContactPoro(FSStep& s);
	void WriteContactPoroSolute(FSStep& s);
	void WriteContactMultiphasic(FSStep& s);
	void WriteContactWall(FSStep& s);
	void WriteContactJoint(FSStep& s);
	void WriteContactTC(FSStep& s);
	void WriteContactTiedPoro(FSStep& s);
	void WriteSpringTied(FSStep& s);

	void WriteDiscrete();
	void WriteMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMultiMaterial(FSMaterial* pmat, XMLElement& el);
	void WriteMaterialParams(FSMaterial* pm);
	void WriteFiberMaterial(FSOldFiberMaterial& f);

	void WriteSurfaceSection(FSFaceList& s);
	void WriteSurface(XMLElement& el, FSItemListBuilder* pl);

protected:
	FSModel*		m_pfem;

protected:
	bool HasSurface(FSItemListBuilder* pl);

protected:
	std::vector<FSItemListBuilder*>	m_pSurf;	//!< list of named surfaces

	bool	m_section[MAX_SECTIONS];	//!< write section flags

	int m_nodes;	// number of nodes
	int	m_nsteps;	// number of steps
	int	m_nrc;		// number of rigid constraints
	int	m_ntotelem;	// total element counter
};
