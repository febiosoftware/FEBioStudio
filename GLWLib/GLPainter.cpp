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
#include "GLPainter.h"
#include <QPainter>
#include "convert.h"

GLPainter::GLPainter(QPainter* painter, GLRenderEngine* re) : m_painter(painter), m_re(re) {}

int GLPainter::deviceWidth() const
{
	return m_painter->device()->width();
}

int GLPainter::deviceHeight() const
{
	return m_painter->device()->height();
}

double GLPainter::devicePixelRatio() const
{
	return m_painter->device()->devicePixelRatio();
}

GLRenderEngine* GLPainter::renderEngine() const
{
	return m_re;
}

const QPen& GLPainter::pen() const
{
	return m_painter->pen();
}

void GLPainter::setPen(const QPen& pen)
{
	m_painter->setPen(pen);
}

void GLPainter::setPen(const QColor& col)
{
	m_painter->setPen(col);
}

void GLPainter::drawLine(int x1, int y1, int x2, int y2)
{
	m_painter->drawLine(x1, y1, x2, y2);
}

void GLPainter::drawRect(int x, int y, int w, int h)
{
	m_painter->drawRect(x, y, w, h);
}

void GLPainter::drawRect(const QRect& rt)
{
	m_painter->drawRect(rt);
}

void GLPainter::fillRect(const QRect& rt, const QColor& col)
{
	m_painter->fillRect(rt, col);
}

void GLPainter::fillRect(int x, int y, int w, int h, const QColor& col)
{
	m_painter->fillRect(x, y, w, h, col);
}

void GLPainter::fillRect(const QRect& rt, const QLinearGradient& col)
{
	m_painter->fillRect(rt, col);
}

void GLPainter::drawRoundedRect(int x, int y, double w, double h, int xrad, int yrad)
{
	m_painter->drawRoundedRect(x, y, w, h, xrad, yrad);
}

void GLPainter::setFont(const QFont& font)
{
	m_painter->setFont(font);
}

QFontMetrics GLPainter::fontMetrics() const
{
	return m_painter->fontMetrics();
}

void GLPainter::drawText(int x, int y, const QString& txt)
{
	m_painter->drawText(x, y, txt);
}

void GLPainter::drawText(int x, int y, int w, int h, int flags, const QString& txt)
{
	m_painter->drawText(x, y, w, h, flags, txt);
}

void GLPainter::drawImage(int x, int y, const QImage& img)
{
	m_painter->drawImage(x, y, img);
}

void GLPainter::beginNativePainting()
{
	m_painter->beginNativePainting();
}

void GLPainter::endNativePainting()
{
	m_painter->endNativePainting();
}
