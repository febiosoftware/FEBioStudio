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

using namespace Post;
using namespace std;

class VTKimport::VTKModel
{
public:
	struct CELL
	{
		int		n[10];
		int		nodes = 0;
		int		type = 0;
		int		id = 0;
	};

	struct DataScalar
	{
		string			m_name;
		vector<double>	m_data;
	};

	struct DataVector
	{
		string			m_name;
		vector<vec3f>	m_data;
	};

	struct DataTensor
	{
		string			m_name;
		vector<mat3f>	m_data;
	};

public:
	std::vector<vec3f>	m_pt;
	std::vector<DataScalar>	m_ptDataScalar;
	std::vector<DataVector>	m_ptDataVector;

	std::vector<CELL>	m_el;
	std::vector<DataScalar>	m_cellDataScalar;
	std::vector<DataVector>	m_cellDataVector;
	std::vector<DataTensor>	m_cellDataTensor;
};

VTKimport::VTKimport(FEPostModel* fem) : FEFileReader(fem)
{
	m_isPolyData = false;
	m_isUnstructuredGrid = false;
	m_readingPointData = false;
	m_readingCellData = false;
	m_vtk = nullptr;
	m_currentTime = 0.0;
	m_fileCount = 0;
}

VTKimport::~VTKimport(void)
{
	delete m_vtk;
}

bool VTKimport::Load(const char* szfile)
{
	FEPostModel& fem = *m_fem;
	fem.Clear();

	// clear flags
	m_isPolyData = false;
	m_isUnstructuredGrid = false;
	m_readingPointData = false;
	m_readingCellData = false;

	if (m_vtk) delete m_vtk;
	m_vtk = new VTKModel;

	// Process root file
	if (readFile(szfile) == false) return false;

	// build the mesh
	if (BuildMesh() == false) return false;

	// add all data fields to the model
	if (UpdateModel() == false) return false;

	// build the state 
	if (BuildState(m_currentTime) == false) return false;

	// This file might be part of a series, so check and try to read it in
	if (ProcessSeries(szfile) == false) return false;
	
	return true;
}

bool VTKimport::ProcessSeries(const char* szfile)
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

			delete m_vtk;
			m_vtk = new VTKModel;
			if (readFile(szfilen) == false)
			{
				// Let's assume that the file was not found, so we probably 
				// reached the end of the series. No reason to make a fuss about that. 
				ClearErrors();
				break;
			}

			// some sanity checks
			FSMesh* pm = fem.GetFEMesh(0);
			if (m_vtk->m_pt.size() != pm->Nodes()) break;
			if (m_vtk->m_el.size() != pm->Elements()) break;

			// build the state
			if (BuildState(m_currentTime) == false) return false;
		} while (true);
	}

	return true;
}

bool VTKimport::readFile(const char* szfile)
{
	if (!Open(szfile, "rt")) return errf("Failed opening file %s.", szfile);

	if (readHeader() == false) return false;

	// process file
	char szline[256] = { 0 }, * ch;
	while (ch = fgets(szline, 255, m_fp))
	{
		if (strstr(ch, "DATASET") != 0)
		{
			if (readDataSet(szline) == false) return false;
		}
		else if (strstr(ch, "POINTS") != 0)
		{
			if (readPoints(szline) == false) return false;
		}
		else if (strstr(ch, "POLYGONS") != 0)
		{
			if (readPolygons(szline) == false) return false;
		}
		else if (strstr(ch, "CELLS") != 0)
		{
			if (readCells(szline) == false) return false;
		}
		else if (strstr(ch, "CELL_TYPES") != 0)
		{
			if (readCellTypes(szline) == false) return false;
		}
		else if (strstr(ch, "POINT_DATA") != 0)
		{
			if (readPointData(szline) == false) return false;
		}
		else if (strstr(ch, "CELL_DATA") != 0)
		{
			if (readCellData(szline) == false) return false;
		}
		else if (strstr(ch, "SCALARS") != 0)
		{
			if (readScalars(szline) == false) return false;
		}
		else if (strstr(ch, "VECTORS") != 0)
		{
			if (readVectors(szline) == false) return false;
		}
		else if (strstr(ch, "TENSORS") != 0)
		{
			if (readTensors(szline) == false) return false;
		}
	}
	Close();

	return true;
}

