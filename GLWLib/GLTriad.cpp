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
#include "GLTriad.h"
#include <GLLib/glx.h>
#include <GLLib/GLMeshBuilder.h>
#include "convert.h"

GLTriad::GLTriad(int x, int y, int w, int h) : GLWidget(x, y, w, h)
{
	m_rot = quatd(0.f, vec3d(1.f, 0.f, 0.f));
	m_bcoord_labels = true;
}

GLTriad::~GLTriad()
{
	delete m_pm;
}

void GLTriad::buildMesh()
{
	const double r0 = .05;
	const double r1 = .15;

	GLMeshBuilder mb;
	mb.start();

	mb.setMaterial(GLMaterial::PLASTIC, GLColor(255, 0, 0));
	mb.pushTransform();
	mb.rotate(90, 0, 1, 0);
	glx::drawCylinder(mb, r0, .9, 5);
	mb.translate(vec3d(0, 0, .8f));
	glx::drawCone(mb, r1, 0.2, 10);
	mb.popTransform();

	mb.setMaterial(GLMaterial::PLASTIC, GLColor(0, 255, 0));
	mb.pushTransform();
	mb.rotate(-90, 1, 0, 0);
	glx::drawCylinder(mb, r0, .9, 5);
	mb.translate(vec3d(0, 0, .8f));
	glx::drawCone(mb, r1, 0.2, 10);
	mb.popTransform();

	mb.setMaterial(GLMaterial::PLASTIC, GLColor(0, 0, 255));
	mb.pushTransform();
	glx::drawCylinder(mb, r0, .9, 5);
	mb.translate(vec3d(0, 0, .8f));
	glx::drawCone(mb, r1, 0.2, 10);
	mb.popTransform();

	mb.finish();
	m_pm = mb.takeMesh(); assert(m_pm);
}

void GLTriad::draw(GLPainter* painter)
{
	GLWidget::draw(painter);

	GLRenderEngine* re = painter->renderEngine();

	int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	if (re)
	{
		painter->beginNativePainting();

		int oldView[4];
		re->viewport(oldView);

		re->pushState();

		re->setLightPosition(0, vec3f(0, 0, -1));

		// set the new viewport based on widget position and size
		double DPR = painter->devicePixelRatio();

		x0 = (int)(DPR * x());
		y0 = oldView[3] - (int)(DPR * (y() + h()));
		x1 = x0 + (int)(DPR * w());
		y1 = oldView[3] - (int)(DPR * y());
		if (x1 < x0) { x0 ^= x1; x1 ^= x0; x0 ^= x1; }
		if (y1 < y0) { y0 ^= y1; y1 ^= y0; y0 ^= y1; }

		int vp[4] = { x0, y0, x1 - x0, y1 - y0 };
		re->setViewport(vp);

		// set up the projection and modelview matrices
		float ar = 1.f;
		if (h() != 0) ar = fabs((float)w() / (float)h());

		re->pushProjection();
		float d = 1.2f;
		if (ar >= 1.f)	re->setOrthoProjection(-d * ar, d * ar, -d, d, -1, 1); else re->setOrthoProjection(-d, d, -d / ar, d / ar, -1, 1);

		re->pushTransform();
		re->resetTransform();

		re->clearDepthBuffer();

		re->enable(GLRenderEngine::StateFlag::DEPTHTEST);

		re->disable(GLRenderEngine::StateFlag::CULLFACE);
		re->setFrontFace(GLRenderEngine::FrontFace::CLOCKWISE);

		quatd q = m_rot;
		vec3d r = q.GetVector();
		float a = 180 * q.GetAngle() / PI;

		if ((a > 0) && (r.Length() > 0))
			re->rotate(a, r.x, r.y, r.z);

		if (m_pm == nullptr) buildMesh();
		if (m_pm)
		{
			re->setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::VERTEX_COLOR);
			re->renderGMesh(*m_pm);
		}

		// restore project matrix
		re->popProjection();

		// restore modelview matrix
		re->popTransform();

		// restore attributes
		re->popState();

		// restore viewport
		re->setViewport(oldView);

		painter->endNativePainting();

		x0 /= DPR;
		x1 /= DPR;
		y0 = (oldView[3] - y0) / DPR;
		y1 = (oldView[3] - y1) / DPR;
	}
	else
	{
		x0 = x();
		x1 = x() + w();
		y1 = y();
		y0 = y() + h();
	}

	// restore identity matrix
	if (m_bcoord_labels)
	{
		float a = 0.8f;
		vec3d ex(a, 0.f, 0.f);
		vec3d ey(0.f, a, 0.f);
		vec3d ez(0.f, 0.f, a);
		m_rot.RotateVector(ex);
		m_rot.RotateVector(ey);
		m_rot.RotateVector(ez);

		ex.x = x0 + (x1 - x0) * (ex.x + 1) * 0.5; ex.y = y0 + (y1 - y0) * (ex.y + 1) * 0.5;
		ey.x = x0 + (x1 - x0) * (ey.x + 1) * 0.5; ey.y = y0 + (y1 - y0) * (ey.y + 1) * 0.5;
		ez.x = x0 + (x1 - x0) * (ez.x + 1) * 0.5; ez.y = y0 + (y1 - y0) * (ez.y + 1) * 0.5;

		painter->setFont(m_font);
		painter->setPen(toQColor(m_fgc));
		painter->drawText(ex.x, ex.y, "X");
		painter->drawText(ey.x, ey.y, "Y");
		painter->drawText(ez.x, ez.y, "Z");
	}
}
