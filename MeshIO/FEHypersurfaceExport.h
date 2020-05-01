#pragma once
#include <MeshTools/FEFileExport.h>

class FEHypersurfaceExport : public FEFileExport
{
public:
	FEHypersurfaceExport(FEProject& prj);
	~FEHypersurfaceExport(void);

	bool Write(const char* szfile) override;
};
