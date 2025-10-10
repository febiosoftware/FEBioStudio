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
#include <FSCore/FSObject.h>
#include <typeinfo>
#include <string>

using std::string;

//-----------------------------------------------------------------------------
namespace Post {

// forward declarations
class FEPostModel;

//-----------------------------------------------------------------------------
// data field flags
enum DataFieldFlags {
	IMPLICIT_DATA = 1			// data field depends on other data fields. This may cause circular dependencies.
};

//-----------------------------------------------------------------------------
//! Base class describing a data field
class ModelDataField : public FSObject
{
public:
	ModelDataField(FEPostModel* glm, DATA_TYPE ntype, DATA_FORMAT nfmt, DATA_CLASS ncls, unsigned int flag = 0);

	virtual ~ModelDataField();

	//! Create a copy
	virtual ModelDataField* Clone() const = 0;

	//! FEMeshData constructor
	virtual FEMeshData* CreateData(FEState* pstate) = 0;

	//! type identifier
	DATA_TYPE Type() { return m_ntype; }

	// Format identifier
	DATA_FORMAT Format() const { return m_nfmt; }

	// Class Identifier
	DATA_CLASS DataClass() const { return m_nclass; }

	//! Set the field ID
	void SetFieldID(int nid) { m_nfield = nid; }

	//! get the field ID
	int GetFieldID() const { return m_nfield; }

	//! type string
	const char* TypeStr() const;

	//! number of components
	int components(Data_Tensor_Type ntype);

	//! number of actual data components
	int dataComponents(Data_Tensor_Type ntype);

	//! return the name of a component
	std::string componentName(int ncomp, Data_Tensor_Type ntype);

	virtual const std::type_info& TypeInfo() { return typeid(ModelDataField); }

	unsigned int Flags() const { return m_flag; }

	void SetArraySize(int n) { m_arraySize = n; }
	int GetArraySize() const { return m_arraySize; }

	void SetArrayNames(std::vector<string>& n);
	std::vector<string> GetArrayNames() const;

	FEPostModel* GetModel() { return m_fem; }

public:
	void SetUnits(const char* sz);
	const char* GetUnits() const;

protected:
	int				m_nfield;	//!< field ID
	DATA_TYPE		m_ntype;	//!< data type
	DATA_FORMAT		m_nfmt;		//!< data format
	DATA_CLASS		m_nclass;	//!< data class
	unsigned int	m_flag;		//!< flags
	std::string		m_units;	//!< units

	int				m_arraySize;	//!< data size for arrays
	std::vector<string>	m_arrayNames;	//!< (optional) names of array components

	FEPostModel*	m_fem;
};

//-----------------------------------------------------------------------------
template<typename T> class FEDataField_T : public ModelDataField
{
public:
	FEDataField_T(FEPostModel* fem, unsigned int flag = 0) : ModelDataField(fem, T::Type(), T::Format(), T::Class(), flag) {}
	FEMeshData* CreateData(FEState* pstate) { return new T(pstate, this); }

	virtual ModelDataField* Clone() const
	{
		FEDataField_T<T>* newData = new FEDataField_T<T>(m_fem);
		newData->SetName(GetName());
		return newData;
	}

	const std::type_info& TypeInfo() { return typeid(T); }
};

//-----------------------------------------------------------------------------
typedef std::vector<ModelDataField*>::iterator FEDataFieldPtr;


//-----------------------------------------------------------------------------
class FEArrayDataField : public ModelDataField
{
public:
	FEArrayDataField(FEPostModel* fem, DATA_CLASS c, DATA_FORMAT f);

	ModelDataField* Clone() const override;

	FEMeshData* CreateData(FEState* pstate) override;
};

//-----------------------------------------------------------------------------
class FEArrayVec3DataField : public ModelDataField
{
public:
	FEArrayVec3DataField(FEPostModel* fem, DATA_CLASS c);

	ModelDataField* Clone() const override;

	FEMeshData* CreateData(FEState* pstate) override;
};

//-------------------------------------------------------------------------------
bool ExportDataField(FEPostModel& fem, const ModelDataField& df, const char* szfile, bool selOnly, bool writeConn, const std::vector<int>& states);
bool ExportNodeDataField(FEPostModel& fem, const ModelDataField& df, FILE* fp, bool selOnly, const std::vector<int>& states);
bool ExportFaceDataField(FEPostModel& fem, const ModelDataField& df, FILE* fp, bool selOnly, bool writeConn, const std::vector<int>& states);
bool ExportElementDataField(FEPostModel& fem, const ModelDataField& df, FILE* fp, bool selOnly, bool writeConn, const std::vector<int>& states);

//-----------------------------------------------------------------------------
void InitStandardDataFields();
int StandardDataFields(); 
std::string GetStandarDataFieldName(int i);
bool AddStandardDataField(FEPostModel& fem, const std::string& dataField);
bool AddStandardDataField(FEPostModel& fem, const std::string& dataField, std::vector<int> selectionList);

//-----------------------------------------------------------------------------
bool AddNodeDataFromFile(FEPostModel& fem, const char* szfile, const char* szname, int ntype);
bool AddFaceDataFromFile(FEPostModel& fem, const char* szfile, const char* szname, int ntype);
bool AddElemDataFromFile(FEPostModel& fem, const char* szfile, const char* szname, int ntype);

class PlotObjectData : public ModelDataField
{
public:
	PlotObjectData(FEPostModel* fem, DATA_TYPE ntype) : ModelDataField(fem, ntype, DATA_ITEM, OBJECT_DATA, 0) {}

	ModelDataField* Clone() const override { assert(false); return nullptr; };
	FEMeshData* CreateData(FEState* pstate) override { assert(false); return nullptr; }
};
}
