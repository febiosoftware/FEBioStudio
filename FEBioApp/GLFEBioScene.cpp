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
#include "GLFEBioScene.h"
#include <QMutexLocker>
#include <GLLib/GLMeshRender.h>
#include <MeshLib/GMesh.h>
#include <FECore/FEMesh.h>
#include <FECore/FESurface.h>
#include <FEBioLib/FEBioModel.h>
#include <PostLib/ColorMap.h>

GLFEBioScene::GLFEBioScene(FEBioModel& fem) : m_fem(fem)
{
	m_febSurface = nullptr;
	m_view = nullptr;
	m_useUserRange = false;
	m_userRange[0] = 0.0;
	m_userRange[1] = 1.0;
	BuildRenderMesh();
	UpdateBoundingBox();
	if (m_box.IsValid())
	{
		double f = m_box.GetMaxExtent();
		if (f == 0) f = 1;
		CGLCamera& cam = GetCamera();
		cam.SetTarget(m_box.Center());
		cam.SetTargetDistance(2.0 * f);
	}
}

GLFEBioScene::~GLFEBioScene()
{
	delete m_febSurface;
	delete m_renderMesh;
}

void GLFEBioScene::SetDataRange(double rngMin, double rngMax)
{
	m_useUserRange = true;
	if (rngMin > rngMax) { double tmp = rngMax; rngMax = rngMin; rngMin = tmp; }
	m_userRange[0] = rngMin;
	m_userRange[1] = rngMax;
}

void GLFEBioScene::SetColorMap(const std::string& colorMapName)
{
	int n = Post::ColorMapManager::FindColorMapFromName(colorMapName);
	assert(n >= 0);
	if (n >= 0) m_col.SetColorMap(n);
}

void GLFEBioScene::Clear()
{
	QMutexLocker lock(&m_mutex);
	if (m_renderMesh == nullptr) return;
	for (int i = 0; i < m_renderMesh->Nodes(); ++i)
	{
		FENode& feNode = m_febSurface->Node(i);
		GMesh::NODE& gnode = m_renderMesh->Node(i);
		gnode.r = feNode.m_r0;
	}
	m_renderMesh->UpdateNormals();
}

void GLFEBioScene::Update(double time)
{
	QMutexLocker lock(&m_mutex);
	if (m_renderMesh == nullptr) return;

	int ndof = -1;
	if (m_dataSource.empty() == false)
	{
		ndof = m_fem.GetDOFIndex(m_dataSource.c_str());
	}

	vector<double> val(m_renderMesh->Nodes(), 0.0);
	for (int i = 0; i < m_renderMesh->Nodes(); ++i)
	{
		FENode& feNode = m_febSurface->Node(i);
		GMesh::NODE& gnode = m_renderMesh->Node(i);
		gnode.r = feNode.m_rt;

		if (ndof >= 0) val[i] = feNode.get(ndof);
	}
	m_renderMesh->UpdateNormals();

	double vmin(0.0), vmax(0.0);
	if (m_useUserRange)
	{
		vmin = m_userRange[0];
		vmax = m_userRange[1];
	}
	else
	{
		auto rng = std::minmax_element(begin(val), end(val));
		vmin = *rng.first;
		vmax = *rng.second;
	}
	if (vmax == vmin) vmax++;

	Post::CColorMap col;
	col.SetRange(vmin, vmax);

	m_glmesh.Create(m_renderMesh->Faces(), GLMesh::FLAG_NORMAL | GLMesh::FLAG_TEXTURE);
	m_glmesh.BeginMesh();
	for (int i = 0; i < m_renderMesh->Faces(); ++i)
	{
		GMesh::FACE& f = m_renderMesh->Face(i);
		for (int j = 0; j < 3; ++j)
		{
			GMesh::NODE& node = m_renderMesh->Node(f.n[j]);
			double t = (val[f.n[j]] - vmin) / (vmax - vmin);
			if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;
			GLMesh::Vertex v{ to_vec3f(node.r), to_vec3f(f.nn[j]), vec3f(t,0.f,0.f), GLColor::White() };
			m_glmesh.AddVertex(v);
		}
	}
	m_glmesh.EndMesh();
	UpdateBoundingBox();
}

void GLFEBioScene::Render(CGLContext& rc)
{
	QMutexLocker lock(&m_mutex);
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_1D);
	glEnable(GL_COLOR_MATERIAL);
	glColor3ub(255, 255, 255);
	m_col.GetTexture().MakeCurrent();
	m_glmesh.Render();
	glPopAttrib();
}

void GLFEBioScene::BuildRenderMesh()
{
	FEMesh& febMesh = m_fem.GetMesh();
	FESurface* surf = febMesh.ElementBoundarySurface();
	m_febSurface = surf;

	int NN = surf->Nodes();

	// we need number of triangles, so count them
	int NF = 0;
	int NE = surf->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FESurfaceElement& el = surf->Element(i);
		if (el.Nodes() == 3) NF += 1;
		if (el.Nodes() == 4) NF += 2;
	}

	GMesh* mesh = new GMesh;
	mesh->Create(NN, NF);

	for (int i = 0; i < NN; ++i)
	{
		GMesh::NODE& gnode = mesh->Node(i);
		FENode& fenode = surf->Node(i);
		gnode.r = fenode.m_r0;
	}

	NF = 0;
	for (int i = 0; i < NE; ++i)
	{
		FESurfaceElement& el = surf->Element(i);
		GMesh::FACE& f1 = mesh->Face(NF++);
		f1.n[0] = el.m_lnode[0];
		f1.n[1] = el.m_lnode[1];
		f1.n[2] = el.m_lnode[2];

		if (el.Nodes() == 4)
		{
			GMesh::FACE& f2 = mesh->Face(NF++);
			f2.n[0] = el.m_lnode[2];
			f2.n[1] = el.m_lnode[3];
			f2.n[2] = el.m_lnode[0];
		}
	}
	mesh->Update();
	mesh->AutoSmooth(60.0);

	m_renderMesh = mesh;

	m_glmesh.CreateFromGMesh(*mesh);
}

BOX GLFEBioScene::GetBoundingBox()
{
	return m_box;
}

void GLFEBioScene::UpdateBoundingBox()
{
	BOX box;
	if (m_renderMesh)
	{
		for (int i = 0; i < m_renderMesh->Nodes(); ++i) box += m_renderMesh->Node(i).r;
	}
	m_box = box;
}
