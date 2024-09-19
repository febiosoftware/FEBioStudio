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
#include <FSCore/FileReader.h>
#include "VTKModel.h"

namespace VTK {

	class vtkLegacyFileReader : public FileReader
	{
		enum DataReadMode {
			NONE,
			POINT_DATA,
			CELL_DATA
		};

	public:
		vtkLegacyFileReader();

		bool Load(const char* szfilename) override;

		const vtkModel& GetVTKModel() const;

	protected:
		bool nextLine();

		bool readHeader();

		bool read_POINTS(VTK::vtkPiece& vtk);
		bool read_LINES(VTK::vtkPiece& vtk);
		bool read_POLYGONS(VTK::vtkPiece& vtk);
		bool read_CELLS(VTK::vtkPiece& vtk);
		bool read_CELL_TYPES(VTK::vtkPiece& vtk);
		bool read_POINT_DATA(VTK::vtkPiece& vtk);
		bool read_CELL_DATA(VTK::vtkPiece& vtk);
		bool read_NORMALS(VTK::vtkPiece& vtk);
		bool read_FIELD(VTK::vtkPiece& vtk);
		bool read_SCALARS(VTK::vtkPiece& vtk);
		bool read_VECTORS(VTK::vtkPiece& vtk);
		bool read_TENSORS(VTK::vtkPiece& vtk);
		bool read_METADATA(VTK::vtkPiece& vtk);

		bool checkLine(const char* sz);

		int parseLine(std::vector<std::string>& str);

	protected:
		vtkModel m_vtk;
		vtkDataFileType	m_dataFileType;
		char	m_szline[256];
		DataReadMode	m_dataReadMode;
	};
}
