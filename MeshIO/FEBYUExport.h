#pragma once

#include <MeshTools/FEFileExport.h>

class FEBYUExport : public FEFileExport
{
public:
	FEBYUExport(FEProject& prj);
	~FEBYUExport(void);

	bool Write(const char* szfile) override;
};
