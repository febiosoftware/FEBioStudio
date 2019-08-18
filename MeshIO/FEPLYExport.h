#pragma once
#include <MeshTools/FEFileExport.h>

class FEPLYExport : public FEFileExport
{
public:
	FEPLYExport(void);
	~FEPLYExport(void);

	bool Export(FEProject& prj, const char* szfile);
};
