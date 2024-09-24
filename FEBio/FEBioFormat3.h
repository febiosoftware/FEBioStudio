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
// Class that represents FEBio format 3.0
class FEBioFormat3 : public FEBioFormat
{
public:
	FEBioFormat3(FEBioFileImport* fileReader, FEBioInputModel& febio);
	~FEBioFormat3();

	bool ParseSection(XMLTag& tag) override;

private:
	// parsers for parent sections
	bool ParseModuleSection     (XMLTag& tag);
	bool ParseMeshSection       (XMLTag& tag);
	bool ParseGeometrySection   (XMLTag& tag);
	bool ParseMeshDomainsSection(XMLTag& tag);
	bool ParseMeshDataSection   (XMLTag& tag);
	bool ParseMeshAdaptorSection(XMLTag& tag);
	bool ParseBoundarySection   (XMLTag& tag);
	bool ParseLoadsSection      (XMLTag& tag);
	bool ParseInitialSection    (XMLTag& tag);
	bool ParseConstraintSection (XMLTag& tag);
	bool ParseContactSection    (XMLTag& tag);
	bool ParseDiscreteSection   (XMLTag& tag);
	bool ParseStepSection       (XMLTag& tag);
	bool ParseRigidSection      (XMLTag& tag);
	bool ParseLoadDataSection   (XMLTag& tag) override;
	bool ParseControlSection    (XMLTag& tag) override;

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
	void ParseBCRigid     (FSStep* pstep, XMLTag& tag);
	void ParseBCFluidRotationalVelocity(FSStep* pstep, XMLTag& tag);
	void ParseBCNormalDisplacement(FSStep* pstep, XMLTag& tag);
	void ParseRigidCable(FSStep* pstep, XMLTag& tag);
	void ParseBCLinearConstraint(FSStep* pstep, XMLTag& tag);

	// mesh data sections
	bool ParseElementDataSection(XMLTag& tag);
	bool ParseSurfaceDataSection(XMLTag& tag);
	bool ParseNodeDataSection(XMLTag& tag);

	// mesh domain sections
	bool ParseSolidDomainSection(XMLTag& tag);
	bool ParseShellDomainSection(XMLTag& tag);

private:
	bool ParseStep(XMLTag& tag);
	bool ParseLoadCurve(XMLTag& tag, LoadCurve& lc);
	bool ParseLoadController(XMLTag& tag);

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

private:
	// constraint input functions
	void ParseVolumeConstraint     (FSStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane        (FSStep* pstep, XMLTag& tag);
    void ParseNrmlFldVlctSrf       (FSStep* pstep, XMLTag& tag);
    void ParseFrictionlessFluidWall(FSStep* pstep, XMLTag& tag);
	void ParseInSituStretchConstraint(FSStep* pstep, XMLTag& tag);
	void ParsePrestrainConstraint    (FSStep* pste, XMLTag& tag);
    void ParseFixedNormalDisplacement(FSStep* pste, XMLTag& tag);

private:
	// rigid input functions
	void ParseRigidConnector(FSStep* pstep, XMLTag& tag, const int rc);
	void ParseRigidConstraint(FSStep* pstep, XMLTag& tag);
	void ParseRigidJoint(FSStep* pstep, XMLTag& tag);

private:
	FEBioInputModel::Part* DefaultPart();

private:
	// Geometry format flag
	// 0 = don't know yet
	// 1 = old fomrat
	// 2 = new format (i.e. parts and instances)
	int m_geomFormat;	
};
