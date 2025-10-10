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
#include "ColorMapManager.h"
#include <assert.h>
using namespace std;

class ColorMapTemplate
{
public:
	ColorMapTemplate() {}
	ColorMapTemplate(const string& name, const CColorMap& map) : m_name(name), m_map(map) {}
	ColorMapTemplate(const ColorMapTemplate& cmt)
	{
		m_name = cmt.m_name;
		m_map = cmt.m_map;
	}
	void operator = (const ColorMapTemplate& cmt)
	{
		m_name = cmt.m_name;
		m_map = cmt.m_map;
	}

	const string& name() const { return m_name; }

	void setName(const std::string& newName) { m_name = newName; }

	CColorMap& colormap() { return m_map; }

private:
	string		m_name;
	CColorMap	m_map;
};

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
	return (int)m_map.size();
}

int ColorMapManager::UserColorMaps()
{
	return (int)m_map.size() - USER;
}

void ColorMapManager::Initialize()
{
	if (m_map.empty() == false) return;

	CColorMap map;
	map.autumn(); AddColormap("Autumn", map);
	map.blue(); AddColormap("Blue", map);
	map.fire(); AddColormap("Fire", map);
	map.gray(); AddColormap("Gray", map);
	map.green(); AddColormap("Green", map);
	map.hotcold(); AddColormap("Hot-Cold", map);
	map.jet(); AddColormap("Jet", map);
	map.parula(); AddColormap("Parula", map);
	map.rbb(); AddColormap("RBB", map);
	map.red(); AddColormap("Red", map);
	map.redgreen(); AddColormap("Red-Green", map);
	map.spring(); AddColormap("Spring", map);
	map.summer(); AddColormap("Summer", map);
	map.winter(); AddColormap("Winter", map);
}

string ColorMapManager::GetColorMapName(int n)
{
	if ((n >= 0) && (n < ColorMaps()))
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
	for (int i = 0; i < m_map.size(); ++i)
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

const CColorMap& ColorMapManager::GetColorMap(int n)
{
	if ((n < 0) || (n >= m_map.size())) n = 0;
	return m_map[n].colormap();
}

void ColorMapManager::SetColormap(int n, const CColorMap& map)
{
	if ((n >= 0) || (n < m_map.size()))
	{
		CColorMap& trg = m_map[n].colormap();
		trg = map;
		trg.SetRange(0.f, 1.f); // make sure the range is not altered
	}
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
