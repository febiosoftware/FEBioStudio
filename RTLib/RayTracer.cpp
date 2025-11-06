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
#include "RayTracer.h"
#include <GLLib/GLMesh.h>
#include <FSCore/FSLogger.h>
using namespace rt;

#include <chrono>
using namespace std::chrono;
using dseconds = duration<double>;


RayTracer::RayTracer()
{
	fieldOfView = 60.0;
	nearPlane = 0.01;
	m_left = m_bottom = -1;
	m_right = m_top = 1;
	percentCompleted = 0.0;
	renderStarted = false;
	useVertexColor = false;
	cancelled = false;

	AddIntParam(0, "Width");
	AddIntParam(0, "Height");
	AddBoolParam(true, "Shadows");
	AddChoiceParam(0, "Multisample")->SetEnumNames(" Off\0 2x2\0 3x3\0 4x4\0");
	AddChoiceParam(0, "Background")->SetEnumNames("Default\0Transparent\0");
#ifndef NDEBUG
	Param* p = AddIntParam(-1, "BHV Levels");
	p->SetIntRange(-1, 20);
	p->SetVisible(false);
#endif
}

RayTracer::~RayTracer()
{
	for (rt::Texture1D* t : tex1d) delete t;
	tex1d.clear();

	for (rt::Texture3D* t : tex3d) delete t;
	tex3d.clear();
}

void RayTracer::setProjection(double fov, double fnear, double far)
{
	fieldOfView = fov;
	nearPlane = fnear;
	ortho = false;
}

void RayTracer::setOrthoProjection(double left, double right, double bottom, double top, double near, double far)
{
	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;
	nearPlane = near;
	ortho = true;
}

void RayTracer::setClearColor(const GLColor& c)
{
	backgroundCol = c;
}

void RayTracer::setBackgroundGradient(const GLColor& c1, const GLColor& c2, GradientType orient)
{
	m_col1 = c1;
	m_col2 = c2;
	m_orient = orient;
}

void RayTracer::setSampleCount(int n)
{
	if (n < 1) n = 1;
	if (n > 4) n = 4;
	SetIntValue(MULTI_SAMPLE, n - 1);
}

void RayTracer::setRenderShadows(bool b)
{
	SetBoolValue(SHADOWS, b);
}

void RayTracer::start()
{
	cancelled = false;
	GLRenderEngine::start();
	mesh.clear();
	modelView.makeIdentity();
}

double RayTracer::progress()
{
	return percentCompleted;
}

bool RayTracer::hasProgress()
{
	return renderStarted;
}

void RayTracer::cancel()
{
	cancelled = true;
}

void RayTracer::finish()
{
	// let's get to work!
	time_point<steady_clock> tic = steady_clock::now();
	preprocess();
	time_point<steady_clock> toc = steady_clock::now();
	double sec1 = duration_cast<dseconds>(toc - tic).count();
	if (output) FSLogger::Write("Preprocessing completed in %lg sec.\n", sec1);
	tic = toc;
	render();
	toc = steady_clock::now();
	double sec2 = duration_cast<dseconds>(toc - tic).count();
	if (output) FSLogger::Write("Rendering completed in %lg sec.\n", sec2);
	if (output) FSLogger::Write("Total elapsed time : %lg\n", sec1 + sec2);

	// clean up
	mesh.clear();
	GLRenderEngine::finish();
}

void RayTracer::pushTransform()
{
	mvStack.push(modelView);
}

void RayTracer::popTransform()
{
	assert(!mvStack.empty());
	modelView = mvStack.top();
	mvStack.pop();
}

void RayTracer::resetTransform()
{
	modelView.makeIdentity();
}

void RayTracer::translate(const vec3d& r)
{
	Matrix4 T = Matrix4::translate(Vec3(r));
	modelView *= T;
}

void RayTracer::rotate(const quatd& rot)
{
	Matrix4 R = Matrix4::rotate(rot);
	modelView *= R;
}

void RayTracer::rotate(double deg, double x, double y, double z)
{
	quatd q(deg * DEG2RAD, vec3d(x, y, z));
	rotate(q);
}

void RayTracer::scale(double x, double y, double z)
{
	Matrix4 S = Matrix4::scale(x, y, z);
	modelView *= S;
}

GLRenderEngine::FrontFace RayTracer::frontFace() const
{
	return front;
}

void RayTracer::setFrontFace(GLRenderEngine::FrontFace f)
{
	front = f;
}

