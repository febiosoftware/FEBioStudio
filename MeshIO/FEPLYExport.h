#pragma once
#include <MeshTools/FEFileExport.h>

class FEPLYExport : public FEFileExport
{
public:
	FEPLYExport(FEProject& prj);
	~FEPLYExport(void);

	bool Write(const char* szfile) override;
};
