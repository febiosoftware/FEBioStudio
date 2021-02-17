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
#include "FEItemList.h"
#include <string>
#include <vector>
//using namespace std;

class FEPart;
class GPartList;
class FEModel;

//-----------------------------------------------------------------------------
// Element data field
class FEElementData : public FEMeshData
{
public:
	FEElementData(FEMesh* mesh = nullptr);
	FEElementData(const FEElementData& d);
	FEElementData& operator = (const FEElementData& d);

	// create a data field
	void Create(FEMesh* pm, FEPart* part, FEMeshData::DATA_TYPE dataType = FEMeshData::DATA_SCALAR);

	// size of data field
	int Size() { return (int)m_data.size(); }

	// get/set
	double get(int i) { return m_data[i]; }
	void set(int i, double v) { m_data[i] = v; }

	// access operator
	double& operator [] (int i) { return m_data[i]; }

	// Get the element set
	const FEPart* GetPart() const { return m_part; }

	void FillRandomBox(double fmin, double fmax);

	void SetScaleFactor(double s);
	double GetScaleFactor() const;

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	vector<double>	m_data;		//!< data values
	FEPart*			m_part;		//!< the part to which the data applies
	double			m_scale;	//!< scale factor
};

//-----------------------------------------------------------------------------
// data field defined on parts
class FEPartData : public FEMeshData
{
public:
	FEPartData(FEMesh* mesh = nullptr);
	FEPartData(const FEPartData& d);
	FEPartData& operator = (const FEPartData& d);

	// create a data field
	bool Create(const vector<int>& partList, FEMeshData::DATA_TYPE dataType = FEMeshData::DATA_SCALAR);

	// size of data field
	int Size() const;

	// get/set
	double get(int i) { return m_data[i]; }
	void set(int i, double v) { m_data[i] = v; }

	// access operator
	double& operator [] (int i) { return m_data[i]; }

	// build the element list
	FEElemList* BuildElemList();

	// get the partlist
	GPartList* GetPartList(FEModel* fem);

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	vector<double>		m_data;		//!< data values
	vector<int>			m_part;		//!< the part to which the data applies
};