void RayTracer::setColor(GLColor c)
{
	currentColor = c;
}

void RayTracer::setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
{
	currentColor = c;
	useVertexColor = (map == GLMaterial::VERTEX_COLOR);
	useTexture1D = (map == GLMaterial::TEXTURE_1D);
	useTexture3D = (map == GLMaterial::TEXTURE_3D);

	rt::Material mat;
	if ((matType == GLMaterial::PLASTIC) || (matType == GLMaterial::GLASS))
	{
		mat.shininess = 128;
	}

	if ((matType == GLMaterial::HIGHLIGHT) || (matType == GLMaterial::CONSTANT))
	{
		mat.shininess = 0;
		mat.lighting = false;
	}

	if (useTexture1D)
	{
		mat.tex1d = currentTexture1D;
		mat.shininess = 0; // don't add specular component when using textures
	}

	if (useTexture3D)
	{
		mat.tex3d = currentTexture3D;
		mat.shininess = 0; // don't add specular component when using textures
	}

	matList.push_back(mat);
	currentMaterial = (int)matList.size() - 1;
}

void RayTracer::setMaterial(const GLMaterial& glmat)
{
	currentColor = glmat.diffuse;
	currentColor.a = (uint8_t)(255.f*glmat.opacity);

	useVertexColor = false;
	useTexture1D = (glmat.diffuseMap == GLMaterial::TEXTURE_1D);

	rt::Material mat;
	if ((glmat.type == GLMaterial::PLASTIC) || (glmat.type == GLMaterial::GLASS))
	{
		mat.shininess = 128* glmat.shininess;
		if (mat.shininess < 0) mat.shininess = 0;
		if (mat.shininess > 128) mat.shininess = 128;
		mat.reflection = glmat.reflection;

		mat.ambient = glmat.ambient;
		mat.specular = glmat.specular;
	}

	if (useTexture1D)
	{
		mat.tex1d = currentTexture1D;
	}

	matList.push_back(mat);
	currentMaterial = (int)matList.size() - 1;
}

void RayTracer::setLightPosition(unsigned int lightIndex, const vec3f& p)
{
	Vec4 r(p, 0);
	lightPos = r;// modelView* r;
}

void RayTracer::setLightSpecularColor(unsigned int lightIndex, const GLColor& col)
{
	lightSpecular = Color(col);
}

void RayTracer::begin(PrimitiveType prim)
{
	assert(immediateMode == false);
	assert(verts.empty());
	immediateMode = true;
	primType = prim;
	verts.reserve(1024 * 1024);
}

void RayTracer::end()
{
	assert(immediateMode);
	if (immediateMode)
	{
		size_t vertices = verts.size();
		size_t n = 0;
		switch (primType)
		{
		case GLRenderEngine::TRIANGLES:
		{
			size_t ntri = vertices / 3;
			for (size_t i = 0; i < ntri; ++i, n += 3)
			{
				rt::Tri tri(verts[n], verts[n+1], verts[n+2]);
				tri.matid = currentMaterial;
				addTriangle(tri);
			}
		}
		break;
		case GLRenderEngine::TRIANGLEFAN:
		{
			size_t ntri = vertices - 2;
			for (size_t i = 0; i < ntri; ++i, n++)
			{
				rt::Tri tri(verts[0], verts[n + 1], verts[n + 2]);
				tri.matid = currentMaterial;
				addTriangle(tri);
			}
		}
		break;
		case GLRenderEngine::QUADSTRIP:
		{
			size_t nquads = (vertices - 2) / 2;
			for (size_t i = 0; i < nquads; ++i, n += 2)
			{
				rt::Tri tri1(verts[n], verts[n + 1], verts[n + 2]);
				rt::Tri tri2(verts[n+2], verts[n + 3], verts[n + 1]);
				tri1.matid = currentMaterial;
				tri2.matid = currentMaterial;
				addTriangle(tri1);
				addTriangle(tri2);
			}
		}
		break;
		}
	}
	immediateMode = false;
	verts.clear();
}

void RayTracer::vertex(const vec3d& r)
{
	Point p;
	p.r = modelView * Vec4(r);
	p.n = modelView * Vec4(currentNormal, 0); p.n.normalize();
	p.c = currentColor;
	p.t = currentTexCoord;
	verts.push_back(p);
}

void RayTracer::normal(const vec3d& r)
{
	currentNormal = Vec3(r);
}

void RayTracer::texCoord1d(double t)
{
	currentTexCoord = Vec3(t, 0, 0);
}

