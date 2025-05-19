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
#include "PVTUFileReader.h"
#include "VTUFileReader.h"
#include <XML/XMLReader.h>
using namespace VTK;

std::string getFilePath(const std::string& file)
{
	size_t pos1 = file.find_last_of("\\");
	size_t pos2 = file.find_last_of("/");
	size_t pos;
	if ((pos1 == std::string::npos) && (pos2 == std::string::npos)) return "";
	if ((pos1 != std::string::npos) && (pos2 != std::string::npos))
	{
		pos = std::max(pos1, pos2);
	}
	else if (pos1 != std::string::npos)
	{
		pos = pos1;
	}
	else
	{
		pos = pos2;
	}

	std::string path = file.substr(0, pos+1);
	return path;
}

PVTUFileReader::PVTUFileReader()
{

}

bool PVTUFileReader::Load(const char* szfile)
{
	m_vtk.Clear();

	// extract the filepath
	m_path = getFilePath(szfile);

	// Open the file
	XMLReader xml;
	if (xml.Open(szfile, false) == false) return false;

	// get the VTKFile tag
	XMLTag tag;
	if (xml.FindTag("VTKFile", tag) == false) return false;
	if (ParseFileHeader(tag) == false) return false;

	// This reader is for unstructured grids at this point
	if (m_type != PUnstructuredGrid) return false;

	// parse the file
	try {
		++tag;
		do
		{
			if (tag == "PUnstructuredGrid")
			{
				if (ParsePUnstructuredGrid(tag, m_vtk) == false) return false;
			}
			else return false;
			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{

	}
	xml.Close();

	return true;
}

bool PVTUFileReader::ParsePUnstructuredGrid(XMLTag& tag, vtkModel& vtk)
{
	++tag;
	do
	{
		if (tag == "Piece")
		{
			const char* szsrc = tag.AttributeValue("Source");

			std::string file;
			if (!m_path.empty())
			{
				file = m_path + szsrc;
			}
			else
			{
				file = szsrc;
			}

			VTUFileReader vtu;
			if (vtu.Load(file.c_str()) == false) return false;

			const VTK::vtkModel& vtk = vtu.GetVTKModel();

			for (int i = 0; i < vtk.Pieces(); ++i)
			{
				const VTK::vtkPiece& piece_i = vtk.Piece(i);

				if (m_vtk.Pieces() == 0) m_vtk.AddPiece(piece_i);
				else
				{
					vtkPiece& piece = m_vtk.Piece(0);
					piece.Merge(piece_i);
				}
			}
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());
	return true;
}
