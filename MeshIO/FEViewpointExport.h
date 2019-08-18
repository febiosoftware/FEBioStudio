#pragma once

#include <MeshTools/FEFileExport.h>

class FEViewpointExport : public FEFileExport
{
public:
	FEViewpointExport(void);
	~FEViewpointExport(void);

	bool Export(FEProject& prj, const char* szfile);
};
