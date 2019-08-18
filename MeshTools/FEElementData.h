#pragma once
#include "FEMeshData.h"
#include <string>
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
// Element data field
class FEElementData : public FEMeshData
{
public:
	FEElementData();
	FEElementData(const FEElementData& d);
	FEElementData& operator = (const FEElementData& d);

	// create a data field
	void Create(FEMesh* pm, double v = 0.0);

	// size of data field
	int Size() { return (int)m_data.size(); }

	// get/set
	double get(int i) { return m_data[i]; }
	void set(int i, double v, int ntag = 1) { m_data[i] = v; m_tag[i] = ntag; }

	// access operator
	double& operator [] (int i) { return m_data[i]; }

	void FillRandomBox(double fmin, double fmax);

	// clear all tags
	void ClearTags(int nval = 0);

	// set a tag
	void SetTag(int nelem, int ntag);

	// get a tag
	int GetTag(int nelem) const { return m_tag[nelem]; }

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	vector<double>	m_data;		//!< data values
	vector<int>		m_tag;		//!< data tags (0 == no value for element)
};
