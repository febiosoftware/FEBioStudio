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
#include <MeshLib/GMesh.h>
#include "GLMesh.h"
#include "GLShader.h"
#include "GLRenderStats.h"
#include <functional>

class GMesh;
class CGLCamera;
class Transform;

class GLMeshRender
{
public:
	GLMeshRender();
	~GLMeshRender();

	void ShowShell2Hex(bool b) { m_bShell2Solid = b; }
	bool ShowShell2Hex() { return m_bShell2Solid; }

	void SetSolidBeamRadiusScale(float f) { m_bSolidBeamRadius = f; }
	float GetSolidBeamRadiusScale() const { return m_bSolidBeamRadius; }

	void ShowBeam2Hex(bool b) { m_bBeam2Solid = b; }
	bool ShowBeam2Hex() { return m_bBeam2Solid; }

	void SetPointSize(float f) { m_pointSize = f; }

	void SetDivisions(int ndivs) { m_ndivs = ndivs; }

public:
	void SetUseShaders(bool b);
	void ClearShaders();
	void AddShader(GLFacetShader* shader);
	void SetDefaultShader(GLFacetShader* shader);

	void SetPointShader(GLPointShader* pointShader);
	void SetLineShader(GLLineShader* lineShader);
	void SetFacetShader(GLFacetShader* facetShader);

public:
	void PushState();
	void PopState();

public:
	void RenderGMeshLines(GMesh* pm);
	void RenderLine(const vec3d& a, const vec3d& b);

public:
	void RenderGMesh(const GMesh& mesh);
	void RenderGMesh(const GMesh& mesh, GLFacetShader& shader);
	void RenderGMesh(const GMesh& mesh, int surfID);
	void RenderGMesh(const GMesh& mesh, int surfID, GLFacetShader& shader);

	void RenderEdges(const GMesh& m);
	void RenderEdges(const GMesh& m, std::function<bool(const GMesh::EDGE& e)> f);
	void RenderEdges(const GMesh& p, int nedge);

	void RenderOutline(CGLCamera& cam, GMesh* pm, const Transform& T, bool outline = false);
	void RenderSurfaceOutline(CGLCamera& cam, GMesh* pm, const Transform& T, int surfID);

	void RenderPoints(GMesh& mesh);
	void RenderPoints(GMesh& mesh, std::vector<int>& nodeList);
	void RenderPoints(GMesh& mesh, std::function<bool(const GMesh::NODE& node)> fnc);

public:
	void RenderNormals(const GMesh& gm, GLLineShader& shader);

public:
	int			m_ndivs;			//!< divisions for smooth render
	bool		m_bShell2Solid;		//!< render shells as solid
	bool		m_bBeam2Solid;		//!< render beams (truss, line elements) as solid
	float		m_bSolidBeamRadius;	//!< radius scale factor when rendering beams as solid
	int			m_nshellref;		//!< shell reference surface
	float		m_pointSize;		//!< size of points
	bool		m_useShaders;		//!< use the shaders

private:
	GLTriMesh	m_glmesh;
	std::vector<GLFacetShader*> m_shaders;
	GLFacetShader* m_defaultShader;

	GLPointShader* m_pointShader;
	GLLineShader* m_lineShader;
	GLFacetShader* m_facetShader;
};
