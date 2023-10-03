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
#include <functional>
#include "GLMesh.h"

class FEElement_;
class FSNode;
class FSEdge;
class FSFace;
class FSLineMesh;
class FSCoreMesh;
class FSMeshBase;
class GMesh;
class CGLContext;
class FSMesh;

class GLMeshRender
{
public:
	enum RenderMode {
		DefaultMode,
		SelectionMode,
		OutlineMode
	};
public:
	GLMeshRender();

	void ShowShell2Hex(bool b) { m_bShell2Solid = b; }
	bool ShowShell2Hex() { return m_bShell2Solid; }

	void SetSolidBeamRadiusScale(float f) { m_bSolidBeamRadius = f; }
	float GetSolidBeamRadiusScale() const { return m_bSolidBeamRadius; }

	void ShowBeam2Hex(bool b) { m_bBeam2Solid = b; }
	bool ShowBeam2Hex() { return m_bBeam2Solid; }

	void SetPointSize(float f) { m_pointSize = f; }

	void SetDivisions(int ndivs) { m_ndivs = ndivs; }

public:
	void SetRenderMode(RenderMode mode);

	void PushState();
	void PopState();

public:
	void RenderGLMesh(GMesh* pm, int nid = -1);
	void RenderGLEdges(GMesh* pm, int nid = -1);
	void RenderOutline(CGLContext& rc, GMesh* pm, bool outline = false);

public:
	void RenderFENodes(FSLineMesh* mesh);
	void RenderFENodes(FSLineMesh& mesh, std::function<bool(const FSNode& node)> f);

public:
	void RenderFEEdges(FSLineMesh& mesh, std::function<bool(const FSEdge& edge)> f);

	void RenderSelectedFEEdges(FSLineMesh* pm);
	void RenderUnselectedFEEdges(FSLineMesh* pm);

	void RenderMeshLines(FSMeshBase* pm);
	void RenderMeshLines(FSMesh& mesh, std::function<bool(const FEElement_& el)> f);

	void RenderFEFaces(FSMeshBase* pm, const std::vector<int>& faceList);
	void RenderFEFaces(FSMeshBase* pm, const std::vector<FSFace>& faceList, std::function<bool(const FSFace& face)> f);
	void RenderFEFaces(FSMeshBase* pm, std::function<bool(const FSFace& face)> f);
	void RenderFEFaces(FSMeshBase* pm, const std::vector<int>& faceList, std::function<bool(const FSFace& face)> f);
	void RenderFEFaces(FSCoreMesh* pm, const std::vector<int>& faceList, std::function<bool(const FSFace& face, GLColor* c)> f);

	void RenderFEFaces(FSCoreMesh* pm, std::function<bool(const FSFace& face, GLColor* c)> f);

	void RenderFESurfaceMeshFaces(FSMeshBase* pm, std::function<bool(const FSFace& face, GLColor* c)> f);

	void RenderFEFacesOutline(FSMeshBase* pm, const std::vector<int>& faceList);
	void RenderFEFacesOutline(FSCoreMesh* pm, const std::vector<FSFace*>& faceList);
	void RenderFEFacesOutline(FSMeshBase* pm, std::function<bool(const FSFace& face)> f);

	void RenderFEElementsOutline(FSMesh& mesh, const std::vector<int>& elemList);
	void RenderFEElementsOutline(FSCoreMesh* pm, const std::vector<FEElement_*>& elemList);

private:
	void RenderElementOutline(FEElement_& el, FSCoreMesh* pm);

public:
	void RenderFEElements(FSMesh& mesh, const std::vector<int>& elemList, bool bsel = false);
	void RenderFEElements(FSMesh& mesh, std::function<bool(const FEElement_& el)> f);
	void RenderFEElements(FSMesh& mesh, std::function<bool(const FEElement_& el, GLColor* c)> f);
	void RenderFEElements(FSMesh& mesh, const std::vector<int>& elemList, std::function<bool(const FEElement_& el)> f);

	void RenderNormals(FSMeshBase* pm, float scale, int tag);

public:
	void RenderBEAM2(FEElement_* pe, FSCoreMesh* pm, bool bsel);

private:
	// drawing routines for faces
	void RenderFEFace(const FSFace& face, FSMeshBase* pm);
	void RenderFace(FSFace& face, FSCoreMesh* pm);
	void RenderFace(FSFace& face, FSCoreMesh* pm, GLColor c[4]);

	void RenderFESurfaceMeshFace(FSFace& face, FSMeshBase* pm, GLColor c[4]);

public:
	void RenderFaceOutline(FSFace& face, FSCoreMesh* pm);

	void SetFaceColor(bool b);
	bool GetFaceColor() const;

private:
	// special render routines for thick shells
	void RenderThickShell(FSFace& face, FSCoreMesh* pm);
	void RenderThickQuad(FSFace& face, FSCoreMesh* pm);
	void RenderThickTri(FSFace& face, FSCoreMesh* pm);
	void RenderThickShellOutline(FSFace& face, FSCoreMesh* pm);

public:
	int			m_ndivs;			//!< divisions for smooth render
	bool		m_bShell2Solid;		//!< render shells as solid
	bool		m_bBeam2Solid;		//!< render beams (truss, line elements) as solid
	float		m_bSolidBeamRadius;	//!< radius scale factor when rendering beams as solid
	int			m_nshellref;		//!< shell reference surface
	float		m_pointSize;		//!< size of points
	bool		m_bfaceColor;		//!< use face colors when rendering
	RenderMode	m_renderMode;

private:
	GLTriMesh	m_glmesh;
};
