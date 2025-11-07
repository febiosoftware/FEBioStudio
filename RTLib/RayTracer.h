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
		bool lighting = true;
		double shininess = 0;
		int tex1d = -1;
		int tex3d = -1;
		double reflection = 0;

		Color ambient  = GLColor::Black();
		Color specular = GLColor::Black();
	};
}

class RayTracer : public GLRenderEngine
{
	enum { WIDTH, HEIGHT, SHADOWS, MULTI_SAMPLE, BACKGROUND, BHV_LEVELS };

public:
	RayTracer();
	~RayTracer();

	void setProjection(double fov, double fnear, double far) override;

	void setOrthoProjection(double left, double right, double bottom, double top, double near, double far) override;

	void setClearColor(const GLColor& c) override;

	void setBackgroundGradient(const GLColor& c1, const GLColor& c2, GradientType orient) override;

	void setSampleCount(int n);

	void setRenderShadows(bool b);

	void setOutput(bool b) { output = b; }

	void setWidth (size_t W) { SetIntValue(WIDTH , (int) W); }
	void setHeight(size_t H) { SetIntValue(HEIGHT, (int) H); }
	RayTraceSurface& surface() { return surf; }

	int surfaceWidth() const override { return GetIntValue(WIDTH); }
	int surfaceHeight() const override { return GetIntValue(HEIGHT); }

public:
	void start() override;
	void finish() override;

	double progress();
	bool hasProgress();

	void cancel();

public:
	void pushTransform() override;
	void popTransform() override;
	void resetTransform() override;

	void translate(const vec3d& r) override;
	void rotate(const quatd& rot) override;
	void rotate(double deg, double x, double y, double z) override;
	void scale(double x, double, double z) override;

public:
	GLRenderEngine::FrontFace frontFace() const;
	void setFrontFace(FrontFace f);

public:
	void setColor(GLColor c) override;
	void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = true)  override;
	void setMaterial(const GLMaterial& mat) override;

	void setLightPosition(unsigned int lightIndex, const vec3f& p) override;
	void setLightSpecularColor(unsigned int lightIndex, const GLColor& col) override;

public: // immediate mode rendering
	void begin(PrimitiveType prim) override;
	void end() override;

	void vertex(const vec3d& r) override;
	void normal(const vec3d& r) override;
	void texCoord1d(double t) override;
	void texCoord2d(double r, double s) override;

public:
	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh = true) override;

public:
	void setClipPlane(unsigned int n, const double* v) override;
	void enableClipPlane(unsigned int n) override;
	void disableClipPlane(unsigned int n) override;

public:
	void setTexture(GLTexture1D& tex) override;
	void setTexture(GLTexture3D& tex) override;

	unsigned int SetEnvironmentMap(const CRGBAImage& img) override;
	void ActivateEnvironmentMap(unsigned int id) override;
	void DeactivateEnvironmentMap(unsigned int id) override;

private:
	void preprocess();
	void render();
	rt::Color castRay(rt::Btree& bhv, rt::Ray& ray);

	void addTriangle(rt::Tri& tri);

	GLColor backgroundColor(const rt::Vec3& r);

private:
	RayTraceSurface surf;

	rt::Mesh mesh;
	rt::Btree bhv;
	rt::Matrix4 modelView;
	std::stack<rt::Matrix4> mvStack;

	std::vector<rt::Texture1D*> tex1d;
	int currentTexture1D = -1;
	bool useTexture1D = false;

	rt::Texture2D envTex;
	bool useEnvTex = false;

	std::vector<rt::Texture3D*> tex3d;
	int currentTexture3D = -1;
	bool useTexture3D = false;

	int currentMaterial = -1;
	std::vector<rt::Material> matList;

	rt::Vec4 lightPos;
	rt::Color lightSpecular = rt::Color(1.f, 1.f, 1.f);

	bool ortho = false;
	double fieldOfView;
	double nearPlane;
	double m_fw, m_fh;
	double m_left, m_right;
	double m_bottom, m_top;

	bool immediateMode = false;
	PrimitiveType primType = POINTS;
	std::vector<rt::Point> verts;
	rt::Vec3 currentNormal;
	rt::Vec3 currentTexCoord;

	bool renderStarted;
	double percentCompleted;

	GLColor currentColor;
	bool useVertexColor;

	GLColor backgroundCol;

	// gradient
	GLColor m_col1, m_col2;
	GLRenderEngine::GradientType m_orient = GLRenderEngine::HORIZONTAL;

	GLRenderEngine::FrontFace front = GLRenderEngine::COUNTER_CLOCKWISE;

	bool useClipPlane = false;
	double clipPlane[4] = { 0,0,0,0 };

	bool cancelled;

	bool output = true;
};
