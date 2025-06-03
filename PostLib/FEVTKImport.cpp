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

#include "stdafx.h"
#include "VTKImport.h"
#include "FEMeshData_T.h"
#include "FEPostModel.h"
#include "FEState.h"
#include <VTKLib/VTKLegacyFileReader.h>
#include <VTKLib/VTUFileReader.h>
#include <VTKLib/PVTUFileReader.h>
#include <VTKLib/PVDFileReader.h>
#include <VTKLib/VTKTools.h>
#include <XML/XMLReader.h>

using namespace Post;
using namespace std;

VTKFileImport::VTKFileImport(FEPostModel* fem) : FEFileReader(fem)
{
	m_ps = nullptr;
	m_currentTime = 0.0;
	m_fileCount = 0;
}

VTKFileImport::~VTKFileImport(void)
{
}

float VTKFileImport::GetFileProgress() const
{
	if (m_vtkReader) return m_vtkReader->GetFileProgress();
	return 1.f;
}

bool VTKFileImport::Load(const char* szfile)
{
	VTK::vtkModel vtk;
	if (!LoadVTKModel(szfile, vtk)) return false;

	if (vtk.DataSets() == 0) return false;

	const VTK::vtkDataSet& dataSet = vtk.DataSet(0);

	const VTK::vtkPiece& piece = dataSet.Piece(0);
	if (!BuildMesh(piece)) return false;
	if (!UpdateModel(piece)) return false;

	for (int i = 0; i < vtk.DataSets(); ++i)
	{
		const VTK::vtkDataSet& dataSet = vtk.DataSet(i);
		const VTK::vtkPiece& piece = dataSet.Piece(0);
		if (!BuildState(dataSet.m_time, piece)) return false;
	}

	// This file might be part of a series, so check and try to read it in
	if (m_processSeries)
		if (ProcessSeries(szfile) == false) return false;

	return true;
}

bool VTKFileImport::ProcessSeries(const char* szfile)
{
	FEPostModel& fem = *m_fem;

	// see if this file could be part of a series. 
	bool bseries = false;
	int nroot = -1;
	int ndigits = 0;
	char base[1024] = { 0 };
	const char* ext = strrchr(szfile, '.');
	if (ext)
	{
		const char* c = ext;
		c--;
		if (isdigit(*c))
		{
			while (isdigit(*c) && (c > szfile)) c--;
			nroot = atoi(c + 1);
			bseries = true;
			ndigits = ext - c - 1;
			strncpy(base, szfile, c - szfile + 1);
		}
	}

	if (bseries)
	{
		// create the format string
		char szfmt[1024] = { 0 };
		sprintf(szfmt, "%s%%0%dd%s", base, ndigits, ext);

		char szfilen[1024] = { 0 };

		m_fileCount = nroot;
		do {
			m_fileCount++;
			sprintf(szfilen, szfmt, m_fileCount);

			m_currentTime = m_fileCount;

			VTK::vtkModel vtk;
			if (!LoadVTKModel(szfilen, vtk))
			{
				// Let's assume that the file was not found, so we probably 
				// reached the end of the series. No reason to make a fuss about that. 
				ClearErrors();
				break;
			}
			else
			{
				const VTK::vtkDataSet& dataSet = vtk.DataSet(0);
				const VTK::vtkPiece& piece = dataSet.Piece(0);

				// some sanity checks
				FSMesh* pm = fem.GetFEMesh(0);
				if (piece.Points() != pm->Nodes()) break;
				if (piece.Cells() != pm->Elements()) break;

				// get the current time from the title
				const char* sztitle = vtk.m_title.c_str();
				if (strncmp(sztitle, "time", 4) == 0)
				{
					m_currentTime = atof(sztitle + 4);
				}

				// build the state
				if (BuildState(m_currentTime, piece) == false) return false;
			}
		} while (true);
	}

	return true;
}