void RayTracer::texCoord2d(double r, double s)
{
	currentTexCoord = Vec3(r, s, 0);
}

void RayTracer::renderGMesh(const GLMesh& gmesh, bool cacheMesh)
{
	int NF = gmesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		const GLMesh::FACE& face = gmesh.Face(i);
		rt::Tri tri;
		if (front == GLRenderEngine::COUNTER_CLOCKWISE)
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[j] = modelView * Vec4(face.vr[j], 1);
				tri.n[j] = modelView * Vec4(face.vn[j], 0); tri.n[j].normalize();
				tri.t[j] = Vec3(face.t[j]);
				tri.c[j] = (useVertexColor ? face.c[j] : currentColor);
			}
		}
		else
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[2-j] = modelView * Vec4(face.vr[j], 1);
				tri.n[2-j] = modelView * Vec4(face.vn[j], 0); tri.n[j].normalize();
				tri.t[2-j] = Vec3(face.t[j]);
				tri.c[2-j] = (useVertexColor ? face.c[j] : currentColor);
			}
		}
		tri.matid = currentMaterial;
		addTriangle(tri);
	}
}

void RayTracer::renderGMesh(const GLMesh& gmesh, int surfId, bool cacheMesh)
{
	if ((surfId < 0) || (surfId >= gmesh.Partitions())) return;

	const GLMesh::PARTITION& p = gmesh.Partition(surfId);
	for (int i = 0; i < p.nf; ++i)
	{
		const GLMesh::FACE& face = gmesh.Face(p.n0 + i);
		rt::Tri tri;
		if (front == GLRenderEngine::COUNTER_CLOCKWISE)
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[j] = modelView * Vec4(face.vr[j], 1);
				tri.n[j] = modelView * Vec4(face.vn[j], 0); tri.n[j].normalize();
				tri.t[j] = Vec3(face.t[j]);
				tri.c[j] = (useVertexColor ? face.c[j] : currentColor);
			}
		}
		else
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[2-j] = modelView * Vec4(face.vr[j], 1);
				tri.n[2-j] = modelView * Vec4(face.vn[j], 0); tri.n[j].normalize();
				tri.t[2-j] = Vec3(face.t[j]);
				tri.c[2-j] = (useVertexColor ? face.c[j] : currentColor);
			}
		}
		tri.matid = currentMaterial;
		addTriangle(tri);
	}
}

Point interpolate(const Point& a, const Point& b, double w)
{
	double w1 = 1.0 - w;
	Point c;
	c.r = a.r * w1 + b.r * w;
	c.n = a.n * w1 + b.n * w; c.n.normalize();
	c.t = a.t * w1 + b.t * w;
	c.c = a.c * w1 + b.c * w;
	return c;
}

const int rtLUT[8][7] = {
	{ 0,  1,  2, -1, -1, -1, -1},
	{ 3,  1,  2,  2,  5,  3, -1},
	{ 2,  0,  3,  3,  4,  2, -1},
	{ 2,  5,  4, -1, -1, -1, -1},
	{ 0,  1,  5,  1,  4,  5, -1},
	{ 1,  4,  3, -1, -1, -1, -1},
	{ 0,  3,  5, -1, -1, -1, -1},
	{-1, -1, -1, -1, -1, -1, -1},
};

void RayTracer::addTriangle(rt::Tri& tri)
{
	// process triangle
	if (!tri.process()) return;

	// TODO: do frustum clipping

	// process clip plane
	if (useClipPlane)
	{
		double* c = clipPlane;
		Vec3 N(c[0], c[1], c[2]);
		Vec3* v = tri.r;
		unsigned int ncase = 0;
		for (int i = 0; i < 3; ++i)
		{
			double l = N * v[i] + c[3];
			if (l < 0) ncase |= (1 << i);
		}

		if (ncase == 0) mesh.addTri(tri);
		else if (ncase < 7)
		{
			Point p[6];
			p[0] = tri.point(0);
			p[1] = tri.point(1);
			p[2] = tri.point(2);

			for (int i = 0; i < 3; ++i)
			{
				Vec3 a = v[i];
				Vec3 b = v[(i + 1) % 3];
				Vec3 t = b - a;
				double D = N * t;
				if (D != 0)
				{
					double l = -(c[3] + N * a)/D;
					if (l < 0) l = 0;
					else if (l > 1) l = 1;
					p[i+3] = interpolate(p[i], p[(i+1)%3], l);
				}
			}

			const int* pf = rtLUT[ncase];
			while (*pf != -1)
			{
				rt::Tri tri_n(p[pf[0]], p[pf[1]], p[pf[2]]);
				tri_n.matid = tri.matid;
				if (tri_n.process())
					mesh.addTri(tri_n);
				pf += 3;
			}
		}
	}
	else mesh.addTri(tri);
}

