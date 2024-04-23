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
#include <XML/XMLReader.h>
#include <FEMLib/FSProject.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEMultiMaterial.h>
#include "FEBioInputModel.h"

//-----------------------------------------------------------------------------
class FEBioFileImport;

//-----------------------------------------------------------------------------
// Class that represents a specific FEBio specification format.
// Sub-classes implement different formats.
class FEBioFormat
{
public:
	FEBioFormat(FEBioFileImport* fileReader, FEBioInputModel& febio);
	virtual ~FEBioFormat();

	// override this function for processing the top-level sections
	virtual bool ParseSection(XMLTag& tag) = 0;

	void SetGeometryOnlyFlag(bool b);
    void SetSkipGeometryFlag(bool b);

protected:
	FEBioFileImport* FileReader() { return m_fileReader; }

	void ParseUnknownTag(XMLTag& tag);
	void ParseUnknownAttribute(XMLTag& tag, const char* szatt);

	bool ReadParam(ParamContainer& PC, XMLTag& tag);
	bool ReadChoiceParam(Param& p, const char* szval);
	void ReadParameters(ParamContainer& PC, XMLTag& tag);

public:
	FSAnalysisStep* NewStep(FSModel& fem, int nanalysis, const char* sz = 0);

	FEBioInputModel& GetFEBioModel() { return m_febio; }

	FSModel& GetFSModel() { return m_febio.GetFSModel(); }

protected:
	// common parse functions
	virtual bool ParseControlSection (XMLTag& tag);
	bool ParseGlobalsSection (XMLTag& tag);
	virtual bool ParseMaterialSection(XMLTag& tag);
	bool ParseOutputSection  (XMLTag& tag);
	virtual bool ParseLoadDataSection(XMLTag& tag);
	bool ParsePlotfileSection(XMLTag& tag);
	bool ParseLogfileSection (XMLTag& tag);

	// material section helper functions
	FSMaterial* ParseMaterial(XMLTag& tag, const char* szmat, int classId = -1);
	FSMaterial* ParseRigidBody(XMLTag& tag);
	void ParseFiberMaterial(FSOldFiberMaterial& fiber, XMLTag& tag);
	FSMaterial* ParseTransIsoMR    (FSMaterial* pm, XMLTag& tag);
	FSMaterial* ParseTransIsoVW    (FSMaterial* pm, XMLTag& tag);
	FSMaterial* ParseBiphasicSolute(FSMaterial* pm, XMLTag& tag);
	FSMaterial* ParseTriphasic     (FSMaterial* pm, XMLTag& tag);
	FSMaterial* ParseMultiphasic   (FSMaterial* pm, XMLTag& tag);
	FSMaterial* ParseReactionDiffusion(FSMaterial* pm, XMLTag& tag);
	FSMaterial* Parse1DFunction(FSMaterial* pm, XMLTag& tag);
    FSMaterial* ParseOsmoManning   (FSMaterial* pm, XMLTag& tag);
	void ParseMatAxis(XMLTag& tag, FSMaterial* mat);
	void ParseFiber(XMLTag& tag, FSMaterial* mat);
	void ParseFiberProperty(XMLTag& tag, FSFiberMaterial* mat);

	FSReactionMaterial* ParseReaction(XMLTag& tag);
	FSReactionMaterial* ParseReaction2(XMLTag& tag);
    FSMembraneReactionMaterial* ParseMembraneReaction(XMLTag& tag);

	void ParseMappedParameter(XMLTag& tag, Param* param);

protected:
	// NOTE: This is only used by FEBioFormat4 and FEBioFormat3.
	//       Do not use in older file readers. 
	void ParseModelComponent(FSModelComponent* pmc, XMLTag& tag);

private:
	FEBioInputModel&		m_febio;
	FEBioFileImport*	m_fileReader;

protected: // TODO: Move to FEBioInputModel?
	bool		m_geomOnly;	// read only geometry section
    bool        m_skipGeom; // read everything but the geometry section
	int			m_nAnalysis;	// analysis type (Obsolete: only used by feb3 or older file readers)

	FSStep*		m_pstep;		// current analysis step
	FSStep*		m_pBCStep;		// step to which BCs are assigned

	std::string		m_defaultSolver;
};

// return the DOF code from a bc string.
int GetDOFCode(const char* sz);

// convertors for custom types
template <> void string_to_type<GLColor>(const std::string& s, GLColor& v);
