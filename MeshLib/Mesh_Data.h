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
#include "FEElement.h"

class FSMesh;
class FSSurfaceMesh;

class Mesh_Data
{
	struct DATA
	{
		double	val[FSElement::MAX_NODES];	// nodal values for element
		int		nval;						// number of nodal values (should equal nr of nodes for corresponding element)
		int		tag;
	};

public:
	Mesh_Data();
	Mesh_Data(const Mesh_Data& d);
	void operator = (const Mesh_Data& d);

	void Clear();

	void Init(FSMesh* mesh, double initVal, int initTag);
	void Init(FSSurfaceMesh* mesh, double initVal, int initTag);

	bool IsValid() const { return (m_data.empty() == false); }

	DATA& operator[](size_t n) { return m_data[n]; }

	// get the current element value
	double GetElementValue(int elem, int node) const { return m_data[elem].val[node]; }

	// get the average element value
	double GetElementAverageValue(int elem);

	// get the data tag
	int GetElementDataTag(int n) const { return m_data[n].tag; }

	// set the element's node value
	void SetElementValue(int elem, int node, double v) { m_data[elem].val[node] = v; }

	// set the element (average) value
	void SetElementValue(int elem, double v);

	// set the data tag
	void SetElementDataTag(int n, int tag) { m_data[n].tag = tag; }

	// update the range of values
	void UpdateValueRange();

	// get the value range
	void GetValueRange(double& vmin, double& vmax) const;

public:
	std::vector<DATA>		m_data;		//!< element values
	double	m_min, m_max;				//!< value range of element data
};

