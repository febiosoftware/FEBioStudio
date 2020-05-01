#pragma once

#include <MeshTools/FEFileExport.h>

class FESTLExport : public FEFileExport
{
public:
	FESTLExport(FEProject& prj);
	~FESTLExport(void);

	bool Write(const char* szfile) override;
};
