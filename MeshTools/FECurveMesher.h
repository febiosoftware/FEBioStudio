#pragma once

class GEdge;
class FECurveMesh;

//-----------------------------------------------------------------------------
// Class for generating a mesh for edges
class FECurveMesher
{
public:
	// constructor
	FECurveMesher();

	// set the element size
	void SetElementSize(double h);

	// create the mesh
	FECurveMesh* BuildMesh(GEdge* edge);

private:
	FECurveMesh* BuildLineMesh(GEdge* edge);
	FECurveMesh* BuildEdgeMesh(GEdge* edge);

private:
	double	m_elemSize;
};
