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

class XMLTag;

namespace VTK {

	// Base class for reading XML formatted VTK files
	class VTKFileReader : public FileReader
	{
	protected:
		enum vtkByteOrder {
			LittleEndian,
			BigEndian
		};

		enum vtkCompressor {
			NoCompression,
			ZLibCompression
		};

		enum vtkDataType {
			UInt32,
			UInt64,
			Float32
		};

	public:
		const VTK::vtkModel& GetVTKModel() const;

	protected:
		VTKFileReader();

	protected:
		bool ParseFileHeader(XMLTag& tag);
		bool ParseUnstructuredGrid(XMLTag& tag, VTK::vtkDataSet& vtk);
		bool ParsePolyData(XMLTag& tag, VTK::vtkDataSet& vtk);
		bool ParsePiece(XMLTag& tag, VTK::vtkDataSet& vtk);
		bool ParsePoints(XMLTag& tag, VTK::vtkPiece& piece);
		bool ParseCells(XMLTag& tag, VTK::vtkPiece& piece);
		bool ParsePolys(XMLTag& tag, VTK::vtkPiece& piece);
		bool ParsePointData(XMLTag& tag, VTK::vtkPiece& piece);
		bool ParseCellData(XMLTag& tag, VTK::vtkPiece& piece);
		bool ParseDataArray(XMLTag& tag, VTK::vtkDataArray& vtkDataArray);
		bool ParseAppendedData(XMLTag& tag, VTK::vtkAppendedData& vtkAppendedData);

		bool ProcessAppendedDataArray(VTK::vtkDataArray& ar, VTK::vtkAppendedData& data);
		bool ProcessDataArrays(VTK::vtkDataSet& vtk, VTK::vtkAppendedData& data);

	protected:
		vtkDataFileType	m_type;
		std::string m_version;
		vtkByteOrder m_byteOrder;
		vtkDataType m_headerType;
		vtkCompressor m_compressor;
		vtkModel m_vtk;
	};
}
