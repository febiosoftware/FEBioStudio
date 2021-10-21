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
	FEBioFormat12(FEBioImport* fileReader, FEBioModel& febio);
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
	void ParseBCFixed         (FEStep* pstep, XMLTag& tag);
	void ParseBCPrescribed    (FEStep* pstep, XMLTag& tag);
	void ParseForceLoad       (FEStep* pstep, XMLTag& tag);
	void ParsePressureLoad    (FEStep* pstep, XMLTag& tag);
	void ParseTractionLoad    (FEStep* pstep, XMLTag& tag);
	void ParseFluidFlux       (FEStep* pstep, XMLTag& tag);
	void ParseBPNormalTraction(FEStep* pstep, XMLTag& tag);
	void ParseHeatFlux        (FEStep* pstep, XMLTag& tag);
	void ParseSoluteFlux      (FEStep* pstep, XMLTag& tag);
	void ParseSprings         (FEStep* pstep, XMLTag& tag);
	void ParseContact         (FEStep* pstep, XMLTag& tag);
	void ParseBodyForce       (FEStep* pstep, XMLTag& tag);
	void ParseHeatSource      (FEStep* pstep, XMLTag& tag);

private:
	// contact input functions
	void ParseContactSliding    (FEStep* pstep, XMLTag& tag);
	void ParseF2FSliding        (FEStep* pstep, XMLTag& tag);
	void ParseContactBiphasic   (FEStep* pstep, XMLTag& tag);
	void ParseContactSolute     (FEStep* pstep, XMLTag& tag);
	void ParseContactMultiphasic(FEStep* pstep, XMLTag& tag);
	void ParseContactTied       (FEStep* pstep, XMLTag& tag);
	void ParseContactSticky     (FEStep* pstep, XMLTag& tag);
	void ParseContactPeriodic   (FEStep* pstep, XMLTag& tag);
	void ParseContactRigid      (FEStep* pstep, XMLTag& tag);
	void ParseContactJoint      (FEStep* pstep, XMLTag& tag);
	void ParseContactTC         (FEStep* pstep, XMLTag& tag);
	void ParseContactTiedPoro   (FEStep* pstep, XMLTag& tag);
	void ParseRigidWall         (FEStep* pstep, XMLTag& tag);
	void ParseLinearConstraint  (FEStep* pstep, XMLTag& tag);
	void ParseContactSurface    (FESurface* ps, XMLTag& tag);

private:
	// constraint input functions
	void ParseRigidConstraint (FEStep* pstep, XMLTag& tag);
	void ParseVolumeConstraint(FEStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane   (FEStep* pstep, XMLTag& tag);

private:
	FEBioModel::PartInstance& GetInstance() { return *GetFEBioModel().GetInstance(0); }
	FEMesh& GetFEMesh() { return *GetInstance().GetMesh(); }
	FEBioMesh& GetFEBioMesh() { return GetFEBioModel().GetPart(0).GetFEBioMesh(); }
	GMeshObject* GetGObject() { return GetInstance().GetGObject(); }
};
