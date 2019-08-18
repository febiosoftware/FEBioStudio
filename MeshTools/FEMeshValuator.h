#pragma once
#include <MeshLib/FEMesh.h>

class FEMeshValuator
{
public:
	// constructor
	FEMeshValuator(FEMesh& mesh);

	// evaluate the particular data field
	void Evaluate(int nfield);

	// evaluate just one element
	double EvaluateElement(int i, int nfield, int* err = 0);

private:
	FEMesh& m_mesh;
};
