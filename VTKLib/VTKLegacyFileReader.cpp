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
#include "VTKLegacyFileReader.h"
using namespace VTK;

vtkLegacyFileReader::vtkLegacyFileReader()
{
	m_dataFileType = VTK::vtkDataFileType::Invalid;
	m_szline[0] = 0;
}

const vtkModel& vtkLegacyFileReader::GetVTKModel() const
{
	return m_vtk;
}

bool vtkLegacyFileReader::Load(const char* szfilename)
{
	m_vtk.Clear();

	if (!Open(szfilename, "rt")) return errf("Failed opening file %s.", szfilename);

	// read the first line 
	if (nextLine() == false) return errf("Unexpected end of file.");
	if (checkLine("# vtk DataFile") == false) return errf("This is not a valid VTK file.");

	// next line is info, so can be skipped
	if (nextLine() == false) return errf("Unexpected end of file.");

	// next line must be ASCII
	if (nextLine() == false) return errf("Unexpected end of file.");
	if (checkLine("ASCII") == false) return errf("Only ASCII VTK files are supported.");

	// read the DATASET line
	if (nextLine() == false) return errf("Unexpected end of file.");
	if (checkLine("DATASET") == false) return errf("Error looking for DATASET keyword.");
	m_dataFileType = VTK::vtkDataFileType::Invalid;
	if (strstr(m_szline, "POLYDATA") != nullptr) m_dataFileType = VTK::vtkDataFileType::PolyData;
	if (strstr(m_szline, "UNSTRUCTURED_GRID") != nullptr) m_dataFileType = VTK::vtkDataFileType::UnstructuredGrid;
	if (m_dataFileType == VTK::vtkDataFileType::Invalid) return errf("Only POLYDATA and UNSTRUCTURED_GRID dataset types are supported.");

	// we need one piece in the model
	vtkPiece piece;

	// start parsing keywords
	if (nextLine() == false) return errf("Unexpected end of file.");
	do
	{
		if (checkLine("POINTS"))
		{
			if (read_POINTS(piece) == false) return false;
		}
		else if (checkLine("LINES"))
		{
			if (read_LINES(piece) == false) return false;
		}
		else if (checkLine("POLYGONS"))
		{
			if (read_POLYGONS(piece) == false)  return false;
		}
		else if (checkLine("CELLS"))
		{
			if (read_CELLS(piece) == false)  return false;
		}
		else if (checkLine("CELL_TYPES"))
		{
			if (read_CELL_TYPES(piece) == false)  return false;
		}
		else if (checkLine("POINT_DATA"))
		{
			if (read_POINT_DATA(piece) == false)  return false;
		}
		else if (checkLine("NORMALS"))
		{
			if (read_NORMALS(piece) == false)  return false;
		}
		else if (checkLine("CELL_DATA"))
		{
			if (read_CELL_DATA(piece) == false)  return false;
		}
		else if (checkLine("FIELD"))
		{
			if (read_FIELD(piece) == false)  return false;
		}
	} while (nextLine());

	Close();

	m_vtk.AddPiece(piece);

	return true;
}

bool vtkLegacyFileReader::nextLine()
{
	const char* ch = nullptr;
	do
	{
		ch = fgets(m_szline, 255, m_fp);
	} while (ch && ((*ch == 0) || (*ch == '\n') || (*ch == '\r')));
	return (ch != nullptr);
}

bool vtkLegacyFileReader::checkLine(const char* sz)
{
	if (sz == nullptr) return false;
	return (strncmp(m_szline, sz, strlen(sz)) == 0);
}

int vtkLegacyFileReader::parseLine(std::vector<std::string>& str)
{
	str.clear();
	const char* ch = m_szline;

	// skip initial whitspace
	while (iswspace(*ch)) ch++;

	string tmp;
	while (*ch)
	{
		if (*ch == ' ')
		{
			str.push_back(tmp);
			tmp.clear();
			while (iswspace(*ch)) ch++;
		}
		else
		{
			if (iswspace(*ch) == 0) tmp.push_back(*ch);
			ch++;
		}
	}
	if (tmp.empty() == false) str.push_back(tmp);

	return (int)str.size();
}

bool vtkLegacyFileReader::read_POINTS(vtkPiece& vtk)
{
	if (checkLine("POINTS") == false) errf("Error looking for POINTS keyword.");
	if ((strstr(m_szline, "float") == nullptr) && (strstr(m_szline, "double") == nullptr))
	{
		// looks like this is attribute is not always present
		// perhaps instead look for nonsupported values instead. 
//		return errf("Only float or double data type is supported for POINTS section.");
	}

	//get number of nodes
	int nodes = atoi(m_szline + 6);
	if (nodes <= 0) return errf("Invalid number of nodes in POINTS section.");

	vtkDataArray& points = vtk.m_points;
	points.init(vtkDataArray::ASCII, vtkDataArray::FLOAT64, 3);
	std::vector<double>& data = points.m_values_float;
	data.reserve(3 * nodes);

	// read the nodes
	double temp[9];
	int nodesRead = 0;
	while (nodesRead < nodes)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

		// There can be up to three nodes defined per line
		int nread = sscanf(m_szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5], &temp[6], &temp[7], &temp[8]);
		if (nread % 3 != 0 && nread > 9)
			return errf("An error occured while reading the nodal coordinates.");

		nodesRead += nread / 3;

		for (int j = 0; j < nread; ++j)
		{
			data.push_back(temp[j]);
		}
	}
	assert(nodesRead == nodes);

	return true;
}

