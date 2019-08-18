#pragma once

#include <MeshTools/FEFileExport.h>

class FEMeshExport : public FEFileExport
{
public:
	FEMeshExport(void);
	~FEMeshExport(void);

	bool Export(FEProject& prj, const char* szfile);
};
