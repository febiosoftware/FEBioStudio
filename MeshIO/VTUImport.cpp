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

#include "VTUImport.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>
#include <XML/XMLReader.h>

class VTKDataArray
{
public:
	enum Types
	{
		UINT8,
		INT32,
		INT64,
		FLOAT32
	};

	enum Format
	{
		ASCII
	};

public:
	VTKDataArray()
	{
		m_type = -1;
		m_format = -1;
		m_numComps = 1;
	}

	VTKDataArray(const VTKDataArray& data)
	{
		m_type = data.m_type;
		m_format = data.m_format;
		m_numComps = data.m_numComps;

		m_values_float = data.m_values_float;
		m_values_int = data.m_values_int;
	}

	void operator = (const VTKDataArray& data)
	{
		m_type = data.m_type;
		m_format = data.m_format;
		m_numComps = data.m_numComps;

		m_values_float = data.m_values_float;
		m_values_int = data.m_values_int;
	}

	int	m_type;
	int m_format;
	int m_numComps;

	size_t size() const
	{
		switch (m_type)
		{
		case FLOAT32: return (m_values_float.size() / m_numComps); break;
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

	void get(int n, double* v) const { *v = m_values_float[n]; }
	void get(int n, int*    v) const { *v = m_values_int[n]; }

	std::vector<double>		m_values_float;
	std::vector<int>		m_values_int;
};

template <class T> class VTKDataArrayReader
{
public:
	VTKDataArrayReader(const VTKDataArray& data) : m_data(data) {}
	T operator [] (int n) { T v; m_data.get(n, &v); return v; }
private:
	const VTKDataArray& m_data;
};

class VTKCell
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

	VTKCell()
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

class VTKPiece
{
public:
	VTKPiece() 
	{
		m_numPoints = 0;
		m_numCells = 0;
	}

	VTKPiece(const VTKPiece& piece)
	{
		m_numPoints = piece.m_numPoints;
		m_numCells = piece.m_numCells;
		m_points = piece.m_points;
		m_cell_connect = piece.m_cell_connect;
		m_cell_offsets = piece.m_cell_offsets;
		m_cell_types = piece.m_cell_types;
	}

	void operator = (const VTKPiece& piece)
	{
		m_numPoints = piece.m_numPoints;
		m_numCells = piece.m_numCells;
		m_points = piece.m_points;
		m_cell_connect = piece.m_cell_connect;
		m_cell_offsets = piece.m_cell_offsets;
		m_cell_types = piece.m_cell_types;
	}

	size_t Points() const { return m_points.size(); }
	size_t Cells() const { return m_cell_types.size(); }

	vec3d Point(int n) const
	{
		VTKDataArrayReader<double> p(m_points);
		return vec3d(p[3 * n], p[3 * n + 1], p[3 * n + 2]);
	}

	VTKCell Cell(int n)
	{
		VTKCell cell;

		m_cell_types.get(n, &cell.m_cellType);

		int n0 = (n == 0 ? 0 : m_cell_offsets.m_values_int[n - 1]);
		int n1 = m_cell_offsets.m_values_int[n];
		cell.m_numNodes = n1 - n0;
		int m = cell.m_numNodes;
		for (int i = 0; i < m; ++i)
		{
			cell.m_node[i] = m_cell_connect.m_values_int[n0 + i];
		}

		return cell;
	}

public:
	int	m_numPoints;
	int m_numCells;

	VTKDataArray	m_points;
	VTKDataArray	m_cell_connect;
	VTKDataArray	m_cell_offsets;
	VTKDataArray	m_cell_types;
};

class VTKModel
{
public:
	void AddPiece(const VTKPiece& piece) { m_pieces.push_back(piece); }

	size_t Pieces() const { return m_pieces.size(); }

	VTKPiece& Piece(int n) { return m_pieces[n]; }

public:
	std::vector<VTKPiece>	m_pieces;
};

VTUimport::VTUimport(FSProject& prj) : FEFileImport(prj)
{

}

VTUimport::~VTUimport(void)
{

}

bool VTUimport::Load(const char* szfile)
{
	// Open the file
	XMLReader xml;
	if (xml.Open(szfile, false) == false) return false;

	// get the VTKFile tag
	XMLTag tag;
	if (xml.FindTag("VTKFile", tag) == false) return false;

	// we only support unstructured grids at this point
	const char* sztype = tag.AttributeValue("type", true);
	if (sztype == nullptr) return false;
	if (strcmp(sztype, "UnstructuredGrid") != 0) return false;

	VTKModel vtk;

	// parse the file
	try {
		++tag;
		do
		{
			if (tag == "UnstructuredGrid")
			{
				if (ParseUnstructuredGrid(tag, vtk) == false) return false;
			}
			else return false;
			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{

	}

	xml.Close();

	return BuildMesh(vtk);
}

bool VTUimport::ParseUnstructuredGrid(XMLTag& tag, VTKModel& vtk)
{
	++tag;
	do
	{
		if (tag == "Piece")
		{
			if (ParsePiece(tag, vtk) == false) return false;
		}
		else return false;
		++tag;
	} while (!tag.isend());

	return true;
}

bool VTUimport::ParsePiece(XMLTag& tag, VTKModel& vtk)
{
	VTKPiece piece;
	piece.m_numPoints = tag.AttributeValue<int>("NumberOfPoints", -1);
	if (piece.m_numPoints <= 0) return false;

	piece.m_numCells = tag.AttributeValue<int>("NumberOfCells", -1);
	if (piece.m_numCells <= 0) return false;

	++tag;
	do
	{
		if (tag == "Points")
		{
			if (ParsePoints(tag, piece) == false) return false;
		}
		else if (tag == "Cells")
		{
			if (ParseCells(tag, piece) == false) return false;
		}
		else tag.skip();
	} 
	while (!tag.isend());

	vtk.AddPiece(piece);

	return true;
}

bool VTUimport::ParsePoints(XMLTag& tag, VTKPiece& piece)
{
	++tag;
	do
	{
		if (tag == "DataArray")
		{
			VTKDataArray& points = piece.m_points;
			if (ParseDataArray(tag, points) == false) return false;

			if (points.m_type != VTKDataArray::FLOAT32) return false;
			if (points.m_numComps != 3) return false;
			if (points.m_values_float.size() != piece.m_numPoints * points.m_numComps) return false;
		}
		else tag.skip();
	} while (!tag.isend());

	++tag;

	return true;
}

bool VTUimport::ParseCells(XMLTag& tag, VTKPiece& piece)
{
	++tag;
	do
	{
		if (tag == "DataArray")
		{
			const char* szname = tag.AttributeValue("Name");
			if (strcmp(szname, "connectivity") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_connect) == false) return false;
			}
			else if (strcmp(szname, "offsets") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_offsets) == false) return false;
			}
			else if (strcmp(szname, "types") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_types) == false) return false;
				if (piece.m_cell_types.m_values_int.size() != piece.m_numCells) return errf("Error reading cell types");
			}
		}
		else tag.skip();
	} 
	while (!tag.isend());

	++tag;

	return true;
}

