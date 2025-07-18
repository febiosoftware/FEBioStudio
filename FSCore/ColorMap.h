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
#include "color.h"

class CColorMap  
{
public:
	enum { MAX_MAP_COLORS = 9 };

public:
	CColorMap();
	CColorMap(const CColorMap& map);
	virtual ~CColorMap();

	void operator = (const CColorMap& map);

	void jet    ();
	void gray   ();
	void autumn ();
	void winter ();
	void spring ();
	void summer ();
	void red    ();
	void redgreen();
	void green  ();
	void hotcold();
	void blue   ();
	void rbb    ();
	void fire   ();
	void parula ();

	GLColor map(float fval) const;

	int Colors() const { return m_ncol; }
	void SetColors(int n) { m_ncol = n; }

	GLColor GetColor(int i) const { return m_col[i]; }
	void SetColor(int i, GLColor c) { m_col[i] = c; }

	float GetColorPos(int i) const { return m_pos[i]; }
	void SetColorPos(int i, float v) { m_pos[i] = v; }

	void Invert();

	void SetRange(float fmin, float fmax);

protected:
	int		m_ncol;
	GLColor	m_col[MAX_MAP_COLORS];
	float	m_pos[MAX_MAP_COLORS];
	float	m_min, m_max;	// range of color map
};
