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
#include <vector>

namespace Post {
//-----------------------------------------------------------------------------
// Imports a Tecplot data text file
class FEASCIIImport : public FEFileReader
{
	enum { MAX_VARS = 16 };
	enum { ZONE_TRIANGLE, ZONE_QUAD, ZONE_TET, ZONE_BRICK };

private:
	struct VARIABLE
	{
		char	szname[128];
	};

	struct NODE
	{
		float	v[MAX_VARS];
	};

	struct ELEM
	{
		int	node[8];
	};

	class ZONE
	{
	public:
		char	szname[128];	// zone name
		int		m_npack;		// packing mode
		int		m_nn;			// number of nodes
		int		m_ne;			// number of elements
		int		m_ntype;		// element type
		double	m_ftime;		// solution time

		std::vector<NODE>	m_Node;
		std::vector<ELEM>	m_Elem;

	public:
		ZONE();
		ZONE(const ZONE& z);
		void operator = (const ZONE& z);
	};

public:
	FEASCIIImport(FEPostModel* fem);
	~FEASCIIImport(void);

	bool Load(const char* szfile) override;

protected:
	bool ReadTitle    ();
	bool ReadVariables();
	bool ReadZone     ();
	bool ReadZoneInfo (ZONE& zone);
	bool ReadNodes   (ZONE& zone);
	bool ReadElements(ZONE& zone);

	bool BuildMesh(FEPostModel& fem);

	bool getline(char* szline, int nmax);

protected:
	char				m_sztitle[256];
	std::list<VARIABLE>	m_Var;
	std::vector<ZONE>	m_Zone;

private:
	char	m_szline[256];
};
}
