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
#include <FEBioXML/XMLReader.h>
using namespace VTK;

PVDFileReader::PVDFileReader()
{

}

float PVDFileReader::GetFileProgress() const { return pct; }

bool PVDFileReader::Load(const char* szfile)
{
	pct = 0.f;

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

struct VTKDataSetInfo
{
	std::string filename;
	double timestamp = 0;
	bool success = false;
};

bool PVDFileReader::ParseCollection(XMLTag& tag, vtkModel& vtk)
{
	pct = 0.f;

	std::vector<VTKDataSetInfo> datasets;

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

			VTKDataSetInfo ds;
			ds.filename = file;
			ds.timestamp = timeValue;
			datasets.push_back(ds);
		}
		else tag.skip();
		++tag;

	} while (!tag.isend());

	if (datasets.empty()) return true;

	std::vector<PVTUFileReader*> vtu(datasets.size(), nullptr);
	for (int i = 0; i < datasets.size(); ++i)
	{
		vtu[i] = new PVTUFileReader;
	}

	int numRead = 0;
	int numDataSets = (int)datasets.size();

#pragma omp parallel for shared(numRead)
	for (int i = 0; i < numDataSets; ++i)
	{
		VTKDataSetInfo& ds = datasets[i];
		ds.success = vtu[i]->Load(ds.filename.c_str());

#pragma omp critical
		{
			numRead++;
			pct = (float)numRead / (float)numDataSets;
		}
	}
	pct = 1.f;


	for (int i = 0; i < datasets.size(); ++i)
	{
		const VTK::vtkModel& vtk = vtu[i]->GetVTKModel();

		for (int j = 0; j < vtk.DataSets(); ++j)
		{
			const VTK::vtkDataSet& set_i = vtk.DataSet(j);
			m_vtk.AddDataSet(set_i, datasets[i].timestamp);
		}

		delete vtu[i];
	}

	return true;
}
