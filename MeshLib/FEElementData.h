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
#include <MeshLib/FEItemList.h>
#include <string>
#include <vector>

class FSElemSet;
class GPartList;
class FSModel;
class GModel;

//-----------------------------------------------------------------------------
// Element data field
class FEElementData : public FEMeshData
{
public:
	enum {MAX_ITEM_SIZE = 9};

public:
	FEElementData(FSMesh* mesh = nullptr);

	// create a data field
	void Create(FSMesh* pm, FSElemSet* part, FEMeshData::DATA_TYPE dataType = FEMeshData::DATA_SCALAR);

	// size of data field
	int Size() { return (int)m_data.size(); }

	// get/set
	double get(int i) { return m_data[i]; }
	void get(int n, double* d);
	void set(int i, double v) { m_data[i] = v; }
	void set(int i, const vec3d& v) 
	{ 
		assert(m_dataType == DATA_VEC3D);
		m_data[3*i  ] = v.x; 
		m_data[3*i+1] = v.y; 
		m_data[3*i+2] = v.z; 
	}
	void set(int i, const mat3d& v);

	// Get the element set
	FSElemSet* GetPart();

	void FillRandomBox(double fmin, double fmax);

	void SetScaleFactor(double s);
	double GetScaleFactor() const;

	int ItemSize() const;

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	double				m_scale;	//!< scale factor

	int	m_stride;

private:
	FEElementData(const FEElementData& d);
	void operator = (const FEElementData& d);
};

//-----------------------------------------------------------------------------
// data field defined on parts
class FEPartData : public FEMeshData
{
public:
	FEPartData(FSMesh* mesh = nullptr);
	FEPartData(const FEPartData& d);
	FEPartData& operator = (const FEPartData& d);

	// create a data field
	bool Create(const std::vector<int>& partList, FEMeshData::DATA_TYPE dataType = FEMeshData::DATA_SCALAR, FEMeshData::DATA_FORMAT dataFmt = FEMeshData::DATA_ITEM);

	// size of data field
	int Size() const;

	// get/set
	double get(int i) { return m_data[i]; }
	void set(int i, double v) { m_data[i] = v; }

	void SetValue(int item, int lid, double v);
	double GetValue(int item, int lid);

	// build the element list
	FEElemList* BuildElemList();

	// get the partlist
	GPartList* GetPartList(GModel* fem);

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	int						m_maxElemItems;	
	std::vector<int>		m_part;		//!< the part to which the data applies
};

inline void FEPartData::SetValue(int i, int j, double v)
{
	m_data[i*m_maxElemItems + j] = v;
}

inline double FEPartData::GetValue(int i, int j)
{
	return m_data[i*m_maxElemItems + j];
}