char* VTKimport::readLine(char* szline)
{
	char* ch = nullptr;
	do {
		ch = fgets(szline, 255, m_fp);
	}
	while (ch && ((ch[0] == 0) || (ch[0] =='\n') || (ch[0] == '\r')));
	if (ch == 0) errf("An unexpected error occured while reading the file data.");
	return ch;
}

bool VTKimport::readHeader()
{
	char szline[256] = { 0 }, * ch;

	// skip first line (should be # vtk ...)
	ch = fgets(szline, 255, m_fp); if (ch == 0) return false;

	// skip the second line (title)
	ch = fgets(szline, 255, m_fp); if (ch == 0) return false;
	if (strncmp(szline, "time", 4) == 0)
	{
		m_currentTime = atof(szline + 4);
	}
	else m_currentTime = m_fileCount;

	// third line should be BINARY or ASCII (we only support ASCII)
	ch = fgets(szline, 255, m_fp); if (ch == 0) return false;
	if (strstr(ch, "ASCII") == 0) return errf("Only ASCII files are read");

	return true;
}

bool VTKimport::readDataSet(char* szline)
{
	if (strstr(szline, "POLYDATA"))
	{
		m_isPolyData = true;
	}
	else if (strstr(szline, "UNSTRUCTURED_GRID"))
	{
		m_isUnstructuredGrid = true;
	}
	else return false;

	return true;
}

bool VTKimport::readPoints(char* szline)
{
	//get number of nodes
	int nodes = atoi(szline + 6);
	if (nodes <= 0) return errf("Invalid number of nodes.");

	m_vtk->m_pt.resize(nodes);

	// read the nodes
	//Check how many nodes are there in each line
	char* ch = readLine(szline); if (ch == 0) return false;

	double tmp[9];
	int nread = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7], &tmp[8]);
	if (nread % 3 != 0 && nread > 9)
		return errf("An error occured while reading the nodal coordinates.");
	int nodes_each_row = nread / 3;
	double temp2 = double(nodes) / nodes_each_row;
	int rows = (int)ceil(temp2);
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0, k = 0; j < nodes_each_row && i * nodes_each_row + j < nodes; j++)
		{
			vec3f& r = m_vtk->m_pt[i * nodes_each_row + j];
			r.x = (float)tmp[k];
			r.y = (float)tmp[k + 1];
			r.z = (float)tmp[k + 2];
			k += 3;
		}

		if (i != rows - 1)
		{
			ch = readLine(szline); if (ch == 0) return false;

			nread = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7], &tmp[8]);
			if (nread % 3 != 0 && nread != -1)
			{
				if (i + 1 != nodes / nodes_each_row)
					return errf("An error occured while reading the nodal coordinates.");
			}
		}
	}

	return true;
}

bool VTKimport::readPolygons(char* szline)
{
	if (m_isPolyData == false) return false;

	int elems = 0;
	int size = 0;

	sscanf(szline, "%*s %d %d", &elems, &size);

	// read the elements
	m_vtk->m_el.resize(elems);
	int n[9];
	for (int i = 0; i < elems; ++i)
	{
		VTKModel::CELL& el = m_vtk->m_el[i];
		if (readLine(szline) == nullptr) return false;

		int nread = sscanf(szline, "%d%d%d%d%d%d%d%d%d", &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8]);
		int min = 0;
		switch (n[0])
		{
		case 3:
			el.type = FE_TRI3;
			el.nodes = 3;
			el.n[0] = n[1] - min;
			el.n[1] = n[2] - min;
			el.n[2] = n[3] - min;
			break;
		case 4:
			el.type = FE_QUAD4;
			el.nodes = 4;
			el.n[0] = n[1] - min;
			el.n[1] = n[2] - min;
			el.n[2] = n[3] - min;
			el.n[3] = n[4] - min;
			break;
		default:
			return errf("Only triangular, quadrilateral and hexahedron polygons are supported.");
		}
	}

	return true;
}

