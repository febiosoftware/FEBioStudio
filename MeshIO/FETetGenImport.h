#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>
#include <list>
using namespace std;

class FETetGenImport : public FEFileImport
{
public:
	struct NODE
	{
		double x, y, z;
	};

	struct ELEM
	{
		int	node[4];
		int	att;
	};

public:
	FETetGenImport();
	~FETetGenImport(void);

	bool Load(FEProject& prj, const char* szfile);

protected:
	bool BuildMesh(FEModel& fem);

private:
	vector<NODE>	m_Node;
	vector<ELEM>	m_Elem;
	int				m_offset;
};
