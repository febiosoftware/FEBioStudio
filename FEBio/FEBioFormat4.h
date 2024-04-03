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
class GMeshObject;

//-----------------------------------------------------------------------------
// Class that represents FEBio format 3.0
class FEBioFormat4 : public FEBioFormat
{
public:
	FEBioFormat4(FEBioFileImport* fileReader, FEBioInputModel& febio);
	~FEBioFormat4();

	bool ParseSection(XMLTag& tag) override;

private:
	// parsers for parent sections
	bool ParseModuleSection    (XMLTag& tag);
	bool ParseMaterialSection  (XMLTag& tag) override;
	bool ParseMeshSection      (XMLTag& tag);
	bool ParseMeshDomainsSection(XMLTag& tag);
	bool ParseMeshDataSection  (XMLTag& tag);
	bool ParseMeshAdaptorSection(XMLTag& tag);
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

	FSStep* NewStep(FSModel& fem, const std::string& typeStr, const char* szname = 0);

	// geometry parsing functions (version 2.0 and up)
	void ParseGeometryNodes      (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryElements   (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryNodeSet    (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryEdgeSet    (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometrySurface    (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryElementSet (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryPartList   (FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryDiscreteSet(FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometrySurfacePair(FEBioInputModel::Part* part, XMLTag& tag);
	void ParseGeometryPart       (XMLTag& tag);
	void ParseGeometryInstance   (XMLTag& tag);

	void ParseSolidDomain(XMLTag& tag);
	void ParseShellDomain(XMLTag& tag);
	void ParseBeamDomain (XMLTag& tag);

private:
	// boundary condition input functions
	void ParseBC		  (FSStep* pstep, XMLTag& tag);

	// mesh data sections
	bool ParseElementDataSection(XMLTag& tag);
	bool ParseSurfaceDataSection(XMLTag& tag);
	bool ParseNodeDataSection(XMLTag& tag);

private:
	bool ParseStep(XMLTag& tag);
	bool ParseLoadController(XMLTag& tag);

private:
	// contact input functions
	void ParseContact(FSStep* pstep, XMLTag& tag);

private:
	// loads parse functions
	void ParseNodeLoad   (FSStep* pstep, XMLTag& tag);
	void ParseSurfaceLoad(FSStep* pstep, XMLTag& tag);
	void ParseBodyLoad   (FSStep* pstep, XMLTag& tag);

private:
	// constraint input functions
	void ParseNLConstraint(FSStep* pstep, XMLTag& tag);

private:
	// rigid input functions
	void ParseRigidConnector(FSStep* pstep, XMLTag& tag);
	void ParseRigidBC(FSStep* pstep, XMLTag& tag);
	void ParseRigidIC(FSStep* pstep, XMLTag& tag);
	void ParseRigidLoad(FSStep* pstep, XMLTag& tag);

private:
	FEBioInputModel::Part* DefaultPart();

private:
	// Geometry format flag
	// 0 = don't know yet
	// 1 = old fomrat
	// 2 = new format (i.e. parts and instances)
	int m_geomFormat;	
};
