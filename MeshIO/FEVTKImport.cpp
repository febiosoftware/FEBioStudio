#include "FEVTKImport.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModel.h>

#ifdef LINUX
#include <wctype.h>
#endif

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
		int	node[FEElement::MAX_NODES];
	};

public:
	VTKMesh() {}

	int nodes() { return (int)m_nodeList.size(); }
	int cells() { return (int)m_cellList.size(); }

public:
	vector<NODE>	m_nodeList;
	vector<CELL>	m_cellList;
};

FEVTKimport::FEVTKimport(FEProject& prj) : FEFileImport(prj)
{
	m_szline[0] = 0;
}

FEVTKimport::~FEVTKimport(void)
{
}

bool FEVTKimport::nextLine()
{
	const char* ch = nullptr;
	do
	{
		ch = fgets(m_szline, 255, m_fp);
	}
	while (ch && ((*ch == 0) || (*ch == '\n') || (*ch == '\r')));
	return (ch != nullptr);
}

bool FEVTKimport::checkLine(const char* sz)
{
	if (sz == nullptr) return false;
	return (strncmp(m_szline, sz, strlen(sz)) == 0);
}

int FEVTKimport::parseLine(std::vector<std::string>& str)
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

bool FEVTKimport::Load(const char* szfile)
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

bool FEVTKimport::read_POINTS(VTKMesh& vtk)
{
	if (checkLine("POINTS") == false) errf("Error looking for POINTS keyword.");
	if (strstr(m_szline, "float") == nullptr) return errf("Only float data type is supported for POINTS section.");

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

bool FEVTKimport::read_POLYGONS(VTKMesh& vtk)
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

bool FEVTKimport::read_CELLS(VTKMesh& vtk)
{
	if (m_dataSetType != VTK_UNSTRUCTURED_GRID) return errf("Invalid section CELLS");

	int elems = 0;
	int size = 0;
	sscanf(m_szline + 5, "%d %d", &elems, &size);
	if (elems == 0) return errf("Invalid number of cells in CELLS section.");
	vtk.m_cellList.resize(elems);

	for (int i = 0; i < elems; ++i)
	{
		VTKMesh::CELL& cell = vtk.m_cellList[i];
		int* n = cell.node;
		int numNodes;
		if (nextLine() == false) return errf("An unexpected error occured while reading the file data.");
		int nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d%d", &numNodes, &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8], &n[9]);
		cell.numNodes = numNodes;
		cell.cellType = VTK_INVALID; // must be determined by CELL_TYPES
		cell.label = 1;
	}

	return true;
}

bool FEVTKimport::read_CELL_TYPES(VTKMesh& vtk)
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
		default:
			return errf("Unsupported cell type foudn in CELL_TYPES");
		}
	}
	return true;
}

bool FEVTKimport::read_POINT_DATA(VTKMesh& vtkMesh)
{
	if (checkLine("POINT_DATA") == false) return errf("Cannot find POINT_DATA keyword.");

	int nodes = atoi(m_szline + 10);
	if (nodes != vtkMesh.nodes()) return errf("Incorrect number of nodes specified in POINT_DATA.");

	return true;
}

bool FEVTKimport::read_CELL_DATA(VTKMesh& vtkMesh)
{
	if (checkLine("CELL_DATA") == false) return errf("Cannot find POINT_DATA keyword.");

	int cells = atoi(m_szline + 10);
	if (cells != vtkMesh.cells()) return errf("Incorrect number of nodes specified in CELL_DATA.");

	return true;
}

bool FEVTKimport::read_NORMALS(VTKMesh& vtkMesh)
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

bool FEVTKimport::read_FIELD(VTKMesh& vtkMesh)
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

bool FEVTKimport::BuildMesh(VTKMesh& vtk)
{
	FEModel& fem = m_prj.GetFEModel();

	// get the number of nodes and elements
	int nodes = vtk.nodes();
	int elems = vtk.cells();

	// create a new mesh
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	// copy nodal data
	for (int i = 0; i < nodes; ++i)
	{
		FENode& node = pm->Node(i);
		VTKMesh::NODE& vtkNode = vtk.m_nodeList[i];
		node.r = vec3d(vtkNode.r[0], vtkNode.r[1], vtkNode.r[2]);
	}

	// copy element data
	for (int i = 0; i < elems; ++i)
	{
		FEElement& el = pm->Element(i);
		VTKMesh::CELL& cell = vtk.m_cellList[i];

		el.m_gid = cell.label - 1; assert(el.m_gid >= 0);
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