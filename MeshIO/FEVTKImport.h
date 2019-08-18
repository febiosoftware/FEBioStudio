#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <vector>
using namespace std;

class FEVTKimport :	public FEFileImport
{

public:
	FEVTKimport();
	~FEVTKimport(void);

	bool Load(FEProject& prj, const char* szfile);
};
