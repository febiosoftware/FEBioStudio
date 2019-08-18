#pragma once
#include <MathLib/math3d.h>
#include <vector>
using namespace std;

class FEModel;
class GObject;
class FEMesh;

//-----------------------------------------------------------------------------
// Class describing the foam generator.
class FoamGen
{
private:
	struct NODE
	{
		vec3d	r;	// nodal position
		double	v;	// nodal value
		vec3d	g;	// gradient
	};

	struct CELL
	{
		int		n[8];
		int		ntag;
	};

	struct CELL2D
	{
		int		n[4];
		int		ntag;
	};

	struct EDGE
	{
		int		n[2];
		double	w;
		vec3d	r;
	};

	struct FACE
	{
		int		n[3];
		int		nid;
	};

	struct N2E
	{
		int		e[6];
		int		ne;
	};

public:
	FoamGen();

	// Create the foam object
	FEMesh* Create();

public:
	int		m_nx, m_ny, m_nz;
	double	m_w, m_h, m_d;
	int		m_nseed;
	int		m_nsmooth;
	double	m_ref;

protected:
	double	m_eps;

protected:
	NODE& Node(int i, int j, int k) { return m_Node[k*(m_nx+1)*(m_ny+1)+j*(m_nx+1)+i]; }

	void CreateGrid();
	void EvalGrid();
	void CalcGradient();	// not used (for now; and not finished)
	void SmoothGrid();
	void SmoothMesh(FEMesh* pm, int niter, double w);
	void DistortGrid();
	FEMesh* CreateMesh();
	FEMesh* WeldMesh(FEMesh* pm);

	int FindEdge(int n1, int n2);

	void SelectFace(int i, FEMesh* pm);

protected:
	vector<NODE>	m_Node;
	vector<CELL>	m_Cell;
	vector<CELL2D>	m_Cell2D[6];
	vector<NODE>	m_Seed;
	vector<EDGE>	m_Edge;
	vector<FACE>	m_Face;
	vector<N2E>		m_NET;
	
	int		m_nface[8];
};
