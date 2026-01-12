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

#include <FEMLib/GMaterial.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FSProject.h>
#include <MeshIO/FSFileImport.h>
#include <FEBioXML/XMLReader.h>
#include "FEBioFormat.h"

// Implements a class to import FEBio files
class FEBioFileImport : public FSFileImport
{
public:
	FEBioFileImport(FSProject& prj);
	~FEBioFileImport();
	bool Load(const char* szfile) override;

	const char* GetLog() { return m_szlog; }
	void ClearLog();

	void AddLogEntry(const char* sz, ...);

	void SetGeometryOnlyFlag(bool b);
    void SetSkipGeometryFlag(bool b);

public:
	bool ImportMaterials(const char* szfile);

protected:
	bool UpdateFEModel(FSModel& fem);

private:
	bool ParseVersion(XMLTag& tag);
	bool ReadFile(XMLTag& tag);

protected: // Error handling
	void ParseUnknownTag(XMLTag& tag);
	void ParseUnknownAttribute(XMLTag& tag, const char* szatt);

protected:
	FEBioFormat*	m_fmt;
	FEBioInputModel*		m_febio;
	int				m_nversion;
	char			m_szpath[1024];

protected:
	char*	m_szlog;	//!< log used for reporting warnings
	bool	m_geomOnly;
    bool    m_skipGeom;

	friend class FEBioFormat;
};


// helper class for reading geometry from feb file
class FEBioGeometryImport : public FEBioFileImport
{
public:
	FEBioGeometryImport(FSProject& prj) : FEBioFileImport(prj) { SetGeometryOnlyFlag(true); }
};
