#pragma once
#include "FEFileReader.h"
#include <MathLib/math3d.h>
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
	GMeshImport(FEPostModel* fem);
	~GMeshImport(void);

	bool Load(const char* szfile) override;

protected:
	bool ReadNodes();
	bool ReadElements();
	bool BuildMesh(FEPostModel& fem);

protected:
	std::vector<NODE>	m_Node;
	std::vector<ELEM>	m_Elem;
};
}
