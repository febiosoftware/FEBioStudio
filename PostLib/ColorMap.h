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
#include <FSCore/color.h>
#include <GLLib/GLTexture1D.h>
#include <vector>
#include <string>
//using namespace std;

using std::vector;
using std::string;

namespace Post {

class ColorMapTemplate;

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

//-----------------------------------------------------------------------------
class CColorTexture
{
public:
	CColorTexture();
	CColorTexture(const CColorTexture& col);
	void operator = (const CColorTexture& col);

	GLTexture1D& GetTexture() { return m_tex; }

	void UpdateTexture();

	int GetDivisions() const;
	void SetDivisions(int n);

	bool GetSmooth() const;
	void SetSmooth(bool b);

	void SetColorMap(int n);
	int GetColorMap() const;

	CColorMap& ColorMap();

private:
	int		m_colorMap;		// index of template to use
	int		m_ndivs;		// number of divisions
	bool	m_bsmooth;		// smooth interpolation or not

	GLTexture1D m_tex;	// the actual texture
};

//-----------------------------------------------------------------------------
// Class for managing available color maps
class ColorMapManager
{
public:
	enum DefaultMap
	{
		AUTUMN,
		BLUE,
		FIRE,
		GRAY,
		GREEN,
		HOTCOLD,
		JET,
		PARULA,
		RBB,
		RED,
		RED_GREEN,
		SPRING,
		SUMMER,
		WINTER,
		USER
	};

public:
	// calls this to generate a list of default maps
	static void Initialize();

	// get the number of templates available
	static int ColorMaps();

	// get the number of user templates available
	static int UserColorMaps();

	// return the name of a template
	static string GetColorMapName(int n);

	// return the index of a color map from its name (returns -1 if not found)
	static int FindColorMapFromName(const std::string& mapName);

	// set the colormap name
	static void SetColorMapName(int n, const std::string& newName);

	// add a color map template
	static void AddColormap(const string& name, const CColorMap& map);

	// get a reference to the color map template
	static CColorMap& GetColorMap(int n);

	// remove a color map template (can't delete default templates)
	static bool RemoveColormap(int n);

	// the default color map
	static int GetDefaultMap();

	// set the default color map
	static void SetDefaultMap(int map);

private:
	// this is a singleton so don't try to instantiate this
	ColorMapManager(){}
	ColorMapManager(const ColorMapManager&){}

private:
	static vector<class ColorMapTemplate>	m_map;
	static int m_defaultMap;
};
}
