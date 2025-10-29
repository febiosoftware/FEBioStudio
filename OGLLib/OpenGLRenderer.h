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
#include <GLLib/GLRenderEngine.h>
#include "OGLBase.h"

class OGLProgram;

class OpenGLRenderer : public GLRenderEngine, public OGLBase
{
	class Imp;

public:
	OpenGLRenderer();
	~OpenGLRenderer();

	void init() override;

	void start() override;

	void finish() override;

public:
	void deleteCachedMesh(GLMesh* gm) override;

public:
	void viewport(int vp[4]) override;
	void setViewport(int v[4]) override;

	void setOrthoProjection(double left, double right, double bottom, double top, double zNear, double zFar) override;

	void resetTransform() override;
	void pushTransform() override;
	void popTransform() override;

	void pushProjection() override;
	void popProjection() override;

	void translate(const vec3d& r) override;
	void rotate(const quatd& rot) override;
	void rotate(double angleDeg, double x, double y, double z) override;
	void scale(double x, double y, double z) override;
	void transform(const vec3d& pos, const quatd& rot) override;
	void multTransform(const double* m) override;

public:
	void pushState() override;
	void popState() override;

	void enable(StateFlag flag) override;
	void disable(StateFlag flag) override;

	void clearDepthBuffer() override;

	void setColor(GLColor c) override;
	void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = true) override;
	void setMaterial(const GLMaterial& mat) override;

	void setPointSize(float f) override;
	void setLineWidth(float f) override;

	float pointSize() override;
	float lineWidth() override;

	FrontFace frontFace() override;
	void setFrontFace(FrontFace f) override;

	void positionCamera(const GLCamera& cam) override;

public:
	void setLightPosition(unsigned int lightIndex, const vec3f& p) override;

public:
	void begin(PrimitiveType prim) override;
	void end() override;

	void vertex(const vec3d& r) override;
	void normal(const vec3d& r) override;
	void texCoord1d(double t) override;
	void texCoord2d(double r, double s) override;

public:
	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh) override;

	void renderGMeshNodes(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderTaggedGMeshNodes(const GLMesh& mesh, int ntag) override;

	void renderGMeshEdges(const GLMesh& mesh, bool cacheMesh) override;
	void renderGMeshEdges(const GLMesh& mesh, int edgeId, bool cacheMesh) override;

	void renderGMeshOutline(const GLCamera& cam, const GLMesh& mesh, const Transform& T) override;
	void renderGMeshOutline(const GLCamera& cam, const GLMesh& mesh, const Transform& T, int surfID) override;

public:
	unsigned int LoadEnvironmentMap(const std::string& fileName) override;
	void ActivateEnvironmentMap(unsigned int mapid) override;
	void DeactivateEnvironmentMap(unsigned int mapid) override;

	void setClipPlane(unsigned int n, const double* v) override;
	void enableClipPlane(unsigned int n) override;
	void disableClipPlane(unsigned int n) override;

public:
	void setTexture(GLTexture1D& tex) override;
	void setTexture(GLTexture2D& tex) override;
	void setTexture(GLTexture3D& tex) override;

private:
	void UseProgram(OGLProgram* program);
	void enableLineMode(bool b);

private:
	Imp& m;
};