bool vtkLegacyFileReader::read_LINES(vtkPiece& vtk)
{
	if (m_dataFileType != vtkDataFileType::PolyData) return errf("Invalid section POLYDATA.");

	if (checkLine("LINES") == false) errf("Cannot find LINES keyword.");

	int elems = 0;
	int size = 0;
	sscanf(m_szline + 8, "%d %d", &elems, &size);

	vtk.m_cell_connect.init(vtkDataArray::ASCII, vtkDataArray::INT64, 1);
	vtk.m_cell_offsets.init(vtkDataArray::ASCII, vtkDataArray::INT64, 1);

	std::vector<int>& connect = vtk.m_cell_connect.m_values_int;
	std::vector<int>& offsets = vtk.m_cell_offsets.m_values_int;

	int nc = 0;
	for (int i = 0; i < elems; ++i)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

		int numNodes;
		int n[10] = { 0 };
		int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d%d", &numNodes, &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9]);

		nc += numNodes;
		offsets.push_back(nc);
		for (int j = 0; j < numNodes; ++j) connect.push_back(n[j]);
	}

	return true;
}


bool vtkLegacyFileReader::read_POLYGONS(vtkPiece& vtk)
{
	if (m_dataFileType != vtkDataFileType::PolyData) return errf("Invalid section POLYDATA.");

	if (checkLine("POLYGONS") == false) errf("Cannot find POLYGON keyword.");

	int elems = 0;
	int size = 0;
	sscanf(m_szline + 8, "%d %d", &elems, &size);

	vtk.m_cell_connect.init(vtkDataArray::ASCII, vtkDataArray::INT64, 1);
	vtk.m_cell_offsets.init(vtkDataArray::ASCII, vtkDataArray::INT64, 1);
	
	std::vector<int>& connect = vtk.m_cell_connect.m_values_int;
	std::vector<int>& offsets = vtk.m_cell_offsets.m_values_int;

	int nc = 0;
	for (int i = 0; i < elems; ++i)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

		int numNodes;
		int n[10] = { 0 };
		int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d%d", &numNodes, &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9]);

		nc += numNodes;
		offsets.push_back(nc);
		for (int j = 0; j < numNodes; ++j) connect.push_back(n[j]);
	}

	return true;
}

bool vtkLegacyFileReader::read_CELLS(vtkPiece& vtk)
{
	if (m_dataFileType != vtkDataFileType::UnstructuredGrid) return errf("Invalid section CELLS");

	int elems = 0;
	int size = 0;
	sscanf(m_szline + 5, "%d %d", &elems, &size);
	if (elems == 0) return errf("Invalid number of cells in CELLS section.");

	// check for cell offsets
	if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

	vtk.m_cell_connect.init(vtkDataArray::ASCII, vtkDataArray::INT64, 1);
	vtk.m_cell_offsets.init(vtkDataArray::ASCII, vtkDataArray::INT64, 1);

	std::vector<int>& connect = vtk.m_cell_connect.m_values_int;
	std::vector<int>& offsets = vtk.m_cell_offsets.m_values_int;

	if (checkLine("OFFSETS") == true) {
		offsets.resize(elems);
		// read the offsets
		int temp[9];
		int offsetsRead = 0;
		while (offsetsRead < elems)
		{
			if (nextLine() == false) return errf("An unexpected error occured while reading the file data in CELLS OFFSETS section.");

			// There can be up to 9 offsets defined per line
			int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d", &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5], &temp[6], &temp[7], &temp[8]);
			if (nread > 9)
				return errf("An error occured while reading the cell offsets.");

			for (int j = 0; j < nread; ++j)
				offsets[offsetsRead + j] = temp[j];

			offsetsRead += nread;
		}
		assert(offsetsRead == elems);

		// now check for connectivity
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data after the CELLS OFFSETS section.");
		if (checkLine("CONNECTIVITY") == true) {
			connect.resize(size);
			// read the connectivity
			int temp[9];
			int cnctvtyRead = 0;
			while (cnctvtyRead < size)
			{
				if (nextLine() == false) return errf("An unexpected error occured while reading the file data in the CELLS CONNECTIVITY section.");

				// There can be up to 9 offsets defined per line
				int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d", &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5], &temp[6], &temp[7], &temp[8]);
				if (nread > 9)
					return errf("An error occured while reading the cell connectivity.");

				for (int j = 0; j < nread; ++j)
					connect[cnctvtyRead + j] = temp[j];

				cnctvtyRead += nread;
			}
			assert(cnctvtyRead == size);
		}
		else
			return errf("An error occured due to missing or incorrect cell connectivity data.");
	}
	else {
		// read cell data directly from file
		int nc = 0;
		for (int i = 0; i < elems; ++i)
		{
			int n[20] = { 0 };
			int numNodes;
			if ((i > 0) && (nextLine() == false)) return errf("An unexpected error occured while reading the file data in the CELLS section.");
			int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d%d", &numNodes, &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9]);
			nc += numNodes;
			offsets.push_back(nc); 
			for (int j = 0; j < numNodes; ++j) connect.push_back(n[j]);
		}
	}

	return true;
}

