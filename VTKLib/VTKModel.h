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
#include <vector>

namespace VTK {

	class vtkDataArray
	{
	public:
		enum Types
		{
			UINT8,
			INT32,
			INT64,
			FLOAT32,
			FLOAT64,
		};

		enum Format
		{
			ASCII,
			BINARY,
			APPENDED
		};

	public:
		vtkDataArray()
		{
			m_type = -1;
			m_format = ASCII;
			m_numComps = 1;
			m_offset = 0;
		}

		void setFormat(const char* szformat)
		{
			if (strcmp(szformat, "ascii") == 0) m_format = Format::ASCII;
			else if (strcmp(szformat, "binary") == 0) m_format = Format::BINARY;
			else if (strcmp(szformat, "appended") == 0) m_format = Format::APPENDED;
			else { assert(false); }
		}

		size_t size() const
		{
			switch (m_type)
			{
			case FLOAT32:
			case FLOAT64: return (m_values_float.size() / m_numComps); break;
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

		bool empty() const { return m_values_int.empty(); }

		void get(int n, double* v) const { *v = m_values_float[n]; }
		void get(int n, int* v) const { *v = m_values_int[n]; }

	public:
		int	m_type;
		int m_format;
		int m_numComps;
		int	m_offset;

		std::vector<double>		m_values_float;
		std::vector<int>		m_values_int;
	};

	template <class T> class vtkDataArrayReader
	{
	public:
		vtkDataArrayReader(const vtkDataArray& data) : m_data(data) {}
		T operator [] (int n) { T v; m_data.get(n, &v); return v; }
	private:
		const vtkDataArray& m_data;
	};

	class vtkCell
	{
	public:
		enum { MAX_NODES = 20 };

		enum CellType
		{
			VTK_VERTEX = 1,
			VTK_POLY_VERTEX = 2,
			VTK_LINE = 3,
			VTK_POLY_LINE = 4,
			VTK_TRIANGLE = 5,
			VTK_TRIANGLE_STRIP = 6,
			VTK_POLYGON = 7,
			VTK_PIXEL = 8,
			VTK_QUAD = 9,
			VTK_TETRA = 10,
			VTK_VOXEL = 11,
			VTK_HEXAHEDRON = 12,
			VTK_WEDGE = 13,
			VTK_PYRAMID = 14
		};

		vtkCell()
		{
			m_label = 0;
			m_cellType = 0;
			m_numNodes = 0;
		}

	public:
		int		m_label;
		int		m_cellType;
		int		m_numNodes;
		int		m_node[MAX_NODES];
	};

	class vtkPiece
	{
	public:
		vtkPiece()
		{
			m_numPoints = 0;
			m_numCells = 0;
		}

		vtkPiece(const vtkPiece& piece)
		{
			m_numPoints = piece.m_numPoints;
			m_numCells = piece.m_numCells;
			m_points = piece.m_points;
			m_cell_connect = piece.m_cell_connect;
			m_cell_offsets = piece.m_cell_offsets;
			m_cell_types = piece.m_cell_types;
		}

		void operator = (const vtkPiece& piece)
		{
			m_numPoints = piece.m_numPoints;
			m_numCells = piece.m_numCells;
			m_points = piece.m_points;
			m_cell_connect = piece.m_cell_connect;
			m_cell_offsets = piece.m_cell_offsets;
			m_cell_types = piece.m_cell_types;
		}

		size_t Points() const { return m_points.size(); }
		size_t Cells() const { return m_cell_offsets.size(); }

		vec3d Point(int n) const
		{
			vtkDataArrayReader<double> p(m_points);
			return vec3d(p[3 * n], p[3 * n + 1], p[3 * n + 2]);
		}

		vtkCell Cell(int n) const
		{
			vtkCell cell;

			if (m_cell_types.empty())
			{
				int n0 = (n > 0 ? m_cell_offsets.m_values_int[n - 1] : 0);
				int n1 = m_cell_offsets.m_values_int[n];
				cell.m_numNodes = n1 - n0;
				cell.m_cellType = vtkCell::VTK_POLYGON;
				int m = cell.m_numNodes;
				for (int i = 0; i < m; ++i)
				{
					cell.m_node[i] = m_cell_connect.m_values_int[n0 + i];
				}
			}
			else
			{
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

	public:
		int m_numPoints;
		int m_numCells;

		vtkDataArray	m_points;
		vtkDataArray	m_cell_connect;
		vtkDataArray	m_cell_offsets;
		vtkDataArray	m_cell_types;
	};

	class vtkAppendedData
	{
	public:
		void SetData(const char* szdata)
		{
			m_data = szdata;
		}

		const char* GetData(size_t offset)
		{
			return m_data.data() + offset;
		}

	private:
		std::string	m_data;
	};

	class vtkModel
	{
	public:
		void AddPiece(const vtkPiece& piece) { m_pieces.push_back(piece); }

		size_t Pieces() const { return m_pieces.size(); }

		vtkPiece& Piece(int n) { return m_pieces[n]; }
		const vtkPiece& Piece(int n) const { return m_pieces[n]; }

		void Clear() { m_pieces.clear(); }

	public:
		std::vector<vtkPiece>	m_pieces;
	};
}
