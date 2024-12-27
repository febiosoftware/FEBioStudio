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
#include "GLRenderEngine.h"

class CGLSceneView;

class OpenGLRenderer : public GLRenderEngine
{
	class Imp;

public:
	OpenGLRenderer(CGLSceneView* view);
	~OpenGLRenderer();

	void start() override;

	void finish() override;

public:
	void deleteCachedMesh(GMesh* gm) override;

public:
	void pushState() override;
	void popState() override;

	void pushTransform() override;
	void popTransform() override;

	void enable(StateFlag flag);
	void disable(StateFlag flag);

	void setColor(GLColor c) override;
	void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = true) override;

	void setPointSize(float f) override;

public:
	void renderPoint(const vec3d& r) override;
	void renderLine(const vec3d& a, const vec3d& b) override;

	void renderGMesh(const GMesh& mesh, bool cacheMesh = true) override;
	void renderGMesh(const GMesh& mesh, int surfId, bool cacheMesh) override;

	void renderGMeshNodes(const GMesh& mesh, bool cacheMesh = true) override;

	void renderGMeshEdges(const GMesh& mesh, bool cacheMesh) override;
	void renderGMeshEdges(const GMesh& mesh, int edgeId, bool cacheMesh) override;

	void renderGMeshOutline(CGLCamera& cam, const GMesh& mesh, const Transform& T, int surfID) override;

public:
	unsigned int LoadEnvironmentMap(const std::string& fileName) override;
	void ActivateEnvironmentMap(unsigned int mapid) override;
	void DeactivateEnvironmentMap(unsigned int mapid) override;

	void setClipPlane(unsigned int n, const double* v) override;

public:
	void renderGlyph(GlyphType glyph, float scale, GLColor c) override;

private:
	Imp& m;
};
