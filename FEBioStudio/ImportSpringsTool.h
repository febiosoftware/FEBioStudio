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
#include "Tool.h"

class GModel;
class GMeshObject;

class CImportSpringsTool : public CBasicTool
{
	struct SPRING
	{
		vec3d	r0;
		vec3d	r1;
		int n0 = -1;
		int n1 = -1;
	};

public:
	CImportSpringsTool(CMainWindow* wnd);

	bool OnApply();

private:
	bool ReadFile();
	bool AddSprings(GModel* fem, GMeshObject* po);
	bool AddTrusses(GModel* fem, GMeshObject* po);
	void Intersect(GMeshObject* po, SPRING& s);
	int ProcessSprings(GMeshObject* po);

	bool ReadTXTFile();
	bool ReadVTKFile();

private:
	QString	m_fileName;
	bool	m_snap;
	double	m_tol;
	bool	m_bintersect;
	int		m_type;

	std::vector<SPRING>	m_springs;
};
