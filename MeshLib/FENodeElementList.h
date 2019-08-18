#pragma once

#include <vector>
using namespace std;

#include "FEMesh.h"

class FENodeElementList
{
public:
	FENodeElementList(FEMesh* pm);
	~FENodeElementList();

	void Build();

	int Valence(int n) const { return m_val[n]; }
	FEElement* Element(int n, int j) { return m_pelem[m_off[n] + j]; }
	int ElementIndex(int n, int j) const { return m_elem[m_off[n] + j]; }

	bool HasElement(int node, int iel);

	vector<int> ElementList(int n) const;

protected:
	FEMesh*	m_pm;

	vector<int>	m_val;
	vector<int>	m_off;
	vector<int>	m_elem;
	vector<FEElement*>	m_pelem;
};
