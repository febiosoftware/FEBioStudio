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
#include "ColorMap.h"
#include <assert.h>
#include <string>
//using namespace std;
using namespace Post;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CColorMap::CColorMap()
{
	m_ncol = 0;
	jet();
	m_min = 0.f;
	m_max = 1.f;
}

CColorMap::CColorMap(const CColorMap& m)
{
	m_ncol = m.m_ncol;
	for (int i=0; i<MAX_MAP_COLORS; ++i)
	{
		m_col[i] = m.m_col[i];
		m_pos[i] = m.m_pos[i];
	}
}

CColorMap::~CColorMap()
{

}

void CColorMap::operator = (const CColorMap& m)
{
	m_ncol = m.m_ncol;
	for (int i=0; i<MAX_MAP_COLORS; ++i)
	{
		m_col[i] = m.m_col[i];
		m_pos[i] = m.m_pos[i];
	}
}

void CColorMap::SetRange(float fmin, float fmax)
{
	m_min = fmin;
	m_max = fmax;
	if (m_min == m_max) m_max += 1.f;
}

GLColor CColorMap::map(float fval) const
{
	float w = (fval - m_min) / (m_max - m_min);

	int n = m_ncol - 1;
	if (w <= m_pos[0]) return m_col[0];
	if (w >= m_pos[n]) return m_col[n];

	int l = 0;
	while (m_pos[l] < w) ++l;

	float dp = m_pos[l] - m_pos[l-1];
	if (dp != 0.f) dp = 1.f/dp; else dp = 1.f;

	GLColor c1 = m_col[l];
	GLColor c2 = m_col[l-1];

	GLColor col;
	col.r = uint8_t(((w - m_pos[l-1])*c1.r + (m_pos[l]-w)*c2.r) * dp);
	col.g = uint8_t(((w - m_pos[l-1])*c1.g + (m_pos[l]-w)*c2.g) * dp);
	col.b = uint8_t(((w - m_pos[l-1])*c1.b + (m_pos[l]-w)*c2.b) * dp);
	col.a = uint8_t(((w - m_pos[l-1])*c1.a + (m_pos[l]-w)*c2.a) * dp);

	return col;
}

void CColorMap::gray()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(  0,  0,  0);
	m_pos[1] = 0.25f; m_col[1] = GLColor( 64, 64, 64);
	m_pos[2] = 0.50f; m_col[2] = GLColor(128,128,128);
	m_pos[3] = 0.75f; m_col[3] = GLColor(192,192,192);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255,255,255);
}

void CColorMap::jet()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(0, 0, 255);
	m_pos[1] = 0.40f; m_col[1] = GLColor(0, 255, 255);
	m_pos[2] = 0.50f; m_col[2] = GLColor(0, 255, 0);
	m_pos[3] = 0.60f; m_col[3] = GLColor(255, 255, 0);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 0, 0);
}

void CColorMap::parula()
{
	m_ncol = 7;

	m_pos[0] = 0.0000f; m_col[0] = GLColor( 0,   0, 143); // 0x00008F
	m_pos[1] = 0.1667f; m_col[1] = GLColor(71,  87, 247); // 0x4757F7
	m_pos[2] = 0.3333f; m_col[2] = GLColor(39, 150, 235); // 0x2796EB
	m_pos[3] = 0.5000f; m_col[3] = GLColor(24, 191, 181); // 0x18BFB5
	m_pos[4] = 0.6666f; m_col[4] = GLColor(128, 203, 88); // 0x80CB58
	m_pos[5] = 0.8333f; m_col[5] = GLColor(253, 189, 60); // 0xFDBD3C
	m_pos[6] = 1.0000f; m_col[6] = GLColor(249, 250, 20); // 0xF9FA14
}

void CColorMap::red()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(0, 0, 0);
	m_pos[1] = 0.25f; m_col[1] = GLColor(64, 0, 0);
	m_pos[2] = 0.50f; m_col[2] = GLColor(128, 0, 0);
	m_pos[3] = 0.75f; m_col[3] = GLColor(192, 0, 0);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 0, 0);
}

void CColorMap::redgreen()
{
	m_ncol = 2;

	m_pos[0] = 0.00f; m_col[0] = GLColor(255, 0, 0);
	m_pos[1] = 1.00f; m_col[1] = GLColor(0, 255, 0);
}

void CColorMap::green()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(0, 0, 0);
	m_pos[1] = 0.25f; m_col[1] = GLColor(0, 64, 0);
	m_pos[2] = 0.50f; m_col[2] = GLColor(0, 128, 0);
	m_pos[3] = 0.75f; m_col[3] = GLColor(0, 192, 0);
	m_pos[4] = 1.00f; m_col[4] = GLColor(0, 255, 0);
}

void CColorMap::hotcold()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(64, 83, 187);
	m_pos[1] = 0.25f; m_col[1] = GLColor(133, 188, 255);
	m_pos[2] = 0.50f; m_col[2] = GLColor(241, 241, 241);
	m_pos[3] = 0.75f; m_col[3] = GLColor(253, 165, 123);
	m_pos[4] = 1.00f; m_col[4] = GLColor(184, 19, 40);
}

void CColorMap::blue()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(0, 0, 0);
	m_pos[1] = 0.25f; m_col[1] = GLColor(0, 0, 64);
	m_pos[2] = 0.50f; m_col[2] = GLColor(0, 0, 128);
	m_pos[3] = 0.75f; m_col[3] = GLColor(0, 0, 192);
	m_pos[4] = 1.00f; m_col[4] = GLColor(0, 0, 255);
}

void CColorMap::autumn()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(50, 50, 0);
	m_pos[1] = 0.25f; m_col[1] = GLColor(100, 50, 0);
	m_pos[2] = 0.50f; m_col[2] = GLColor(150, 75, 0);
	m_pos[3] = 0.75f; m_col[3] = GLColor(200, 150, 0);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 200, 0);
}

