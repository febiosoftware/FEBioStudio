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
#include <vector>
#include <string>

class CPalette
{
public:
	CPalette(const std::string& name);
	CPalette(const CPalette& p);
	void operator = (const CPalette& p);

	const std::string& Name() const { return m_name; }

	// add a color
	void AddColor(const GLColor& c);

	// return number of colors
	int Colors() const;

	// return a color
	// (out-of-range returns black)
	GLColor Color(int i) const;
	
private:
	std::vector<GLColor>	m_col;
	std::string			m_name;
};

class CPaletteManager
{
public:
	static CPaletteManager& GetInstance();

	static const CPalette& CurrentPalette();

	void AddPalette(const CPalette& p);

	int Palettes() const;

	const CPalette& Palette(int i) const;

	bool Save(const std::string& file, const CPalette& pal);

	bool Load(const std::string& file);

	int CurrentIndex() const { return m_currentIndex; }

	void SetCurrentIndex(int n);

private:
	std::vector<CPalette>	m_pal;
	int					m_currentIndex = 0;

private:
	CPaletteManager();
	CPaletteManager(const CPaletteManager&){}
	static CPaletteManager*	m_this;
};
