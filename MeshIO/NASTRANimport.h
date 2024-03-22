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

//! Class for reading NASTRAN files. 
class NASTRANimport : public FSFileImport
{
protected:
	enum {MAX_ITEMS = 32};
	struct CARD
	{
		char	szitem[MAX_ITEMS][17];
		int		nitems;
	};

	struct GRID
	{
		double	x, y, z;
	};

	struct ELEM
	{
		int	pid;
		int	nn;
		int	n[20];
	};

	struct PSOLID
	{
		int	pid;
		int	mid;
	};

	struct MAT1
	{
		int	mid;
		double	E, v, d;
	};

public:
	NASTRANimport(FSProject& prj) : FSFileImport(prj) {}
	bool Load(const char* szfile);

protected:
	bool ParseFile();

	char* get_line(char* szline);
	bool read_card(CARD& c, char* szline);

	bool read_GRID  (CARD& c);
	bool read_CTETRA(CARD& c);
	bool read_PSOLID(CARD& c);
	bool read_MAT1  (CARD& c);
	bool read_CHEXA (CARD& c);

	bool BuildMesh(FSModel& fem);

protected:
	list<GRID>		m_Node;
	list<ELEM>		m_Elem;
	list<PSOLID>	m_Part;
	list<MAT1>		m_Mat;
};
