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
#include "GLRenderStats.h"
#include "GLMaterial.h"
#include "GLTexture1D.h"
#include "GLTexture2D.h"
#include "GLTexture3D.h"
#include <FECore/FETransform.h>
#include <FSCore/FSObject.h>

class GLMesh;
class GLCamera;

class GLRenderEngine : public FSObject
{
public:
	enum StateFlag {
		LIGHTING = 1,
		DEPTHTEST,
		CULLFACE,
		BLENDING,
		LINESTIPPLE
	};

	enum PrimitiveType {
		POINTS,
		LINES,
		LINELOOP,
		LINESTRIP,
		TRIANGLES,
		TRIANGLEFAN,
		QUADS,
		QUADSTRIP
	};

	enum FrontFace {
		COUNTER_CLOCKWISE,
		CLOCKWISE
	};

public:
	GLRenderEngine() {}
	virtual ~GLRenderEngine() {}

	void ResetStats();
	virtual GLRenderStats GetRenderStats() const;

public:
	virtual void init() {}
	virtual void start() { ResetStats(); }
	virtual void finish() {}

	virtual void flush() {}

public:
	virtual void deleteCachedMesh(GLMesh* gm) {}
	 
public:
	virtual void viewport(int vp[4]) {}
	virtual void setViewport(int v[4]) {}

	virtual void resetTransform() {}
	virtual void pushTransform() {}
	virtual void popTransform() {}

	virtual void pushProjection() {}
	virtual void popProjection() {}

	virtual void translate(const vec3d& r) {}
	virtual void rotate(const quatd& rot) {}
	virtual void rotate(double angleDeg, double x, double y, double z) {}
	virtual void scale(double x, double y, double z) {}
	virtual void transform(const vec3d& pos, const quatd& rot) {}
	virtual void multTransform(const double* m) {}

	void transform(const vec3d& r0, const vec3d& r1);
	void rotate(const vec3d& r, vec3d ref = vec3d(0, 0, 1));

public:
	virtual void pushState() {}
	virtual void popState() {}

	virtual void enable(StateFlag flag) {}
	virtual void disable(StateFlag flag) {}

	virtual void clearDepthBuffer() {}

	virtual void setColor(GLColor c) {}
	virtual void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = false) {}
	virtual void setMaterial(const GLMaterial& mat) {}

	virtual void setPointSize(float f) {}
	virtual void setLineWidth(float f) {}
	virtual void setLineStipple(int factor, unsigned short pattern) {}

	virtual float pointSize() { return 0.f; }
	virtual float lineWidth() { return 0.f; }

	virtual FrontFace frontFace() { return FrontFace::COUNTER_CLOCKWISE; }
	virtual void setFrontFace(FrontFace f) {}

	virtual void setProjection(double fov, double near, double far) {}
	virtual void setOrthoProjection(double left, double right, double bottom, double top, double zNear, double zFar) {}

public:
	virtual void setLightPosition(unsigned int lightIndex, const vec3f& p) {}
	virtual void setLightSpecularColor(unsigned int lightIndex, const GLColor& col) {}

	virtual void setBackgroundColor(const GLColor& c) {}

public: // immediate mode rendering
	virtual void beginShape() {}
	virtual void endShape() {}

	virtual void begin(PrimitiveType prim) {}
	virtual void end() {}

	virtual void vertex(const vec3d& r) {}
	virtual void normal(const vec3d& r) {}
	virtual void texCoord1d(double t) {}
	virtual void texCoord2d(double r, double s) {}

	void vertex(double x, double y, double z = 0.0) { vertex(vec3d(x, y, z)); }

public: // uses immediate mode
	void renderPoint(const vec3d& r);
	void renderLine(const vec3d& a, const vec3d& b);
	void renderLine(const vec3d& a, const vec3d& b, const vec3d& c);
	void renderPath(const std::vector<vec3d>& points);
	void renderRect(double x0, double y0, double x1, double y1);

public:
	virtual void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) {}
	virtual void renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh = true) {}

	virtual void renderGMeshNodes(const GLMesh& mesh, bool cacheMesh = true) {}

	virtual void renderGMeshEdges(const GLMesh& mesh, bool cacheMesh = true) {}
	virtual void renderGMeshEdges(const GLMesh& mesh, int edgeId, bool cacheMesh = true) {}

	void renderGMeshOutline(const GLCamera& cam, const GLMesh& mesh, const Transform& T);
	void renderGMeshOutline(const GLCamera& cam, const GLMesh& mesh, const Transform& T, int surfID);

public:
	virtual unsigned int LoadEnvironmentMap(const std::string& fileName) { return 0; }
	virtual void ActivateEnvironmentMap(unsigned int mapid) {}
	virtual void DeactivateEnvironmentMap(unsigned int mapid) {}

	virtual void setClipPlane(unsigned int n, const double* v) {}
	virtual void enableClipPlane(unsigned int n) {}
	virtual void disableClipPlane(unsigned int n) {}

public:
	virtual void setTexture(GLTexture1D& tex) {}
	virtual void setTexture(GLTexture2D& tex) {}
	virtual void setTexture(GLTexture3D& tex) {}

protected:
	GLRenderStats m_stats;
};