bool VTUimport::ParseDataArray(XMLTag& tag, VTKDataArray& vtkDataArray)
{
	// make sure this is an ascii format
	const char* szformat = tag.AttributeValue("Format", true);
	if (szformat == nullptr) szformat = tag.AttributeValue("format");
	if (strcmp(szformat, "ascii") != 0) return false;
	vtkDataArray.m_format = VTKDataArray::ASCII;

	// get the type
	const char* sztype = tag.AttributeValue("type");
	if      (strcmp(sztype, "Float32") == 0) vtkDataArray.m_type = VTKDataArray::FLOAT32;
	else if (strcmp(sztype, "UInt8"  ) == 0) vtkDataArray.m_type = VTKDataArray::UINT8;
	else if (strcmp(sztype, "Int64"  ) == 0) vtkDataArray.m_type = VTKDataArray::INT64;
	else if (strcmp(sztype, "Int32"  ) == 0) vtkDataArray.m_type = VTKDataArray::INT64;
	else return errf("Unknown data array type %s", sztype);

	// get the number of components
	vtkDataArray.m_numComps = tag.AttributeValue<int>("NumberOfComponents", 1);

	// get the value
	if (vtkDataArray.m_type == VTKDataArray::FLOAT32)
	{
		tag.value(vtkDataArray.m_values_float);
	}
	else if (vtkDataArray.m_type == VTKDataArray::UINT8)
	{
		tag.value2(vtkDataArray.m_values_int);
	}
	else if (vtkDataArray.m_type == VTKDataArray::INT32)
	{
		tag.value2(vtkDataArray.m_values_int);
	}
	else if (vtkDataArray.m_type == VTKDataArray::INT64)
	{
		tag.value2(vtkDataArray.m_values_int);
	}

	// There can be children, so we need to skip this tag
	tag.skip();

	return true;
}

bool VTUimport::BuildMesh(VTKModel& vtk)
{
	FSModel& fem = m_prj.GetFSModel();

	for (int n = 0; n < vtk.Pieces(); ++n)
	{
		VTKPiece& piece = vtk.Piece(n);
	
		// get the number of nodes and elements
		int nodes = piece.Points();
		int elems = piece.Cells();

		// create a new mesh
		FSMesh* pm = new FSMesh();
		pm->Create(nodes, elems);

		// copy nodal data
		for (int i = 0; i < nodes; ++i)
		{
			FSNode& node = pm->Node(i);
			node.r = piece.Point(i);
		}

		// copy element data
		for (int i = 0; i < elems; ++i)
		{
			FSElement& el = pm->Element(i);

			VTKCell cell = piece.Cell(i);

			el.m_gid = cell.m_label; assert(el.m_gid >= 0);
			if (el.m_gid < 0) el.m_gid = 0;

			switch (cell.m_cellType)
			{
			case VTKCell::VTK_TRIANGLE  : el.SetType(FE_TRI3); break;
			case VTKCell::VTK_QUAD      : el.SetType(FE_QUAD4); break;
			case VTKCell::VTK_TETRA     : el.SetType(FE_TET4); break;
			case VTKCell::VTK_HEXAHEDRON: el.SetType(FE_HEX8); break;
			case VTKCell::VTK_WEDGE     : el.SetType(FE_PENTA6); break;
			case VTKCell::VTK_PYRAMID   : el.SetType(FE_PYRA5); break;
			default:
				delete pm;
				return errf("Error trying to build mesh");
			}

			int nn = el.Nodes();
			assert(nn == cell.m_numNodes);
			for (int j = 0; j < nn; ++j) el.m_node[j] = cell.m_node[j];
		}

		pm->RebuildMesh();
		GMeshObject* po = new GMeshObject(pm);
		po->Update();

		char szname[256];
		sprintf(szname, "vtkObject%d", n + 1);
		po->SetName(szname);
		fem.GetModel().AddObject(po);
	}

	return true;
}
