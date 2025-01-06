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
#include "RTBTree.h"
#include "RTMath.h"
#include "RTTexture.h"
#include <stack>

namespace rt {
	struct Material 
	{
		int shininess = 0;
		int tex1d = -1;
		double opacity = 1;
	};
}

class RayTracer : public GLRenderEngine
{
public:
	RayTracer(RayTraceSurface& target);
	~RayTracer();

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
	void rotate(double deg, double x, double y, double z) override;
	void scale(double x, double, double z) override;

public:
	GLRenderEngine::FrontFace frontFace() const;
	void setFrontFace(FrontFace f);

	void positionCamera(const GLCamera& cam) override;

public:
	void setColor(GLColor c) override;
	void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = true)  override;

	void setLightPosition(unsigned int lightIndex, const vec3f& p) override;

public:
	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh = true) override;

public:
	void setClipPlane(unsigned int n, const double* v) override;
	void enableClipPlane(unsigned int n) override;
	void disableClipPlane(unsigned int n) override;

public:
	void setTexture(GLTexture1D& tex) override;

private:
	void preprocess();
	void render();
	rt::Color castRay(rt::Btree& octree, rt::Ray& ray);

	void addTriangle(rt::Tri& tri);

private:
	RayTraceSurface& surf;
	rt::Mesh mesh;
	rt::Btree btree;
	rt::Matrix4 modelView;
	std::stack<rt::Matrix4> mvStack;

	std::vector<rt::Texture1D*> tex1d;
	int currentTexture = -1;
	bool useTexture1D = false;

	int currentMaterial = -1;
	std::vector<rt::Material> matList;

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

	GLRenderEngine::FrontFace front = GLRenderEngine::COUNTER_CLOCKWISE;

	bool useClipPlane = false;
	double clipPlane[4] = { 0,0,0,0 };

	bool cancelled;
};
