#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

class STEPImport : public FEFileImport
{
public:
	STEPImport(FEProject& prj);
	~STEPImport();

	bool Load(const char* szfile);
};

