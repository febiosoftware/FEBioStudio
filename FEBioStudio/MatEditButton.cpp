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
#include "MatEditButton.h"
#include <GLLib/GLContext.h>
#include "DlgEditMaterial.h"
#include <RTLib/RayTracer.h>
#include <RTLib/RayTraceSurface.h>
#include <QPainter>

class CMatEditButton::Imp
{
public:
	GLMaterial	mat;
	QImage img;
	EditMaterialScene scene;
	RayTracer rt;

	Imp()
	{
#ifndef NDEBUG
		scene.res = 16;
#else
		scene.res = 25;
#endif

	}
};

CMatEditButton::CMatEditButton(QWidget* parent) : QFrame(parent), m(*(new Imp))
{
	setFrameShape(QFrame::NoFrame);
	setMinimumSize(sizeHint());
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

CMatEditButton::~CMatEditButton()
{
	delete& m;
}

GLMaterial CMatEditButton::color() const { return m.mat; }

void CMatEditButton::resizeEvent(QResizeEvent* event)
{
	QRect rect = contentsRect();
	int W = rect.width();
	int H = rect.height();

	m.img = QImage(W, H, QImage::Format_ARGB32);
	m.img.fill(Qt::red);

	updateImage();
}

void CMatEditButton::updateImage()
{
	int W = m.img.width();
	int H = m.img.height();

	m.scene.mat = m.mat;

	m.rt.setWidth(W);
	m.rt.setHeight(H);

#ifndef NDEBUG
	m.rt.setSampleCount(1);
#else
	m.rt.setSampleCount(2);
#endif

#ifdef NDEBUG
	m.rt.setOutput(false);
#endif

	m.rt.setRenderShadows(false);

	GLContext rc;
	rc.m_cam = &m.scene.GetCamera();

	m.rt.start();
	m.scene.Render(m.rt, rc);
	m.rt.finish();

	RayTraceSurface& trg = m.rt.surface();

	for (size_t j = 0; j < H; ++j)
		for (size_t i = 0; i < W; ++i)
		{
			GLColor c = trg.colorValue(i, j);
			QRgb rgb = qRgba(c.r, c.g, c.b, c.a);
			m.img.setPixel((int)i, (int)j, rgb);
		}
}

void CMatEditButton::paintEvent(QPaintEvent* ev)
{
	QFrame::paintEvent(ev);

	QRect rt = contentsRect();
	QPainter p(this);
	p.drawImage(rt.x(), rt.y(), m.img);
	p.end();
}

void CMatEditButton::setMaterial(const GLMaterial& mat)
{
	m.mat = mat;
	updateImage();
	repaint();
}

void CMatEditButton::mouseReleaseEvent(QMouseEvent* ev)
{
	CDlgEditMaterial dlg(this);
	dlg.SetMaterial(m.mat);
	if (dlg.exec())
	{
		m.mat = dlg.GetMaterial();
		updateImage();
		repaint();
		emit materialChanged(m.mat);
	}
}
