#pragma once
#include <MeshTools/FEFileExport.h>

class FETetGenExport : public FEFileExport
{
public:
	FETetGenExport(void);
	~FETetGenExport(void);

	bool Export(FEProject& prj, const char* szfile);
};
