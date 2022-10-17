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

#include "FEElementLibrary.h"
using namespace std;

vector<FSElemTraits> FSElementLibrary::m_lib;

void FSElementLibrary::addElement(int ntype, int nshape, int nclass, int nodes, int faces, int edges)
{
	FSElemTraits t = {ntype, nshape, nclass, nodes, faces, edges};
	m_lib.push_back(t);
}

void FSElementLibrary::InitLibrary()
{
	m_lib.clear();

	// NOTE: When adding new elements make sure to set the faces to zero for shells, and the edges to zero for solids.
	addElement(FE_INVALID_ELEMENT_TYPE, 0, 0, 0, 0, 0);
	addElement(FE_HEX8   , ELEM_HEX  , ELEM_SOLID,  8, 6, 0);
	addElement(FE_TET4   , ELEM_TET  , ELEM_SOLID,  4, 4, 0);
	addElement(FE_PENTA6 , ELEM_PENTA, ELEM_SOLID,  6, 5, 0);
	addElement(FE_QUAD4  , ELEM_QUAD , ELEM_SHELL,  4, 0, 4);
	addElement(FE_TRI3   , ELEM_TRI  , ELEM_SHELL,  3, 0, 3);
	addElement(FE_BEAM2  , ELEM_LINE , ELEM_BEAM ,  2, 0, 0);
	addElement(FE_HEX20  , ELEM_HEX  , ELEM_SOLID, 20, 6, 0);
	addElement(FE_QUAD8  , ELEM_QUAD , ELEM_SHELL,  8, 0, 4);
	addElement(FE_BEAM3  , ELEM_LINE , ELEM_BEAM ,  3, 0, 0);
	addElement(FE_TET10  , ELEM_TET  , ELEM_SOLID, 10, 4, 0);
	addElement(FE_TRI6   , ELEM_TRI  , ELEM_SHELL,  6, 0, 3);
	addElement(FE_TET15  , ELEM_TET  , ELEM_SOLID, 15, 4, 0);
	addElement(FE_HEX27  , ELEM_HEX  , ELEM_SOLID, 27, 6, 0);
	addElement(FE_TRI7   , ELEM_TRI  , ELEM_SHELL,  7, 0, 3);
	addElement(FE_QUAD9  , ELEM_QUAD , ELEM_SHELL,  9, 0, 4);
	addElement(FE_PENTA15, ELEM_PENTA, ELEM_SOLID, 15, 5, 0);
	addElement(FE_PYRA5  , ELEM_PYRA , ELEM_SOLID,  5, 5, 0);
	addElement(FE_TET20  , ELEM_TET  , ELEM_SOLID, 20, 4, 0);
	addElement(FE_TRI10  , ELEM_TRI  , ELEM_SHELL, 10, 0, 3);
	addElement(FE_TET5   , ELEM_TET  , ELEM_SOLID,  5, 4, 0);
	addElement(FE_PYRA13 , ELEM_PYRA , ELEM_SOLID, 13, 5, 0);
}

const FSElemTraits* FSElementLibrary::GetTraits(int type)
{
	int ntype = (int) type;
	if ((ntype >= 0) && (ntype < m_lib.size()))
	{
		return &m_lib[ntype];
	}
	else
	{
		assert(false);
		return 0;
	}
}
