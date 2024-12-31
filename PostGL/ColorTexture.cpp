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
#include "ColorTexture.h"
using namespace Post;

CColorTexture::CColorTexture()
{
	m_colorMap = ColorMapManager::GetDefaultMap();
	m_ndivs = 10;
	m_bsmooth = true;

	UpdateTexture();
}

CColorTexture::CColorTexture(const CColorTexture& col)
{
	m_colorMap = col.m_colorMap;
	m_ndivs = col.m_ndivs;
	m_bsmooth = col.m_bsmooth;

	m_tex = col.m_tex;
}

void CColorTexture::operator = (const CColorTexture& col)
{
	m_colorMap = col.m_colorMap;
	m_ndivs = col.m_ndivs;
	m_bsmooth = col.m_bsmooth;

	m_tex = col.m_tex;
}

void CColorTexture::UpdateTexture()
{
	int n = m_tex.Size();
	unsigned char* pb = m_tex.GetBytes();

	int N = (m_bsmooth ? n : m_ndivs);
	if (N < 2) N = 2;

	// make sure the color map points to an existing map
	if ((m_colorMap < 0) || (m_colorMap >= ColorMapManager::ColorMaps())) m_colorMap = ColorMapManager::GetDefaultMap();
	CColorMap& map = ColorMapManager::GetColorMap(m_colorMap);

	GLColor c;
	for (int i = 0; i < n; i++, pb += 3)
	{
		float f = (float)(i * N / n);

		c = map.map(f / (N - 1));

		pb[0] = c.r;
		pb[1] = c.g;
		pb[2] = c.b;
	}

	m_tex.Update();
}

int CColorTexture::GetDivisions() const
{
	return m_ndivs;
}

void CColorTexture::SetDivisions(int n)
{
	if (n != m_ndivs)
	{
		m_ndivs = n;
		UpdateTexture();
	}
}

bool CColorTexture::GetSmooth() const
{
	return m_bsmooth;
}

void CColorTexture::SetSmooth(bool b)
{
	if (b != m_bsmooth)
	{
		m_bsmooth = b;
		UpdateTexture();
	}
}

void CColorTexture::SetColorMap(int n)
{
	if (n != m_colorMap)
	{
		m_colorMap = n;
		UpdateTexture();
	}
}

int CColorTexture::GetColorMap() const
{
	return m_colorMap;
}

CColorMap& CColorTexture::ColorMap()
{
	return ColorMapManager::GetColorMap(m_colorMap);
}