bool vtkLegacyFileReader::read_CELL_TYPES(vtkPiece& vtk)
{
	if (m_dataFileType != vtkDataFileType::UnstructuredGrid) return errf("Invalid section CELLS");
	if (checkLine("CELL_TYPES") == false) return errf("Cannot find CELL_TYPES keyword.");

	int elems = atoi(m_szline + 10);

	if (elems != vtk.m_cell_offsets.size()) return errf("Incorrect number of cells in CELL_TYPES.");
	
	vtk.m_cell_types.init(vtkDataArray::ASCII, vtkDataArray::INT64, 1);
	std::vector<int>& cellType = vtk.m_cell_types.m_values_int;

	for (int i = 0; i < elems; ++i)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
		int n = atoi(m_szline);
		cellType.push_back(n);
	}
	return true;
}

bool vtkLegacyFileReader::read_POINT_DATA(vtkPiece& vtk)
{
	if (checkLine("POINT_DATA") == false) return errf("Cannot find POINT_DATA keyword.");

	int nodes = atoi(m_szline + 10);
	if (nodes != vtk.Points()) return errf("Incorrect number of nodes specified in POINT_DATA.");

	return true;
}

bool vtkLegacyFileReader::read_NORMALS(vtkPiece& vtk)
{
	std::vector<string> att;
	int nread = parseLine(att);
	if (att[0] != "NORMALS") return errf("Cannot find NORMALS keyword.");

	if ((nread == 3) && (att[2] != "float")) return errf("Only floats supported in NORMALS");

	int nodes = (int)vtk.Points();
	int lines = nodes / 3;
	if ((nodes % 3) != 0) lines++;
	for (int i = 0; i < lines; ++i)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

		// TODO: Do something with this data
	}

	return true;
}

bool vtkLegacyFileReader::read_CELL_DATA(vtkPiece& vtkMesh)
{
	if (checkLine("CELL_DATA") == false) return errf("Cannot find CELL_DATA keyword.");

	int cells = atoi(m_szline + 10);
	if (cells != vtkMesh.Cells()) return errf("Incorrect number of cells specified in CELL_DATA.");

	if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
	if (strncmp(m_szline, "SCALARS", 7) == 0)
	{
		std::vector<string> att;
		parseLine(att);
		if (att[2] == "int")
		{
			if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
			if (strncmp(m_szline, "LOOKUP_TABLE", 12) == 0)
			{
				// read the offsets
				int temp[9];
				int idsRead = 0;
				while (idsRead < cells)
				{
					if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

					// There can be up to 9 offsets defined per line
					int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d", &temp[0], &temp[1], &temp[2], &temp[3], &temp[4], &temp[5], &temp[6], &temp[7], &temp[8]);
					if (nread > 9)
						return errf("An error occured while reading the nodal coordinates.");

					for (int j = 0; j < nread; ++j) {
//						VTKMesh::CELL& cell = vtkMesh.m_cellList[idsRead + j];
//						cell.label = temp[j];
					}

					idsRead += nread;
				}
				assert(idsRead == cells);

			}
		}
	}

	return true;
}

bool vtkLegacyFileReader::read_FIELD(vtkPiece& vtkMesh)
{
	std::vector<string> att;
	int nread = parseLine(att);
	if (att[0] != "FIELD") return errf("Cannot find FIELD keyword.");
	int numArrays = atoi(att[2].c_str());

	for (int n = 0; n < numArrays; ++n)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
		std::vector<string> att;
		int nread = parseLine(att);
		if (nread < 3) return errf("Invalid number of attributes in field definition.");

		int numComp = atoi(att[1].c_str());
		int numTuples = atoi(att[2].c_str());

		bool isLabels = ((att[0] == "labels") && (numComp == 1) && (numTuples == vtkMesh.Cells()));

		double v[9];
		int nsize = numComp * numTuples;
		int nreadTotal = 0;
		while (nreadTotal < nsize)
		{
			if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

			int nread = sscanf(m_szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6, v + 7, v + 8);

			// TODO: Do something with this data
			if (isLabels)
			{
//				for (int i = 0; i < nread; ++i) vtkMesh.m_cellList[nreadTotal + i].label = (int)v[i];
			}
			nreadTotal += nread;
		}
		assert(nreadTotal == nsize);
	}
	return true;
}
