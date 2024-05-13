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
#include "VTKModel.h"
#include <assert.h>
#include <string.h> // for strcmp on linux
using namespace VTK;

vtkDataArray::vtkDataArray()
{
	m_type = -1;
	m_format = ASCII;
	m_numComps = 1;
	m_offset = 0;
}

void vtkDataArray::setFormat(const char* szformat)
{
	if      (strcmp(szformat, "ascii"   ) == 0) m_format = Format::ASCII;
	else if (strcmp(szformat, "binary"  ) == 0) m_format = Format::BINARY;
	else if (strcmp(szformat, "appended") == 0) m_format = Format::APPENDED;
	else { assert(false); }
}

size_t vtkDataArray::size() const
{
	switch (m_type)
	{
	case FLOAT32:
	case FLOAT64: return (m_values_float.size() / m_numComps); break;
	case INT8:
	case UINT8:
	case INT32:
	case INT64:
		return (m_values_int.size() / m_numComps); break;
		break;
	default:
		assert(false);
		break;
	}
	return 0;
}

void vtkDataArray::init(vtkDataArray::Format format, vtkDataArray::Types type, int components)
{
	m_format = format;
	m_type = type;
	m_numComps = components;
}

vtkPoint vtkPiece::Point(int n) const
{
	vtkDataArrayReader<double> p(m_points);
	return vtkPoint{ p[3 * n], p[3 * n + 1], p[3 * n + 2] };
}

vtkCell vtkPiece::Cell(int n) const
{
	vtkCell cell;

	if (m_cell_types.empty())
	{
		// This is for POLYDATA files
		int n0 = (n > 0 ? m_cell_offsets.m_values_int[n - 1] : 0);
		int n1 = m_cell_offsets.m_values_int[n];
		cell.m_numNodes = n1 - n0;
		if (cell.m_numNodes == 2) cell.m_cellType = vtkCell::VTK_LINE;
		else cell.m_cellType = vtkCell::VTK_POLYGON;
		int m = cell.m_numNodes;
		for (int i = 0; i < m; ++i)
		{
			cell.m_node[i] = m_cell_connect.m_values_int[n0 + i];
		}
	}
	else
	{
		// This is for UNSTRUCTURED GRID files
		m_cell_types.get(n, &cell.m_cellType);

		int n0 = (n == 0 ? 0 : m_cell_offsets.m_values_int[n - 1]);
		int n1 = m_cell_offsets.m_values_int[n];
		cell.m_numNodes = n1 - n0;
		int m = cell.m_numNodes;
		for (int i = 0; i < m; ++i)
		{
			cell.m_node[i] = m_cell_connect.m_values_int[n0 + i];
		}
	}

	return cell;
}