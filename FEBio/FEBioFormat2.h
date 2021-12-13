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
// Class that represents FEBio formats 2.0
class FEBioFormat2 : public FEBioFormat
{
public:
	FEBioFormat2(FEBioImport* fileReader, FEBioInputModel& febio);
	~FEBioFormat2();

	bool ParseSection(XMLTag& tag);

private:
	bool ParseModuleSection     (XMLTag& tag);
	bool ParseGeometrySection   (XMLTag& tag);
	bool ParseBoundarySection   (XMLTag& tag);
	bool ParseLoadsSection      (XMLTag& tag);
	bool ParseInitialSection    (XMLTag& tag);
	bool ParseConstraintSection (XMLTag& tag);
	bool ParseStepSection       (XMLTag& tag);
	bool ParseContactSection    (XMLTag& tag);
	bool ParseDiscreteSection   (XMLTag& tag);

private:
	// boundary condition input functions
	void ParseBCFixed     (FSStep* pstep, XMLTag& tag);
	void ParseBCPrescribed(FSStep* pstep, XMLTag& tag);
	void ParseSprings     (FSStep* pstep, XMLTag& tag);
	void ParseContact     (FSStep* pstep, XMLTag& tag);
	void ParseBodyForce   (FSStep* pstep, XMLTag& tag);
	void ParseHeatSource  (FSStep* pstep, XMLTag& tag);

	// contact input functions
	void ParseContactSliding    (FSStep* pstep, XMLTag& tag);
	void ParseContactF2FSliding (FSStep* pstep, XMLTag& tag);
	void ParseContactBiphasic   (FSStep* pstep, XMLTag& tag);
	void ParseContactSolute     (FSStep* pstep, XMLTag& tag);
	void ParseContactMultiphasic(FSStep* pstep, XMLTag& tag);
	void ParseContactTiedMultiphasic(FSStep* pstep, XMLTag& tag);
	void ParseContactTied       (FSStep* pstep, XMLTag& tag);
	void ParseContactTiedF2F    (FSStep* pstep, XMLTag& tag);
	void ParseContactTiedElastic(FSStep* pstep, XMLTag& tag);
	void ParseContactSticky     (FSStep* pstep, XMLTag& tag);
	void ParseContactPeriodic   (FSStep* pstep, XMLTag& tag);
	void ParseContactRigid      (FSStep* pstep, XMLTag& tag);
	void ParseContactJoint      (FSStep* pstep, XMLTag& tag);
	void ParseContactTC         (FSStep* pstep, XMLTag& tag);
	void ParseContactTiedPoro   (FSStep* pstep, XMLTag& tag);
	void ParseRigidWall         (FSStep* pstep, XMLTag& tag);
	void ParseLinearConstraint  (FSStep* pstep, XMLTag& tag);
	FSSurface* ParseContactSurface(XMLTag& tag, int format = 0);
	void ParseContactParams(XMLTag& tag, FSPairedInterface* pc, int nid);
	void ParseConstraint(FSStep* pstep, XMLTag& tag);

	// constraint input functions
	void ParseRigidConstraint      (FSStep* pstep, XMLTag& tag);
	void ParseVolumeConstraint     (FSStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane        (FSStep* pstep, XMLTag& tag);
    void ParseNrmlFldVlctSrf       (FSStep* pstep, XMLTag& tag);
    void ParseFrictionlessFluidWall(FSStep* pstep, XMLTag& tag);

	// connector input functions
	void ParseConnector(FSStep* pstep, XMLTag& tag, const int rc);

	// loads parse functions (version 2.0 and up)
	void ParseNodeLoad   (FSStep* pstep, XMLTag& tag);
	void ParseSurfaceLoad(FSStep* pstep, XMLTag& tag);
	void ParseBodyLoad   (FSStep* pstep, XMLTag& tag);

	// surface load functions (version 2.0 and up)
	void ParseLoadPressure          (FSStep* pstep, XMLTag& tag);
	void ParseLoadTraction          (FSStep* pstep, XMLTag& tag);
	void ParseLoadFluidTraction     (FSStep* pstep, XMLTag& tag);
    void ParseLoadFluidVelocity     (FSStep* pstep, XMLTag& tag);
    void ParseLoadFluidNormalVelocity(FSStep* pstep, XMLTag& tag);
    void ParseLoadFluidRotationalVelocity(FSStep* pstep, XMLTag& tag);
    void ParseLoadFluidFlowResistance(FSStep* pstep, XMLTag& tag);
    void ParseLoadFluidBackflowStabilization(FSStep* pstep, XMLTag& tag);
    void ParseLoadFluidTangentialStabilization(FSStep* pstep, XMLTag& tag);
    void ParseLoadFSITraction       (FSStep* pstep, XMLTag& tag);
	void ParseLoadFluidFlux         (FSStep* pstep, XMLTag& tag);
	void ParseLoadSoluteFlux        (FSStep* pstep, XMLTag& tag);
	void ParseLoadNormalTraction    (FSStep* pstep, XMLTag& tag);
	void ParseLoadHeatFlux          (FSStep* pstep, XMLTag& tag);
	void ParseLoadConvectiveHeatFlux(FSStep* pstep, XMLTag& tag);
	void ParseLoadConcentrationFlux (FSStep* pstep, XMLTag& tag);
	FSSurface* ParseLoadSurface     (XMLTag& tag);

	// geometry parsing functions (version 2.0 and up)
	void ParseGeometryNodes      (FEBioInputModel::Part& part, XMLTag& tag);
	void ParseGeometryElements   (FEBioInputModel::Part& part, XMLTag& tag);
	void ParseGeometryElementData(FEBioInputModel::Part& part, XMLTag& tag);
	void ParseGeometryNodeSet    (FEBioInputModel::Part& part, XMLTag& tag);
	void ParseGeometrySurface    (FEBioInputModel::Part& part, XMLTag& tag);

	// helper functions (version 2.5 and up)
	FSNodeSet* ParseNodeSet(XMLTag& tag);

private:
	FEBioInputModel::PartInstance& GetInstance() { return *GetFEBioModel().GetInstance(0); }
	FSMesh& GetFEMesh() { return *GetInstance().GetMesh(); }
	FEBioMesh& GetFEBioMesh() { return GetFEBioModel().GetPart(0).GetFEBioMesh(); }
	GMeshObject* GetGObject() { return GetInstance().GetGObject(); }
};
