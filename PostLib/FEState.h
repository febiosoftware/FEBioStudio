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
//! forward declaration of the mesh
class FSMesh;

namespace Post {
	class FEPostModel;

//-----------------------------------------------------------------------------
//! Status flags for mesh items
enum StatusFlags {
	ACTIVE	= 0x01,			//!< item has data
	VISIBLE	= 0x02			//!< item is visible (i.e. not eroded)
};

//-----------------------------------------------------------------------------
//! Data structure for storing nodal information
struct NODEDATA
{
	vec3f	m_rt;	//!< nodal position determined by displacement map
	float	m_val;	//!< current nodal value
	int		m_ntag;	//!< active flag
};

//! Data structure for storing edge information
struct EDGEDATA
{
	float	m_val;		//!< current value
	int		m_ntag;		//!< active flag
	float	m_nv[FSEdge::MAX_NODES]; //!< nodal values
};

//! Data structure for storing element information
struct ELEMDATA
{
	float			m_val;		//!< current element value
	unsigned int	m_state;	//!< state flags
	float			m_h[FSElement::MAX_NODES];		//!< shell thickness (TODO: Can we move this to the face data?)
};

//! Data structure for storing face information
struct FACEDATA
{
	int		m_ntag;		//!< active flag
	float	m_val;		//!< current face value
};

//! Class for managing object data arrays
class ObjectData
{
public:
	//! Constructor
	ObjectData();
	//! Destructor
	~ObjectData();

	//! Add a float value to the data array
	void push_back(float f);
	//! Add a vec3f value to the data array
	void push_back(vec3f f);
	//! Add a mat3f value to the data array
	void push_back(mat3f f);

	//! Get data value at index n as type T
	template <class T> T get(int n) { return *((T*)(data + off[n])); }

	//! Set data value at index n to value v of type T
	template <class T> void set(int n, const T& v) { *((T*)(data + off[n])) = v; }

private:
	//! Append n bytes to the data array
	void append(int n);

private:
	float*	data;	//!< data array
	int		nsize;	//!< size of data array
	std::vector<int>	off;	//!< offset array
};

//! Base class for object data
class OBJECTDATA
{
public:
	//! Constructor
	OBJECTDATA() { data = nullptr; }
	//! Virtual destructor
	virtual ~OBJECTDATA() {}

public:
	vec3d	pos;	//!< position
	quatd	rot;	//!< rotation
	ObjectData*		data;	//!< pointer to object data
};

//! Object point data class
class OBJ_POINT_DATA : public OBJECTDATA
{
public:
	vec3d	m_rt;	//!< point position
};

//! Object line data structure
struct OBJ_LINE_DATA : public OBJECTDATA
{
public:
	vec3d	m_r1;	//!< first point of line
	vec3d	m_r2;	//!< second point of line
};

//-----------------------------------------------------------------------------
//! Class for storing reference state information
class FERefState
{
public:
	//! Constructor
	FERefState(FEPostModel* fem);

public:
	std::vector<NODEDATA>	m_Node;	//!< nodal data in reference state
};

//-----------------------------------------------------------------------------
//! This class stores a state of a model. A state is defined by data for each
//! of the field variables associated by the model. 
class FEState
{
public:
	//! Constructor with time, model, and mesh
	FEState(float time, FEPostModel* fem, FSMesh* mesh);
	//! Constructor with time, model, and existing state
	FEState(float time, FEPostModel* fem, FEState* state);

	//! Set the ID of this state
	void SetID(int n);

	//! Get the ID of this state
	int GetID() const;

	//! Set the finite element mesh
	void SetFEMesh(FSMesh* pm) { m_mesh = pm; }
	//! Get the finite element mesh
	FSMesh* GetFEMesh() { return m_mesh; }

	//! Get the FEPostModel
	FEPostModel* GetFSModel() { return m_fem; }

	//! Get object data at index n
	OBJECTDATA& GetObjectData(int n);

	//! Rebuild all data structures
	void RebuildData();

	//! Add point object data
	void AddPointObjectData();

	//! Get node position at given node index
	vec3f NodePosition(int node);
	//! Get node reference position at given node index
	vec3f NodeRefPosition(int node);

public:
	float	m_time;		//!< time value
	int		m_nField;	//!< the field whos values are contained in m_pval
	int		m_id;		//!< index in state array of FEPostModel
	bool	m_bsmooth;	//!< smoothing flag
	int		m_status;	//!< status flag

	std::vector<NODEDATA>	m_NODE;		//!< nodal data
	std::vector<EDGEDATA>	m_EDGE;		//!< edge data
	std::vector<FACEDATA>	m_FACE;		//!< face data
	std::vector<ELEMDATA>	m_ELEM;		//!< element data

	std::vector<OBJ_POINT_DATA>	m_objPt;		//!< object point data
	std::vector<OBJ_LINE_DATA>	m_objLn;		//!< object line data

	ValArray	m_ElemData;	//!< element data array
	ValArray	m_FaceData;	//!< face data array
	ValArray	m_EdgeData;	//!< edge data array

	//! Data
	FEMeshDataList	m_Data;	//!< mesh data list

public:
	FEPostModel*	m_fem;	//!< model this state belongs to
	FERefState*		m_ref;	//!< the reference state for this state
	FSMesh*		m_mesh;	//!< The mesh this state uses
};

//! Integrate values over specified nodes
double IntegrateNodes(FSMesh& mesh, const std::vector<int>& nodeList, Post::FEState* ps);
//! Integrate values over specified edges
double IntegrateEdges(FSMesh& mesh, const std::vector<int>& edgeList, Post::FEState* ps);

//! This function calculates the integral over a surface. Note that if the surface
//! is triangular, then we calculate the integral from a degenerate quad.
double IntegrateFaces(FSMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps);
//! Integrate values over reference faces
double IntegrateReferenceFaces(FSMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps);

//! Integrates the surface normal scaled by the data field
vec3d IntegrateSurfaceNormal(FSMesh& mesh, Post::FEState* ps);

//! This function calculates the integral over a volume. Note that if the volume
//! is not hexahedral, then we calculate the integral from a degenerate hex.
double IntegrateElems(FSMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps);
//! Integrate values over reference elements
double IntegrateReferenceElems(FSMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps);

}