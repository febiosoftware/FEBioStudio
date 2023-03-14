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
#include <FEMLib/IHasItemList.h>
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

	enum DATA_FORMAT {
		DATA_NODE,
		DATA_ITEM,
		DATA_MULT
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

	// return mesh this data field belongs to
	FSMesh* GetMesh() const;

	// get the data
	const std::vector<double>& GetData() const { return m_data; };
	void SetData(const std::vector<double>& data) { m_data = data; }

	// access operator
	double& operator [] (int i) { return m_data[i]; }

protected:
	void SetMesh(FSMesh* mesh);
	DATA_TYPE		m_dataType;
	DATA_FORMAT		m_dataFmt;
	std::vector<double>	m_data;

private:
	DATA_CLASS		m_dataClass;
	FSMesh*			m_pMesh;
};
