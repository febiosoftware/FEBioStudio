#pragma once
#include "FEMeshData.h"
#include <vector>

class FENodeData : public FEMeshData
{
public:
	FENodeData();
	FENodeData(const FENodeData& d);
	FENodeData& operator = (const FENodeData& d);

	// create data field
	void Create(FEMesh* pm, double v = 0.0);

	// size of data field
	int Size() const { return (int)m_data.size(); }

	// get/set
	double get(int i) const { return m_data[i]; }
	void set(int i, double v) { m_data[i] = v; }

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	vector<double>	m_data;
};
