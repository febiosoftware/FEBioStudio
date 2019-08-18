#pragma once
#include <vector>

namespace Post {
	
class FEMeshBase;

class FENodeFaceTable
{
public:
	FENodeFaceTable(FEMeshBase* pm);
	int FindFace(int inode, int n[10], int m);

protected:
	void Build();

protected:
	FEMeshBase*	m_pm;
	std::vector< std::vector<int> >	m_NFT;
};
}
