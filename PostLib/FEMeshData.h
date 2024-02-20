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
#include <MeshLib/enums.h>

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
enum Data_Tensor_Type {
	TENSOR_SCALAR,
	TENSOR_VECTOR,
	TENSOR_TENSOR2,
};

//-----------------------------------------------------------------------------
//! Base class for mesh data classes
//!
class FEMeshData
{
public:
	FEMeshData(FEState* state, DATA_TYPE ntype, DATA_FORMAT nfmt) { m_state = state; m_ntype = ntype; m_nfmt = nfmt; }
	virtual ~FEMeshData() { m_state = 0; }

	DATA_TYPE GetType() { return m_ntype; }
	DATA_FORMAT GetFormat() { return m_nfmt; }

	FEState* GetFEState() { return m_state; }

	FEPostMesh* GetFEMesh();

	FEPostModel* GetFSModel();

protected:
	FEState*	m_state;
	DATA_TYPE	m_ntype;
	DATA_FORMAT	m_nfmt;
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

template <> class FEMeshDataTraits<float>  { public: static DATA_TYPE Type() { return DATA_SCALAR;  }};
template <> class FEMeshDataTraits<vec3f>  { public: static DATA_TYPE Type() { return DATA_VEC3;   }};
template <> class FEMeshDataTraits<mat3f>  { public: static DATA_TYPE Type() { return DATA_MAT3;   }};
template <> class FEMeshDataTraits<mat3fs> { public: static DATA_TYPE Type() { return DATA_MAT3S;  }};
template <> class FEMeshDataTraits<mat3fd> { public: static DATA_TYPE Type() { return DATA_MAT3SD; }};
template <> class FEMeshDataTraits<tens4fs>{ public: static DATA_TYPE Type() { return DATA_TENS4S; }};

//-----------------------------------------------------------------------------
// traits class for converting from Data_Type to type
template <DATA_TYPE t> class FEDataTypeTraits {};

template <> class FEDataTypeTraits<DATA_SCALAR> { public: typedef float dataType; };
template <> class FEDataTypeTraits<DATA_VEC3 > { public: typedef vec3f dataType; };
template <> class FEDataTypeTraits<DATA_MAT3 > { public: typedef mat3f dataType; };
template <> class FEDataTypeTraits<DATA_MAT3S> { public: typedef mat3fs dataType; };
template <> class FEDataTypeTraits<DATA_MAT3SD>{ public: typedef mat3fd dataType; };
template <> class FEDataTypeTraits<DATA_TENS4S>{ public: typedef tens4fs dataType; };

// spatial gradient of shape functions
void shape_grad(FEPostModel& fem, int elem, double q[3], int nstate, vec3f* G);

// material gradient of shape functions
void shape_grad_ref(FEPostModel& fem, int elem, double q[3], int nstate, vec3f* G);
}
