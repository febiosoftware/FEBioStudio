#pragma once
#include "FESurfaceModifier.h"

//-----------------------------------------------------------------------------
// This modifier implements a list of tools to fixing meshes
class FEFixMesh : public FESurfaceModifier
{
public:
	FEFixMesh();
	FESurfaceMesh* Apply(FESurfaceMesh* pm);

	bool RemoveDuplicateFaces  (FESurfaceMesh* pm);
	bool RemoveNonManifoldFaces(FESurfaceMesh* pm);
	bool FixElementWinding     (FESurfaceMesh* pm);
	bool InvertMesh            (FESurfaceMesh* pm);
	bool FillAllHoles		   (FESurfaceMesh* pm);
	bool RemoveDuplicateEdges  (FESurfaceMesh* pm);
};
