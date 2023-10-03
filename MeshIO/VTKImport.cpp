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

#include "VTKImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>

#ifdef LINUX
#include <wctype.h>
#endif
using namespace std;

enum VTK_DATASET_TYPE {
	VTK_INVALID,
	VTK_POLYDATA,
	VTK_UNSTRUCTURED_GRID
};
enum VTK_CELL_TYPE {
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

class VTKMesh
{
public:
	struct NODE {
		double r[3];
	};

	struct CELL
	{
		int	label;
		int	numNodes;
		int cellType;
		int	node[FSElement::MAX_NODES];
	};

public:
	VTKMesh() {}

	int nodes() { return (int)m_nodeList.size(); }
	int cells() { return (int)m_cellList.size(); }

public:
	vector<NODE>	m_nodeList;
	vector<CELL>	m_cellList;
    vector<int>     m_celloffsets;
    vector<int>     m_cellcnctvty;
};

VTKimport::VTKimport(FSProject& prj) : FSFileImport(prj)
{
	m_szline[0] = 0;
}

VTKimport::~VTKimport(void)
{
}

bool VTKimport::nextLine()
{
	const char* ch = nullptr;
	do
	{
		ch = fgets(m_szline, 255, m_fp);
	}
	while (ch && ((*ch == 0) || (*ch == '\n') || (*ch == '\r')));
	return (ch != nullptr);
}

bool VTKimport::checkLine(const char* sz)
{
	if (sz == nullptr) return false;
	return (strncmp(m_szline, sz, strlen(sz)) == 0);
}

int VTKimport::parseLine(std::vector<std::string>& str)
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

	return str.size();
}

bool VTKimport::Load(const char* szfile)
{
	if (!Open(szfile, "rt")) return errf("Failed opening file %s.", szfile);

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
	m_dataSetType = VTK_INVALID;
	if (strstr(m_szline, "POLYDATA") != nullptr) m_dataSetType = VTK_POLYDATA;
	if (strstr(m_szline, "UNSTRUCTURED_GRID") != nullptr) m_dataSetType = VTK_UNSTRUCTURED_GRID;
	if (m_dataSetType == VTK_INVALID) return errf("Only POLYDATA and UNSTRUCTURED_GRID dataset types are supported.");

	// VTK mesh data
	VTKMesh vtk;

	// start parsing keywords
	if (nextLine() == false) return errf("Unexpected end of file.");
	do
	{
		if (checkLine("POINTS"))
		{
			if (read_POINTS(vtk) == false) return false;
		}
		else if (checkLine("POLYGONS"))
		{
			if (read_POLYGONS(vtk) == false)  return false;
		}
		else if (checkLine("CELLS"))
		{
			if (read_CELLS(vtk) == false)  return false;
		}
		else if (checkLine("CELL_TYPES"))
		{
			if (read_CELL_TYPES(vtk) == false)  return false;
		}
		else if (checkLine("POINT_DATA"))
		{
			if (read_POINT_DATA(vtk) == false)  return false;
		}
		else if (checkLine("NORMALS"))
		{
			if (read_NORMALS(vtk) == false)  return false;
		}
		else if (checkLine("CELL_DATA"))
		{
			if (read_CELL_DATA(vtk) == false)  return false;
		}
		else if (checkLine("FIELD"))
		{
			if (read_FIELD(vtk) == false)  return false;
		}
	}
	while (nextLine());

	Close();

	return BuildMesh(vtk);
}

bool VTKimport::read_POINTS(VTKMesh& vtk)
{
	if (checkLine("POINTS") == false) errf("Error looking for POINTS keyword.");
	if ((strstr(m_szline, "float") == nullptr) && (strstr(m_szline, "double") == nullptr))
	{
		return errf("Only float or double data type is supported for POINTS section.");
	}

	//get number of nodes
	int nodes = atoi(m_szline + 6);
	if (nodes <= 0) return errf("Invalid number of nodes in POINTS section.");

	vtk.m_nodeList.resize(nodes);

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

		int nodes_in_row = nread / 3;

		for (int j = 0, k = 0; j < nodes_in_row; ++j)
		{
			VTKMesh::NODE& node = vtk.m_nodeList[nodesRead++];
			node.r[0] = temp[k];
			node.r[1] = temp[k + 1];
			node.r[2] = temp[k + 2];
			k += 3;
		}
	}
	assert(nodesRead == nodes);

	return true;
}

