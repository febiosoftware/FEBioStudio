#pragma once
#include <MeshTools/FEFileExport.h>

class OArchive;

class PRVObjectExport : public FEFileExport
{
public:
	PRVObjectExport(FEProject& prj);

	bool Write(const char* szfile) override;

private:
	bool SaveObjects(OArchive& ar, FEProject& prj);

private:
	bool	m_selectedObjectsOnly;
	bool	m_exportDiscrete;
};
