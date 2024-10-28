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
#include "GLWidget.h"
#include <PostLib/ColorMap.h>
#include <PostGL/ColorTexture.h>

class GLLegendBar : public GLWidget
{
public:
	enum { GRADIENT, DISCRETE };
	enum { ORIENT_HORIZONTAL, ORIENT_VERTICAL };

public:
	GLLegendBar(Post::CColorTexture* pm, int x, int y, int w, int h, int orientation = ORIENT_VERTICAL);

	void draw(QPainter* painter);

	void SetType(int n) { m_ntype = n; }

	void SetOrientation(int n);
	int Orientation() const { return m_nrot; }

	bool ShowLabels() { return m_blabels; }
	void ShowLabels(bool bshow) { m_blabels = bshow; }

	bool ShowTitle() const { return m_btitle; }
	void ShowTitle(bool b) { m_btitle = b; }

	int GetPrecision() { return m_nprec; }
	void SetPrecision(int n) { n = (n < 1 ? 1 : (n > 7 ? 7 : n)); m_nprec = n; }

	int GetLabelPosition() const { return m_labelPos; }
	void SetLabelPosition(int n) { m_labelPos = n; }

	void SetRange(float fmin, float fmax);
	void GetRange(float& fmin, float& fmax);

	int GetDivisions();
	void SetDivisions(int n);

	bool SmoothTexture() const;
	void SetSmoothTexture(bool b);

	float LineThickness() const { return m_lineWidth; }
	void SetLineThickness(float f) { m_lineWidth = f; }

protected:
	void draw_gradient_vert(QPainter* painter);
	void draw_gradient_horz(QPainter* painter);
	void draw_discrete_vert(QPainter* painter);
	void draw_discrete_horz(QPainter* painter);

	void draw_bg(int x0, int y0, int x1, int y1, QPainter* painter);

protected:
	int		m_ntype;	// type of bar
	int		m_nrot;		// orientation
	bool	m_btitle;	// show title
	bool	m_blabels;	// show labels
	int		m_labelPos;	// label placement
	int		m_nprec;	// precision
	float	m_fmin;		// min of range
	float	m_fmax;		// max of range
	float	m_lineWidth;	// line width

	Post::CColorTexture* m_pMap;
};

