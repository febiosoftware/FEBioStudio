#pragma once
#include <MathLib/math3d.h>
#include <FSCore/color.h>

class FEElement_;
class FECoreMesh;
class GLMesh;
class FEFace;

class GLMeshRender
{
public:
	GLMeshRender();

	void ShowShell2Hex(bool b) { m_bShell2Solid = b; }
	bool ShowShell2Hex() { return m_bShell2Solid; }

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

public:
	// drawing routines for faces
	void RenderFace(FEFace& face, FECoreMesh* pm, int ndivs);
	void RenderFace(FEFace& face, FECoreMesh* pm, GLColor c[4], int ndivs);

	void RenderFaceOutline(FEFace& face, FECoreMesh* pm, int ndivs);

	// special render routines for thick shells
	void RenderThickShell(FEFace& face, FECoreMesh* pm);
	void RenderThickQuad (FEFace& face, FECoreMesh* pm);
	void RenderThickTri  (FEFace& face, FECoreMesh* pm);
	void RenderThickShellOutline(FEFace& face, FECoreMesh* pm);

public:
	bool		m_bShell2Solid;	//!< render shells as solid
	int			m_nshellref;	//!< shell reference surface
};

// drawing routines for faces
// Note: Call this from within glBegin(GL_TRIANGLES)\glEnd() section
void RenderQUAD4(FECoreMesh* pm, FEFace& f);
void RenderQUAD8(FECoreMesh* pm, FEFace& f);
void RenderQUAD9(FECoreMesh* pm, FEFace& f);
void RenderTRI3 (FECoreMesh* pm, FEFace& f);
void RenderTRI6 (FECoreMesh* pm, FEFace& f);
void RenderTRI7 (FECoreMesh* pm, FEFace& f);
void RenderTRI10(FECoreMesh* pm, FEFace& f);

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
