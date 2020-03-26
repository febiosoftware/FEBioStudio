#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>


class BREPImport : public FEFileImport
{
public:
	BREPImport();
	~BREPImport();

	bool Load(FEProject& prj, const char* szfile);
};

// NOTE: There is already an IGES file reader in FEIGESFileImport.h
class IGESImport : public FEFileImport
{
public:
	IGESImport();
	~IGESImport();

	bool Load(FEProject& prj, const char* szfile);
};
