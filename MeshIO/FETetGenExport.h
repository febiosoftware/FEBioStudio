#pragma once
#include <MeshTools/FEFileExport.h>

class FETetGenExport : public FEFileExport
{
public:
	FETetGenExport(FEProject& prj);
	~FETetGenExport(void);

	bool Write(const char* szfile) override;
};
