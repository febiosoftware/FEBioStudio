#pragma once
#include <MeshTools/FEFileExport.h>

class FEMeshExport : public FEFileExport
{
public:
	FEMeshExport(FEProject& prj);
	~FEMeshExport(void);

	bool Write(const char* szfile) override;
};