bool VTKimport::readCells(char* szline)
{
	if (m_isUnstructuredGrid == false) return false;

	int elems = 0;
	int size = 0;

	sscanf(szline, "%*s %d %d", &elems, &size);

	// read the elements
	m_vtk->m_el.resize(elems);
	int n[9];
	for (int i = 0; i < elems; ++i)
	{
		VTKModel::CELL& el = m_vtk->m_el[i];
		el.type = FE_INVALID_ELEMENT_TYPE; // is determined by CELL_TYPE

		if (readLine(szline) == nullptr) return false;

		int nread = sscanf(szline, "%d%d%d%d%d%d%d%d%d", &n[0], &n[1], &n[2], &n[3], &n[4], &n[5], &n[6], &n[7], &n[8]);
		int min = 0;
		switch (n[0])
		{
		case 3:
			el.nodes = 3;
			el.n[0] = n[1] - min;
			el.n[1] = n[2] - min;
			el.n[2] = n[3] - min;
			break;
		case 4:
			el.nodes = 4;
			el.n[0] = n[1] - min;
			el.n[1] = n[2] - min;
			el.n[2] = n[3] - min;
			el.n[3] = n[4] - min;
			break;
		case 8:
			el.nodes = 8;
			el.n[0] = n[1] - min;
			el.n[1] = n[2] - min;
			el.n[2] = n[3] - min;
			el.n[3] = n[4] - min;
			el.n[4] = n[5] - min;
			el.n[5] = n[6] - min;
			el.n[6] = n[7] - min;
			el.n[7] = n[8] - min;
			break;
		default:
			return errf("Only triangular, quadrilateral, and hexahedron polygons are supported.");
		}
	}

	return true;
}

bool VTKimport::readCellTypes(char* szline)
{
	if (m_isUnstructuredGrid == false) return false;

	int elems = 0;
	sscanf(szline, "%*s%d", &elems);
	if (elems != m_vtk->m_el.size()) return false;

	for (int i = 0; i < elems; ++i)
	{
		if (readLine(szline) == nullptr) return false;

		VTKModel::CELL& el = m_vtk->m_el[i];
		int ntype = atoi(szline);
		switch (ntype)
		{
		case 5 : el.type = FE_TRI3; break;
		case 8 : el.type = FE_QUAD4; break;
		case 10: el.type = FE_TET4; break;
		case 12: el.type = FE_HEX8; break;
		case 13: el.type = FE_PENTA6; break;
		case 14: el.type = FE_PYRA5; break;
		case 22: el.type = FE_TRI6; break;
		case 23: el.type = FE_QUAD8; break;
		case 24: el.type = FE_TET10; break;
		case 25: el.type = FE_HEX20; break;
		default:
			return false;
		}
	}

	return true;
}

bool VTKimport::readPointData(char* szline)
{
	m_readingPointData = true;
	m_readingCellData = false;

	int nsize = atoi(szline + 10);
	int nodes = m_vtk->m_pt.size();
	if (nodes != nsize) return false;

	return true;
}

bool VTKimport::readCellData(char* szline)
{
	m_readingPointData = false;
	m_readingCellData = true;

	int nsize = atoi(szline + 10);
	int elems = m_vtk->m_el.size();
	if (elems != nsize) return false;

	return true;
}

bool VTKimport::readScalars(char* szline)
{
	char dataAttr[64] = { 0 };
	char dataName[64] = { 0 };
	char dataType[64] = { 0 };
	int nread = sscanf(szline, "%s %s %s", dataAttr, dataName, dataType);
	if (strcmp(dataAttr, "SCALARS") != 0) return false;

	// skip the lookup table tag
	char* ch = readLine(szline);

	if (m_readingPointData)
	{
		int nodes = m_vtk->m_pt.size();
		VTKModel::DataScalar data;
		data.m_name = dataName;
		data.m_data.resize(nodes);

		double v[9];
		int nreadTotal = 0;
		while (nreadTotal < nodes)
		{
			char* ch = readLine(szline); if (ch == 0) return false;

			int nread = sscanf(szline, "%lg%lg%lg%lg%lg%lg%lg%lg%lg", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8]);
			for (int j = 0; j < nread; j++)
			{
				data.m_data[nreadTotal++] = v[j];
			}
		}
		m_vtk->m_ptDataScalar.push_back(data);
	}
	else if (m_readingCellData)
	{
		int elems = m_vtk->m_el.size();
		if ((strcmp(dataType, "float") == 0) || (strcmp(dataType, "double") == 0))
		{
			VTKModel::DataScalar data;
			data.m_name = dataName;
			data.m_data.resize(elems);

			for (int i = 0; i < elems; ++i)
			{
				ch = readLine(szline);
				double v = 0.0;
				sscanf(szline, "%lg", &v);

				data.m_data[i] = v;
			}
			m_vtk->m_cellDataScalar.push_back(data);
		}
		else
		{
			// assume these are part IDs
			for (int i = 0; i < elems; ++i)
			{
				if (readLine(szline) == 0) return false;

				VTKModel::CELL& cell = m_vtk->m_el[i];
				cell.id = atoi(szline);
			}
		}
	}
	else return false;

	return true;
}

