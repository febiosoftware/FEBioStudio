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
#pragma once
#include <QRect>
#include <QPen>
#include <QFont>
#include <QLinearGradient>
#include <FSCore/color.h>
#include <GLLib/GLRenderEngine.h>

class QPainter;

// The GL Painter provides similar functionality to QPainter, but is designed
// to work with a GLRenderEngine. That is, the render engine does the actual rendering
class GLPainter
{
public:
	GLPainter(QPainter* painter, GLRenderEngine* re);

public:
	int deviceWidth() const;
	int deviceHeight() const;
	double devicePixelRatio() const;

	GLRenderEngine* renderEngine() const;

public:
	const QPen& pen() const;
	void setPen(const QPen& pen);
	void setPen(const QColor& col);

	void drawLine(int x1, int y1, int x2, int y2);

	void drawRect(int x, int y, int w, int h);
	void drawRect(const QRect& rt);
	void fillRect(const QRect& rt, const QColor& col);
	void fillRect(int x, int y, int w, int h, const QColor& col);
	void fillRect(const QRect& rt, const QLinearGradient& col);
	void drawRoundedRect(int x, int y, double w, double h, int xrad, int yrad);

	void setFont(const QFont& font);
	QFontMetrics fontMetrics() const;

	void drawText(int x, int y, const QString& txt);
	void drawText(int x, int y, int w, int h, int flags, const QString& txt);

	void drawImage(int x, int y, const QImage& img);

	void beginNativePainting();
	void endNativePainting();

private:
	QPainter* m_painter;
	GLRenderEngine* m_re;
};
