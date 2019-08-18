#pragma once

#include <vector>
using namespace std;

#include "FEMeshBase.h"

class FENodeFaceList
{
public:
	FENodeFaceList(FEMeshBase* pm);
	~FENodeFaceList(void);

	void Build();
	bool BuildSorted();

	int Valence(int i) { return m_val[i]; }
	FEFace* Face(int n, int i) { return m_pface[m_off[n] + i]; }
	int FaceIndex(int n, int i) { return m_nface[m_off[n] + i]; }

	bool HasFace(int n, FEFace* pf);

	int FindFace(const FEFace& f);

protected:
	bool Sort(int node);

protected:
	FEMeshBase*	m_pm;

	vector<int>	m_val;
	vector<FEFace*>	m_pface;
	vector<int>		m_nface;
	vector<int>	m_off;
};
