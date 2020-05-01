#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>


class BREPImport : public FEFileImport
{
public:
	BREPImport(FEProject& prj);
	~BREPImport();

	bool Load(const char* szfile);
};

// NOTE: There is already an IGES file reader in FEIGESFileImport.h
class IGESImport : public FEFileImport
{
public:
	IGESImport(FEProject& prj);
	~IGESImport();

	bool Load(const char* szfile);
};
