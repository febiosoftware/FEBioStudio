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
#include "stdafx.h"
#include <PostLib/FEPostModel.h>
#include "GLPlane.h"
#include <GLLib/GLRenderEngine.h>
using namespace Post;

CGLPlane::CGLPlane(FEPostModel* pm)
{
	m_pfem = pm;
	m_e[2] = vec3d(0,0,1);
}

CGLPlane::~CGLPlane(void)
{
}

void CGLPlane::Create(int n[3])
{
	FSMeshBase* pm = m_pfem->GetFEMesh(0);
	if (pm && (n[0] > 0) && (n[1] > 0) && (n[2] > 0))
	{
		FSNode& n1 = pm->Node(n[0]-1);
		FSNode& n2 = pm->Node(n[1]-1);
		FSNode& n3 = pm->Node(n[2]-1);

		vec3d r1 = n1.r;
		vec3d r2 = n2.r;
		vec3d r3 = n3.r;

		m_rc = (r1 + r2 + r3)/3.0;

		BOX box = m_pfem->GetBoundingBox();
		double R = box.Radius();

		m_e[0] = r1 - r2;
		m_e[1] = r3 - r2;
		m_e[2] = m_e[0] ^ m_e[1]; 
		m_e[1] = m_e[2] ^ m_e[0];

		m_e[0].Normalize();
		m_e[1].Normalize();
		m_e[2].Normalize();
	}
}

void CGLPlane::Render(GLRenderEngine& re, GLContext& rc)
{
	FSMeshBase* pm = m_pfem->GetFEMesh(0);

	re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::NONE, false);

	double R = m_pfem->GetBoundingBox().Radius();

	re.pushTransform();
	re.translate(m_rc);
	quatd q(vec3d(0,0,1), m_e[2]);
	re.rotate(q);

	re.setColor(GLColor(255, 0, 0, 128));
	re.normal(vec3d(0,0,1));
	re.begin(GLRenderEngine::QUADS);
	{
		re.vertex(vec3d(-R, -R, 0)); 
		re.vertex(vec3d( R, -R, 0)); 
		re.vertex(vec3d( R,  R, 0)); 
		re.vertex(vec3d(-R,  R, 0)); 
	}
	re.end();

	re.popTransform();
}
