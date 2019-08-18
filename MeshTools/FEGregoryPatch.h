#pragma once
#include "FEMesher.h"

class FEGregoryPatch :	public FEMesher
{
public:
	enum {W, H, NX, NY, MX, MY};

	class GNode
	{
	public:
		vec3d	m_r;	// node position
		vec3d	m_n;	// node normal
	};

	class GPatch
	{
	public:
		int		m_node[4];	// vertex indices
		vec3d	m_ye[4][2];	// edge control points
		vec3d	m_yi[4][2];	// interior control points
	};

public:
	FEGregoryPatch(void);
	~FEGregoryPatch(void);

	FEMesh* BuildMesh();

	GNode&  GetNode(int i) { return m_GNode[i]; }
	FEMesh* BuildFEMesh();
	void BuildPatchData();

protected:
	void BuildPatches();
	void BuildFaces(FEMesh* pm);

	vec3d EvalPatch(GPatch& p, double r, double s);

	int NodeIndex(int i, int j) { return j*(m_nx*m_mx+1) + i; }

protected:
	vector<GNode>	m_GNode;
	vector<GPatch>	m_GPatch;

	double	m_w, m_h;
	int	m_nx, m_ny;
	int	m_mx, m_my;
};
