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
	FEBioFormat25(FEBioFileImport* fileReader, FEBioInputModel& febio);
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
	void ParseBCFixed     (FSStep* pstep, XMLTag& tag);
	void ParseBCPrescribed(FSStep* pstep, XMLTag& tag);
	void ParseBCRigidBody (FSStep* pstep, XMLTag& tag);
	void ParseBCRigid     (FSStep* pstep, XMLTag& tag);
	void ParseInitRigidBodu(FSStep* pstep, XMLTag& tag);

private:
	// contact input functions
	void ParseContact(FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactSliding        (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactF2FSliding     (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactBiphasic       (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactSolute         (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactMultiphasic    (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactTied           (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactTiedElastic    (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactF2FTied        (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactSticky         (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactPeriodic       (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactTC             (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactTiedPoro       (FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactTiedMultiphasic(FSStep* pstep, XMLTag& tag);
	FSPairedInterface* ParseContactGapHeatFlux    (FSStep* pstep, XMLTag& tag);
	void ParseContactJoint(FSStep* pstep, XMLTag& tag);
	void ParseRigidWall         (FSStep* pstep, XMLTag& tag);
	void ParseLinearConstraint  (FSStep* pstep, XMLTag& tag);

private:
	// loads parse functions
	void ParseNodeLoad   (FSStep* pstep, XMLTag& tag);
	void ParseSurfaceLoad(FSStep* pstep, XMLTag& tag);
	void ParseBodyLoad   (FSStep* pstep, XMLTag& tag);

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
	void ParseBodyForce           (FSStep* pstep, XMLTag& tag);
	void ParseNonConstBodyForce   (FSStep* pstep, XMLTag& tag);
	void ParseHeatSource          (FSStep* pstep, XMLTag& tag);
    void ParseCentrifugalBodyForce(FSStep* pstep, XMLTag& tag);

private:
	// constraint input functions
	void ParseVolumeConstraint     (FSStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane        (FSStep* pstep, XMLTag& tag);
    void ParseNrmlFldVlctSrf       (FSStep* pstep, XMLTag& tag);
    void ParseFrictionlessFluidWall(FSStep* pstep, XMLTag& tag);
	void ParseInSituStretchConstraint(FSStep* pstep, XMLTag& tag);
	void ParsePrestrainConstraint(FSStep* pste, XMLTag& tag);

private:
	// connector input functions
	void ParseConnector(FSStep* pstep, XMLTag& tag, const int rc);

private:
	FEBioInputModel::Part* DefaultPart();

private:
	// Geometry format flag
	// 0 = don't know yet
	// 1 = old fomrat
	// 2 = new format (i.e. parts and instances)
	int m_geomFormat;	
};
