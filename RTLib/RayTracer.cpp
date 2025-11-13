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
#include "rasterize.h"

using namespace rt;
using namespace gl;

#include <chrono>
using namespace std::chrono;
using dseconds = duration<double>;

void rt::meshGeometry::start()
{
	mesh.clear();
	bhv.clear();
}

void rt::meshGeometry::finish()
{
	size_t triangles = mesh.triangles();
	for (size_t i = 0; i < triangles; ++i) mesh.triangle(i).id = (int)i;

	int levels = bhv_levels;
	if (levels < 0)
	{
		levels = (int)log2((double)triangles);
	}
	if (levels < 0) levels = 0;
	if (levels > 20) levels = 20;

	bhv.output = output;
	bhv.Build(mesh, levels);
}

bool rt::meshGeometry::intersect(const Ray& ray, rt::Point& q)
{
	return bhv.intersect(ray, q);
}

bool rt::sphere::intersect(const Ray& ray, rt::Point& q)
{
	Vec3 z = ray.origin - o;
	Vec3 t = ray.direction;
	double a = t*t;
	double b = 2.0 * (t*z);
	double c = z*z - r*r;

	double D2 = b * b - 4.0*a * c;
	if (D2 >= 0)
	{
		double D = sqrt(D2);
		double l1 = (-b - D) / (2.0 * a);
		double l2 = (-b + D) / (2.0 * a);

		if ((l1 > 0) && ((l2 <= 0) || (l1 < l2)))
		{
			q.r = ray.origin + t * l1;
		}
		else if ((l2 > 0) && ((l1 <= 0) || (l2 < l1)))
		{
			q.r = ray.origin + t * l2;
		}
		else return false;

		q.n = q.r - o;
		q.n.normalize();
		q.c = col;
		q.matid = matid;

		return true;
	}
	else return false;
}

void rt::Geometry::finish()
{
	for (auto it : geom) it->finish();
}

bool rt::Geometry::intersect(const Ray& rt, rt::Point& q)
{
	if (geom.empty()) return false;
	if (geom.size() == 1) return geom[0]->intersect(rt, q);

	bool intersected = false;
	double Dmin = 0;
	for (auto g : geom)
	{
		rt::Point p;
		bool b = g->intersect(rt, p);
		if (b)
		{
			double D = (p.r - rt.origin) * rt.direction;
			if (D > 0)
			{
				if (intersected == false)
				{
					q = p;
					intersected = true;
					Dmin = D;
				}
				else if (D < Dmin)
				{
					q = p;
					Dmin = D;
				}
			}
		}
	}
	return intersected;
}

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

	geom.clear();
}

void RayTracer::setProjection(double fov, double fnear, double far)
{
	fieldOfView = fov;
	nearPlane = fnear;
	farPlane = far;
	ortho = false;

	double ar = (double)surfaceWidth() / (double)surfaceHeight();

	projMatrix.perspective(fov, ar, fnear, far);
}

void RayTracer::setOrthoProjection(double left, double right, double bottom, double top, double near, double far)
{
	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;
	nearPlane = near;
	farPlane = far;
	ortho = true;

	projMatrix.ortho(left, right, bottom, top, near, far);
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
	geom.clear();

	// add the meshed geometry
	meshGeometry* mg = new meshGeometry(mesh);
	int levels = -1;
#ifndef NDEBUG
	levels = GetIntValue(BHV_LEVELS);
#endif
	mg->setBHVLevels(levels);
	mg->setOutput(output);
	geom.push_back(mg);

	projMatrix.makeIdentity();
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
	geom.clear();
	GLRenderEngine::finish();
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
	GLRenderEngine::setColor(c);
}

