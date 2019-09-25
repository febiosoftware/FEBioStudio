#pragma once
#include <MathLib/math3d.h>

class FEElement_;
class FECoreMesh;
class GLMesh;
class FEFace;

class GLMeshRender
{
public:
	void RenderGLMesh(GLMesh* pm, int nid = -1);
	void RenderGLEdges(GLMesh* pm, int nid = -1);

public:
	// drawing routines for elements
	void RenderHEX8   (FEElement_ *pe, FECoreMesh *pm, bool bsel);
	void RenderHEX20  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderHEX27  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderPENTA  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderTET4   (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderTET10  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderTET15  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderTET20  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderQUAD   (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderQUAD8  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderQUAD9  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderTRI3   (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderTRI6   (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderPYRA5  (FEElement_* pe, FECoreMesh* pm, bool bsel);
	void RenderPENTA15(FEElement_* pe, FECoreMesh *pm, bool bsel);
};

// drawing routines for faces
void RenderQUAD4(FECoreMesh* pm, FEFace& f, bool bsmooth, bool bnode);
void RenderQUAD8(FECoreMesh* pm, FEFace& f, bool bsmooth, bool bnode);
void RenderQUAD9(FECoreMesh* pm, FEFace& f, bool bsmooth, bool bnode);
void RenderTRI3 (FECoreMesh* pm, FEFace& f, bool bsmooth, bool bnode);
void RenderTRI6 (FECoreMesh* pm, FEFace& f, bool bsmooth, bool bnode);
void RenderTRI7 (FECoreMesh* pm, FEFace& f, bool bsmooth, bool bnode);
void RenderTRI10(FECoreMesh* pm, FEFace& f, bool bsmooth, bool bnode);

void RenderSmoothQUAD4(vec3f r[4], vec3f n[4], float q[4], int ndivs);
