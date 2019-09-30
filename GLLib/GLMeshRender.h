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
// Note: Call this from within glBegin(GL_TRIANGLES)\glEnd() section
void RenderQUAD4(FECoreMesh* pm, FEFace& f, bool bsmooth);
void RenderQUAD8(FECoreMesh* pm, FEFace& f, bool bsmooth);
void RenderQUAD9(FECoreMesh* pm, FEFace& f, bool bsmooth);
void RenderTRI3 (FECoreMesh* pm, FEFace& f, bool bsmooth);
void RenderTRI6 (FECoreMesh* pm, FEFace& f, bool bsmooth);
void RenderTRI7 (FECoreMesh* pm, FEFace& f, bool bsmooth);
void RenderTRI10(FECoreMesh* pm, FEFace& f, bool bsmooth);

void RenderSmoothQUAD4(FECoreMesh* pm, FEFace& face, int ndivs);
void RenderSmoothQUAD8(FECoreMesh* pm, FEFace& face, int ndivs);
void RenderSmoothQUAD9(FECoreMesh* pm, FEFace& face, int ndivs);
void RenderSmoothTRI3 (FECoreMesh* pm, FEFace& face, int ndivs);
void RenderSmoothTRI6 (FECoreMesh* pm, FEFace& face, int ndivs);
void RenderSmoothTRI7 (FECoreMesh* pm, FEFace& face, int ndivs);
void RenderSmoothTRI10(FECoreMesh* pm, FEFace& face, int ndivs);

void RenderFace1Outline(FECoreMesh* pm, FEFace& face);
void RenderFace2Outline(FECoreMesh* pm, FEFace& face, int ndivs);
void RenderFace3Outline(FECoreMesh* pm, FEFace& face, int ndivs);