void RayTracer::setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
{
	currentColor = c;
	useVertexColor = (map == GLMaterial::VERTEX_COLOR);
	useTexture1D = (map == GLMaterial::TEXTURE_1D);
	useTexture3D = (map == GLMaterial::TEXTURE_3D);

	rt::Material mat;

	if ((matType == GLMaterial::HIGHLIGHT) || (matType == GLMaterial::CONSTANT))
	{
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
	lightPos = currentTransform()* r;
}

void RayTracer::setLightSpecularColor(unsigned int lightIndex, const GLColor& col)
{
	lightSpecular = Color(col);
}

void RayTracer::setLightAmbientColor(unsigned int lightIndex, const GLColor& col)
{
	lightAmbient = Color(col);
}

void RayTracer::setLightDiffuseColor(unsigned int lightIndex, const GLColor& col)
{
	lightDiffuse = Color(col);
}

void RayTracer::setLightEnabled(unsigned int lightIndex, bool b)
{
	lightEnabled = b;
}

void RayTracer::renderGMesh(const GLMesh& gmesh, bool cacheMesh)
{
	gl::Matrix4 mv = currentTransform();
	int NF = gmesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		const GLMesh::FACE& face = gmesh.Face(i);
		rt::Tri tri;
		if (front == GLRenderEngine::COUNTER_CLOCKWISE)
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[j] = mv * Vec4(face.vr[j], 1);
				tri.n[j] = mv * Vec4(face.vn[j], 0); tri.n[j].normalize();
				tri.t[j] = Vec3(face.t[j]);
				tri.c[j] = (useVertexColor ? face.c[j] : currentColor);
			}
		}
		else
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[2-j] = mv * Vec4(face.vr[j], 1);
				tri.n[2-j] = mv * Vec4(face.vn[j], 0); tri.n[j].normalize();
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
	gl::Matrix4 mv = currentTransform();

	const GLMesh::PARTITION& p = gmesh.Partition(surfId);
	for (int i = 0; i < p.nf; ++i)
	{
		const GLMesh::FACE& face = gmesh.Face(p.n0 + i);
		rt::Tri tri;
		if (front == GLRenderEngine::COUNTER_CLOCKWISE)
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[j] = mv * Vec4(face.vr[j], 1);
				tri.n[j] = mv * Vec4(face.vn[j], 0); tri.n[j].normalize();
				tri.t[j] = Vec3(face.t[j]);
				tri.c[j] = (useVertexColor ? face.c[j] : currentColor);
			}
		}
		else
		{
			for (int j = 0; j < 3; ++j)
			{
				tri.r[2-j] = mv * Vec4(face.vr[j], 1);
				tri.n[2-j] = mv * Vec4(face.vn[j], 0); tri.n[j].normalize();
				tri.t[2-j] = Vec3(face.t[j]);
				tri.c[2-j] = (useVertexColor ? face.c[j] : currentColor);
			}
		}
		tri.matid = currentMaterial;
		addTriangle(tri);
	}
}

void RayTracer::renderGMeshEdges(const GLMesh& mesh, bool cacheMesh)
{
	gl::Matrix4 mv = currentTransform();
	int NE = mesh.Edges();
	for (int i = 0; i < NE; ++i)
	{
		const GLMesh::EDGE& edge = mesh.Edge(i);
		rt::Line line;
		for (int j = 0; j < 2; ++j)
		{
			line.r[j] = mv * Vec4(edge.vr[j], 1);
			line.c[j] = (useVertexColor ? edge.c[j] : currentColor);
		}
		addLine(line);
	}
}

