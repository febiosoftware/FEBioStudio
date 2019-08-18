#pragma once

#include "FEFileReader.h"
#include "math3d.h"
#include <vector>

namespace Post{
class GMeshImport : public FEFileReader
{
	struct NODE 
	{
		vec3f	r;
	};

	struct ELEM
	{
		int	ntype;
		int	node[8];
	};

public:
	GMeshImport(void);
	~GMeshImport(void);

	bool Load(FEModel& fem, const char* szfile);

protected:
	bool ReadNodes();
	bool ReadElements();
	bool BuildMesh(FEModel& fem);

protected:
	std::vector<NODE>	m_Node;
	std::vector<ELEM>	m_Elem;
};
}
