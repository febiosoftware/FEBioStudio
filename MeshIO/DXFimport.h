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
#include <MeshIO/FSFileImport.h>
#include <FEMLib/FSProject.h>
#include <list>

class DXFimport : public FSFileImport
{
	enum {MAX_LINE=256};

protected:
	struct NODE
	{
		double x, y, z;
	};

	struct FACE
	{
		int		a, b, c, d;
		int		nodes;
	};

	struct POLYFACE
	{
		std::list<NODE>	m_Node;
		std::list<FACE>	m_Face;
	};

	struct FACE3D
	{
		vec3d	r[4];
		int		nn;
	};

public:
	DXFimport(FSProject& prj);
	virtual ~DXFimport();

	bool Load(const char* szfile) override;

protected:
	bool SearchFor(const char* sz);

	bool read_POLYLINE();
	bool read_3DFACE();

	void MakePolyFace(std::list<FACE3D>& Face);

protected:
	char	m_szline[256];

	std::list<POLYFACE*>	m_Obj;
	std::list<FACE3D>	m_Face;
	FSModel*		m_pfem;
};