void RayTracer::setClipPlane(unsigned int n, const double* v)
{
	if (n == 0)
	{
		Vec4 N(v[0], v[1], v[2], 0);
		Vec4 Np = modelView * N;
		double v3 = v[3] - (modelView[0][3] * Np[0] + modelView[1][3] * Np[1] + modelView[2][3] * Np[2]);

		clipPlane[0] = Np[0];
		clipPlane[1] = Np[1];
		clipPlane[2] = Np[2];
		clipPlane[3] = v3;
	}
}

void RayTracer::enableClipPlane(unsigned int n)
{
	if (n == 0)
	{
		useClipPlane = true;
	}
}

void RayTracer::disableClipPlane(unsigned int n)
{
	if (n == 0)
	{
		useClipPlane = false;
	}
}

void RayTracer::setTexture(GLTexture1D& tex)
{
	rt::Texture1D* t1d = new rt::Texture1D();
	t1d->setImageData(tex.Size(), tex.GetBytes());
	tex1d.push_back(t1d);
	currentTexture1D = (int)tex1d.size() - 1;
}

void RayTracer::setTexture(GLTexture3D& tex)
{
	rt::Texture3D* t3d = new rt::Texture3D();
	t3d->setImageData(tex.Get3DImage());
	tex3d.push_back(t3d);
	currentTexture3D = (int)tex3d.size() - 1;
	if (currentMaterial >= 0) matList[currentMaterial].tex3d = currentTexture3D;
}

void RayTracer::preprocess()
{
	size_t triangles = mesh.triangles();
	for (size_t i = 0; i < triangles; ++i) mesh.triangle(i).id = (int) i;

	int levels = -1;
#ifndef NDEBUG
	levels = GetIntValue(BHV_LEVELS);
#endif
	if (levels < 0)
	{
		levels = (int)log2((double)triangles);
	}
	if (levels < 0) levels = 0;
	if (levels > 20) levels = 20;

	bhv.output = output;
	bhv.Build(mesh, levels);
}

void RayTracer::render()
{
	renderStarted = false;
	percentCompleted = 0;

	// start ray-tracing process
	size_t W = GetIntValue(WIDTH);
	size_t H = GetIntValue(HEIGHT);
	if ((W == 0) || (H == 0)) return;
	surf.create(W, H);

	double ar = (double)W / (double)H;

	if (ortho)
	{
		m_fw = 0.5*(m_right - m_left);
		m_fh = 0.5*(m_top - m_bottom);
	}
	else
	{
		m_fh = nearPlane * tan(0.5 * fieldOfView * DEG2RAD);
		m_fw = m_fh * ar;
	}

	int samples = GetIntValue(MULTI_SAMPLE) + 1;
	if (samples < 1) samples = 1;
	if (samples > 4) samples = 4;

	int bgOption = GetIntValue(BACKGROUND);

	renderStarted = true;
#pragma omp parallel
	for (size_t j = 0; j < H; ++j)
	{
#pragma omp master
		percentCompleted = (100.0 * j) / (double) H;

#pragma omp for nowait schedule(dynamic, 5)
		for (int i = 0; i < W; ++i)
		{
			if (!cancelled)
			{
				double x = -m_fw + 2.0 * i * m_fw / (W - 1.0);
				double y = m_fh - 2.0 * j * m_fh / (H - 1.0);
				double z = -nearPlane;

				double dx = m_fw / W;
				double dy = m_fh / H;

				Color c(0, 0, 0, 0);
				for (int k = 0; k < samples; ++k)
					for (int l = 0; l < samples; ++l)
					{
						double fx = (2.0 * k + 1 - samples) / (double)samples;
						double fy = (2.0 * l + 1 - samples) / (double)samples;

						double xf = x + fx * dx;
						double yf = y + fy * dy;

						Vec3 origin(xf, yf, z);
						Vec3 direction;
						if (ortho)
							direction = Vec3(0, 0, -1);
						else
						{
							direction = origin;
							direction.normalize();
						}

						Ray ray(origin, direction);

						Color fragCol = castRay(bhv, ray);

						c += fragCol;
					}
				c /= (double)(samples * samples);

				float* v = surf.value(i, j);
				v[0] = (float)c.r();
				v[1] = (float)c.g();
				v[2] = (float)c.b();
				v[3] = (bgOption ? (float)c.a() : 1.f);
			}
		}
	}
	percentCompleted = 100.0;
}

