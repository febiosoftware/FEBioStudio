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
	FEBioFormat3(FEBioImport* fileReader, FEBioModel& febio);
	~FEBioFormat3();

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
	bool ParseRigidSection     (XMLTag& tag);
	bool ParseLoadDataSection  (XMLTag& tag) override;
	bool ParseControlSection   (XMLTag& tag) override;

private:
	// geometry parsing functions (version 2.0 and up)
	void ParseGeometryNodes      (FEBioModel::Part* part, XMLTag& tag);
	void ParseGeometryElements   (FEBioModel::Part* part, XMLTag& tag);
	void ParseGeometryNodeSet    (FEBioModel::Part* part, XMLTag& tag);
	void ParseGeometrySurface    (FEBioModel::Part* part, XMLTag& tag);
	void ParseGeometryElementSet (FEBioModel::Part* part, XMLTag& tag);
	void ParseGeometryDiscreteSet(FEBioModel::Part* part, XMLTag& tag);
	void ParseGeometrySurfacePair(FEBioModel::Part* part, XMLTag& tag);
	void ParseGeometryPart       (XMLTag& tag);
	void ParseGeometryInstance   (XMLTag& tag);

private:
	// boundary condition input functions
	void ParseBCFixed     (FEStep* pstep, XMLTag& tag);
	void ParseBCPrescribed(FEStep* pstep, XMLTag& tag);
	void ParseBCRigid     (FEStep* pstep, XMLTag& tag);

private:
	bool ParseStep(XMLTag& tag);

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

private:
	// constraint input functions
	void ParseVolumeConstraint     (FEStep* pstep, XMLTag& tag);
	void ParseSymmetryPlane        (FEStep* pstep, XMLTag& tag);
    void ParseNrmlFldVlctSrf       (FEStep* pstep, XMLTag& tag);
    void ParseFrictionlessFluidWall(FEStep* pstep, XMLTag& tag);

private:
	// connector input functions
	void ParseConnector(FEStep* pstep, XMLTag& tag, const int rc);

	// helper functions (version 2.5 and up)
	FEBioModel::DiscreteSet ParseDiscreteSet(XMLTag& tag);

private:
	FEBioModel::Part* DefaultPart();

private:
	// Geometry format flag
	// 0 = don't know yet
	// 1 = old fomrat
	// 2 = new format (i.e. parts and instances)
	int m_geomFormat;	
};
