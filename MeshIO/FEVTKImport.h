#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <vector>
using namespace std;

class VTKMesh;

class FEVTKimport :	public FEFileImport
{

public:
	FEVTKimport(FEProject& prj);
	~FEVTKimport(void);

	bool Load(const char* szfile);

private:
	bool nextLine();

	bool read_POINTS(VTKMesh& vtkMesh);
	bool read_POLYGONS(VTKMesh& vtkMesh);
	bool read_CELLS(VTKMesh& vtkMesh);
	bool read_CELL_TYPES(VTKMesh& vtkMesh);
	bool read_POINT_DATA(VTKMesh& vtkMesh);
	bool read_CELL_DATA(VTKMesh& vtkMesh);
	bool read_NORMALS(VTKMesh& vtkMesh);
	bool read_FIELD(VTKMesh& vtkMesh);

	bool BuildMesh(VTKMesh& vtkMesh);

	bool checkLine(const char* sz);

	int parseLine(std::vector<std::string>& str);

private:
	char	m_szline[256];
	int		m_dataSetType;
};
