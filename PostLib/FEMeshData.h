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
#include <vector>
#include <FSCore/math3d.h>

namespace Post {

class FEPostModel;
class FEState;
class FEPostMesh;

//-----------------------------------------------------------------------------
// Data class: defines possible class types for data fields
// TODO: Why is this implemented as a binary flag
enum Data_Class {
	CLASS_NODE   = 1,
	CLASS_FACE   = 2,
	CLASS_ELEM   = 4,
	CLASS_OBJECT = 8
};

//-----------------------------------------------------------------------------
// Data types:
// - DATA_FLOAT: single precision scalar
// - DATA_VEC3F: 3D vector of floats (3 comp)
// - DATA_MAT3D: 3D Matrix of doubles (9 comp)
// - DATA_MAT3FS: 3D symmetric Matrix of floats (6 comp)
// - DATA_MAT3FD: 3D diagonal Matrix of floats (3 comp)
// - DATA_TENS4FS: 6D symmetric Matrix of floats (21 comp)
// - DATA_ARRAY  : variable array (see ModelDataField::GetArraySize())
// - DATA_ARRAY_VEC3F: variable array of 3D vectors of float (comp =  3*ModelDataField::GetArraySize())
enum Data_Type {
	DATA_FLOAT,
	DATA_VEC3F,
	DATA_MAT3FS,
	DATA_MAT3FD,
    DATA_TENS4FS,
	DATA_MAT3D,
	DATA_MAT3F,
	DATA_ARRAY,
	DATA_ARRAY_VEC3F
};

//-----------------------------------------------------------------------------
// Data storage formats:
// - DATA_NODE: store data at the nodes of the region
// - DATA_ITEM: store data for each item of the region
// - DATA_COMP: store data at each node of each item of the region
// - DATA_REGION: store one data value for the entire region
enum Data_Format {
	DATA_NODE,
	DATA_ITEM,
	DATA_COMP,
	DATA_REGION
};

//-----------------------------------------------------------------------------
enum Data_Tensor_Type {
	DATA_SCALAR,
	DATA_VECTOR,
	DATA_TENSOR2,
};

//-----------------------------------------------------------------------------
//! Base class for mesh data classes
//!
class FEMeshData
{
public:
	FEMeshData(FEState* state, Data_Type ntype, Data_Format nfmt) { m_state = state; m_ntype = ntype; m_nfmt = nfmt; }
	virtual ~FEMeshData() { m_state = 0; }

	Data_Type GetType() { return m_ntype; }
	Data_Format GetFormat() { return m_nfmt; }

	FEState* GetFEState() { return m_state; }

	FEPostMesh* GetFEMesh();

	FEPostModel* GetFSModel();

protected:
	FEState*	m_state;
	Data_Type	m_ntype;
	Data_Format	m_nfmt;
};

//-----------------------------------------------------------------------------
//! This class helps managing lists of FEMeshData classes
//!
class FEMeshDataList
{
public:
	FEMeshDataList(){}
	~FEMeshDataList() { clear(); }

	FEMeshData& operator [] (int i) { return *m_data[i]; }

	void push_back(FEMeshData* pd) { m_data.push_back(pd); }

	void clear();

	int size() { return (int) m_data.size(); }

	void erase(int i) { m_data.erase(m_data.begin() + i); }

protected:
	std::vector<FEMeshData*>		m_data;
};

//-----------------------------------------------------------------------------
// traits class for converting from type to Data_Type
template <class T> class FEMeshDataTraits {};

template <> class FEMeshDataTraits<float>  { public: static Data_Type Type() { return DATA_FLOAT;   }};
template <> class FEMeshDataTraits<vec3f>  { public: static Data_Type Type() { return DATA_VEC3F;   }};
template <> class FEMeshDataTraits<mat3d>  { public: static Data_Type Type() { return DATA_MAT3D;   }};
template <> class FEMeshDataTraits<mat3f>  { public: static Data_Type Type() { return DATA_MAT3F;   }};
template <> class FEMeshDataTraits<mat3fs> { public: static Data_Type Type() { return DATA_MAT3FS;  }};
template <> class FEMeshDataTraits<mat3fd> { public: static Data_Type Type() { return DATA_MAT3FD;  }};
template <> class FEMeshDataTraits<tens4fs>{ public: static Data_Type Type() { return DATA_TENS4FS; }};

//-----------------------------------------------------------------------------
// traits class for converting from Data_Type to type
template <Data_Type t> class FEDataTypeTraits {};

template <> class FEDataTypeTraits<DATA_FLOAT > { public: typedef float dataType; };
template <> class FEDataTypeTraits<DATA_VEC3F > { public: typedef vec3f dataType; };
template <> class FEDataTypeTraits<DATA_MAT3D > { public: typedef mat3d dataType; };
template <> class FEDataTypeTraits<DATA_MAT3F > { public: typedef mat3f dataType; };
template <> class FEDataTypeTraits<DATA_MAT3FS> { public: typedef mat3fs dataType; };
template <> class FEDataTypeTraits<DATA_MAT3FD> { public: typedef mat3fd dataType; };
template <> class FEDataTypeTraits<DATA_TENS4FS>{ public: typedef tens4fs dataType; };

// spatial gradient of shape functions
void shape_grad(FEPostModel& fem, int elem, double q[3], int nstate, vec3f* G);

// material gradient of shape functions
void shape_grad_ref(FEPostModel& fem, int elem, double q[3], int nstate, vec3f* G);
}
