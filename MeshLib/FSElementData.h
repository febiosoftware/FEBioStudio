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
#include "FSMeshData.h"
#include <MeshLib/FSItemList.h>
#include <string>
#include <vector>

class FSElemSet;
class FSPartSet;
class FSModel;
class GModel;

//-----------------------------------------------------------------------------
// Element data field
class FSElementData : public FSMeshData
{
public:
	enum {MAX_ITEM_SIZE = 9};

public:
	FSElementData(FSMesh* mesh = nullptr);
	FSElementData(FSMesh* mesh, DATA_TYPE dataType, DATA_FORMAT dataFormat);

	// create a data field
	void Create(FSMesh* pm, FSElemSet* part, DATA_TYPE dataType, DATA_FORMAT dataFormat);

	void SetItemList(FSItemListBuilder* item, int n = 0) override;

	// Get the element set
	FSElemSet* GetElementSet();

	void FillRandomBox(double fmin, double fmax);

	void SetScaleFactor(double s);
	double GetScaleFactor() const;

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	double		m_scale = 1;	//!< scale factor
	int			m_maxNodesPerElem = 0;

private:
	FSElementData(const FSElementData& d);
	void operator = (const FSElementData& d);

	void AllocateData();
};

// data field defined on parts
class FSPartData : public FSMeshData
{
public:
	FSPartData(FSMesh* mesh = nullptr);
	FSPartData(FSMesh* mesh, DATA_TYPE dataType, DATA_FORMAT dataFmt);

	// create a data field
	bool Create(FSPartSet* partList, DATA_TYPE dataType = DATA_SCALAR, DATA_FORMAT dataFmt = DATA_ITEM);

	// add a part (returns false if part is already a member or part set is empty)
	bool AddPart(int localPartID);
	
	void SetValue(int item, int lid, double v);
	double GetValue(int item, int lid);

	// build the element list
	FSElemList* BuildElemList();

	// get the part list
	FSPartSet* GetPartSet();

	int GetElementIndex(int nelem);

	void SetItemList(FSItemListBuilder* item, int n = 0) override;

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	int					m_maxElemItems;	
	std::vector<int>	m_lut;

private:
	FSPartData(const FSPartData& d);
	FSPartData& operator = (const FSPartData& d);

	void AllocateData();
};

inline void FSPartData::SetValue(int i, int j, double v)
{
	m_data[i*m_maxElemItems + j] = v;
}

inline double FSPartData::GetValue(int i, int j)
{
	return m_data[i*m_maxElemItems + j];
}