bool VTKFileImport::BuildMesh(const VTK::vtkPiece& vtk)
{
	// create a new mesh
	int nodes = (int)vtk.Points();
	int elems = (int)vtk.Cells();
	FSMesh* pm = new FSMesh;

	// Build the FE mesh from the VTK mesh
	// don't split polys since otherwise this could mess up the data counts
	if (VTKTools::BuildFEMesh(vtk, pm, m_nodeMap, false, m_bmapNodes) == false)
	{
		delete pm;
		return false;
	}

	// We need to build the mesh before allocating a state so that we have 
	// the faces. 
	pm->RebuildMesh();

	FEPostModel& fem = *m_fem;
	fem.AddMesh(pm);

	// count the parts
	int nparts = pm->CountElementPartitions();

	// add one material for each part
	for (int i = 0; i < nparts; ++i)
	{
		Material mat;
		fem.AddMaterial(mat);
	}

	// the mesh will be partitioned in the BuildMesh function based on the material IDs
	// so, we have to match the material IDs to the part IDs
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		el.m_MatID = el.m_gid;
	}

	// make sure the mat IDs are 0-based and sequential
	int minId, maxId;
	for (int i = 0; i < pm->Elements(); ++i)
	{
		int mid = pm->Element(i).m_MatID;
		if ((i == 0) || (mid < minId)) minId = mid;
		if ((i == 0) || (mid > maxId)) maxId = mid;
	}
	int nsize = maxId - minId + 1;
	vector<int> lut(nsize, 0);
	for (int i = 0; i < pm->Elements(); ++i)
	{
		int mid = pm->Element(i).m_MatID - minId;
		lut[mid]++;
	}
	int m = 0;
	for (int i = 0; i < nsize; ++i)
	{
		if (lut[i] > 0) lut[i] = m++;
		else lut[i] = -1;
	}
	for (int i = 0; i < pm->Elements(); ++i)
	{
		int mid = pm->Element(i).m_MatID - minId;
		pm->Element(i).m_MatID = lut[mid]; assert(lut[mid] >= 0);
	}

	// update the mesh
	pm->RebuildMesh();
	fem.UpdateBoundingBox();

	return true;
}

bool VTKFileImport::UpdateModel(const VTK::vtkPiece& vtk)
{
	FEPostModel& fem = *m_fem;
	for (int i = 0; i < vtk.m_pointData.size(); ++i)
	{
		const VTK::vtkDataArray& data = vtk.m_pointData[i];
		if ((data.m_type == VTK::vtkDataArray::FLOAT32) || (data.m_type == VTK::vtkDataArray::FLOAT64))
		{

			switch (data.m_numComps)
			{
			case 1: fem.AddDataField(new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA), data.m_name); break;
			case 3: fem.AddDataField(new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA), data.m_name); break;
			case 9: fem.AddDataField(new FEDataField_T<FENodeData<mat3f> >(&fem, EXPORT_DATA), data.m_name); break;
			default:
				assert(false);
				return false;
			}
		}
	}
	for (int i = 0; i < vtk.m_cellData.size(); ++i)
	{
		const VTK::vtkDataArray& data = vtk.m_cellData[i];
		if ((data.m_type == VTK::vtkDataArray::FLOAT32) || (data.m_type == VTK::vtkDataArray::FLOAT64))
		{
			switch (data.m_numComps)
			{
			case 1: fem.AddDataField(new FEDataField_T<FEElementData<float, DATA_ITEM> >(&fem, EXPORT_DATA), data.m_name); break;
			case 3: fem.AddDataField(new FEDataField_T<FEElementData<vec3f, DATA_ITEM> >(&fem, EXPORT_DATA), data.m_name); break;
			case 9: fem.AddDataField(new FEDataField_T<FEElementData<mat3f, DATA_ITEM> >(&fem, EXPORT_DATA), data.m_name); break;
			default:
				assert(false);
				return false;
			}
		}
	}

	return true;
}

