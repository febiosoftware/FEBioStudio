#pragma once

#include <MeshTools/FEFileExport.h>

class FEHypersurfaceExport : public FEFileExport
{
public:
	FEHypersurfaceExport(void);
	~FEHypersurfaceExport(void);

	bool Export(FEProject& prj, const char* szfile);
};
