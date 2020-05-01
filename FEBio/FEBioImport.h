#pragma once

#include <MeshTools/GMaterial.h>
#include <FEMLib/FEMultiMaterial.h>
#include <MeshTools/FEProject.h>
#include <MeshIO/FileReader.h>
#include <XML/XMLReader.h>
#include "FEBioFormat.h"

//-----------------------------------------------------------------------------
// Implements a class to import FEBio files
// 
class FEBioImport : public FEFileImport
{
public:
	FEBioImport(FEProject& prj);
	~FEBioImport();
	bool Load(const char* szfile) override;

	const char* GetLog() { return m_szlog; }
	void ClearLog();

	void AddLogEntry(const char* sz, ...);

	void SetGeometryOnlyFlag(bool b);

protected:
	bool UpdateFEModel(FEModel& fem);

private:
	bool ParseVersion(XMLTag& tag);
	bool ReadFile(XMLTag& tag);

protected: // Error handling
	void ParseUnknownTag(XMLTag& tag);
	void ParseUnknownAttribute(XMLTag& tag, const char* szatt);

protected:
	FEBioFormat*	m_fmt;
	FEBioModel*		m_febio;
	int				m_nversion;
	char			m_szpath[1024];

protected:
	char*	m_szlog;	//!< log used for reporting warnings
	bool	m_geomOnly;

	friend class FEBioFormat;
};
