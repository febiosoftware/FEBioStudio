#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

class FEPLYImport : public FEFileImport
{
public:
	FEPLYImport();
	~FEPLYImport();

	bool Load(FEProject& prj, const char* szfile);
};
