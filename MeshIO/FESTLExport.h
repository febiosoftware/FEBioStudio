#pragma once

#include <MeshTools/FEFileExport.h>

class FESTLExport : public FEFileExport
{
public:
	FESTLExport(void);
	~FESTLExport(void);

	bool Export(FEProject& prj, const char* szfile);
};
