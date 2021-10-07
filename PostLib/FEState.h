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
//using namespace std;

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
	// nodal coordinates
	vec3f	m_r0;
	vec3f	m_r1;

	// tangents
	vec3d	m_t0;
	vec3d	m_t1;

	// values
	float	m_val[2];
	float	m_user_data[2];
	int		m_elem[2];

	// segment ID
	int	m_segId;

	// flags to identify ends
	int m_end[2];
};

#define MAX_POINT_DATA_FIELDS	32

class POINTDATA
{
public:
	int		nlabel;
	vec3f	m_r;
	float	val[MAX_POINT_DATA_FIELDS];

public:
	POINTDATA() : val{ 0.f } {}
	POINTDATA(const POINTDATA& p)
	{
		nlabel = p.nlabel;
		m_r = p.m_r;
		for (int i = 0; i < MAX_POINT_DATA_FIELDS; ++i) val[i] = p.val[i];
	}
};

class LineData
{
public:
	LineData() {}

	int Lines() const { return (int)m_Line.size(); }
	LINEDATA& Line(int n) { return m_Line[n]; }
	const LINEDATA& Line(int n) const { return m_Line[n]; }

	void Add(LINEDATA& line) { m_Line.push_back(line); }

	void processLines();

private:
	vector<LINEDATA>	m_Line;
};

class ObjectData
{
public:
	ObjectData();
	~ObjectData();

	void push_back(float f);
	void push_back(vec3f f);

	template <class T> T get(int n) { return *((T*)(data + off[n])); }

	template <class T> void set(int n, const T& v) { *((T*)(data + off[n])) = v; }

private:
	void append(int n);

private:
	float*	data;
	int		nsize;
	vector<int>	off;
};

class OBJECT_DATA
{
public:
	OBJECT_DATA() { data = nullptr; }
	virtual ~OBJECT_DATA() {}

public:
	vec3d	pos;
	quatd	rot;
	ObjectData*		data;
};

class OBJ_POINT_DATA : public OBJECT_DATA
{
public:
	vec3d	m_rt;
};

struct OBJ_LINE_DATA : public OBJECT_DATA
{
public:
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
	void AddPoint(vec3f a, const std::vector<float>& data, int nlabel = 0);

	LineData& GetLineData() { return m_Line; }

	POINTDATA& Point(int n) { return m_Point[n]; }
	int Points() { return (int) m_Point.size(); }
	void ClearPoints() { m_Point.clear(); }

	void SetFEMesh(FEPostMesh* pm) { m_mesh = pm; }
	FEPostMesh* GetFEMesh() { return m_mesh; }

	FEPostModel* GetFEModel() { return m_fem; }

	OBJECT_DATA& GetObjectData(int n);

	void RebuildData();

public:
	float	m_time;		// time value
	int		m_nField;	// the field whos values are contained in m_pval
	int		m_id;		// index in state array of FEPostModel
	bool	m_bsmooth;
	int		m_status;	// status flag

	vector<NODEDATA>	m_NODE;		// nodal data
	vector<EDGEDATA>	m_EDGE;		// edge data
	vector<FACEDATA>	m_FACE;		// face data
	vector<ELEMDATA>	m_ELEM;		// element data
	LineData			m_Line;		// line data
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
