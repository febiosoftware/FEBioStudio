#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <vector>
using namespace std;

class FEBYUimport :	public FEFileImport
{
protected:
	struct PART
	{
		int n0;	// first polygon number
		int n1; // last polygon number
	};

public:
	FEBYUimport(FEProject& prj);
	~FEBYUimport(void);

	bool Load(const char* szfile);

protected:
	vector<PART>	m_Part;
};
