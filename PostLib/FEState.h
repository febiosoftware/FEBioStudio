#pragma once
#include "FEMeshData.h"
#include "FEElement.h"
#include <vector>
#include "ValArray.h"
using namespace std;

//-----------------------------------------------------------------------------
// forward declaration of the mesh
namespace Post {
	class FEModel;
	class FEMeshBase;

//-----------------------------------------------------------------------------
enum StatusFlags {
	ACTIVE	= 0x01,			// item has data
	VISIBLE	= 0x02			// item is visible (i.e. not eroded)
};

//-----------------------------------------------------------------------------
// Data classes for mesh items
struct NODEDATA
{
	vec3f	m_rt;	// nodal position determined by displacement map
	float	m_val;	// current nodal value
	int		m_ntag;	// active flag
};

struct EDGEDATA
{
	float	m_val;		// current value
	int		m_ntag;		// active flag
	float	m_nv[FEEdge::MAX_NODES]; // nodal values
};

struct ELEMDATA
{
	float			m_val;		// current element value
	unsigned int	m_state;	// state flags
	float			m_h[FEGenericElement::MAX_NODES];		// shell thickness (TODO: Can we move this to the face data?)
};

struct FACEDATA
{
	int		m_ntag;		// active flag
	float	m_val;		// current face value
};

struct LINEDATA
{
	vec3f	m_r0;
	vec3f	m_r1;
	float	m_val[2];
	float	m_user_data[2];
};

struct POINTDATA
{
	int		nlabel;
	vec3f	m_r;
};

//-----------------------------------------------------------------------------
// This class stores a state of a model. A state is defined by data for each
// of the field variables associated by the model. 
class FEState
{
public:
	FEState(float time, FEModel* fem, FEMeshBase* mesh);
	FEState(float time, FEModel* fem, FEState* state);

	void SetID(int n);

	int GetID() const;

	void AddLine(vec3f a, vec3f b, float data_a = 0.f, float data_b = 0.f);

	void AddPoint(vec3f a, int nlabel = 0);

	LINEDATA& Line(int n) { return m_Line[n]; }
	int Lines() { return (int) m_Line.size(); }

	POINTDATA& Point(int n) { return m_Point[n]; }
	int Points() { return (int) m_Point.size(); }

	void SetFEMesh(FEMeshBase* pm) { m_mesh = pm; }
	FEMeshBase* GetFEMesh() { return m_mesh; }

	FEModel* GetFEModel() { return m_fem; }

public:
	float	m_time;		// time value
	int		m_nField;	// the field whos values are contained in m_pval
	int		m_id;		// index in state array of FEModel
	bool	m_bsmooth;

	vector<NODEDATA>	m_NODE;		// nodal data
	vector<EDGEDATA>	m_EDGE;		// edge data
	vector<FACEDATA>	m_FACE;		// face data
	vector<ELEMDATA>	m_ELEM;		// element data
	vector<LINEDATA>	m_Line;		// line data
	vector<POINTDATA>	m_Point;	// point data

	ValArray	m_ElemData;	// element data
	ValArray	m_FaceData;	// face data

	// Data
	FEMeshDataList	m_Data;	// data

public:
	FEModel*	m_fem;	//!< model this state belongs to
	FEMeshBase*	m_mesh;	//!< The mesh this state uses
};
}
