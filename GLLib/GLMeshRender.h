/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include <MathLib/math3d.h>
#include <FSCore/color.h>

class FEElement_;
class FEEdge;
class FEFace;
class FELineMesh;
class FECoreMesh;
class FEMeshBase;
class GLMesh;

class GLMeshRender
{
public:
	GLMeshRender();

	void ShowShell2Hex(bool b) { m_bShell2Solid = b; }
	bool ShowShell2Hex() { return m_bShell2Solid; }

	void SetPointSize(float f) { m_pointSize = f; }

	void SetDivisions(int ndivs) { m_ndivs = ndivs; }

public:
	void RenderGLMesh(GLMesh* pm, int nid = -1);
	void RenderGLEdges(GLMesh* pm, int nid = -1);


public:
	void RenderFENodes(FELineMesh* mesh);

	void RenderSelectedFEEdges(FELineMesh* pm);
	void RenderUnselectedFEEdges(FELineMesh* pm);
	void RenderFaceEdge(FEFace& face, int j, FEMeshBase* pm, int ndivs);

	void RenderMeshLines(FEMeshBase* pm);
	void RenderSelectedFEFaces(FEMeshBase* pm);
	void RenderUnselectedFEFaces(FEMeshBase* pm);
	void RenderSelectedFEFacesOutline(FEMeshBase* pm);

	void RenderElementOutline(FEElement_& el, FECoreMesh* pm, int ndivs);

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
    void RenderPYRA13 (FEElement_* pe, FECoreMesh* pm, bool bsel);

	void RenderHEX8(FEElement_ *pe, FECoreMesh *pm, GLColor* col);
	void RenderPENTA6(FEElement_ *pe, FECoreMesh *pm, GLColor* col);
	void RenderTET4(FEElement_ *pe, FECoreMesh *pm, GLColor* col);
	void RenderTET10(FEElement_* pe, FECoreMesh* pm, GLColor* col);
	void RenderTRI3(FEElement_* pe, FECoreMesh* pm, GLColor* col);
	void RenderQUAD(FEElement_* pe, FECoreMesh* pm, GLColor* col);

public:
	// drawing routines for faces
	void RenderFEFace(FEFace& face, FEMeshBase* pm);
	void RenderFace(FEFace& face, FECoreMesh* pm);
	void RenderFace(FEFace& face, FECoreMesh* pm, GLColor c[4], int ndivs);

	void RenderFaceOutline(FEFace& face, FECoreMesh* pm, int ndivs);

private:
	// special render routines for thick shells
	void RenderThickShell(FEFace& face, FECoreMesh* pm);
	void RenderThickQuad (FEFace& face, FECoreMesh* pm);
	void RenderThickTri  (FEFace& face, FECoreMesh* pm);
	void RenderThickShellOutline(FEFace& face, FECoreMesh* pm);

public:
	int			m_ndivs;			//!< divisions for smooth render
	bool		m_bShell2Solid;		//!< render shells as solid
	int			m_nshellref;		//!< shell reference surface
	float		m_pointSize;		//!< size of points
};

// drawing routines for edges
// Note: Call this from within glBegin(GL_LINES)\glEnd() section
void RenderFEEdge(FEEdge& edge, FELineMesh* pm);

// drawing routines for faces
// Note: Call these functions from within glBegin(GL_TRIANGLES)\glEnd() section
void RenderQUAD4(FEMeshBase* pm, FEFace& f);
void RenderQUAD8(FEMeshBase* pm, FEFace& f);
void RenderQUAD9(FEMeshBase* pm, FEFace& f);
void RenderTRI3 (FEMeshBase* pm, FEFace& f);
void RenderTRI6 (FEMeshBase* pm, FEFace& f);
void RenderTRI7 (FEMeshBase* pm, FEFace& f);
void RenderTRI10(FEMeshBase* pm, FEFace& f);

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
