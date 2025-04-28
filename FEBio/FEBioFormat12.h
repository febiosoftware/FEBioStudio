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
// Class that represents FEBio format 1.2
class FEBioFormat12 : public FEBioFormat
{
	struct DISP_CARD
	{
		int	lc, bc, node;
		double s;
	};

	struct FORCE_CARD
	{
		int	lc, bc, node;
		double s;
	};

public:
	FEBioFormat12(FEBioFileImport* fileReader, FEBioInputModel& febio);
	~FEBioFormat12();

	bool ParseSection(XMLTag& tag);

private:
	bool ParseModuleSection    (XMLTag& tag);
	bool ParseGeometrySection  (XMLTag& tag); // for version 1.2 and before
	bool ParseBoundarySection  (XMLTag& tag); // modified in version 1.2
	bool ParseLoadsSection     (XMLTag& tag); // new in version 1.2
	bool ParseInitialSection   (XMLTag& tag);
	bool ParseConstraintSection(XMLTag& tag); // new in version 1.1 and up
	bool ParseStepSection      (XMLTag& tag);

private:
	// boundary condition input functions
	void ParseBCFixed         (FSStep* pstep, XMLTag& tag);
	void ParseBCPrescribed    (FSStep* pstep, XMLTag& tag);
	void ParseForceLoad       (FSStep* pstep, XMLTag& tag);
	void ParsePressureLoad    (FSStep* pstep, XMLTag& tag);
	void ParseTractionLoad    (FSStep* pstep, XMLTag& tag);
	void ParseFluidFlux       (FSStep* pstep, XMLTag& tag);
	void ParseBPNormalTraction(FSStep* pstep, XMLTag& tag);
	void ParseHeatFlux        (FSStep* pstep, XMLTag& tag);
	void ParseSoluteFlux      (FSStep* pstep, XMLTag& tag);
	void ParseSprings         (FSStep* pstep, XMLTag& tag);
	void ParseContact         (FSStep* pstep, XMLTag& tag);
	void ParseBodyForce       (FSStep* pstep, XMLTag& tag);
	void ParseHeatSource      (FSStep* pstep, XMLTag& tag);

private:
	// contact input functions
	void ParseContactSliding    (FSStep* pstep, XMLTag& tag);
	void ParseF2FSliding        (FSStep* pstep, XMLTag& tag);
	void ParseContactBiphasic   (FSStep* pstep, XMLTag& tag);
	void ParseContactSolute     (FSStep* pstep, XMLTag& tag);
	void ParseContactMultiphasic(FSStep* pstep, XMLTag& tag);
	void ParseContactTied       (FSStep* pstep, XMLTag& tag);
	void ParseContactSticky     (FSStep* pstep, XMLTag& tag);
	void ParseContactPeriodic   (FSStep* pstep, XMLTag& tag);
	void ParseContactRigid      (FSStep* pstep, XMLTag& tag);
	void ParseContactJoint      (FSStep* pstep, XMLTag& tag);
	void ParseContactTC         (FSStep* pstep, XMLTag& tag);
	void ParseContactTiedPoro   (FSStep* pstep, XMLTag& tag);
	void ParseRigidWall         (FSStep* pstep, XMLTag& tag);
	void ParseLinearConstraint  (FSStep* pstep, XMLTag& tag);

	FSSurface* ParseContactSurface(XMLTag& tag);

private:
	// constraint input functions
	void ParseRigidConstraint (FSStep* pstep, XMLTag& tag);
	void ParseVolumeConstraint(FSStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane   (FSStep* pstep, XMLTag& tag);

private:
	FEBioInputModel::PartInstance& GetInstance() { return *GetFEBioModel().GetInstance(0); }
	FSMesh& GetFEMesh() { return *GetInstance().GetMesh(); }
	FEBioMesh& GetFEBioMesh() { return GetFEBioModel().GetPart(0).GetFEBioMesh(); }
	GMeshObject* GetGObject() { return GetInstance().GetGObject(); }
};
