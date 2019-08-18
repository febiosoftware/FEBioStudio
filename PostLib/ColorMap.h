#pragma once
#include "GLTexture1D.h"
#include "color.h"
#include <vector>
#include <string>
using namespace std;

namespace Post {

class CGLTexture1D;

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
	void green  ();
	void hotcold();
	void blue   ();
	void rbb    ();
	void fire   ();

	GLColor map(float fval) const;

	int Colors() const { return m_ncol; }
	void SetColors(int n) { m_ncol = n; }

	GLColor GetColor(int i) const { return m_col[i]; }
	void SetColor(int i, GLColor c) { m_col[i] = c; }

	float GetColorPos(int i) const { return m_pos[i]; }
	void SetColorPos(int i, float v) { m_pos[i] = v; }

	void Invert();

protected:
	int		m_ncol;
	GLColor	m_col[MAX_MAP_COLORS];
	float	m_pos[MAX_MAP_COLORS];
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
		RBB,
		RED,
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

	// set the colormap name
	static void SetColorMapName(int n, const std::string& newName);

	// add a color map template
	static void AddColormap(const string& name, const CColorMap& map);

	// get a reference to the color map template
	static CColorMap& GetColorMap(int n);

	// remove a color map template (can't delete default templates)
	static bool RemoveColormap(int n);

	// the default color map
	static int GetDefaultMap() { return JET; }

private:
	// this is a singleton so don't try to instantiate this
	ColorMapManager(){}
	ColorMapManager(const ColorMapManager&){}

private:
	static vector<class ColorMapTemplate>	m_map;
};
}
