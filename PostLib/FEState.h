/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include "FEMeshData.h"
#include <MeshLib/FEElement.h>
#include <vector>
#include "ValArray.h"
using namespace std;

//-----------------------------------------------------------------------------
// forward declaration of the mesh
namespace Post {
	class FEPostModel;
	class FEPostMesh;

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
	float			m_h[FEElement::MAX_NODES];		// shell thickness (TODO: Can we move this to the face data?)
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
	int		m_elem[2];
};

struct POINTDATA
{
	int		nlabel;
	vec3f	m_r;
};

struct OBJ_POINT_DATA
{
	vec3d	m_pos;
	quatd	m_rot;
};

struct OBJ_LINE_DATA
{
	vec3d	m_r1;
	vec3d	m_r2;
};

//-----------------------------------------------------------------------------
// class for storing reference state
class FERefState
{
public:
	FERefState(FEPostModel* fem);

public:
	vector<NODEDATA>	m_Node;
};

//-----------------------------------------------------------------------------
// This class stores a state of a model. A state is defined by data for each
// of the field variables associated by the model. 
class FEState
{
public:
	FEState(float time, FEPostModel* fem, FEPostMesh* mesh);
	FEState(float time, FEPostModel* fem, FEState* state);

	void SetID(int n);

	int GetID() const;

	void AddLine(vec3f a, vec3f b, float data_a = 0.f, float data_b = 0.f, int el0 = -1, int el1 = -1);

	void AddPoint(vec3f a, int nlabel = 0);

	LINEDATA& Line(int n) { return m_Line[n]; }
	int Lines() { return (int) m_Line.size(); }

	POINTDATA& Point(int n) { return m_Point[n]; }
	int Points() { return (int) m_Point.size(); }

	void SetFEMesh(FEPostMesh* pm) { m_mesh = pm; }
	FEPostMesh* GetFEMesh() { return m_mesh; }

	FEPostModel* GetFEModel() { return m_fem; }

public:
	float	m_time;		// time value
	int		m_nField;	// the field whos values are contained in m_pval
	int		m_id;		// index in state array of FEPostModel
	bool	m_bsmooth;

	vector<NODEDATA>	m_NODE;		// nodal data
	vector<EDGEDATA>	m_EDGE;		// edge data
	vector<FACEDATA>	m_FACE;		// face data
	vector<ELEMDATA>	m_ELEM;		// element data
	vector<LINEDATA>	m_Line;		// line data
	vector<POINTDATA>	m_Point;	// point data

	vector<OBJ_POINT_DATA>	m_objPt;		// object data
	vector<OBJ_LINE_DATA>	m_objLn;		// object data

	ValArray	m_ElemData;	// element data
	ValArray	m_FaceData;	// face data

	// Data
	FEMeshDataList	m_Data;	// data

public:
	FEPostModel*	m_fem;	//!< model this state belongs to
	FERefState*		m_ref;	//!< the reference state for this state
	FEPostMesh*		m_mesh;	//!< The mesh this state uses
};
}
