#pragma once
#include <XML/XMLReader.h>
#include <MeshTools/FEProject.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEMultiMaterial.h>
#include "FEBioModel.h"

//-----------------------------------------------------------------------------
class FEBioImport;

//-----------------------------------------------------------------------------
// Class that represents a specific FEBio specification format.
// Sub-classes implement different formats.
class FEBioFormat
{
public:
	FEBioFormat(FEBioImport* fileReader, FEBioModel& febio);
	virtual ~FEBioFormat();

	// override this function for processing the top-level sections
	virtual bool ParseSection(XMLTag& tag) = 0;

	void SetGeometryOnlyFlag(bool b);

protected:
	FEBioImport* FileReader() { return m_fileReader; }

	void ParseUnknownTag(XMLTag& tag);
	void ParseUnknownAttribute(XMLTag& tag, const char* szatt);

	bool ReadParam(ParamContainer& PC, XMLTag& tag);
	bool ReadChoiceParam(Param& p, XMLTag& tag);
	void ReadParameters(ParamContainer& PC, XMLTag& tag);


	FEAnalysisStep* NewStep(FEModel& fem, int nanalysis, const char* sz = 0);

	FEBioModel& GetFEBioModel() { return m_febio; }

	FEModel& GetFEModel() { return m_febio.GetFEModel(); }

protected:
	// common parse functions
	virtual bool ParseControlSection (XMLTag& tag);
	bool ParseGlobalsSection (XMLTag& tag);
	bool ParseMaterialSection(XMLTag& tag);
	bool ParseOutputSection  (XMLTag& tag);
	virtual bool ParseLoadDataSection(XMLTag& tag);
	bool ParsePlotfileSection(XMLTag& tag);
	bool ParseLogfileSection (XMLTag& tag);

	// material section helper functions
	FEMaterial* ParseMaterial(XMLTag& tag, const char* szmat);
	FEMaterial* ParseRigidBody(XMLTag& tag);
	void ParseFiberMaterial(FEOldFiberMaterial& fiber, XMLTag& tag);
	FEMaterial* ParseTransIsoMR    (FEMaterial* pm, XMLTag& tag);
	FEMaterial* ParseTransIsoVW    (FEMaterial* pm, XMLTag& tag);
	FEMaterial* ParseBiphasicSolute(FEMaterial* pm, XMLTag& tag);
	FEMaterial* ParseTriphasic     (FEMaterial* pm, XMLTag& tag);
	FEMaterial* ParseMultiphasic   (FEMaterial* pm, XMLTag& tag);
	FEMaterial* ParseReactionDiffusion(FEMaterial* pm, XMLTag& tag);
	FEMaterial* Parse1DFunction(FEMaterial* pm, XMLTag& tag);
    FEMaterial* ParseOsmoManning   (FEMaterial* pm, XMLTag& tag);
	void ParseMatAxis(XMLTag& tag, FEMaterial* mat);
	void ParseFiber(XMLTag& tag, FEMaterial* mat);

	FEReactionMaterial* ParseReaction(XMLTag& tag);
	FEReactionMaterial* ParseReaction2(XMLTag& tag);

private:
	FEBioModel&		m_febio;
	FEBioImport*	m_fileReader;

protected: // TODO: Move to FEBioModel?
	bool		m_geomOnly;	// read only geometry section
	int			m_nAnalysis;	// analysis type
	FEStep*		m_pstep;		// current analysis step
	FEStep*		m_pBCStep;		// step to which BCs are assigned
};

// return the DOF code from a bc string.
int GetDOFCode(const char* sz);