bool VTKimport::read_POLYGONS(VTKMesh& vtk)
{
	if (m_dataSetType != VTK_POLYDATA) return errf("Invalid section POLYDATA.");

	if (checkLine("POLYGONS") == false) errf("Cannot find POLYGON keyword.");

	int elems = 0;
	int size = 0;
	sscanf(m_szline + 8, "%d %d", &elems, &size);
	vtk.m_cellList.resize(elems);

	for (int i = 0; i < elems; ++i)
	{
		VTKMesh::CELL& cell = vtk.m_cellList[i];
		int* n = cell.node;
		int numNodes;
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

		int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d%d", &numNodes, &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9]);
		cell.numNodes = numNodes;
		cell.label = 1;
		if (numNodes == 3) cell.cellType = VTK_TRIANGLE;
		else if (numNodes == 4) cell.cellType = VTK_QUAD;
		else return errf("Invalid polygon type.");
	}

	return true;
}

bool VTKimport::read_CELLS(VTKMesh& vtk)
{
	if (m_dataSetType != VTK_UNSTRUCTURED_GRID) return errf("Invalid section CELLS");

	int elems = 0;
	int size = 0;
	sscanf(m_szline + 5, "%d %d", &elems, &size);
	if (elems == 0) return errf("Invalid number of cells in CELLS section.");

    // check for cell offsets
    if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
    
    if (checkLine("OFFSETS") == true) {
        vtk.m_celloffsets.resize(elems);
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
                vtk.m_celloffsets[offsetsRead+j] = temp[j];
            
            offsetsRead += nread;
        }
        assert(offsetsRead == elems);
        
        // now check for connectivity
        if (nextLine() == false) return errf("An unexpected error occured while reading the file data after the CELLS OFFSETS section.");
        if (checkLine("CONNECTIVITY") == true) {
            vtk.m_cellcnctvty.resize(size);
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
                    vtk.m_cellcnctvty[cnctvtyRead+j] = temp[j];
                
                cnctvtyRead += nread;
            }
            assert(cnctvtyRead == size);
        }
        else
            return errf("An error occured due to missing or incorrect cell connectivity data.");
        
        // now generate cell data from offsets and connectivity
        // adjust element number by one
        elems--;
        vtk.m_cellList.resize(elems);
        int N = 0;
        for (int i = 0; i < elems; ++i)
        {
            VTKMesh::CELL& cell = vtk.m_cellList[i];
            int* n = cell.node;
            int numNodes = vtk.m_celloffsets[i+1] - vtk.m_celloffsets[i];
            for (int i=0; i<numNodes; ++i)
                n[i] = vtk.m_cellcnctvty[N+i];
            N += numNodes;
            cell.numNodes = numNodes;
            cell.cellType = VTK_INVALID; // must be determined by CELL_TYPES
            cell.label = 1;
        }
    }
    else {
        // read cell data directly from file
        vtk.m_cellList.resize(elems);
        for (int i = 0; i < elems; ++i)
        {
            VTKMesh::CELL& cell = vtk.m_cellList[i];
            int* n = cell.node;
            int numNodes;
            if ((i > 0) && (nextLine() == false)) return errf("An unexpected error occured while reading the file data in the CELLS section.");
            int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d%d", &numNodes, &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9]);
            cell.numNodes = numNodes;
            cell.cellType = VTK_INVALID; // must be determined by CELL_TYPES
            cell.label = 1;
        }
    }

	return true;
}

bool VTKimport::read_CELL_TYPES(VTKMesh& vtk)
{
	if (m_dataSetType != VTK_UNSTRUCTURED_GRID) return errf("Invalid section CELLS");
	if (checkLine("CELL_TYPES") == false) return errf("Cannot find CELL_TYPES keyword.");

	int elems = atoi(m_szline + 10);
	if (elems != vtk.m_cellList.size()) return errf("Incorrect number of cells in CELL_TYPES.");

	for (int i = 0; i < elems; ++i)
	{
		VTKMesh::CELL& cell = vtk.m_cellList[i];
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
		int n = atoi(m_szline);
		switch (n)
		{
		case VTK_TETRA     : cell.cellType = VTK_TETRA; break;
		case VTK_HEXAHEDRON: cell.cellType = VTK_HEXAHEDRON; break;
		case VTK_WEDGE     : cell.cellType = VTK_WEDGE; break;
		case VTK_PYRAMID   : cell.cellType = VTK_PYRAMID; break;
        case VTK_QUAD      : cell.cellType = VTK_QUAD; break;
        case VTK_TRIANGLE  : cell.cellType = VTK_TRIANGLE; break;
		default:
			return errf("Unsupported cell type foudn in CELL_TYPES");
		}
	}
	return true;
}

