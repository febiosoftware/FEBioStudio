#pragma once

#include <MeshTools/FEFileExport.h>

class FEBYUExport : public FEFileExport
{
public:
	FEBYUExport(void);
	~FEBYUExport(void);

	bool Export(FEProject& prj, const char* szfile);
};
