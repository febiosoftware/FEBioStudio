#pragma once
#include <FSCore/FSObject.h>
#include <string>
using namespace std;

class FEMesh;

// TODO: Should I inherit this from FSObject?
class FEMeshData
{
public:
	enum DATA_CLASS {
		NODE_DATA,
		SURFACE_DATA,
		PART_DATA
	};

	enum DATA_TYPE {
		DATA_SCALAR,
		DATA_VEC3D
	};

public:
	FEMeshData(DATA_CLASS);
	virtual ~FEMeshData();

	// get/set name
	void SetName(const string& name);
	const string& GetName() const;

	// get the data class of this mesh data
	DATA_CLASS GetDataClass() const;

	// get the data type of this mesh data
	DATA_TYPE GetDataType() const;

	// return mesh this data field belongs to
	FEMesh* GetMesh() const;

protected:
	void SetMesh(FEMesh* mesh);
	DATA_TYPE		m_dataType;

private:
	DATA_CLASS		m_dataClass;
	FEMesh*			m_pMesh;
	string			m_name;
};
