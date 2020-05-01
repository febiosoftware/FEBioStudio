#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

class FEPLYImport : public FEFileImport
{
public:
	FEPLYImport(FEProject& prj);
	~FEPLYImport();

	bool Load(const char* szfile);
};
