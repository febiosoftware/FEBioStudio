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
#include <vector>

class FEItemListBuilder;
class GObject;
class FSNodeSet;

class FENodeData : public FEMeshData
{
public:
	FENodeData(GObject* po);

	void Create(FSNodeSet* nset, double v = 0.0, FEMeshData::DATA_TYPE dataType = FEMeshData::DATA_SCALAR);

	// size of data field
	int Size() const { return (int)m_data.size(); }

	// get/set
	double GetScalar(size_t i) const;
	void SetScalar(size_t i, double v);

	vec3d GetVec3d(size_t i) const;
	void SetVec3d(size_t i, const vec3d& v);

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	int m_dataSize;	// size of each data item
	std::vector<double>	m_data;
	GObject*		m_po;

private:
	FENodeData(const FENodeData& d);
	FENodeData& operator = (const FENodeData& d);
};

inline double FENodeData::GetScalar(size_t i) const { assert(m_dataType == FEMeshData::DATA_SCALAR); return m_data[i]; }
inline void FENodeData::SetScalar(size_t i, double v) { assert(m_dataType == FEMeshData::DATA_SCALAR); m_data[i] = v; }

inline vec3d FENodeData::GetVec3d(size_t i) const
{ 
	assert(m_dataType == FEMeshData::DATA_VEC3D);
	const double* d = &m_data[3 * i];
	return vec3d(d[0], d[1], d[2]); 
}

inline void FENodeData::SetVec3d(size_t i, const vec3d& v)
{
	assert(m_dataType == FEMeshData::DATA_VEC3D);
	double* d = &m_data[3 * i];
	d[0] = v.x;
	d[1] = v.y;
	d[2] = v.z;
}
