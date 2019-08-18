#pragma once
#include <MeshTools/FEFileExport.h>

class OArchive;

class PRVObjectExport : public FEFileExport
{
public:
	PRVObjectExport();

	bool Export(FEProject& prj, const char* szfile);

private:
	bool SaveObjects(OArchive& ar, FEProject& prj);

private:
	bool	m_selectedObjectsOnly;
	bool	m_exportDiscrete;
};
