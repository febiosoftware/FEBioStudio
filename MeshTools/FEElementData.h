#pragma once
#include "FEMeshData.h"
#include <string>
#include <vector>
using namespace std;

class FEPart;
class GPartList;

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
	bool Create(GPartList* part, FEMeshData::DATA_TYPE dataType = FEMeshData::DATA_SCALAR);

	// Get the part list
	const GPartList* GetPartList();

	// size of data field
	int Size() const;

	// get/set
	double get(int i) { return m_data[i]; }
	void set(int i, double v) { m_data[i] = v; }

	// access operator
	double& operator [] (int i) { return m_data[i]; }

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	vector<double>		m_data;		//!< data values
	GPartList*			m_part;		//!< the part to which the data applies
};
