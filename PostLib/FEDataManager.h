#pragma once
#include "FEDataField.h"
#include <vector>
using namespace std;

namespace Post {
//-----------------------------------------------------------------------------
// The data manager stores the attributes (name and type) of the different
// data fields.
class FEDataManager
{
public:
	FEDataManager(FEModel* pm);
	~FEDataManager(void);

	//! add a data field
	void AddDataField(FEDataField* pd);

	//! delete a data field
	void DeleteDataField(FEDataField* pd);

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

protected:
	vector<FEDataField*>	m_Data;
	FEModel*	m_pm;
};
}
