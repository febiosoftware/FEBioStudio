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
#include <FSCore/FSObject.h>
#include "IHasItemList.h"
#include <string>
//using namespace std;

class FSMesh;

class FEMeshData : public FSObject, public FSHasOneItemList
{
public:
	enum DATA_CLASS {
		NODE_DATA,
		SURFACE_DATA,
		ELEMENT_DATA,
		PART_DATA
	};

	// NOTE: this is serialized. Don't change order!
	enum DATA_TYPE {
		DATA_SCALAR,
		DATA_VEC3D,
		DATA_MAT3D
	};

	// NOTE: this is serialized. Don't change order!
	enum DATA_FORMAT {
		DATA_ITEM,	// one value per mesh item
		DATA_NODE,	// one value for each node of selection
		DATA_MULT	// n values for each mesh item, where n is the nr. of nodes of that item
	};

public:
	FEMeshData(DATA_CLASS);
	virtual ~FEMeshData();

	// get the data class of this mesh data
	DATA_CLASS GetDataClass() const;

	// get the data type of this mesh data
	DATA_TYPE GetDataType() const;

	// get the data format
	DATA_FORMAT GetDataFormat() const;

	// size of data field
	int DataSize() const;

	// nr of data items
	int DataItems() const;

	// size of each data item
	int ItemSize() const;

	// return mesh this data field belongs to
	FSMesh* GetMesh() const;

	// get the data
	const std::vector<double>& GetData() const { return m_data; };
	void SetData(const std::vector<double>& data) { m_data = data; }

	// access operator
	double& operator [] (int i) { return m_data[i]; }

	void set(size_t i, double v);
	void set(size_t i, const vec3d& v);
	void set(size_t i, const mat3d& v);

	double get(size_t i) const;
	void get(size_t i, double* d);

	void setScalar(size_t i, double v);
	void setVec3d(size_t i, const vec3d& v);

	double getScalar(size_t i) const;
	vec3d getVec3d(size_t i) const;
	mat3d getMat3d(size_t i) const;

protected:
	void SetMesh(FSMesh* mesh);
	void SetDataType(DATA_TYPE dataType);
	void SetDataFormat(DATA_FORMAT dataFormat);

	std::vector<double>	m_data;

private:
	DATA_CLASS		m_dataClass;
	DATA_TYPE		m_dataType;
	DATA_FORMAT		m_dataFmt;
	int				m_itemSize;	// size of each data item
	FSMesh*			m_pMesh;
};

inline void FEMeshData::set(size_t i, double v)
{
	m_data[i] = v;
}

inline void FEMeshData::set(size_t i, const vec3d& v)
{
	assert(m_dataType == FEMeshData::DATA_VEC3D);
	m_data[3*i  ] = v.x;
	m_data[3*i+1] = v.y;
	m_data[3*i+2] = v.z;
}

inline void FEMeshData::set(size_t i, const mat3d& v)
{
	assert(m_dataType == FEMeshData::DATA_MAT3D);
	m_data[9 * i    ] = v(0,0); m_data[9 * i + 1] = v(0,1); m_data[9 * i + 2] = v(0,2);
	m_data[9 * i + 3] = v(1,0); m_data[9 * i + 4] = v(1,1); m_data[9 * i + 5] = v(1,2);
	m_data[9 * i + 6] = v(2,0); m_data[9 * i + 7] = v(2,1); m_data[9 * i + 8] = v(2,2);
}

inline void FEMeshData::setScalar(size_t i, double v)
{
	assert(m_dataType == FEMeshData::DATA_SCALAR);
	m_data[i] = v;
}

inline void FEMeshData::setVec3d(size_t i, const vec3d& v)
{
	assert(m_dataType == FEMeshData::DATA_VEC3D);
	m_data[3*i + 0] = v.x;
	m_data[3*i + 1] = v.y;
	m_data[3*i + 2] = v.z;
}

inline double FEMeshData::getScalar(size_t i) const 
{
	assert(m_dataType == FEMeshData::DATA_SCALAR);
	return m_data[i];
}

inline vec3d FEMeshData::getVec3d(size_t i) const
{
	assert(m_dataType == FEMeshData::DATA_VEC3D);
	return vec3d(m_data[3*i], m_data[3*i+1], m_data[3*i+2]);
}

inline mat3d FEMeshData::getMat3d(size_t i) const
{
	assert(m_dataType == FEMeshData::DATA_MAT3D);
	const double* d = &m_data[0] + 9 * i;
	return mat3d(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);
}

inline double FEMeshData::get(size_t i) const 
{ 
	return m_data[i];
}

inline void FEMeshData::get(size_t n, double* d)
{
	for (int i = 0; i < m_itemSize; ++i)
		d[i] = m_data[m_itemSize * n + i];
}
