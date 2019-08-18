#pragma once
#include "FEModifier.h"

//-----------------------------------------------------------------------------
//! This class implements a modifier that coarses a plygonal mesh using an
//! approximated centroidal voronoi diagram.
class FEMeshSmoothingModifier : public FEModifier
{

public:
	//! Constructor
	FEMeshSmoothingModifier();

	//! Apply the smoothing modifier
	FEMesh* Apply(FEMesh* pm);
	double area_triangle(vec3d r[3]);
	double distance(vec3d x,vec3d y );
	void Laplacian_Smoothing(FEMesh* pm,vector<int> hashmap);
	void Laplacian_Smoothing2(FEMesh* pm,vector<int> hashmap);
	void Taubin_Smoothing(FEMesh* pm,vector<int> hashmap);
	void Crease_Enhancing_Diffusion(FEMesh* pm,vector<int> hashmap);
	void Add_Noise(FEMesh* pm,vector<int> hashmap);
public:
	double	m_threshold1;
	double	m_threshold2;
	int		m_iteration;
	double	m_noise;
	int m_method;
};
