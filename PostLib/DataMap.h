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
#include <FSCore/math3d.h>
#include <vector>

class FSMesh;

namespace Post {

//-----------------------------------------------------------------------------
template <typename T>
class DataMap
{
public:
	DataMap(void) { m_pmesh = 0; }
	~DataMap(void) {}

	int States() { return (int)m_Data.size(); }
	std::vector<T>& State(int n) { return m_Data[n]; }
	int GetTag(int n) { return m_tag[n]; }
	void SetTag(int n, int ntag) { m_tag[n] = ntag; }
	void SetTags(int n)
	{
		for (int i = 0; i < m_tag.size(); ++i) m_tag[i] = n;
	}

	void Create(int nstates, int items, T val = T(0), int ntag = 0)
	{
		m_tag.assign(nstates, ntag);
		m_Data.resize(nstates);
		for (int i=0; i<nstates; ++i) m_Data[i].assign(items, val);
	}

	void Clear() { m_Data.clear(); m_tag.clear(); }

	void SetFEMesh(FSMesh* pm) { m_pmesh = pm; }

protected:
	std::vector<int>	m_tag;
	std::vector< std::vector<T> >	m_Data;
	FSMesh*	m_pmesh;
};

//-----------------------------------------------------------------------------
class VectorMap : public DataMap<vec3f>
{
public:
	// calculate the data gradient
	void Gradient(int ntime, std::vector<float>& v);
};
}
