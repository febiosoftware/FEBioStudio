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
#include "Mesh_Data.h"
#include "FEMesh.h"
#include "FESurfaceMesh.h"

//-----------------------------------------------------------------------------
Mesh_Data::Mesh_Data()
{
	m_min = m_max = 0.0;
}

//-----------------------------------------------------------------------------
Mesh_Data::Mesh_Data(const Mesh_Data& d)
{
	m_data = d.m_data;
	m_min = d.m_min;
	m_max = d.m_max;
}

//-----------------------------------------------------------------------------
void Mesh_Data::operator = (const Mesh_Data& d)
{
	m_data = d.m_data;
	m_min = d.m_min;
	m_max = d.m_max;
}

//-----------------------------------------------------------------------------
void Mesh_Data::Clear()
{
	m_data.clear();
	m_min = m_max = 0.0;
}

//-----------------------------------------------------------------------------
void Mesh_Data::Init(FSMesh* mesh, double initVal, int initTag)
{
	int NE = mesh->Elements();
	m_data.resize(NE);
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh->Element(i);
		DATA& di = m_data[i];
		int ne = el.Nodes();
		di.nval = ne;
		di.tag = initTag;
		for (int j = 0; j < ne; ++j)
		{
			di.val[j] = initVal;
		}
	}
}

//-----------------------------------------------------------------------------
void Mesh_Data::Init(FSSurfaceMesh* mesh, double initVal, int initTag)
{
	int NF = mesh->Faces();
	m_data.resize(NF);
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		DATA& di = m_data[i];
		int ne = f.Nodes();
		di.nval = ne;
		di.tag = initTag;
		for (int j = 0; j < ne; ++j)
		{
			di.val[j] = initVal;
		}
	}
}

//-----------------------------------------------------------------------------
// get the average element value
double Mesh_Data::GetElementAverageValue(int elem)
{
	double v = 0.0;
	if (m_data[elem].tag != 0)
	{
		for (int i = 0; i < m_data[elem].nval; ++i) v += m_data[elem].val[i];
		v /= (double)m_data[elem].nval;
	}
	return v;
}

//-----------------------------------------------------------------------------
// set the element (average) value
void Mesh_Data::SetElementValue(int elem, double v)
{
	int ne = m_data[elem].nval;
	for (int i = 0; i < ne; ++i) m_data[elem].val[i] = v;
}

//-----------------------------------------------------------------------------
void Mesh_Data::UpdateValueRange()
{
	m_min = m_max = 0;

	// find the first active value
	int N = (int)m_data.size();
	int i = 0;
	for (i = 0; i < N; ++i)
	{
		if (m_data[i].tag != 0)
		{
			m_min = m_max = m_data[i].val[0];
			break;
		}
	}

	// update range
	for (i = 0; i < N; ++i)
	{
		DATA& di = m_data[i];
		if (di.tag != 0)
		{
			for (int j = 0; j < di.nval; ++j)
			{
				if (di.val[j] > m_max) m_max = di.val[j];
				if (di.val[j] < m_min) m_min = di.val[j];
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Mesh_Data::GetValueRange(double& vmin, double& vmax) const
{
	vmin = m_min;
	vmax = m_max;
}
