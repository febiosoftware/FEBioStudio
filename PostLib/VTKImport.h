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
#include "FEFileReader.h"
#include <vector>

namespace Post {

class FEState;

class VTKimport :	public FEFileReader
{
	class VTKModel;

public:
	VTKimport(FEPostModel* fem);
	~VTKimport(void);

	bool Load(const char* szfile) override;

protected:
	bool readFile(const char* szfile);

	char* readLine(char* szline);
	bool readHeader();
	bool readDataSet  (char* szline);
	bool readPoints   (char* szline);
	bool readPolygons (char* szline);
	bool readCells    (char* szline);
	bool readCellTypes(char* szline);
	bool readPointData(char* szline);
	bool readCellData (char* szline);
	bool readScalars(char* szline);
	bool readVectors(char* szline);
	bool readTensors(char* szline);
	
protected:
	bool BuildMesh();
	bool UpdateModel();
	bool BuildState(double time);
	bool ProcessSeries(const char* szfile);

	FEState*		m_ps;

	bool	m_isPolyData;
	bool	m_isUnstructuredGrid;
	bool	m_readingPointData;
	bool	m_readingCellData;

	double	m_currentTime;
	int		m_fileCount;

	VTKModel* m_vtk;
};
}
