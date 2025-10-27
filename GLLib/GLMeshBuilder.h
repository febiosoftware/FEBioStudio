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
#include "GLMesh.h"
#include "GLRenderEngine.h"
#include "glmath.h"
#include <stack>

// Helper class for generating a GLMesh through render commands
class GLMeshBuilder : public GLRenderEngine
{
public:
	GLMeshBuilder();
	~GLMeshBuilder();

	void start() override;
	void finish() override;

	GLMesh* takeMesh();

public:
	void pushTransform() override;
	void popTransform() override;
	void translate(const vec3d& r) override;
	void rotate(const quatd& rot) override;
	void rotate(double deg, double x, double y, double z) override;
	void scale(double x, double, double z) override;

	void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = true) override;

	void beginShape();
	void endShape();

	void begin(PrimitiveType prim) override;
	void end() override;

	void vertex(const vec3d& r) override;
	void normal(const vec3d& r) override;

private:
	void buildPoints();
	void buildLines();
	void buildLineStrip();
	void buildLineLoop();
	void buildTriangles();
	void buildQuadStrip();

private:
	GLMesh* m_pm = nullptr;

	mat4d modelView;
	bool isMVIdentity = true;
	std::stack<mat4d> mvStack;

	std::vector<GLMesh::NODE> vertList;
	GLColor currentColor;
	vec3d currentNormal;
	PrimitiveType currentPrimitive;
};
