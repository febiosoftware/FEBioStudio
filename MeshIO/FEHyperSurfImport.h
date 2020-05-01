#pragma once

#include "FileReader.h"
#include <MeshTools/FEProject.h>

class FEHyperSurfImport : public FEFileImport
{
public:
	FEHyperSurfImport(FEProject& prj);
	~FEHyperSurfImport(void);

	bool Load(const char* szfile) override;
};