bool VTKimport::readVectors(char* szline)
{
	char dataAttr[64] = { 0 };
	char dataName[64] = { 0 };
	char dataType[64] = { 0 };
	int nread = sscanf(szline, "%s %s %s", dataAttr, dataName, dataType);
	if (strcmp(dataAttr, "VECTORS") != 0) return false;

	if (m_readingPointData)
	{
		int nodes = m_vtk->m_pt.size();

		VTKModel::DataVector data;
		data.m_name = dataName;
		data.m_data.resize(nodes);

		float v[3];
		for (int i = 0; i < nodes; ++i)
		{
			if (readLine(szline) == 0) return false;
			nread = sscanf(szline, "%g%g%g", &v[0], &v[1], &v[2]);

			data.m_data[i] = vec3f(v[0], v[1], v[2]);
		}

		m_vtk->m_ptDataVector.push_back(data);
	}
	else if (m_readingCellData)
	{
		int elems = m_vtk->m_el.size();

		VTKModel::DataVector data;
		data.m_name = dataName;
		data.m_data.resize(elems);

		float v[3];
		for (int i = 0; i < elems; ++i)
		{
			if (readLine(szline) == 0) return false;
			nread = sscanf(szline, "%g%g%g", &v[0], &v[1], &v[2]);

			data.m_data[i] = vec3f(v[0], v[1], v[2]);
		}

		m_vtk->m_cellDataVector.push_back(data);
	}

	return true;
}

