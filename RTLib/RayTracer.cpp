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
using namespace rt;

RayTracer::RayTracer(RayTraceSurface& trg) : surf(trg)
{
	fieldOfView = 60.0;
	nearPlane = 0.01;
	percentCompleted = 0.0;
	useVertexColor = false;
}

void RayTracer::setupProjection(double fov, double fnear)
{
	fieldOfView = fov;
	nearPlane = fnear;
}

void RayTracer::setMultiSample(int ms)
{
	if (ms < 1) ms = 1;
	if (ms > 4) ms = 4;
	multiSample = ms;
}

void RayTracer::start()
{
	GLRenderEngine::start();
	mesh.clear();
	modelView.makeIdentity();
}

double RayTracer::progress()
{
	return percentCompleted;
}

void RayTracer::finish()
{
	// let's get to work!
	render();

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

void RayTracer::positionCamera(const GLCamera& cam)
{
	// reset the modelview matrix mode
	modelView.makeIdentity();

	// target in camera coordinates
	vec3d r = cam.Target();

	// position the target in camera coordinates
	translate(-r);

	// orient the camera
	rotate(cam.m_rot.Value());

	// translate to world coordinates
	translate(-cam.GetPosition());
}

void RayTracer::setColor(GLColor c)
{
	currentColor = c;
}

void RayTracer::setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
{
	currentColor = c;
	useVertexColor = (map == GLMaterial::VERTEX_COLOR);
}

void RayTracer::setLightPosition(unsigned int lightIndex, const vec3f& p)
{
	Vec4 r(p, 0);
	lightPos = modelView * r;
}

void RayTracer::renderGMesh(const GLMesh& gmesh, bool cacheMesh)
{
	int NF = gmesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		const GLMesh::FACE& face = gmesh.Face(i);
		rt::Tri tri;
		for (int j = 0; j < 3; ++j)
		{
			tri.r[j] = modelView * Vec4(face.vr[j], 1);
			tri.n[j] = modelView * Vec4(face.vn[j], 0); tri.n[j].normalize();
			tri.t[j] = Vec3(face.t[j]);
			tri.c[j] = (useVertexColor ? face.c[j] : currentColor);
		}
		mesh.addTri(tri);
	}
}

void RayTracer::renderGMesh(const GLMesh& gmesh, int surfId, bool cacheMesh)
{
	const GLMesh::PARTITION& p = gmesh.Partition(surfId);
	for (int i = 0; i < p.nf; ++i)
	{
		const GLMesh::FACE& face = gmesh.Face(p.n0 + i);
		rt::Tri tri;
		for (int j = 0; j < 3; ++j)
		{
			tri.r[j] = modelView * Vec4(face.vr[j], 1);
			tri.n[j] = modelView * Vec4(face.vn[j], 0); tri.n[j].normalize();
			tri.t[j] = Vec3(face.t[j]);
			tri.c[j] = (useVertexColor ? face.c[j] : currentColor);
		}
		mesh.addTri(tri);
	}
}

void RayTracer::render()
{
	// start ray-tracing process
	size_t W = surf.width();
	size_t H = surf.height();
	if ((W == 0) || (H == 0)) return;

	double ar = (double)W / (double)H;

	double fw = nearPlane * tan(0.5 * fieldOfView * DEG2RAD);
	double fh = fw / ar;

	rt::Box box;
	for (size_t i = 0; i < mesh.triangles(); ++i)
	{
		rt::Tri& tri = mesh.triangle(i);
		box += tri.r[0];
		box += tri.r[1];
		box += tri.r[2];
	}

	int samples = multiSample;
	if (samples < 1) samples = 1;
	if (samples > 4) samples = 4;

	percentCompleted = 0;
#pragma omp parallel
	for (size_t j = 0; j < H; ++j)
	{
#pragma omp critical
		percentCompleted = 100.0 * j / H;

#pragma omp for nowait schedule(dynamic)
		for (int i = 0; i < W; ++i)
		{
			double x = -fw + 2.0 * i * fw / (W - 1.0);
			double y =  fh - 2.0 * j * fh / (H - 1.0);
			double z = -nearPlane;

			double dx = fw / W;
			double dy = fh / H;

			Color c(0, 0, 0, 0);
			for (int k=0; k< samples; ++k)
				for (int l = 0; l < samples; ++l)
				{
					double fx = (2.0 * k + 1 - samples) / (double)samples;
					double fy = (2.0 * l + 1 - samples) / (double)samples;

					double xf = x + fx*dx;
					double yf = y + fy*dy;

					Vec3 origin(xf, yf, z);
					Vec3 direction = origin; direction.normalize();

					Ray ray(origin, direction);

					Color fragCol;
					if (box.intersect(ray))
					{
						fragCol = castRay(ray);
					}

					c += fragCol;
				}
			c /= (double)(samples * samples);

			float* v = surf.value(i, j);
			v[0] = (float)c.r();
			v[1] = (float)c.g();
			v[2] = (float)c.b();
			v[3] = (float)c.a();
		}
	}
	percentCompleted = 100.0;
}

rt::Color RayTracer::castRay(rt::Ray& ray)
{
	rt::Point q;
	Color fragCol(0,0,0,0);
	if (intersect(mesh, ray, q))
	{
		Color& c = q.c;

		Vec3& t = ray.direction;
		Vec3& N = q.n;
		Vec3 L(lightPos);

		// diffuse component
		double f = N * L;
		if (f < 0) f = 0;
		Color diffuse = c * f;

		// ambient
		Color ambient = c * 0.1;

		// specular component
		Vec3 H = t - N * (2 * (t * N));
		f = H * L;
		double s = (f > 0 ? pow(f, 32) : 0);
		Color spec = Color(s, s, s);

		fragCol = diffuse + spec + ambient;
		fragCol.a(c.a());

		fragCol.clamp();
	}
	return fragCol;
}
