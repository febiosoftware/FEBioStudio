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
#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <vector>
//using namespace std;

class VTKMesh;

class FEVTKimport :	public FSFileImport
{

public:
	FEVTKimport(FSProject& prj);
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
