#pragma once
#include <vector>

class FEMesh;

class TetOverlap
{
public:
	TetOverlap();

	bool Apply(FEMesh* mesh, std::vector<std::pair<int, int> >& tetList);
};
