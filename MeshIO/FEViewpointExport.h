#pragma once

#include <MeshTools/FEFileExport.h>

class FEViewpointExport : public FEFileExport
{
public:
	FEViewpointExport(FEProject& prj);
	~FEViewpointExport(void);

	bool Write(const char* szfile) override;
};
