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
#include "PVDFileReader.h"
#include "PVTUFileReader.h"
#include <XML/XMLReader.h>
using namespace VTK;

PVDFileReader::PVDFileReader()
{

}

bool PVDFileReader::Load(const char* szfile)
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
	if (m_type != Collection) return false;

	// parse the file
	try {
		++tag;
		do
		{
			if (tag == "Collection")
			{
				if (ParseCollection(tag, m_vtk) == false) return false;
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

bool PVDFileReader::ParseCollection(XMLTag& tag, vtkModel& vtk)
{
	++tag;
	do
	{
		if (tag == "DataSet")
		{
			const char* szsrc = tag.AttributeValue("file");

			const char* sztime = tag.AttributeValue("timestep");
			double timeValue = (sztime ? atof(sztime) : 0);

			std::string file;
			if (!m_path.empty())
			{
				file = m_path + szsrc;
			}
			else
			{
				file = szsrc;
			}

			PVTUFileReader vtu;
			if (vtu.Load(file.c_str()) == false) return false;

			const VTK::vtkModel& vtk = vtu.GetVTKModel();

			for (int i = 0; i < vtk.DataSets(); ++i)
			{
				const VTK::vtkDataSet& set_i = vtk.DataSet(i);
				m_vtk.AddDataSet(set_i, timeValue);
			}
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());
	return true;
}
