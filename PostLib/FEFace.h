#pragma once
#include "FEItem.h"
#include "FEEdge.h"
#include <MathLib/math3d.h>
#include <MeshLib/FEFace.h>
#include <assert.h>

namespace Post {

//-----------------------------------------------------------------------------
// Class that describes an exterior face of the mesh
class FEFace : public FEItem
{
public:
	enum { MAX_NODES = 10 };

public:
	int	node[MAX_NODES];	// array of indices to the four nodes of a face
	int	m_ntype;			// type of face

	int	m_nsg;				// smoothing group ID
	int	m_mat;				// material id
	int	m_nbr[4];			// neighbour faces
	int	m_elem[2];			// first index = element to which this face belongs, second index = local element face number

	vec3f	m_fn;				// face normal
	vec3f	m_nn[MAX_NODES];	// node normals
	float	m_tex[MAX_NODES];	// nodal 1D-texture coordinates
	float	m_texe;				// element texture coordinate

public:
	FEFace();

	bool operator == (const FEFace& face)
	{
		const int* pn = face.node;
		if (m_ntype != face.m_ntype) return false;
		switch (m_ntype)
		{
		case FE_FACE_TRI3:
		case FE_FACE_TRI6:
		case FE_FACE_TRI7:
		case FE_FACE_TRI10:
			if ((pn[0] != node[0]) && (pn[0] != node[1]) && (pn[0] != node[2])) return false;
			if ((pn[1] != node[0]) && (pn[1] != node[1]) && (pn[1] != node[2])) return false;
			if ((pn[2] != node[0]) && (pn[2] != node[1]) && (pn[2] != node[2])) return false;
			break;
		case FE_FACE_QUAD4:
		case FE_FACE_QUAD8:
		case FE_FACE_QUAD9:
			if ((pn[0] != node[0]) && (pn[0] != node[1]) && (pn[0] != node[2]) && (pn[0] != node[3])) return false;
			if ((pn[1] != node[0]) && (pn[1] != node[1]) && (pn[1] != node[2]) && (pn[1] != node[3])) return false;
			if ((pn[2] != node[0]) && (pn[2] != node[1]) && (pn[2] != node[2]) && (pn[2] != node[3])) return false;
			if ((pn[3] != node[0]) && (pn[3] != node[1]) && (pn[3] != node[2]) && (pn[3] != node[3])) return false;
			break;
		}
		return true;
	}

	bool HasEdge(int n1, int n2)
	{
		switch (m_ntype)
		{
		case FE_FACE_TRI3:
		case FE_FACE_TRI6:
		case FE_FACE_TRI7:
		case FE_FACE_TRI10:
			if (((node[0] == n1) && (node[1] == n2)) || ((node[1] == n1) && (node[0] == n2))) return true;
			if (((node[1] == n1) && (node[2] == n2)) || ((node[2] == n1) && (node[1] == n2))) return true;
			if (((node[2] == n1) && (node[0] == n2)) || ((node[0] == n1) && (node[2] == n2))) return true;
			break;
		case FE_FACE_QUAD4:
		case FE_FACE_QUAD8:
		case FE_FACE_QUAD9:
			if (((node[0] == n1) && (node[1] == n2)) || ((node[1] == n1) && (node[0] == n2))) return true;
			if (((node[1] == n1) && (node[2] == n2)) || ((node[2] == n1) && (node[1] == n2))) return true;
			if (((node[2] == n1) && (node[3] == n2)) || ((node[3] == n1) && (node[2] == n2))) return true;
			if (((node[3] == n1) && (node[0] == n2)) || ((node[0] == n1) && (node[3] == n2))) return true;
			break;
		}
		return false;
	}

	bool HasNode(int n) const
	{
		int N = Nodes();
		for (int i = 0; i<N; ++i) if (node[i] == n) return true;
		return false;
	}

	int Nodes() const
	{
		const int n[8] = { -1, 3, 4, 6, 7, 8, 9, 10 };
		assert((m_ntype >= 0) && (m_ntype <= 6));
		return n[m_ntype];
	}

	int Edges() const
	{
		const int n[8] = { -1, 3, 4, 3, 3, 4, 4, 3 };
		assert((m_ntype >= 0) && (m_ntype <= 6));
		return n[m_ntype];
	}

	FEEdge Edge(int i);

	// evaluate shape function at iso-parameteric point (r,s)
	void shape(double* H, double r, double s);

	// evaluate a vector expression at iso-points (r,s)
	double eval(double* d, double r, double s);

	// evaluate a vector expression at iso-points (r,s)
	vec3f eval(vec3f* v, double r, double s);
};
}
