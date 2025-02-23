/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include <MeshLib/FSElement.h>
#include <vector>
#include "ValArray.h"
#include <FSCore/math3d.h>

//-----------------------------------------------------------------------------
// forward declaration of the mesh
class FSMesh;

namespace Post {
	class FEPostModel;

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
	float	m_nv[FSEdge::MAX_NODES]; // nodal values
};

struct ELEMDATA
{
	float			m_val;		// current element value
	unsigned int	m_state;	// state flags
	float			m_h[FSElement::MAX_NODES];		// shell thickness (TODO: Can we move this to the face data?)
};

struct FACEDATA
{
	int		m_ntag;		// active flag
	float	m_val;		// current face value
};

class ObjectData
{
public:
	ObjectData();
	~ObjectData();

	void push_back(float f);
	void push_back(vec3f f);
	void push_back(mat3f f);

	template <class T> T get(int n) { return *((T*)(data + off[n])); }

	template <class T> void set(int n, const T& v) { *((T*)(data + off[n])) = v; }

private:
	void append(int n);

private:
	float*	data;
	int		nsize;
	std::vector<int>	off;
};

class OBJECTDATA
{
public:
	OBJECTDATA() { data = nullptr; }
	virtual ~OBJECTDATA() {}

public:
	vec3d	pos;
	quatd	rot;
	ObjectData*		data;
};

class OBJ_POINT_DATA : public OBJECTDATA
{
public:
	vec3d	m_rt;
};

struct OBJ_LINE_DATA : public OBJECTDATA
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
	std::vector<NODEDATA>	m_Node;
};

//-----------------------------------------------------------------------------
// This class stores a state of a model. A state is defined by data for each
// of the field variables associated by the model. 
class FEState
{
public:
	FEState(float time, FEPostModel* fem, FSMesh* mesh);
	FEState(float time, FEPostModel* fem, FEState* state);

	void SetID(int n);

	int GetID() const;

	void SetFEMesh(FSMesh* pm) { m_mesh = pm; }
	FSMesh* GetFEMesh() { return m_mesh; }

	FEPostModel* GetFSModel() { return m_fem; }

	OBJECTDATA& GetObjectData(int n);

	void RebuildData();

	void AddPointObjectData();

	vec3f NodePosition(int node);
	vec3f NodeRefPosition(int node);

public:
	float	m_time;		// time value
	int		m_nField;	// the field whos values are contained in m_pval
	int		m_id;		// index in state array of FEPostModel
	bool	m_bsmooth;
	int		m_status;	// status flag

	std::vector<NODEDATA>	m_NODE;		// nodal data
	std::vector<EDGEDATA>	m_EDGE;		// edge data
	std::vector<FACEDATA>	m_FACE;		// face data
	std::vector<ELEMDATA>	m_ELEM;		// element data

	std::vector<OBJ_POINT_DATA>	m_objPt;		// object data
	std::vector<OBJ_LINE_DATA>	m_objLn;		// object data

	ValArray	m_ElemData;	// element data
	ValArray	m_FaceData;	// face data
	ValArray	m_EdgeData;	// edge data

	// Data
	FEMeshDataList	m_Data;	// data

public:
	FEPostModel*	m_fem;	//!< model this state belongs to
	FERefState*		m_ref;	//!< the reference state for this state
	FSMesh*		m_mesh;	//!< The mesh this state uses
};

double IntegrateNodes(FSMesh& mesh, const std::vector<int>& nodeList, Post::FEState* ps);
double IntegrateEdges(FSMesh& mesh, const std::vector<int>& edgeList, Post::FEState* ps);

// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double IntegrateFaces(FSMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps);
double IntegrateReferenceFaces(FSMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps);

// integrates the surface normal scaled by the data field
vec3d IntegrateSurfaceNormal(FSMesh& mesh, Post::FEState* ps);

// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double IntegrateElems(FSMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps);
double IntegrateReferenceElems(FSMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps);

}
