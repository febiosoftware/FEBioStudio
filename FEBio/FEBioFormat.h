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
	FEMaterial* ParseMaterial(XMLTag& tag, const char* szmat, int classId = -1);
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
	void ParseFiberProperty(XMLTag& tag, FEFiberMaterial* mat);

	FEReactionMaterial* ParseReaction(XMLTag& tag);
	FEReactionMaterial* ParseReaction2(XMLTag& tag);
    FEMembraneReactionMaterial* ParseMembraneReaction(XMLTag& tag);

	void ParseMappedParameter(XMLTag& tag, Param* param);

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
