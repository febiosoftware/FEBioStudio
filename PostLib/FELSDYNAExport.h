#pragma once

#include "FEModel.h"
#include <stdio.h>

namespace Post {

class FELSDYNAExport
{
public:
	FELSDYNAExport();
	bool Save(FEModel& fem, int ntime, const char* szfile);

protected:
	bool ExportMesh(FEModel& fem, int ntime, const char* szfile);
	bool ExportSurface(FEModel& fem, int ntime, const char* szfile);
	bool ExportSelectedSurface(FEModel& fem, int ntime, const char* szfile);

protected:
	void NodalResults(FEModel& fem, int ntime, FILE* fp);

public:
	bool	m_bsel;
	bool	m_bsurf;
	bool	m_bnode;
};

}