bool VTKFileImport::BuildState(double time, const VTK::vtkPiece& vtk)
{
	// add a state
	FEState* ps = new FEState((float)time, m_fem, m_fem->GetFEMesh(0));
	m_ps = ps;

	FEPostModel& fem = *m_fem;
	fem.AddState(ps);

	FSMesh* pm = ps->GetFEMesh();
	int nodes = pm->Nodes();
	int elems = pm->Elements();

	int vtkNodes = vtk.Points();

	int nfield = 0;
	for (int i = 0; i < vtk.m_pointData.size(); ++i)
	{
		const VTK::vtkDataArray& data = vtk.m_pointData[i];
		if ((data.m_type == VTK::vtkDataArray::FLOAT32) || (data.m_type == VTK::vtkDataArray::FLOAT64))
		{
			if (data.m_numComps == 1)
			{
				const std::vector<double>& val = data.m_values_float;
				FENodeData<float>& df = dynamic_cast<FENodeData<float>&>(ps->m_Data[nfield++]);
				for (int j = 0; j < pm->Nodes(); ++j) df[j] = (float)val[m_nodeMap[j]];
			}
			else if (data.m_numComps == 3)
			{
				const std::vector<double>& val = data.m_values_float;
				FENodeData<vec3f>& df = dynamic_cast<FENodeData<vec3f>&>(ps->m_Data[nfield++]);
				for (int j = 0; j < pm->Nodes(); ++j)
				{
					int n = m_nodeMap[j];
					vec3f v;
					v.x = (float)val[3*n  ];
					v.y = (float)val[3*n+1];
					v.z = (float)val[3*n+2];
					df[j] = v;
				}
			}
			else if (data.m_numComps == 9)
			{
				const std::vector<double>& val = data.m_values_float;
				FENodeData<mat3f>& df = dynamic_cast<FENodeData<mat3f>&>(ps->m_Data[nfield++]);
				for (int j = 0; j < pm->Nodes(); ++j)
				{
					int n = m_nodeMap[j];
					mat3f v;
					v[0][0] = (float)val[9*n  ]; v[0][1] = (float)val[9*n+1]; v[0][2] = (float)val[9*n+2];
					v[1][0] = (float)val[9*n+3]; v[1][1] = (float)val[9*n+4]; v[1][2] = (float)val[9*n+5];
					v[2][0] = (float)val[9*n+6]; v[2][1] = (float)val[9*n+7]; v[2][2] = (float)val[9*n+8];
					df[j] = v;
				}
			}
		}
	}
	for (int i = 0; i < vtk.m_cellData.size(); ++i)
	{
		const VTK::vtkDataArray& data = vtk.m_cellData[i];
		if ((data.m_type == VTK::vtkDataArray::FLOAT32) || (data.m_type == VTK::vtkDataArray::FLOAT64))
		{
			if (data.m_numComps == 1)
			{
				const std::vector<double>& val = data.m_values_float;
				FEElementData<float, DATA_ITEM>& ed = dynamic_cast<FEElementData<float, DATA_ITEM>&>(ps->m_Data[nfield++]);
				for (int j = 0; j < elems; ++j) ed.add(j, (float)val[j]);
			}
			else if (data.m_numComps == 3)
			{
				const std::vector<double>& val = data.m_values_float;
				FEElementData<vec3f, DATA_ITEM>& ed = dynamic_cast<FEElementData<vec3f, DATA_ITEM>&>(ps->m_Data[nfield++]);
				for (int j = 0; j < elems; ++j)
				{
					vec3f v;
					v.x = (float)val[3*j  ];
					v.y = (float)val[3*j+1];
					v.z = (float)val[3*j+2];
					ed.add(j, v);
				}
			}
			else if (data.m_numComps == 9)
			{
				const std::vector<double>& val = data.m_values_float;
				FEElementData<mat3f, DATA_ITEM>& ed = dynamic_cast<FEElementData<mat3f, DATA_ITEM>&>(ps->m_Data[nfield++]);
				for (int j = 0; j < elems; ++j)
				{
					mat3f v;
					v[0][0] = (float)val[9*j  ]; v[0][1] = (float)val[9*j+1]; v[0][2] = (float)val[9*j+2];
					v[1][0] = (float)val[9*j+3]; v[1][1] = (float)val[9*j+4]; v[1][2] = (float)val[9*j+5];
					v[2][0] = (float)val[9*j+6]; v[2][1] = (float)val[9*j+7]; v[2][2] = (float)val[9*j+8];
					ed.add(j, v);
				}
			}
		}
	}

	return true;
}

