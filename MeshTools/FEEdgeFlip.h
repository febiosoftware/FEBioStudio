#pragma once
#include "FESurfaceModifier.h"
#include <MeshLib/FEFaceEdgeList.h>

class FEEdgeFlip : public FESurfaceModifier
{
public:
	FEEdgeFlip();
	~FEEdgeFlip();

	FESurfaceMesh* Apply(FESurfaceMesh* pm) override;

private:
	void Cleanup();
	void MarkEdges(FESurfaceMesh* mesh);
	bool FlipEdge(int iedge, FESurfaceMesh* mesh);
	bool ShouldFlip(int a[3], int b[3], FESurfaceMesh* mesh);
	void DoFlipEdge(int iedge, int a[3], int b[3], int k0, int k1, FESurfaceMesh* mesh);

private:
	FEEdgeList*			m_EL;
	FEFaceEdgeList*		m_FEL;
	FEEdgeFaceList*		m_EFL;
	vector<int>			m_tag;
};
