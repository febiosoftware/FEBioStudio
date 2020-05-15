#pragma once
#include <vector>

class FEMesh;
class FEMeshBase;

namespace MeshTools {
	std::vector<int> FindSurfaceOverlap(FEMesh* mesh, FEMeshBase* trg);
}
