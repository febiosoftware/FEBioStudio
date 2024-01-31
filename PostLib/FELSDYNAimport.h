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
#include "FEFileReader.h"
#include <list>

namespace Post {

class FEPostMesh;

class FELSDYNAimport : public FEFileReader  
{
protected:
	struct ELEMENT_SHELL
	{
		int		id;
		int		mid;
		int		n[4];
		double	h[4];

		ELEMENT_SHELL() { h[0] = h[1] = h[2] = h[3] = 0; }
	};

	struct ELEMENT_SOLID
	{
		int id;
		int	mid;
		int n[8];
	};

	struct NODE
	{
		int id;
		double	x, y, z;
		double v;
	};

public:
	FELSDYNAimport(FEPostModel* fem);
	virtual ~FELSDYNAimport();
	
	bool Load(const char* szfile) override;

	int FindNode(int id) const noexcept;

	void read_displacements(bool b) { m_bdispl = b; }

protected:
	char* get_line(char* szline);

	bool Read_Element_Solid();
	bool Read_Element_Shell();
	bool Read_Element_Shell_Thickness();
	bool Read_Node();
	bool Read_Nodal_Results();

	void BuildMaterials(FEPostModel& fem);
	bool BuildMesh(FEPostModel& fem);

	void BuildNLT();

protected:
	std::list<ELEMENT_SOLID>		m_solid;
	std::list<ELEMENT_SHELL>		m_shell;
	std::list<NODE>					m_node;

	FEPostMesh*			m_pm;

	std::vector<int> m_NLT;
	int m_nltoff;

	bool	m_bnresults;	// nodal results included?
	bool	m_bshellthick;	// shell thicknesses included?
	bool	m_bdispl;		// define displacement field?

	char	m_szline[256] = { 0 };
};

}
