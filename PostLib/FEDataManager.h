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
#include "FEDataField.h"
#include <vector>

namespace Post {

// forward declaration
class FEPostModel;

//-----------------------------------------------------------------------------
//! The data manager stores the attributes (name and type) of the different
//! data fields.
class FEDataManager
{
public:
	FEDataManager(FEPostModel* pm);
	~FEDataManager(void);

	//! add a data field
	void AddDataField(ModelDataField* pd, const std::string& name = "", const char* szunits = nullptr);

	//! delete a data field
	void DeleteDataField(ModelDataField* pd);

	//! get the nodal datafield
	FEDataFieldPtr FirstDataField();
	int DataFields() const;

	//! clear data
	void Clear();

	//! find the index of a datafield
	int FindDataField(const std::string& fieldName);

	//! find the data field given an index
	FEDataFieldPtr DataField(int i);

	std::string getDataString(int nfield, Data_Tensor_Type ntype);

	const char* getDataUnits(int nfield);

	// see if a field ID is valid
	bool IsValid(int fieldId) const;

protected:
	std::vector<ModelDataField*>	m_Data;
	FEPostModel*	m_pm;
};
}
