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
#include <FSCore/color.h>

class FEElement_;
class FSEdge;
class FSFace;
class FSLineMesh;
class FSCoreMesh;
class FSMeshBase;
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
	void RenderFENodes(FSLineMesh* mesh);

	void RenderSelectedFEEdges(FSLineMesh* pm);
	void RenderUnselectedFEEdges(FSLineMesh* pm);
	void RenderFaceEdge(FSFace& face, int j, FSMeshBase* pm, int ndivs);

	void RenderMeshLines(FSMeshBase* pm);
	void RenderSelectedFEFaces(FSMeshBase* pm);
	void RenderUnselectedFEFaces(FSMeshBase* pm);
	void RenderSelectedFEFacesOutline(FSMeshBase* pm);

	void RenderElementOutline(FEElement_& el, FSCoreMesh* pm, int ndivs);

public:
	// drawing routines for elements
	void RenderHEX8   (FEElement_ *pe, FSCoreMesh *pm, bool bsel);
	void RenderHEX20  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderHEX27  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderPENTA  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderTET4   (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderTET10  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderTET15  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderTET20  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderQUAD   (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderQUAD8  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderQUAD9  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderTRI3   (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderTRI6   (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderPYRA5  (FEElement_* pe, FSCoreMesh* pm, bool bsel);
	void RenderPENTA15(FEElement_* pe, FSCoreMesh *pm, bool bsel);
    void RenderPYRA13 (FEElement_* pe, FSCoreMesh* pm, bool bsel);

	void RenderHEX8(FEElement_ *pe, FSCoreMesh *pm, GLColor* col);
	void RenderTET4(FEElement_ *pe, FSCoreMesh *pm, GLColor* col);
	void RenderTET10(FEElement_* pe, FSCoreMesh* pm, GLColor* col);
	void RenderTRI3(FEElement_* pe, FSCoreMesh* pm, GLColor* col);
	void RenderQUAD(FEElement_* pe, FSCoreMesh* pm, GLColor* col);

public:
	// drawing routines for faces
	void RenderFEFace(FSFace& face, FSMeshBase* pm);
	void RenderFace(FSFace& face, FSCoreMesh* pm);
	void RenderFace(FSFace& face, FSCoreMesh* pm, GLColor c[4], int ndivs);

	void RenderFaceOutline(FSFace& face, FSCoreMesh* pm, int ndivs);

	void SetFaceColor(bool b);
	bool GetFaceColor() const;

private:
	// special render routines for thick shells
	void RenderThickShell(FSFace& face, FSCoreMesh* pm);
	void RenderThickQuad (FSFace& face, FSCoreMesh* pm);
	void RenderThickTri  (FSFace& face, FSCoreMesh* pm);
	void RenderThickShellOutline(FSFace& face, FSCoreMesh* pm);

public:
	int			m_ndivs;			//!< divisions for smooth render
	bool		m_bShell2Solid;		//!< render shells as solid
	int			m_nshellref;		//!< shell reference surface
	float		m_pointSize;		//!< size of points
	bool		m_bfaceColor;		//!< use face colors when rendering
};

// drawing routines for edges
// Note: Call this from within glBegin(GL_LINES)\glEnd() section
void RenderFEEdge(FSEdge& edge, FSLineMesh* pm);

// drawing routines for faces
// Note: Call these functions from within glBegin(GL_TRIANGLES)\glEnd() section
void RenderQUAD4(FSMeshBase* pm, FSFace& f);
void RenderQUAD8(FSMeshBase* pm, FSFace& f);
void RenderQUAD9(FSMeshBase* pm, FSFace& f);
void RenderTRI3 (FSMeshBase* pm, FSFace& f);
void RenderTRI6 (FSMeshBase* pm, FSFace& f);
void RenderTRI7 (FSMeshBase* pm, FSFace& f);
void RenderTRI10(FSMeshBase* pm, FSFace& f);

void RenderSmoothQUAD4(FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderSmoothQUAD8(FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderSmoothQUAD9(FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderSmoothTRI3 (FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderSmoothTRI6 (FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderSmoothTRI7 (FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderSmoothTRI10(FSCoreMesh* pm, FSFace& face, int ndivs);

void RenderFace1Outline(FSCoreMesh* pm, FSFace& face);
void RenderFace2Outline(FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderFace3Outline(FSCoreMesh* pm, FSFace& face, int ndivs);
