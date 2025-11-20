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
#include <GLLib/GLMath.h>
#include "RayTraceSurface.h"
#include "RTMesh.h"
#include "RTBTree.h"
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

		gl::Color ambient  = GLColor::Black();
		gl::Color specular = GLColor::Black();
	};

	class geometryItem {
	public:
		geometryItem() {}
		virtual ~geometryItem() {}

		virtual void start() {}
		virtual void finish() {}

		virtual bool intersect(const gl::Ray& ray, rt::Point& q) = 0;
	};

	class meshGeometry : public geometryItem
	{
	public:
		meshGeometry(rt::Mesh& m) : mesh(m) {}

		void setBHVLevels(int n) { bhv_levels = n; }
		void setOutput(bool b) { output = b; }

		void start() override;
		void finish() override;

		bool intersect(const gl::Ray& ray, rt::Point& q) override;

	private:
		rt::Mesh& mesh;
		rt::Btree bhv;
		int bhv_levels = -1;
		bool output = false;
	};

	class sphere : public geometryItem
	{
	public:
		sphere(const gl::Vec3& c, double R) : o(c), r(R) {}

		bool intersect(const gl::Ray& ray, rt::Point& q) override;

	public:
		int matid = -1;
		gl::Color col;

	private:
		gl::Vec3 o;
		double r = 1;
	};

	class Geometry
	{
	public:
		Geometry() {}
		~Geometry() { clear(); }

		void clear()
		{
			for (auto it : geom) delete it;
			geom.clear();
		}

		void finish();

		geometryItem* operator [] (size_t n) { return geom[n]; }

		bool empty() const { return geom.empty(); }

		void push_back(geometryItem* p) { geom.push_back(p); }

		bool intersect(const gl::Ray& rt, rt::Point& q);

		std::vector<geometryItem*>::iterator begin() { return geom.begin(); }
		std::vector<geometryItem*>::iterator end() { return geom.end(); }

	private:
		std::vector<geometryItem*> geom;
	};
}

class RayTracer : public GLRenderEngine
{
	enum { WIDTH, HEIGHT, SHADOWS, SHADOW_STRENGTH, MULTI_SAMPLE, BACKGROUND, BHV_LEVELS };

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

	void useMultiThread(bool b) { use_multithread = b; }

public:
	void start() override;
	void finish() override;

	double progress();
	bool hasProgress();

	void cancel();

public:
	GLRenderEngine::FrontFace frontFace() const;
	void setFrontFace(FrontFace f);

public:
	void setColor(GLColor c) override;
	void setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map = GLMaterial::DiffuseMap::NONE, bool frontOnly = true)  override;
	void setMaterial(const GLMaterial& mat) override;

	void setLightPosition(unsigned int lightIndex, const vec3f& p) override;
	void setLightAmbientColor (unsigned int lightIndex, const GLColor& col) override;
	void setLightDiffuseColor (unsigned int lightIndex, const GLColor& col) override;
	void setLightSpecularColor(unsigned int lightIndex, const GLColor& col) override;
	void setLightEnabled(unsigned int lightIndex, bool b) override;

public:
	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh = true) override;

	void renderGMeshEdges(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMeshEdges(const GLMesh& mesh, int partition, bool cacheMesh = true) override;

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

public:
	void addSphere(const vec3d& c, double R);

private:
	void preprocess();
	void render();

	// casts a ray and returns the intersection point
	// returns false if no intersection is found
	bool castRay(const gl::Ray& ray, rt::Point& pt);

	void addTriangle(rt::Tri& tri);

	void addLine(rt::Line& line);

	GLColor backgroundColor(const gl::Vec3& r);

	bool intersect(const gl::Ray& ray, rt::Point& q);

	rt::Fragment fragment(int i, int j, int samples);

	void renderLines();

	gl::Vec3 toNDC(gl::Vec4 v);
	gl::Vec3 NDCtoView(const gl::Vec3& v);

	bool clipLine(gl::Vec3& a, gl::Vec3& b);

private:
	RayTraceSurface surf;

	rt::Mesh mesh;
	rt::Geometry geom;

	gl::Matrix4 projMatrix;

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

	bool lightEnabled = true;
	gl::Vec4 lightPos;
	gl::Color lightSpecular = gl::Color(1.f, 1.f, 1.f);
	gl::Color lightDiffuse  = gl::Color(1.f, 1.f, 1.f);
	gl::Color lightAmbient  = gl::Color(0.2f, 0.2f, 0.2f);

	bool ortho = false;
	double fieldOfView;
	double nearPlane, farPlane;
	double m_fw, m_fh;
	double m_left, m_right;
	double m_bottom, m_top;

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
	bool use_multithread = true;
	bool output = true;
};
