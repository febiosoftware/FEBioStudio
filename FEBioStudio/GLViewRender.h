#pragma once
#include <FSCore/box.h>
#include <MeshLib/FEMesh.h>

class CGLView;
class GLMesh;

// class that does the actual rendering
class GLViewRender
{
public:
	GLViewRender(CGLView* view);

public:
	void RenderGLMesh(GLMesh* pm, int nid = -1);
	void RenderGLEdges(GLMesh* pm, int nid = -1);

	void RenderBox(const BOX& box);

public:
	void RenderHEX8   (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderHEX20  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderHEX27  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderPENTA  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderTET4   (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderTET10  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderTET15  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderTET20  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderQUAD   (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderQUAD8  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderQUAD9  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderTRI3   (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderTRI6   (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderPYRA5  (FEElement_* pe, FEMesh* pm, bool bsel);
	void RenderPENTA15(FEElement_* pe, FEMesh *pm, bool bsel);

private:
	CGLView*	m_view;
};