void RayTracer::renderGMeshEdges(const GLMesh& mesh, int partition, bool cacheMesh)
{
	if ((partition < 0) || (partition >= mesh.EILs())) return;

	gl::Matrix4 mv = currentTransform();
	const std::pair<int, int> p = mesh.EIL(partition);
	int NE = p.second;
	for (int i = 0; i < NE; ++i)
	{
		const GLMesh::EDGE& edge = mesh.Edge(i + p.first);
		rt::Line line;
		for (int j = 0; j < 2; ++j)
		{
			line.r[j] = mv * Vec4(edge.vr[j], 1);
			line.c[j] = (useVertexColor ? edge.c[j] : currentColor);
		}
		addLine(line);
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


void RayTracer::addLine(rt::Line& line)
{
	// make sure the line is in front of the near plane
	if ((line.r[0].z() >= -nearPlane) && (line.r[1].z() >= -nearPlane)) return;

	// clip to the near plane
	if ((line.r[0].z() > -nearPlane) || (line.r[1].z() > -nearPlane))
	{
		Point p[3];
		p[0] = line.point(0);
		p[1] = line.point(1);
		Vec3 a = line.r[0];
		Vec3 b = line.r[1];
		Vec3 t = b - a;
		double D = t.z();
		if (D != 0)
		{
			double l = (-nearPlane - a.z()) / D;
			if (l < 0) l = 0;
			else if (l > 1) l = 1;
			p[2] = interpolate(p[0], p[1], l);
			if (line.r[0].z() > -nearPlane)
			{
				line.r[0] = p[2].r;
				line.c[0] = p[2].c;
			}
			else
			{
				line.r[1] = p[2].r;
				line.c[1] = p[2].c;
			}
		}
	}

	// process triangle
	if (!line.process()) return;

	// TODO: do frustum clipping

	// process clip plane
	if (useClipPlane)
	{
		double* c = clipPlane;
		Vec3 N(c[0], c[1], c[2]);
		Vec3* v = line.r;
		unsigned int ncase = 0;
		for (int i = 0; i < 2; ++i)
		{
			double l = N * v[i] + c[3];
			if (l < 0) ncase |= (1 << i);
		}

		if (ncase == 0) mesh.addLine(line);
		else if (ncase < 2)
		{
			Point p[3];
			p[0] = line.point(0);
			p[1] = line.point(1);

			Vec3 a = v[0];
			Vec3 b = v[1];
			Vec3 t = b - a;
			double D = N * t;
			if (D != 0)
			{
				double l = -(c[3] + N * a) / D;
				if (l < 0) l = 0;
				else if (l > 1) l = 1;
				p[2] = interpolate(p[0], p[1], l);
			}

			rt::Line line_n;
			line_n.r[0] = p[0].r;
			line_n.r[1] = p[2].r;
			line_n.c[0] = p[0].c;
			line_n.c[1] = p[2].c;
			if (line_n.process())
				mesh.addLine(line_n);
		}
	}
	else mesh.addLine(line);
}

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
		gl::Matrix4 mv = currentTransform();
		Vec4 Np = mv * N;
		double v3 = v[3] - (mv[0][3] * Np[0] + mv[1][3] * Np[1] + mv[2][3] * Np[2]);

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
	geom.finish();
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

	if (use_multithread)
	{

#pragma omp parallel
		for (size_t j = 0; j < H; ++j)
		{
#pragma omp master
			percentCompleted = (100.0 * j) / (double)H;

#pragma omp for nowait schedule(dynamic, 5)
			for (int i = 0; i < W; ++i)
			{
				if (!cancelled)
				{
					rt::Fragment f = fragment(i, j, samples);

					float* v = surf.value(i, j);
					v[0] = (float)f.color.r();
					v[1] = (float)f.color.g();
					v[2] = (float)f.color.b();
					v[3] = (bgOption ? (float)f.color.a() : 1.f);
					v[4] = f.depth;
				}
			}
		}
	}
	else
	{
		for (size_t j = 0; j < H; ++j)
		{
			percentCompleted = (100.0 * j) / (double)H;
			for (int i = 0; i < W; ++i)
			{
				if (!cancelled)
				{
					rt::Fragment f = fragment(i, j, samples);

					float* v = surf.value(i, j);
					v[0] = (float)f.color.r();
					v[1] = (float)f.color.g();
					v[2] = (float)f.color.b();
					v[3] = (bgOption ? (float)f.color.a() : 1.f);
					v[4] = f.depth;
				}
			}
		}
	}

	percentCompleted = 100.0;

	// render the lines
	renderLines();
}

rt::Fragment RayTracer::fragment(int i, int j, int samples)
{
	int W = surf.width();
	int H = surf.height();

	double x = -m_fw + 2.0 * i * m_fw / (W - 1.0);
	double y = m_fh - 2.0 * j * m_fh / (H - 1.0);
	double z = -nearPlane;

	double dx = m_fw / W;
	double dy = m_fh / H;

	Color c(0, 0, 0, 0);
	float depth = 0.f;
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

			Fragment fragCol = castRay(ray);

			c += fragCol.color;
			depth += fragCol.depth;
		}
	c /= (double)(samples * samples);
	depth /= (float)(samples * samples);

	depth = projMatrix.depthToNDC(depth, nearPlane, farPlane);

	return Fragment{ c, depth };
}