GLColor RayTracer::backgroundColor(const rt::Vec3& p)
{
	double r = 0.5* (p.x() / m_fw)+0.5;
	double s = 0.5* (p.y() / m_fh)+0.5;
	if (r < 0) r = 0; if (r > 1) r = 1;
	if (s < 0) s = 0; if (s > 1) s = 1;

	GLColor c;
	if (m_orient == GLRenderEngine::HORIZONTAL)
		c = m_col2 * (1 - s) + m_col1 * s;
	else
		c = m_col1 * (1 - r) + m_col2 * r;

	c.a = 1;

	return c;
}

rt::Color RayTracer::castRay(rt::Btree& bhv, rt::Ray& ray)
{
	rt::Point q;

	int bgOption = GetIntValue(BACKGROUND);

	Color fragCol;
	switch (bgOption)
	{
	case 0: // default
		if (ray.bounce == 0)
			fragCol = backgroundColor(ray.origin);
		else
			fragCol = backgroundCol;
		break;
	case 1: // transparent
		fragCol = Color(0, 0, 0, 0);
		break;
	}
	bool intersectMesh = (bhv.intersect(ray, q));
	bool renderShadows = GetBoolValue(SHADOWS);

	if (intersectMesh)
	{
		// get some material props
		rt::Material mat;
		if (q.matid >= 0)
		{
			rt::Material& m = matList[q.matid];
			mat = m;
		}

		Color c = q.c;
		Vec3& t = ray.direction;
		Vec3 N = q.n;

		if (mat.lighting)
		{
			// reflection
			if ((mat.reflection > 0) && (ray.bounce < 2))
			{
				Vec3 H = t - N * (2 * (t * N));
				H.normalize();
				Ray ray2(q.r, H, q.tri_id);
				ray2.bounce = ray.bounce + 1;
				c = c * (1 - mat.reflection) + castRay(bhv, ray2) * mat.reflection;
			}
		}

		// see if we've reached the front or back face
		if (t * N >= 0)
		{
			// it's the back. Change normal and color
			N = -N;
			c = Color(0.8, 0.6, 0.6, c.a());
		}
		Vec3 L(lightPos); L.normalize();

		// apply texture
		if (mat.tex1d >= 0)
		{
			Color t = tex1d[mat.tex1d]->sample((float)q.t[0]);
			c *= t;
		}

		if (mat.tex3d >= 0)
		{
			Color t = tex3d[mat.tex3d]->sample((float)q.t[0], (float)q.t[1], (float)q.t[2]);
			double a = t.a();
			c.a() *= a;
		}
		
		bool isOccluded = false;
		if (mat.lighting)
		{
			// calculate an ambient value
			fragCol = mat.ambient;
			double f = N * Vec3(0, 0, 1);
			if (f < 0) f = 0;
			fragCol += c * (f * 0.2);

			// see if the point is occluded or not
			if (renderShadows)
			{
				Vec3 p = q.r;
				Ray ray2(p, L, q.tri_id);
				ray2.bounce = ray.bounce + 1;
				rt::Point q2;
				if (bhv.intersect(ray2, q2)) isOccluded = true;
			}

			if (!isOccluded)
			{
				// diffuse component
				double f = N * L;
				if (f < 0) f = 0;
				Color diffuse = c * f;
				fragCol += diffuse;
			}
		}
		else fragCol = c;

		double opacity = c.a();
		if ((c.a() < 0.99) && (ray.bounce < 100))
		{
			Vec3 p = q.r;
			Ray ray2(p, ray.direction, q.tri_id);
			ray2.bounce = ray.bounce + 1;

			rt::Point q2;
			Color c2 = castRay(bhv, ray2);
			fragCol = fragCol * opacity + c2 * (1 - opacity);
		}

		// add specular component
		if (!isOccluded && (mat.shininess >= 0) && mat.lighting)
		{
			Vec3 H = t - N * (2 * (t * N));
			H.normalize();
			double f = H * L;
			double s = (f > 0 ? pow(f, mat.shininess) : 0);
			fragCol += lightSpecular * mat.specular* s;
		}
		fragCol.a() = c.a();
	}
	fragCol.clamp();
	return fragCol;
}
