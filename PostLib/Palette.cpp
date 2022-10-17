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
#include "Palette.h"
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
using namespace Post;

CPalette::CPalette(const string& name) : m_name(name)
{
}

CPalette::CPalette(const CPalette& p)
{
	m_col = p.m_col;
	m_name = p.m_name;
}

void CPalette::operator = (const CPalette& p)
{
	m_col = p.m_col;
	m_name = p.m_name;
}

int CPalette::Colors() const
{ 
	return (int)m_col.size(); 
}

void CPalette::AddColor(const GLColor& c)
{
	m_col.push_back(c);
}

GLColor CPalette::Color(int i) const
{
	if ((i>=0) && (i< Colors())) return m_col[i];
	return GLColor(0,0,0);
}

//=============================================================================
CPaletteManager*	CPaletteManager::m_this = 0;

CPaletteManager::CPaletteManager()
{
	// add standard palette
	CPalette pal("Standard");

	pal.AddColor(GLColor(128, 255, 0));
	pal.AddColor(GLColor(128,  0,255));
	pal.AddColor(GLColor(  0,  0,255));
	pal.AddColor(GLColor(255,255,  0));
	pal.AddColor(GLColor(255,  0,255));
	pal.AddColor(GLColor(  0,255,255));
	pal.AddColor(GLColor(255,128,  0));
	pal.AddColor(GLColor(255,  0,128));
	pal.AddColor(GLColor(255,  0,  0));
	pal.AddColor(GLColor(  0,255,128));
	pal.AddColor(GLColor(  0,255,  0));
	pal.AddColor(GLColor(  0,128,255));
	pal.AddColor(GLColor(128,  0,  0));
	pal.AddColor(GLColor(  0,128,  0));
	pal.AddColor(GLColor(  0,  0,128));
	pal.AddColor(GLColor(128,128,  0));
	pal.AddColor(GLColor(128,  0,128));
	pal.AddColor(GLColor(  0,128,128));
	pal.AddColor(GLColor(255,255,255));
	pal.AddColor(GLColor(192,192,192));
	pal.AddColor(GLColor(164,164,164));
	pal.AddColor(GLColor(128,128,128));
	pal.AddColor(GLColor( 92, 92, 92));
	pal.AddColor(GLColor( 64, 64, 64));
	pal.AddColor(GLColor( 32, 32, 32));

	AddPalette(pal);

	m_currentIndex = 0;
}

CPaletteManager& CPaletteManager::GetInstance()
{
	if (m_this == 0) m_this = new CPaletteManager;
	return *m_this;
}

void CPaletteManager::AddPalette(const CPalette& p)
{
	m_pal.push_back(p);
}

int CPaletteManager::Palettes() const
{
	return (int) m_pal.size();
}

const CPalette& CPaletteManager::Palette(int i) const 
{
	 return m_pal[i]; 
}

const CPalette& CPaletteManager::CurrentPalette()
{
	CPaletteManager& This = CPaletteManager::GetInstance();
	return This.m_pal[This.m_currentIndex];
}

void CPaletteManager::SetCurrentIndex(int n)
{
	if ((n>=0) && (n<Palettes())) m_currentIndex = n;
}

bool CPaletteManager::Save(const string& file, const CPalette& pal)
{
	XMLWriter xml;
	if (xml.open(file.c_str()) == false) return false;

	XMLElement el("PostViewResource");
	el.add_attribute("version", "1.0");
	xml.add_branch(el);
	{
		XMLElement el("Palette");
		el.add_attribute("name", pal.Name().c_str());
		xml.add_branch(el);
		{
			int NCOL = pal.Colors();
			for (int i=0; i<NCOL; ++i)
			{
				GLColor c = pal.Color(i);
				int v[3] = {c.r, c.g, c.b};
				xml.add_leaf("color", v, 3);
			}
		}
		xml.close_branch();
	}
	xml.close_branch();
	xml.close();
	return true;

}

bool CPaletteManager::Load(const string& file)
{
	// Open the file in an xml reader
	XMLReader xml;
	if (xml.Open(file.c_str()) == false) return false;

	// find the root tag
	XMLTag tag;
	if (xml.FindTag("PostViewResource", tag) == false) return false; 

	char szbuf[256] = { 0 };
	++tag;
	do
	{
		if (tag == "Palette")
		{
			// get the palette's name, or create a new one
			const char* szname = tag.AttributeValue("name", true);
			if (szname == 0)
			{
				sprintf(szbuf, "Palette %d", Palettes() + 1);
				szname = szbuf;
			}

			// create the palette
			CPalette pal(szname);

			if (tag.isempty() == false)
			{
				++tag;
				do
				{
					if (tag == "color")
					{
						int c[3] = {0,0,0};
						tag.value(c, 3);

						GLColor col((Byte) c[0], (Byte) c[1], (Byte) c[2]);
						pal.AddColor(col);

						++tag;
					}
					else xml.SkipTag(tag);
				}
				while (!tag.isend());

				AddPalette(pal);
			}
			
			++tag;
		}
		else xml.SkipTag(tag);
	}
	while (!tag.isend());

	return true;
}
