#pragma once
#include "FEBioImport.h"
#include "FEBioFormat.h"

//-----------------------------------------------------------------------------
// Class that represents older FEBio formats: formats 1.1
class FEBioFormatOld : public FEBioFormat
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
	FEBioFormatOld(FEBioImport* fileReader, FEBioModel& febio);
	~FEBioFormatOld();

	bool ParseSection(XMLTag& tag);

private:
	bool ParseModuleSection    (XMLTag& tag);
	bool ParseGeometrySection  (XMLTag& tag); // for version 1.2 and before
	bool ParseBoundarySection  (XMLTag& tag); // modified in version 1.2
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
	void ParseContactF2FSliding (FEStep* pstep, XMLTag& tag);
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

private:
	FEBioModel::PartInstance& GetInstance() { return *GetFEBioModel().GetInstance(0); }
	FEMesh& GetFEMesh() { return *GetInstance().GetMesh(); }
	FEBioMesh& GetFEBioMesh() { return GetFEBioModel().GetPart(0).GetFEBioMesh(); }
	GMeshObject* GetGObject() { return GetInstance().GetGObject(); }
};
