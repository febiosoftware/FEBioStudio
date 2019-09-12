#pragma once

#include <vector>
using namespace std;

#include "FEMeshBase.h"

struct NodeFaceRef {
	int		fid;	// face index (into mesh' Face array)
	int		nid;	// local node index
	FEFace*	pf;		// face pointer
};

class FENodeFaceList
{
public:
	FENodeFaceList();
	~FENodeFaceList(void);

	void Build(FEMeshBase* pm);
	bool BuildSorted(FEMeshBase* pm);

	void Clear();

	bool IsEmpty() const;

	int Valence(int i) const { return (int) m_face[i].size(); }
	FEFace* Face(int n, int i) { return m_face[n][i].pf; }
	int FaceIndex(int n, int i) { return m_face[n][i].fid; }

	bool HasFace(int n, FEFace* pf);

	int FindFace(const FEFace& f);

	int FindFace(int inode, int n[10], int m);

	const vector<NodeFaceRef>& FaceList(int n) const;

protected:
	bool Sort(int node);

protected:
	FEMeshBase*	m_pm;
	vector< vector<NodeFaceRef> >	m_face;
};
