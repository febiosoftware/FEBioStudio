#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

class STEPImport : public FEFileImport
{
public:
	STEPImport();
	~STEPImport();

	bool Load(FEProject& prj, const char* szfile);
};