GLColor RayTracer::backgroundColor(const Vec3& p)
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

bool RayTracer::intersect(const Ray& ray, rt::Point& q)
{
	return geom.intersect(ray, q);
}

rt::Fragment RayTracer::castRay(Ray& ray)
{
	rt::Point q;

	int bgOption = GetIntValue(BACKGROUND);

	Fragment frag;
	frag.depth = -farPlane;
	switch (bgOption)
	{
	case 0: // default
		if (ray.bounce == 0)
			frag.color = backgroundColor(ray.origin);
		else
			frag.color = backgroundCol;
		break;
	case 1: // transparent
		frag.color = Color(0, 0, 0, 0);
		break;
	}
	bool intersectMesh = intersect(ray, q);
	bool renderShadows = GetBoolValue(SHADOWS);

	if (intersectMesh)
	{
		// set depth value
		frag.depth = q.r.z();

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
			// environment map
			if ((mat.reflection > 0) && (!envTex.isNull()))
			{
				double r = mat.reflection;
				Vec3 N = q.n;

				Vec3 R = gl::reflect(t, N);

				float u = atan2(R.z(), R.x()) / (2.0 * PI) + 0.5;
				float v = 0.5 - asin(R.y()) / PI;

				Color envCol = envTex.sample(u, v);
				c = envCol * r + c * (1 - r);
			}

			// reflection
/*			if ((mat.reflection > 0) && (ray.bounce < 2))
			{
				Vec3 H = t - N * (2 * (t * N));
				H.normalize();
				Ray ray2(q.r, H);
				ray2.bounce = ray.bounce + 1;
				c = c * (1 - mat.reflection) + castRay(bhv, ray2) * mat.reflection;
			}
*/
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
		if (mat.lighting && lightEnabled)
		{
			// calculate an ambient value
			frag.color = mat.ambient * lightAmbient;
			double f = N * Vec3(0, 0, 1);
			if (f < 0) f = 0;
			frag.color += c * (f * 0.2);

			// see if the point is occluded or not
			if (renderShadows)
			{
				Vec3 p = q.r;
				Ray ray2(p, L);
				ray2.bounce = ray.bounce + 1;
				rt::Point q2;
				if (intersect(ray2, q2)) isOccluded = true;
			}

			if (!isOccluded)
			{
				// diffuse component
				double f = N * L;
				if (f < 0) f = 0;
				Color diffuse = c * f;
				frag.color += diffuse*lightDiffuse;
			}
		}
		else frag.color = c;

		double opacity = c.a();
		if ((c.a() < 0.99) && (ray.bounce < 100))
		{
			Vec3 p = q.r;
			Ray ray2(p, ray.direction);
			ray2.bounce = ray.bounce + 1;

			rt::Point q2;
			Fragment f2 = castRay(ray2);
			frag.color = frag.color * opacity + f2.color * (1 - opacity);
		}

		// add specular component
		if (!isOccluded && (mat.shininess >= 0) && mat.lighting)
		{
			Vec3 H = t - N * (2 * (t * N));
			H.normalize();
			double f = H * L;
			double s = (f > 0 ? pow(f, mat.shininess) : 0);
			frag.color += lightSpecular * mat.specular* s;
		}
		frag.color.a() = c.a();
	}
	frag.color.clamp();

	return frag;
}

unsigned int RayTracer::SetEnvironmentMap(const CRGBAImage& img)
{
	envTex.setImageData(img);
	return 1;
}

void RayTracer::ActivateEnvironmentMap(unsigned int id)
{

}

void RayTracer::DeactivateEnvironmentMap(unsigned int id)
{

}

void RayTracer::addSphere(const vec3d& c, double R)
{
	gl::Matrix4 mv = currentTransform();
	Vec4 p = mv*Vec4(c.x, c.y, c.z, 1);
	Vec3 o(p.x(), p.y(), p.z());
	rt::sphere* S = new rt::sphere(o, R);
	S->col = currentColor;
	S->matid = currentMaterial;
	geom.push_back(S);
}

Vec3 RayTracer::toNDC(Vec4 v)
{
	// device coordinates
	v = projMatrix * v;

	// normalized device coordinates
	v /= v.w();

	return Vec3(v.x(), v.y(), v.z());
}

