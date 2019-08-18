#pragma once
#include <MeshLib/FEMesh.h>

class FESurfaceIntersect
{
public:
	int Apply(FESurface* psrc, FESurface* ptrg, double mindist);

private:
	double Distance(FEFaceList& s, const vec3d& r);
};
