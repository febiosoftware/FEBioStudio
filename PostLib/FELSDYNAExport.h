#pragma once

#include "FEPostModel.h"
#include <stdio.h>

namespace Post {

class FELSDYNAExport
{
public:
	FELSDYNAExport();
	bool Save(FEPostModel& fem, int ntime, const char* szfile);

protected:
	bool ExportMesh(FEPostModel& fem, int ntime, const char* szfile);
	bool ExportSurface(FEPostModel& fem, int ntime, const char* szfile);
	bool ExportSelectedSurface(FEPostModel& fem, int ntime, const char* szfile);

protected:
	void NodalResults(FEPostModel& fem, int ntime, FILE* fp);

public:
	bool	m_bsel;
	bool	m_bsurf;
	bool	m_bnode;
};

}
