#pragma once

#include "FileReader.h"
#include <MeshTools/FEProject.h>

class FEHyperSurfImport : public FEFileImport
{
public:
	FEHyperSurfImport();
	~FEHyperSurfImport(void);

	bool Load(FEProject& prj, const char* szfile);
};