bool VTKImport::LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk)
{
	VTK::vtkLegacyFileReader vtkReader;
	if (!vtkReader.Load(szfilename))
	{
		setErrorString(vtkReader.GetErrorString());
		return false;
	}

	vtk = vtkReader.GetVTKModel();
	return true;
}

bool VTUImport::LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk)
{
	VTK::VTUFileReader vtuReader;
	if (!vtuReader.Load(szfilename))
	{
		setErrorString(vtuReader.GetErrorString());
		return false;
	}

	vtk = vtuReader.GetVTKModel();
	return true;
}

VTMImport::VTMImport(FEPostModel* fem) : VTKFileImport(fem)
{
}

bool VTMImport::LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk)
{
	XMLReader xml;
	if (xml.Open(szfilename, false) == false) return errf("Failed to open file.");

	XMLTag tag;
	if (xml.FindTag("VTKFile", tag) == false) return errf("This is not a valid VTK multi-block dataset file.");
	const char* sztype = tag.AttributeValue("type");
	if (strcmp(sztype, "vtkMultiBlockDataSet") != 0) return errf("This is not a valid VTK multi-block dataset file.");

	string datafile;
	double timeValue = 0.0;

	++tag;
	do
	{
		if (tag == "vtkMultiBlockDataSet")
		{
			++tag;
			do
			{
				if (tag == "Block")
				{
					++tag;
					do
					{
						if (tag == "DataSet")
						{
							datafile = tag.AttributeValue("file");
						}
						else tag.skip();
						++tag;
					} while (!tag.isend());

				}
				else tag.skip();
				++tag;
			} while (!tag.isend());
		}
		else if (tag == "FieldData")
		{
			++tag;
			do
			{
				if (tag == "DataArray")
				{
					tag.value(timeValue);
				}
				else tag.skip();
				++tag;
			} while (!tag.isend());

		}
		else tag.skip();
		++tag;
	} while (!tag.isend());

	// extract the path from the filename
	char filePath[512] = { 0 };
	const char* ch = strrchr(szfilename, '/');
	if (ch == nullptr) ch = strrchr(szfilename, '\\');
	if (ch)
	{
		size_t n = ch - szfilename + 1;
		strncpy(filePath, szfilename, n);
	}

	string dataPath = string(filePath) + datafile;

	// Now let's try to read it in 
	VTK::VTUFileReader vtuFile;
	if (!vtuFile.Load(dataPath.c_str())) return errf("Failed to read data file");

	vtk = vtuFile.GetVTKModel();
	m_currentTime = timeValue;

	return true;
}

bool PVTUImport::LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk)
{
	VTK::PVTUFileReader pvtuReader;
	if (!pvtuReader.Load(szfilename))
	{
		setErrorString(pvtuReader.GetErrorString());
		return false;
	}

	vtk = pvtuReader.GetVTKModel();
	return true;
}

bool PVDImport::LoadVTKModel(const char* szfilename, VTK::vtkModel& vtk)
{
	VTK::PVDFileReader pvdReader;
	SetFileReader(&pvdReader);
	bool success = pvdReader.Load(szfilename);
	SetFileReader(nullptr);

	if (!success)
	{
		setErrorString(pvdReader.GetErrorString());
		return false;
	}

	vtk = pvdReader.GetVTKModel();
	return true;
}
