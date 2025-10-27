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
#include "GLMeshBuilder.h"

GLMeshBuilder::GLMeshBuilder()
{
	m_pm = nullptr;
	vertList.reserve(1024);
}

GLMeshBuilder::~GLMeshBuilder()
{
	if (m_pm) delete m_pm;
}

void GLMeshBuilder::start()
{
	beginShape();
}

void GLMeshBuilder::finish()
{
	if (m_pm) endShape();
}

void GLMeshBuilder::beginShape()
{
	if (m_pm) delete m_pm;
	m_pm = new GLMesh();

	modelView.makeIdentity();
	isMVIdentity = true;
	while (!mvStack.empty()) mvStack.pop();
}

void GLMeshBuilder::endShape()
{
	if (m_pm) m_pm->UpdateBoundingBox();
}

GLMesh* GLMeshBuilder::takeMesh()
{
	GLMesh* pm = m_pm;
	m_pm = nullptr;
	return pm;
}

void GLMeshBuilder::pushTransform()
{
	mvStack.push(modelView);
}

void GLMeshBuilder::popTransform()
{
	assert(!mvStack.empty());
	modelView = mvStack.top();
	mvStack.pop();
}

void GLMeshBuilder::translate(const vec3d& r)
{
	mat4d T = mat4d::translate(vec3d(r));
	modelView *= T;
	isMVIdentity = false;
}

void GLMeshBuilder::rotate(const quatd& rot)
{
	mat4d R = mat4d::rotate(rot);
	modelView *= R;
	isMVIdentity = false;
}

void GLMeshBuilder::rotate(double deg, double x, double y, double z)
{
	quatd q(deg * DEG2RAD, vec3d(x, y, z));
	rotate(q);
}

void GLMeshBuilder::scale(double x, double y, double z)
{
	mat4d S = mat4d::scale(x, y, z);
	modelView *= S;
	isMVIdentity = false;
}

void GLMeshBuilder::setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
{
	currentColor = c;
}

void GLMeshBuilder::vertex(const vec3d& r)
{
	GLMesh::NODE p;
	if (isMVIdentity)
	{
		p.r = to_vec3f(r);
		p.n = to_vec3f(currentNormal);
	}
	else
	{
		vec4d q = modelView * vec4d(r);
		vec4d N = modelView * vec4d(currentNormal, 0);
		p.r = vec3f(q[0], q[1], q[2]);
		p.n = vec3f(N[0], N[1], N[2]); p.n.Normalize();
	}
	p.c = currentColor;
	vertList.push_back(p);
}

void GLMeshBuilder::normal(const vec3d& r)
{
	currentNormal = r; currentNormal.Normalize();
}

void GLMeshBuilder::setColor(GLColor c)
{
	currentColor = c;
}

void GLMeshBuilder::begin(PrimitiveType prim)
{
	currentPrimitive = prim;
}

void GLMeshBuilder::end()
{
	switch (currentPrimitive)
	{
	case PrimitiveType::POINTS   : buildPoints(); break;
	case PrimitiveType::LINES    : buildLines(); break;
	case PrimitiveType::LINESTRIP: buildLineStrip(); break;
	case PrimitiveType::LINELOOP : buildLineLoop(); break;
	case PrimitiveType::TRIANGLES: buildTriangles(); break;
	case PrimitiveType::QUADSTRIP: buildQuadStrip(); break;
	default:
		assert(false);
	}

	vertList.clear();
}

void GLMeshBuilder::buildPoints()
{
	assert(currentPrimitive == PrimitiveType::POINTS);
	assert(m_pm);
	if (m_pm == nullptr) return;

	for (int i = 0; i < vertList.size(); ++i)
	{
		m_pm->AddNode(vertList[i]);
	}
}

void GLMeshBuilder::buildLines()
{
	assert(currentPrimitive == PrimitiveType::LINES);
	assert(m_pm);
	if (m_pm == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = m_pm->AddNode(vertList[i]);
	}

	if (N > 1)
	{
		for (int i = 0; i < N; i += 2)
		{
			m_pm->AddEdge(NL[i], NL[i + 1]);
		}
	}
}

void GLMeshBuilder::buildLineStrip()
{
	assert(currentPrimitive == PrimitiveType::LINESTRIP);
	assert(m_pm);
	if (m_pm == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = m_pm->AddNode(vertList[i]);
	}

	if (N > 1)
	{
		for (int i = 0; i < N-1; i++)
		{
			m_pm->AddEdge(NL[i], NL[i + 1]);
		}
	}
}

void GLMeshBuilder::buildLineLoop()
{
	assert(currentPrimitive == PrimitiveType::LINELOOP);
	assert(m_pm);
	if (m_pm == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = m_pm->AddNode(vertList[i]);
	}

	if (N > 1)
	{
		for (int i = 0; i < N; i++)
		{
			int ip1 = (i + 1) % N;
			m_pm->AddEdge(NL[i], NL[ip1]);
		}
	}
}

void GLMeshBuilder::buildTriangles()
{
	assert(currentPrimitive == PrimitiveType::TRIANGLES);
	assert(m_pm);
	if (m_pm == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = m_pm->AddNode(vertList[i]);
	}

	if (N > 2)
	{
		for (int i = 0; i < N; i += 3)
		{
			m_pm->AddFace(NL[i], NL[i+1], NL[i+2]);
		}
	}
}

void GLMeshBuilder::buildQuadStrip()
{
	assert(currentPrimitive == PrimitiveType::QUADSTRIP);
	assert(m_pm);
	if (m_pm == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = m_pm->AddNode(vertList[i]);
	}

	if (N > 3)
	{
		int n0 = NL[0];
		int n1 = NL[1];
		for (int i = 0; i < N-3; i += 2)
		{
			int n2 = NL[i + 2];
			int n3 = NL[i + 3];
			m_pm->AddFace(n0, n1, n2);
			m_pm->AddFace(n1, n3, n2);
			n0 = n2;
			n1 = n3;
		}
	}
}