bool VTKimport::read_POINT_DATA(VTKMesh& vtkMesh)
{
	if (checkLine("POINT_DATA") == false) return errf("Cannot find POINT_DATA keyword.");

	int nodes = atoi(m_szline + 10);
	if (nodes != vtkMesh.nodes()) return errf("Incorrect number of nodes specified in POINT_DATA.");

	return true;
}

bool VTKimport::read_CELL_DATA(VTKMesh& vtkMesh)
{
	if (checkLine("CELL_DATA") == false) return errf("Cannot find CELL_DATA keyword.");

	int cells = atoi(m_szline + 10);
	if (cells != vtkMesh.cells()) return errf("Incorrect number of cells specified in CELL_DATA.");

	if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
	if (strncmp(m_szline, "SCALARS", 7) == 0)
	{
		vector<string> att;
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
                        VTKMesh::CELL& cell = vtkMesh.m_cellList[idsRead+j];
                        cell.label = temp[j];
                    }
                    
                    idsRead += nread;
                }
                assert(idsRead == cells);
                
			}
		}
	}

	return true;
}

bool VTKimport::read_NORMALS(VTKMesh& vtkMesh)
{
	vector<string> att;
	int nread = parseLine(att);
	if (att[0] != "NORMALS") return errf("Cannot find NORMALS keyword.");

	if ((nread == 3) && (att[2] != "float")) return errf("Only floats supported in NORMALS");

	int nodes = vtkMesh.nodes();
	int lines = nodes / 3;
	if ((nodes % 3) != 0) lines++;
	for (int i = 0; i < lines; ++i)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

		// TODO: Do something with this data
	}

	return true;
}

bool VTKimport::read_FIELD(VTKMesh& vtkMesh)
{
	vector<string> att;
	int nread = parseLine(att);
	if (att[0] != "FIELD") return errf("Cannot find FIELD keyword.");
	int numArrays = atoi(att[2].c_str());

	for (int n = 0; n < numArrays; ++n)
	{
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
		vector<string> att;
		int nread = parseLine(att);
		if (nread < 3) return errf("Invalid number of attributes in field definition.");

		int numComp = atoi(att[1].c_str());
		int numTuples = atoi(att[2].c_str());

		bool isLabels = ((att[0] == "labels") && (numComp == 1) && (numTuples == vtkMesh.cells()));

		double v[9];
		int nsize = numComp*numTuples;
		int nreadTotal = 0;
		while (nreadTotal < nsize)
		{
			if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");

			int nread = sscanf(m_szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", v, v+1, v+2, v+3, v+4, v+5, v+6, v+7, v+8);

			// TODO: Do something with this data
			if (isLabels)
			{
				for (int i = 0; i < nread; ++i) vtkMesh.m_cellList[nreadTotal + i].label = (int)v[i];
			}
			nreadTotal += nread;
		}
		assert(nreadTotal == nsize);
	}
    return true;
}

bool VTKimport::BuildMesh(VTKMesh& vtk)
{
	FSModel& fem = m_prj.GetFSModel();

	// get the number of nodes and elements
	int nodes = vtk.nodes();
	int elems = vtk.cells();

	// create a new mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// copy nodal data
	for (int i = 0; i < nodes; ++i)
	{
		FSNode& node = pm->Node(i);
		VTKMesh::NODE& vtkNode = vtk.m_nodeList[i];
		node.r = vec3d(vtkNode.r[0], vtkNode.r[1], vtkNode.r[2]);
	}

	// copy element data
	for (int i = 0; i < elems; ++i)
	{
		FSElement& el = pm->Element(i);
		VTKMesh::CELL& cell = vtk.m_cellList[i];

		el.m_gid = cell.label; assert(el.m_gid >= 0);
		if (el.m_gid < 0) el.m_gid = 0;

		switch (cell.cellType)
		{
		case VTK_TRIANGLE  : el.SetType(FE_TRI3  ); break;
		case VTK_QUAD      : el.SetType(FE_QUAD4 ); break;
		case VTK_TETRA     : el.SetType(FE_TET4  ); break;
		case VTK_HEXAHEDRON: el.SetType(FE_HEX8  ); break;
		case VTK_WEDGE     : el.SetType(FE_PENTA6); break;
		case VTK_PYRAMID   : el.SetType(FE_PYRA5 ); break;
		default:
			delete pm;
			return errf("Error trying to build mesh");
		}

		int nn = el.Nodes();
		assert(nn == cell.numNodes);
		for (int j = 0; j < nn; ++j) el.m_node[j] = cell.node[j];
	}

	pm->RebuildMesh();
	GMeshObject* po = new GMeshObject(pm);
	po->Update();

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
