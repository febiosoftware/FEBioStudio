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
#include "FEBioImport.h"
#include "FEBioFormat.h"

//-----------------------------------------------------------------------------
class GMeshObject;

//-----------------------------------------------------------------------------
// Class that represents FEBio format 2.5
class FEBioFormat25 : public FEBioFormat
{
public:
	FEBioFormat25(FEBioImport* fileReader, FEBioInputModel& febio);
	~FEBioFormat25();

	bool ParseSection(XMLTag& tag);

private:
	// parsers for parent sections
	bool ParseModuleSection    (XMLTag& tag);
	bool ParseGeometrySection  (XMLTag& tag);
	bool ParseMeshDataSection  (XMLTag& tag);
	bool ParseBoundarySection  (XMLTag& tag);
	bool ParseLoadsSection     (XMLTag& tag);
	bool ParseInitialSection   (XMLTag& tag);
	bool ParseConstraintSection(XMLTag& tag);
	bool ParseContactSection   (XMLTag& tag);
	bool ParseDiscreteSection  (XMLTag& tag);
	bool ParseStepSection      (XMLTag& tag);

private:
	// mesh data section parsers
	bool ParseNodeData(XMLTag& tag);
	bool ParseSurfaceData(XMLTag& tag);
	bool ParseElementData(XMLTag& tag);

private:
	// geometry parsing functions (version 2.0 and up)
	void ParseGeometryNodes      (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryElements   (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryNodeSet    (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometrySurface    (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryElementSet (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryDiscreteSet(FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometrySurfacePair(FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryPart       (XMLTag& tag);
	void ParseGeometryInstance   (XMLTag& tag);

private:
	// boundary condition input functions
	void ParseBCFixed     (FEStep* pstep, XMLTag& tag);
	void ParseBCPrescribed(FEStep* pstep, XMLTag& tag);
	void ParseBCRigidBody (FEStep* pstep, XMLTag& tag);
	void ParseBCRigid     (FEStep* pstep, XMLTag& tag);
	void ParseInitRigidBodu(FEStep* pstep, XMLTag& tag);

private:
	// contact input functions
	void ParseContact(FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactSliding        (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactF2FSliding     (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactBiphasic       (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactSolute         (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactMultiphasic    (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactTied           (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactTiedElastic    (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactF2FTied        (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactSticky         (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactPeriodic       (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactTC             (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactTiedPoro       (FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactTiedMultiphasic(FEStep* pstep, XMLTag& tag);
	FEPairedInterface* ParseContactGapHeatFlux    (FEStep* pstep, XMLTag& tag);
	void ParseContactJoint(FEStep* pstep, XMLTag& tag);
	void ParseRigidWall         (FEStep* pstep, XMLTag& tag);
	void ParseLinearConstraint  (FEStep* pstep, XMLTag& tag);

private:
	// loads parse functions
	void ParseNodeLoad   (FEStep* pstep, XMLTag& tag);
	void ParseSurfaceLoad(FEStep* pstep, XMLTag& tag);
	void ParseBodyLoad   (FEStep* pstep, XMLTag& tag);

	// surface load functions (version 2.0 and up)
	FSSurfaceLoad* ParseLoadPressure          (XMLTag& tag);
	FSSurfaceLoad* ParseLoadTraction          (XMLTag& tag);
	FSSurfaceLoad* ParseLoadFluidTraction     (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidPressure     (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidVelocity               (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidNormalVelocity         (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidRotationalVelocity     (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidFlowResistance         (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidFlowRCR                (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidBackFlowStabilization  (XMLTag& tag);
    FSSurfaceLoad* ParseLoadFluidTangentialStabilization(XMLTag& tag);
    FSSurfaceLoad* ParseLoadFSITraction       (XMLTag& tag);
    FSSurfaceLoad* ParseLoadBFSITraction      (XMLTag& tag);
	FSSurfaceLoad* ParseLoadFluidFlux         (XMLTag& tag);
	FSSurfaceLoad* ParseLoadSoluteFlux        (XMLTag& tag);
	FSSurfaceLoad* ParseLoadNormalTraction    (XMLTag& tag);
    FSSurfaceLoad* ParseLoadMatchingOsmoticCoefficient  (XMLTag& tag);
	FSSurfaceLoad* ParseLoadHeatFlux          (XMLTag& tag);
	FSSurfaceLoad* ParseLoadConvectiveHeatFlux(XMLTag& tag);
	FSSurfaceLoad* ParseConcentrationFlux     (XMLTag& tag);

	// body loads
	void ParseBodyForce           (FEStep* pstep, XMLTag& tag);
	void ParseNonConstBodyForce   (FEStep* pstep, XMLTag& tag);
	void ParseHeatSource          (FEStep* pstep, XMLTag& tag);
    void ParseCentrifugalBodyForce(FEStep* pstep, XMLTag& tag);

private:
	// constraint input functions
	void ParseVolumeConstraint     (FEStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane        (FEStep* pstep, XMLTag& tag);
    void ParseNrmlFldVlctSrf       (FEStep* pstep, XMLTag& tag);
    void ParseFrictionlessFluidWall(FEStep* pstep, XMLTag& tag);
	void ParseInSituStretchConstraint(FEStep* pstep, XMLTag& tag);
	void ParsePrestrainConstraint(FEStep* pste, XMLTag& tag);

private:
	// connector input functions
	void ParseConnector(FEStep* pstep, XMLTag& tag, const int rc);

	// helper functions (version 2.5 and up)
	FEBioInputModel::DiscreteSet ParseDiscreteSet(XMLTag& tag);

private:
	FEBioInputModel::Part* DefaultPart();

private:
	// Geometry format flag
	// 0 = don't know yet
	// 1 = old fomrat
	// 2 = new format (i.e. parts and instances)
	int m_geomFormat;	
};