Vec3 RayTracer::NDCtoView(const Vec3& v)
{
	// viewport transform
	Vec3 p;
	int W = surfaceWidth();
	int H = surfaceHeight();
	p.x( (v[0] + 1) * 0.5 * W );
	p.y( (v[1] + 1) * 0.5 * H );
	p.z(v[2]);

	return p;
}

void RayTracer::renderLines()
{
	int W = surfaceWidth();
	int H = surfaceHeight();

	percentCompleted = 0;

	for (int i = 0; i < mesh.lines(); ++i)
	{
		if (cancelled) break;

		percentCompleted = (100.0 * i) / (double)mesh.lines();

		rt::Line& line = mesh.line(i);

		// convert to device coordinates
		Vec4 a = Vec4(line.r[0], 1);
		Vec4 b = Vec4(line.r[1], 1);

		// convert to normalized device coordinates
		Vec3 a_ndc = toNDC(a);
		Vec3 b_ndc = toNDC(b);

		bool isInside = clipLine(a_ndc, b_ndc);

		if (isInside)
		{
			Vec3 an = NDCtoView(a_ndc);
			Vec3 bn = NDCtoView(b_ndc);

			// convert to view coordinates
			int x0 = (int)an.x(); int y0 = (int)an.y();
			int x1 = (int)bn.x(); int y1 = (int)bn.y();

			Color col0 = line.c[0];
			Color col1 = line.c[1];

			float dz = 0.0002f;

			bool swapped = false;
			std::vector<rt::Pixel> pts = rt::rasterizeLineAA(x0, y0, x1, y1, swapped);
			int n = 0;
			for (auto& p : pts)
			{
				int x = p.x;
				int y = H - p.y - 1;

				double w = (pts.size() > 1 ? (double)n / (double)(pts.size() - 1) : 0.0);

				if ((x >= 0) && (x < W) && (y >= 0) && (y < H))
				{
					Color c;

					double z = 0;
					if (swapped)
					{
						c = col1 * (1.0 - w) + col0 * w;
						z = bn.z() * (1.0 - w) + an.z() * w;
					}
					else
					{
						c = col0 * (1.0 - w) + col1 * w;
						z = an.z() * (1.0 - w) + bn.z() * w;
					}

					float* v = surf.value(x, y);
					if ((z < 1) && (z > -1) && (z <= v[4] + dz))
					{
						float a = c.a() * p.brightness;

						v[0] = c.r() * a + v[0] * (1.f - a);
						v[1] = c.g() * a + v[1] * (1.f - a);
						v[2] = c.b() * a + v[2] * (1.f - a);
						v[3] = a + v[3] * (1.f - a);
					}
				}

				n++;
			}
		}
	}

	percentCompleted = 100.0;
}

bool RayTracer::clipLine(Vec3& a, Vec3& b)
{
    // NDC cube bounds
    const double xmin = -1, xmax = 1;
    const double ymin = -1, ymax = 1;
    const double zmin = -1, zmax = 1;

    double t0 = 0.0, t1 = 1.0;
    Vec3 d = b - a;

    auto clip = [&](double p, double q, double& t0, double& t1) -> bool {
        if (p == 0) return q >= 0; // Parallel, inside if q >= 0
        double r = q / p;
        if (p < 0) {
            if (r > t1) return false;
            if (r > t0) t0 = r;
        } else {
            if (r < t0) return false;
            if (r < t1) t1 = r;
        }
        return true;
    };

    // Left, Right
    if (!clip(-d.x(), a.x() - xmin, t0, t1)) return false;
    if (!clip( d.x(), xmax - a.x(), t0, t1)) return false;
    // Bottom, Top
    if (!clip(-d.y(), a.y() - ymin, t0, t1)) return false;
    if (!clip( d.y(), ymax - a.y(), t0, t1)) return false;
    // Near, Far
    if (!clip(-d.z(), a.z() - zmin, t0, t1)) return false;
    if (!clip( d.z(), zmax - a.z(), t0, t1)) return false;

    if (t1 < t0) return false;

    Vec3 new_a = a + d * t0;
    Vec3 new_b = a + d * t1;
    a = new_a;
    b = new_b;
    return true;
}