void CColorMap::winter()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(255, 0, 255);
	m_pos[1] = 0.25f; m_col[1] = GLColor(128, 0, 255);
	m_pos[2] = 0.50f; m_col[2] = GLColor(0, 0, 255);
	m_pos[3] = 0.75f; m_col[3] = GLColor(0, 255, 255);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 255, 255);
}

void CColorMap::spring()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(0, 255, 0);
	m_pos[1] = 0.25f; m_col[1] = GLColor(128, 255, 0);
	m_pos[2] = 0.50f; m_col[2] = GLColor(255, 255, 0);
	m_pos[3] = 0.75f; m_col[3] = GLColor(255, 255, 128);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 255, 255);
}

void CColorMap::summer()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(255, 128, 0);
	m_pos[1] = 0.25f; m_col[1] = GLColor(255, 128, 0);
	m_pos[2] = 0.50f; m_col[2] = GLColor(255, 255, 0);
	m_pos[3] = 0.75f; m_col[3] = GLColor(255, 255, 128);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 255, 255);
}

void CColorMap::rbb()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(128, 128, 255);
	m_pos[1] = 0.25f; m_col[1] = GLColor(0, 0, 255);
	m_pos[2] = 0.50f; m_col[2] = GLColor(0, 0, 0);
	m_pos[3] = 0.75f; m_col[3] = GLColor(255, 0, 0);
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 128, 128); // salmon
}

void CColorMap::fire()
{
	m_ncol = 5;

	m_pos[0] = 0.00f; m_col[0] = GLColor(0, 0, 0); // black
	m_pos[1] = 0.25f; m_col[1] = GLColor(128, 0, 255); // purple
	m_pos[2] = 0.50f; m_col[2] = GLColor(255, 0, 0); // red
	m_pos[3] = 0.75f; m_col[3] = GLColor(255, 255, 0); // yellow
	m_pos[4] = 1.00f; m_col[4] = GLColor(255, 255, 255); // white
}

//-----------------------------------------------------------------------------
// Invert the colormap
void CColorMap::Invert()
{
	for (int i=0; i<m_ncol/2; ++i)
	{
		GLColor c = m_col[i];
		m_col[i] = m_col[m_ncol-i-1];
		m_col[m_ncol-i-1] = c;
	}
}

//=============================================================================
class Post::ColorMapTemplate
{
public:
	ColorMapTemplate(){}
	ColorMapTemplate(const string& name, const CColorMap& map) : m_name(name), m_map(map) {}
	ColorMapTemplate(const ColorMapTemplate& cmt)
	{
		m_name = cmt.m_name;
		m_map  = cmt.m_map;
	}
	void operator = (const ColorMapTemplate& cmt)
	{
		m_name = cmt.m_name;
		m_map  = cmt.m_map;
	}

	const string& name() const { return m_name; }

	void setName(const std::string& newName) { m_name = newName; }

	CColorMap& colormap() { return m_map; }

private: 
	string		m_name;
	CColorMap	m_map;
};

//=============================================================================
vector<class ColorMapTemplate>	ColorMapManager::m_map;
int ColorMapManager::m_defaultMap = ColorMapManager::JET;

// set the default color map
void ColorMapManager::SetDefaultMap(int map)
{
	if ((map >= 0) && (map < ColorMaps()))
	{
		m_defaultMap = map;
	}
}

int ColorMapManager::GetDefaultMap()
{ 
	return m_defaultMap; 
}

int ColorMapManager::ColorMaps()
{
	return (int) m_map.size();
}

int ColorMapManager::UserColorMaps()
{
	return (int)m_map.size() - USER;
}

void ColorMapManager::Initialize()
{
	if (m_map.empty() == false) return;

	CColorMap map;
	map.autumn (); AddColormap("Autumn", map);
	map.blue   (); AddColormap("Blue"  , map);
	map.fire   (); AddColormap("Fire"  , map);
	map.gray   (); AddColormap("Gray"  , map);
	map.green  (); AddColormap("Green" , map);
	map.hotcold(); AddColormap("Hot-Cold", map);
	map.jet    (); AddColormap("Jet"   , map);
	map.parula (); AddColormap("Parula", map);
	map.rbb    (); AddColormap("RBB"   , map);
	map.red    (); AddColormap("Red"   , map);
	map.redgreen(); AddColormap("Red-Green", map);
	map.spring (); AddColormap("Spring", map);
	map.summer (); AddColormap("Summer", map);
	map.winter (); AddColormap("Winter", map);
}

string ColorMapManager::GetColorMapName(int n)
{
	if ((n>=0) && (n < ColorMaps()))
	{
		return m_map[n].name();
	}
	else 
	{
		assert(false);	
		return "";
	}
}

int ColorMapManager::FindColorMapFromName(const std::string& mapName)
{
	for (int i=0; i<m_map.size(); ++i)
	{
		if (m_map[i].name() == mapName) return i;
	}
	return -1;
}

// set the colormap name
void ColorMapManager::SetColorMapName(int n, const std::string& newName)
{
	m_map[n].setName(newName);
}

CColorMap& ColorMapManager::GetColorMap(int n)
{
	if ((n<0) || (n>=m_map.size())) n = 0;
	return m_map[n].colormap();
}

void ColorMapManager::AddColormap(const string& name, const CColorMap& map)
{
	ColorMapTemplate newTemplate(name, map);
	m_map.push_back(newTemplate);	
}

// remove a color map template (can't delete default templates)
bool ColorMapManager::RemoveColormap(int n)
{
	if (n < USER) return false;
	m_map.erase(m_map.begin() + n);
	return true;
}
