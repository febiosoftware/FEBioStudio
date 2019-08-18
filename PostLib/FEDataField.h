#pragma once
#include "FEMeshData.h"
#include <typeinfo>
#include <string>

//-----------------------------------------------------------------------------
// forward declaration of the FEModel class
namespace Post {

class FEModel;
class CGLModel;

//-----------------------------------------------------------------------------
// data field flags
enum DataFieldFlags {
	EXPORT_DATA = 1			// data field can be exported
};

//-----------------------------------------------------------------------------
// Base class describing a data field
class FEDataField
{
public:
	enum { MAX_DATA_NAME = 64 };

public:
	FEDataField(const std::string& name, Data_Type ntype, Data_Format nfmt, Data_Class ncls, unsigned int flag);

	virtual ~FEDataField(){}

	//! get the name of the field
	const std::string& GetName() const { return m_name; }

	//! set the name of the field
	void SetName(const std::string& newName);

	//! Create a copy
	virtual FEDataField* Clone() const = 0;

	//! FEMeshData constructor
	virtual FEMeshData* CreateData(FEState* pstate) = 0;

	//! type identifier
	Data_Type Type() { return m_ntype; }

	// Format identifier
	Data_Format Format() const { return m_nfmt; }

	// Class Identifier
	Data_Class DataClass() const { return m_nclass; }

	//! Set the field ID
	void SetFieldID(int nid) { m_nfield = nid; }

	//! get the field ID
	int GetFieldID() const { return m_nfield; }

	//! type string
	const char* TypeStr() const;

	//! number of components
	int components(Data_Tensor_Type ntype);

	//! return the name of a component
	std::string componentName(int ncomp, Data_Tensor_Type ntype);

	virtual const std::type_info& TypeInfo() { return typeid(FEDataField); }

	unsigned int Flags() const { return m_flag; }

	void SetArraySize(int n) { m_arraySize = n; }
	int GetArraySize() const { return m_arraySize; }

	void SetArrayNames(vector<string>& n);
	vector<string> GetArrayNames() const;

protected:
	int				m_nfield;	//!< field ID
	Data_Type		m_ntype;	//!< data type
	Data_Format		m_nfmt;		//!< data format
	Data_Class		m_nclass;	//!< data class
	unsigned int	m_flag;		//!< flags
	std::string		m_name;		//!< data field name

	int				m_arraySize;	//!< data size for arrays
	vector<string>	m_arrayNames;	//!< (optional) names of array components


public:
	// TODO: Add properties list for data fields (e.g. strains and curvature could use this)
	// strain parameters
	int		m_nref;	// reference state
};

//-----------------------------------------------------------------------------
template<typename T> class FEDataField_T : public FEDataField
{
public:
	FEDataField_T(const std::string& name, unsigned int flag = 0) : FEDataField(name, T::Type(), T::Format(), T::Class(), flag) {}
	FEMeshData* CreateData(FEState* pstate) { return new T(pstate, this); }

	virtual FEDataField* Clone() const
	{
		FEDataField_T<T>* newData = new FEDataField_T<T>(GetName());
		return newData;
	}

	const std::type_info& TypeInfo() { return typeid(T); }
};

//-----------------------------------------------------------------------------
typedef vector<FEDataField*>::iterator FEDataFieldPtr;


//-----------------------------------------------------------------------------
class FEArrayDataField : public FEDataField
{
public:
	FEArrayDataField(const std::string& name, Data_Class c, Data_Format f, unsigned int flag = 0);

	FEDataField* Clone() const override;

	FEMeshData* CreateData(FEState* pstate) override;
};

//-----------------------------------------------------------------------------
class FEArrayVec3DataField : public FEDataField
{
public:
	FEArrayVec3DataField(const std::string& name, Data_Class c, unsigned int flag = 0);

	FEDataField* Clone() const override;

	FEMeshData* CreateData(FEState* pstate) override;
};

//-------------------------------------------------------------------------------
bool ExportDataField(CGLModel& glm, const FEDataField& df, const char* szfile);
bool ExportNodeDataField(CGLModel& glm, const FEDataField& df, FILE* fp);
bool ExportFaceDataField(CGLModel& glm, const FEDataField& df, FILE* fp);
bool ExportElementDataField(CGLModel& glm, const FEDataField& df, FILE* fp);

bool AddStandardDataField(CGLModel& glm, int ndata, bool bselection_only);
bool AddNodeDataFromFile(CGLModel& glm, const char* szfile, const char* szname, int ntype);
bool AddFaceDataFromFile(CGLModel& glm, const char* szfile, const char* szname, int ntype);
bool AddElemDataFromFile(CGLModel& glm, const char* szfile, const char* szname, int ntype);
}
