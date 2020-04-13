#pragma once
#include "FEMeshData.h"
#include <vector>

class FEItemListBuilder;
class GObject;
class FENodeSet;

class FENodeData : public FEMeshData
{
public:
	FENodeData(GObject* po);

	// create data field
	void Create(double v = 0.0);

	// size of data field
	int Size() const { return (int)m_data.size(); }

	// get/set
	double get(int i) const { return m_data[i]; }
	void set(int i, double v) { m_data[i] = v; }

	// get the item list
	FEItemListBuilder* GetItemList();

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	vector<double>	m_data;
	GObject*		m_po;
	FENodeSet*		m_nodeSet;

private:
	FENodeData(const FENodeData& d);
	FENodeData& operator = (const FENodeData& d);
};
