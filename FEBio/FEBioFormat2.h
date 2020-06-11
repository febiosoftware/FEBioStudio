/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
	FEBioFormat2(FEBioImport* fileReader, FEBioModel& febio);
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
	void ParseBCFixed     (FEStep* pstep, XMLTag& tag);
	void ParseBCPrescribed(FEStep* pstep, XMLTag& tag);
	void ParseSprings     (FEStep* pstep, XMLTag& tag);
	void ParseContact     (FEStep* pstep, XMLTag& tag);
	void ParseBodyForce   (FEStep* pstep, XMLTag& tag);
	void ParseHeatSource  (FEStep* pstep, XMLTag& tag);

	// contact input functions
	void ParseContactSliding    (FEStep* pstep, XMLTag& tag);
	void ParseContactF2FSliding (FEStep* pstep, XMLTag& tag);
	void ParseContactBiphasic   (FEStep* pstep, XMLTag& tag);
	void ParseContactSolute     (FEStep* pstep, XMLTag& tag);
	void ParseContactMultiphasic(FEStep* pstep, XMLTag& tag);
	void ParseContactTiedMultiphasic(FEStep* pstep, XMLTag& tag);
	void ParseContactTied(FEStep* pstep, XMLTag& tag);
	void ParseContactTiedElastic(FEStep* pstep, XMLTag& tag);
	void ParseContactSticky     (FEStep* pstep, XMLTag& tag);
	void ParseContactPeriodic   (FEStep* pstep, XMLTag& tag);
	void ParseContactRigid      (FEStep* pstep, XMLTag& tag);
	void ParseContactJoint      (FEStep* pstep, XMLTag& tag);
	void ParseContactTC         (FEStep* pstep, XMLTag& tag);
	void ParseContactTiedPoro   (FEStep* pstep, XMLTag& tag);
	void ParseRigidWall         (FEStep* pstep, XMLTag& tag);
	void ParseLinearConstraint  (FEStep* pstep, XMLTag& tag);
	FESurface* ParseContactSurface(XMLTag& tag, int format = 0);
	void ParseContactParams(XMLTag& tag, FEPairedInterface* pc, int nid);

	// constraint input functions
	void ParseRigidConstraint      (FEStep* pstep, XMLTag& tag);
	void ParseVolumeConstraint     (FEStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane        (FEStep* pstep, XMLTag& tag);
    void ParseNrmlFldVlctSrf       (FEStep* pstep, XMLTag& tag);
    void ParseFrictionlessFluidWall(FEStep* pstep, XMLTag& tag);

	// connector input functions
	void ParseConnector(FEStep* pstep, XMLTag& tag, const int rc);

	// loads parse functions (version 2.0 and up)
	void ParseNodeLoad   (FEStep* pstep, XMLTag& tag);
	void ParseSurfaceLoad(FEStep* pstep, XMLTag& tag);
	void ParseBodyLoad   (FEStep* pstep, XMLTag& tag);

	// surface load functions (version 2.0 and up)
	void ParseLoadPressure          (FEStep* pstep, XMLTag& tag);
	void ParseLoadTraction          (FEStep* pstep, XMLTag& tag);
	void ParseLoadFluidTraction     (FEStep* pstep, XMLTag& tag);
    void ParseLoadFluidVelocity     (FEStep* pstep, XMLTag& tag);
    void ParseLoadFluidNormalVelocity(FEStep* pstep, XMLTag& tag);
    void ParseLoadFluidRotationalVelocity(FEStep* pstep, XMLTag& tag);
    void ParseLoadFluidFlowResistance(FEStep* pstep, XMLTag& tag);
    void ParseLoadFluidBackflowStabilization(FEStep* pstep, XMLTag& tag);
    void ParseLoadFluidTangentialStabilization(FEStep* pstep, XMLTag& tag);
    void ParseLoadFSITraction       (FEStep* pstep, XMLTag& tag);
	void ParseLoadFluidFlux         (FEStep* pstep, XMLTag& tag);
	void ParseLoadSoluteFlux        (FEStep* pstep, XMLTag& tag);
	void ParseLoadNormalTraction    (FEStep* pstep, XMLTag& tag);
	void ParseLoadHeatFlux          (FEStep* pstep, XMLTag& tag);
	void ParseLoadConvectiveHeatFlux(FEStep* pstep, XMLTag& tag);
	void ParseLoadConcentrationFlux (FEStep* pstep, XMLTag& tag);
	FESurface* ParseLoadSurface     (XMLTag& tag);

	// geometry parsing functions (version 2.0 and up)
	void ParseGeometryNodes      (FEBioModel::Part& part, XMLTag& tag);
	void ParseGeometryElements   (FEBioModel::Part& part, XMLTag& tag);
	void ParseGeometryElementData(FEBioModel::Part& part, XMLTag& tag);
	void ParseGeometryNodeSet    (FEBioModel::Part& part, XMLTag& tag);
	void ParseGeometrySurface    (FEBioModel::Part& part, XMLTag& tag);

	// helper functions (version 2.5 and up)
	FENodeSet* ParseNodeSet(XMLTag& tag);

private:
	FEBioModel::PartInstance& GetInstance() { return *GetFEBioModel().GetInstance(0); }
	FEMesh& GetFEMesh() { return *GetInstance().GetMesh(); }
	FEBioMesh& GetFEBioMesh() { return GetFEBioModel().GetPart(0).GetFEBioMesh(); }
	GMeshObject* GetGObject() { return GetInstance().GetGObject(); }
};
