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
#include <FSCore/FSObject.h>
#include <string>

namespace Post {

	class FEPostModel;

	// TODO: inherit from FileWriter?
class FEASCIIExport : public FSBase
{
public:
	FEASCIIExport();
	bool Save(FEPostModel* pfem, int n0, int n1, const char* szfile);

	bool UpdateData(bool bsave) override;

public:
	bool	m_bselonly;	// export selection only
	bool	m_bcoords;	// export nodal coords
	bool	m_belem;	// element connectivity
	bool	m_bface;	// facet connectivity
	bool	m_bndata;	// export nodal values
	bool	m_bedata;	// export element data
	bool	m_bfnormals;	// export facet normals
	int		m_alltimes;
	std::string m_fmt;	// format string
};
}
