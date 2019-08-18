#pragma once
#include "color.h"
#include <vector>
#include <string>
using namespace std;

namespace Post {

class CPalette
{
public:
	CPalette(const string& name);
	CPalette(const CPalette& p);
	void operator = (const CPalette& p);

	const string& Name() const { return m_name; }

	// add a color
	void AddColor(const GLColor& c);

	// return number of colors
	int Colors() const;

	// return a color
	// (out-of-range returns black)
	GLColor Color(int i) const;
	
private:
	vector<GLColor>	m_col;
	string			m_name;
};

class CPaletteManager
{
public:
	static CPaletteManager& GetInstance();

	static const CPalette& CurrentPalette();

	void AddPalette(const CPalette& p);

	int Palettes() const;

	const CPalette& Palette(int i) const;

	bool Save(const string& file, const CPalette& pal);

	bool Load(const string& file);

	int CurrentIndex() const { return m_currentIndex; }

	void SetCurrentIndex(int n);

private:
	vector<CPalette>	m_pal;
	int					m_currentIndex;

private:
	CPaletteManager();
	CPaletteManager(const CPaletteManager&){}
	static CPaletteManager*	m_this;
};
}
