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
#include <FSCore/math3d.h>
#include <FSCore/color.h>
#include <string>
#include <GLLib/GLRenderStats.h>
#include <GLLib/GLMaterial.h>
#include <GLLib/GLCamera.h>
#include <FECore/FETransform.h>

class GMesh;

class GLRenderEngine
{
public:
	enum StateFlag {
		LIGHTING = 1,
		CLIPPLANE
	};

	enum GlyphType {
		RIGID_BODY,
		RIGID_WALL,
		RIGID_JOINT,
		REVOLUTE_JOINT,
		PRISMATIC_JOINT,
		CYLINDRICAL_JOINT,
		PLANAR_JOINT,
		RIGID_LOCK
	};

public:
	GLRenderEngine() {}
	virtual ~GLRenderEngine() {}

	void ResetStats();
	GLRenderStats GetRenderStats();

public:
	virtual void pushState() {}
	virtual void popState() {}

	virtual void enable(StateFlag flag) {}
	virtual void disable(StateFlag flag) {}

	virtual void setColor(GLColor c) {}
	virtual void setMaterial(GLMaterial::Type mat, GLColor c) {}

public:
	virtual void renderPoint(const vec3d& r) {}
	virtual void renderLine(const vec3d& a, const vec3d& b) {}

	virtual void renderGMesh(const GMesh& mesh, bool cacheMesh = true) {}
	virtual void renderGMesh(const GMesh& mesh, int surfId, bool cacheMesh = true) {}

	virtual void renderGMeshNodes(const GMesh& mesh, bool cacheMesh = true) {}

	virtual void renderGMeshEdges(const GMesh& mesh, bool cacheMesh = true) {}
	virtual void renderGMeshEdges(const GMesh& mesh, int edgeId, bool cacheMesh = true) {}

	virtual void renderGMeshOutline(CGLCamera& cam, const GMesh& mesh, const Transform& T, int surfID) {}

public:
	virtual unsigned int LoadEnvironmentMap(const std::string& fileName) { return 0; }
	virtual void ActivateEnvironmentMap(unsigned int mapid) {}
	virtual void DeactivateEnvironmentMap(unsigned int mapid) {}

	virtual void setClipPlane(unsigned int n, const double* v) {}

public:
	virtual void renderGlyph(GlyphType glyph, float scale, GLColor c) {}

protected:
	GLRenderStats m_stats;
};
