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
#include <GLLib/GLRenderEngine.h>
#include "RayTraceSurface.h"
#include "RTMesh.h"
#include "RTMath.h"
#include <stack>

class RayTracer : public GLRenderEngine
{
public:
	RayTracer(RayTraceSurface& target);

	void setupProjection(double fov, double fnear);

	void setMultiSample(int ms);

	void setBTreeLevels(int levels);

	void useShadows(bool b);

	RayTraceSurface& surface() { return surf; }

public:
	void start() override;
	void finish() override;

	double progress();
	bool hasProgress();

	void cancel();

public:
	void pushTransform() override;
	void popTransform() override;
	void translate(const vec3d& r) override;
	void rotate(const quatd& rot) override;

	void positionCamera(const GLCamera& cam) override;

public:
	void setColor(GLColor c) override;
	void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = true)  override;

	void setLightPosition(unsigned int lightIndex, const vec3f& p) override;

public:
	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh = true) override;

private:
	void render();
	rt::Color castRay(rt::Btree& octree, rt::Ray& ray);

private:
	RayTraceSurface& surf;
	rt::Mesh mesh;
	rt::Matrix4 modelView;
	std::stack<rt::Matrix4> mvStack;

	rt::Vec4 lightPos;

	double fieldOfView;
	double nearPlane;

	bool renderShadows = true;
	int multiSample = 1;
	int treeLevels = 8;

	bool renderStarted;
	double percentCompleted;

	GLColor currentColor;
	bool useVertexColor;

	bool cancelled;
};