bool VTKimport::readTensors(char* szline)
{
	char dataAttr[64] = { 0 };
	char dataName[64] = { 0 };
	char dataType[64] = { 0 };
	int nread = sscanf(szline, "%s %s %s", dataAttr, dataName, dataType);
	if (strcmp(dataAttr, "TENSORS") != 0) return false;

	if (m_readingPointData)
	{
		// TODO: implement this
		return false;
	}
	else if (m_readingCellData)
	{
		int elems = m_vtk->m_el.size();

		VTKModel::DataTensor data;
		data.m_name = dataName;
		data.m_data.resize(elems);

		float v[9];
		for (int i = 0; i < elems; ++i)
		{
			if (readLine(szline) == 0) return false;
			nread = sscanf(szline, "%g%g%g", &v[0], &v[1], &v[2]);

			if (readLine(szline) == 0) return false;
			nread = sscanf(szline, "%g%g%g", &v[3], &v[4], &v[5]);

			if (readLine(szline) == 0) return false;
			nread = sscanf(szline, "%g%g%g", &v[6], &v[7], &v[8]);

			data.m_data[i] = mat3f(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
		}

		m_vtk->m_cellDataTensor.push_back(data);
	}

	return true;
}

bool VTKimport::BuildMesh()
{
	// create a new mesh
	int nodes = m_vtk->m_pt.size();
	int elems = m_vtk->m_el.size();
	FEPostMesh* pm = new FEPostMesh;
	pm->Create(nodes, elems);

	for (int i = 0; i < nodes; ++i)
	{
		vec3f& r = m_vtk->m_pt[i];
		FSNode& node = pm->Node(i);
		node.r = to_vec3d(r);
	}

	for (int i = 0; i < elems; ++i)
	{
		VTKModel::CELL& cell = m_vtk->m_el[i];
		if (cell.type == FE_INVALID_ELEMENT_TYPE) return false;
		FSElement& el = pm->Element(i);
		el.SetType(cell.type);
		el.m_gid = cell.id;
		assert(el.Nodes() == cell.nodes);
		for (int j = 0; j < cell.nodes; ++j) el.m_node[j] = cell.n[j];
	}

	// We need to build the mesh before allocating a state so that we have 
	// the faces. 
	pm->BuildMesh();

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
	pm->BuildMesh();
	fem.UpdateBoundingBox();

	return true;
}

bool VTKimport::UpdateModel()
{
	FEPostModel& fem = *m_fem;
	for (int i = 0; i < m_vtk->m_ptDataScalar.size(); ++i)
	{
		VTKModel::DataScalar& data = m_vtk->m_ptDataScalar[i];
		fem.AddDataField(new FEDataField_T<FENodeData<float> >(&fem, EXPORT_DATA), data.m_name);
	}
	for (int i = 0; i < m_vtk->m_ptDataVector.size(); ++i)
	{
		VTKModel::DataVector& data = m_vtk->m_ptDataVector[i];
		fem.AddDataField(new FEDataField_T<FENodeData<vec3f> >(&fem, EXPORT_DATA), data.m_name);
	}

	for (int i = 0; i < m_vtk->m_cellDataScalar.size(); ++i)
	{
		VTKModel::DataScalar& data = m_vtk->m_cellDataScalar[i];
		fem.AddDataField(new FEDataField_T<FEElementData<float, DATA_ITEM> >(&fem, EXPORT_DATA), data.m_name);
	}

	for (int i = 0; i < m_vtk->m_cellDataVector.size(); ++i)
	{
		VTKModel::DataVector& data = m_vtk->m_cellDataVector[i];
		fem.AddDataField(new FEDataField_T<FEElementData<vec3f, DATA_ITEM> >(&fem, EXPORT_DATA), data.m_name);
	}

	for (int i = 0; i < m_vtk->m_cellDataTensor.size(); ++i)
	{
		VTKModel::DataTensor& data = m_vtk->m_cellDataTensor[i];
		fem.AddDataField(new FEDataField_T<FEElementData<mat3f, DATA_ITEM> >(&fem, EXPORT_DATA), data.m_name);
	}

	return true;
}

bool VTKimport::BuildState(double time)
{
	// add a state
	FEState* ps = new FEState(time, m_fem, m_fem->GetFEMesh(0));
	m_ps = ps;

	FEPostModel& fem = *m_fem;
	fem.AddState(ps);

	FSMesh* pm = ps->GetFEMesh();
	int nodes = pm->Nodes();
	int elems = pm->Elements();

	int nfield = 0;
	for (int i = 0; i < m_vtk->m_ptDataScalar.size(); ++i)
	{
		VTKModel::DataScalar& data = m_vtk->m_ptDataScalar[i];
		FENodeData<float>& df = dynamic_cast<FENodeData<float>&>(ps->m_Data[nfield++]);
		for (int j = 0; j < pm->Nodes(); ++j) df[j] = (float)data.m_data[j];
	}

	for (int i = 0; i < m_vtk->m_ptDataVector.size(); ++i)
	{
		VTKModel::DataVector& data = m_vtk->m_ptDataVector[i];
		FENodeData<vec3f>& df = dynamic_cast<FENodeData<vec3f>&>(ps->m_Data[nfield++]);
		for (int j = 0; j < pm->Nodes(); ++j) df[j] = data.m_data[j];
	}

	for (int i = 0; i < m_vtk->m_cellDataScalar.size(); ++i)
	{
		VTKModel::DataScalar& data = m_vtk->m_cellDataScalar[i];
		FEElementData<float, DATA_ITEM>& ed = dynamic_cast<FEElementData<float, DATA_ITEM>&>(ps->m_Data[nfield++]);
		for (int j = 0; j < elems; ++j) ed.add(j, (float)data.m_data[j]);
	}

	for (int i = 0; i < m_vtk->m_cellDataVector.size(); ++i)
	{
		VTKModel::DataVector& data = m_vtk->m_cellDataVector[i];
		FEElementData<vec3f, DATA_ITEM>& ed = dynamic_cast<FEElementData<vec3f, DATA_ITEM>&>(ps->m_Data[nfield++]);
		for (int j = 0; j < elems; ++j) ed.add(j, data.m_data[j]);
	}

	for (int i = 0; i < m_vtk->m_cellDataTensor.size(); ++i)
	{
		VTKModel::DataTensor& data = m_vtk->m_cellDataTensor[i];
		FEElementData<mat3f, DATA_ITEM>& ed = dynamic_cast<FEElementData<mat3f, DATA_ITEM>&>(ps->m_Data[nfield++]);
		for (int j = 0; j < elems; ++j) ed.add(j, data.m_data[j]);
	}

	return true;
}
