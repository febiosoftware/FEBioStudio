#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <vector>
using namespace std;

class FEVTKimport :	public FEFileImport
{

public:
	FEVTKimport(FEProject& prj);
	~FEVTKimport(void);

	bool Load(const char* szfile);
};
