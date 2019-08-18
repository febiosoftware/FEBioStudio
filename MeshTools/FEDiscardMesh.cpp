#include "stdafx.h"
#include "FEDiscardMesh.h"

FEDiscardMesh::FEDiscardMesh() : FEModifier("Discard mesh")
{
}

FEMesh* FEDiscardMesh::Apply(FEMesh* pm)
{
	return pm->ExtractFaces(false);
}
