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

#include "FEVTKExport.h"
#include <stdio.h>
#include "FEPostModel.h"
#include "FEMeshData_T.h"

using namespace Post;
using namespace std;

enum VTK_CELLTYPE {
	VTK_VERTEX =                 1,
	VTK_POLY_VERTEX =            2,
	VTK_LINE =                   3,
	VTK_POLY_LINE =              4,
	VTK_TRIANGLE =               5,
	VTK_TRIANGLE_STRIP =         6,
	VTK_POLYGON =                7,
	VTK_PIXEL =                  8,
	VTK_QUAD =                   9,
	VTK_TETRA =                  10,
	VTK_VOXEL =                  11,
	VTK_HEXAHEDRON =             12,
	VTK_WEDGE =                  13,
	VTK_PYRAMID =                14,
	VTK_QUADRATIC_EDGE =         21,
	VTK_QUADRATIC_TRIANGLE =     22,
	VTK_QUADRATIC_QUAD =         23,
	VTK_QUADRATIC_TETRA =        24,
	VTK_QUADRATIC_HEXAHEDRON =   25,
	VTK_QUADRATIC_WEDGE =        26,
    VTK_QUADRATIC_PYRAMID =      27
};

void Space2_(char* szname)
{
	int n = (int)strlen(szname);
	for (int i = 0; i<n; ++i)
		if (szname[i] == ' ') szname[i] = '_';
}


FEVTKExport::FEVTKExport(void)
{
	m_bwriteAllStates = false;
	m_bselElemsOnly = false;
	m_bwriteSeriesFile = false;
	m_bwritePartIDs = false;

	AddBoolParam(m_bwriteAllStates , "write_all_states", "Write all states");
	AddBoolParam(m_bselElemsOnly   , "sel_elems_only"  , "Selected elements only");
	AddBoolParam(m_bwriteSeriesFile, "write_series"    , "Write VTK series");
	AddBoolParam(m_bwritePartIDs   , "write_part_ids"  , "Write element part IDs as cell data");

	m_fp = nullptr;
	m_nodes = m_elems = 0;
}

bool FEVTKExport::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_bwriteAllStates  = GetBoolValue(0);
		m_bselElemsOnly    = GetBoolValue(1);
		m_bwriteSeriesFile = GetBoolValue(2);
		m_bwritePartIDs    = GetBoolValue(3);
	}
	else
	{
		SetBoolValue(0, m_bwriteAllStates);
		SetBoolValue(1, m_bselElemsOnly);
		SetBoolValue(2, m_bwriteSeriesFile);
		SetBoolValue(3, m_bwritePartIDs);
	}

	return false;
}

FEVTKExport::~FEVTKExport(void)
{
}

void FEVTKExport::ExportAllStates(bool b)
{
    m_bwriteAllStates = b;
}

void FEVTKExport::ExportSelectedElementsOnly(bool b)
{
	m_bselElemsOnly = b;
}

void FEVTKExport::WriteSeriesFile(bool b)
{
	m_bwriteSeriesFile = b;
}

bool FEVTKExport::Save(FEPostModel& fem, const char* szfile)
{
    int ns = fem.GetStates();
    if (ns == 0) return false;

	// tag all the elements and nodes that should be exported
	// TODO: This assumes there is only one mesh
	FSMesh* pm = fem.GetFEMesh(0);
	if (pm == 0) return false;

	if (m_bselElemsOnly == false)
	{
		for (int i = 0; i < pm->Nodes(); ++i) pm->Node(i).m_ntag = i;
		for (int i = 0; i < pm->Elements(); ++i) pm->Element(i).m_ntag = i;
		m_nodes = pm->Nodes();
		m_elems = pm->Elements();
	}
	else
	{
		m_elems = 0;
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.IsSelected())
			{
				el.m_ntag = m_elems++;
				for (int n = 0; n < el.Nodes(); ++n) pm->Node(el.m_node[n]).m_ntag = 1;
			}
			else el.m_ntag = -1;
		}

		m_nodes = 0;
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			if (pm->Node(i).m_ntag == 1)
			{
				pm->Node(i).m_ntag = m_nodes++;
			}
			else pm->Node(i).m_ntag = -1;
		}
	}
	if ((m_nodes == 0) || (m_elems == 0)) return false;

	if (m_bwriteAllStates)
	{
		char szroot[256] = { 0 }, szname[256] = { 0 }, szext[16] = { 0 };
		const char* sz;
		sz = strrchr(szfile, '.');
		if (sz == 0) {
			strcpy(szroot, szfile);
			strcat(szroot,".");
			strcpy(szext,".vtk");
		}
		else {
			size_t l = sz - szfile + 1;
			strncpy(szroot, szfile, l);
			szroot[l] = 0;
			strcpy(szext, sz);
		}

		// strip the path of the root
		const char* szbase = strrchr(szroot, '/');
		if (szbase == nullptr)
		{
			szbase = strrchr(szroot, '\\');
			if (szbase == nullptr) szbase = szroot; else szbase++;
		}
		else szbase++;

		vector<pair<string, float> > series;
    
		// save each state in a separate file
		int l0 = (int) log10((double)ns) + 1;
		for (int is=0; is<ns; ++is) 
		{
			if (sprintf(szname, "%st%0*d%s", szroot, l0,is,szext) < 0) return false;

			FEState* ps = fem.GetState(is);

			if (WriteState(szname, ps) == false) return false;

			if (sprintf(szname, "%st%0*d%s", szbase, l0, is, szext) < 0) return false;
			series.push_back(pair<string, float>(szname, ps->m_time));
		}

		if (m_bwriteSeriesFile)
		{
			sprintf(szname, "%svtk.series", szroot);
			WriteVTKSeriesFile(szname, series);
		}

		return true;
	}
	else
	{
		FEState* state = fem.CurrentState();
		return WriteState(szfile, state);
	}
}        

bool FEVTKExport::WriteState(const char* szname, FEState* ps)
{
	FSMesh* pm = ps->GetFEMesh();
	if (pm == 0) return false;

	m_fp = fopen(szname, "wt");
	if (m_fp == 0) return false;
        
	// --- H E A D E R ---
	WriteHeader(ps);
	
	// --- N O D E S ---
	WritePoints(ps);
        
    // --- E L E M E N T S ---
	WriteCells(ps);
        
	// --- N O D E   D A T A ---
	WritePointData(ps);

	// --- E L E M E N T   C E L L   D A T A ---
	WriteCellData(ps);
        
    fclose(m_fp);
	m_fp = nullptr;

	return true;
}

//-----------------------------------------------------------------------------
void FEVTKExport::WriteHeader(FEState* ps)
{
	fprintf(m_fp, "%s\n"       ,"# vtk DataFile Version 3.0");
	fprintf(m_fp, "%s %g\n"    ,"vtk output at time", ps->m_time);
	fprintf(m_fp, "%s\n"       ,"ASCII");
	fprintf(m_fp, "%s\n"       ,"DATASET UNSTRUCTURED_GRID");
}

//-----------------------------------------------------------------------------
void FEVTKExport::WritePoints(FEState* ps)
{
	FSMesh& m = *ps->GetFEMesh();
	fprintf(m_fp, "POINTS %d float\n", m_nodes);
	int nodes = m.Nodes();
	for (int j=0, k = 0; j<nodes; j++)
	{
		if (m.Node(j).m_ntag != -1)
		{
			vec3f& r = ps->m_NODE[j].m_rt;
			fprintf(m_fp, "%g %g %g ", r.x, r.y, r.z);
			k++;
		}
		if (k == 3) { fprintf(m_fp, "\n"); k = 0; }
	}
	fprintf(m_fp, "%s\n" ,"");
}

//-----------------------------------------------------------------------------
void FEVTKExport::WriteCells(FEState* ps)
{
	FSMesh&m = *ps->GetFEMesh();
	int NE = m.Elements();
    int nsize = 0;
	for (int j = 0; j < NE; ++j)
	{
		FSElement& el = m.Element(j);
		if (el.m_ntag != -1)
			nsize += el.Nodes() + 1;
	}

	// Write CELLS
    fprintf(m_fp, "CELLS %d %d\n", m_elems, nsize);
    for (int j=0; j<m.Elements(); ++j)
    {
		FSElement& el = m.Element(j);
		if (el.m_ntag != -1)
		{
			fprintf(m_fp, "%d ", el.Nodes());
			for (int k = 0; k < el.Nodes(); ++k)
			{
				int n = m.Node(el.m_node[k]).m_ntag;
				fprintf(m_fp, "%d ", n);
			}
			fprintf(m_fp, "\n");
		}
    }
        
	// Write CELL_TYPES
    fprintf(m_fp, "\nCELL_TYPES %d\n", m_elems);
	for (int j = 0; j<m.Elements(); ++j)
    {
		FSElement& el = m.Element(j);
		if (el.m_ntag != -1)
		{
			int vtk_type;
			switch (el.Type()) {
				case FE_HEX8   : vtk_type = VTK_HEXAHEDRON; break;
				case FE_TET4   : vtk_type = VTK_TETRA; break;
				case FE_PENTA6 : vtk_type = VTK_WEDGE; break;
				case FE_PYRA5  : vtk_type = VTK_PYRAMID; break;
				case FE_QUAD4  : vtk_type = VTK_QUAD; break;
				case FE_TRI3   : vtk_type = VTK_TRIANGLE; break;
				case FE_BEAM2  : vtk_type = VTK_LINE; break;
				case FE_HEX20  : vtk_type = VTK_QUADRATIC_HEXAHEDRON; break;
				case FE_QUAD8  : vtk_type = VTK_QUADRATIC_QUAD; break;
				case FE_BEAM3  : vtk_type = VTK_QUADRATIC_EDGE; break;
				case FE_TET10  : vtk_type = VTK_QUADRATIC_TETRA; break;
				case FE_TET15  : vtk_type = VTK_QUADRATIC_TETRA; break;
				case FE_PENTA15: vtk_type = VTK_QUADRATIC_WEDGE; break;
				case FE_HEX27  : vtk_type = VTK_QUADRATIC_HEXAHEDRON; break;
				case FE_PYRA13 : vtk_type = VTK_QUADRATIC_PYRAMID; break;
				case FE_TRI6   : vtk_type = VTK_QUADRATIC_TRIANGLE; break;
				case FE_QUAD9  : vtk_type = VTK_QUADRATIC_QUAD; break;
				default: vtk_type = -1; break;
			}

			fprintf(m_fp, "%d\n", vtk_type);
		}
	}
}

//-----------------------------------------------------------------------------
void FEVTKExport::WritePointData(FEState* ps)
{
	// make sure the state has data
	int NDATA = ps->m_Data.size();
	if (NDATA == 0) return;

	FSMesh& mesh = *ps->GetFEMesh();
	int nodes = mesh.Nodes();
	vector<int> tag(nodes, 0);
	for (int i = 0; i < nodes; ++i) tag[i] = (mesh.Node(i).m_ntag >= 0 ? 1 : 0);

	fprintf(m_fp, "\nPOINT_DATA %d\n", m_nodes);
	FEPostModel& fem = *ps->GetFSModel();
	FEDataManager& DM = *fem.GetDataManager();
	FEDataFieldPtr pd = DM.FirstDataField();
	for (int n = 0; n<NDATA; ++n, ++pd)
	{
		ModelDataField& data = *(*pd);
		if (data.DataClass() == NODE_DATA)
		{
			FEMeshData& meshData = ps->m_Data[n];
			char szname[256];
			strcpy(szname, data.GetName().c_str());
			Space2_(szname);

			// value array
			vector<float> val;
			if (FillNodeDataArray(val, meshData))
			{
				// write the value array
				int ntype = meshData.GetType();
				if (ntype == DATA_SCALAR) {
					fprintf(m_fp, "%s %s %s\n", "SCALARS", szname, "float");
					fprintf(m_fp, "%s %s\n", "LOOKUP_TABLE", "default");
					for (int i = 0; i<val.size(); ++i) 
						if (tag[i] != 0)
							fprintf(m_fp, "%g\n", val[i]);
				}
				else if (ntype == DATA_VEC3) {
					fprintf(m_fp, "%s %s %s\n", "VECTORS", szname, "float");
					for (int i = 0, n = 0; i<val.size(); i += 3, ++n) 
						if (tag[n] != 0) fprintf(m_fp, "%g %g %g\n", val[i], val[i + 1], val[i + 2]);
				}
				else if (ntype == DATA_MAT3S) {
					fprintf(m_fp, "%s %s %s\n", "TENSORS", szname, "float");
					for (int i = 0, n = 0; i<val.size(); i += 6, n++)
						if (tag[n] != 0)
							fprintf(m_fp, "%g %g %g\n%g %g %g\n%g %g %g\n\n",
								val[i    ], val[i + 3], val[i + 5],
								val[i + 3], val[i + 1], val[i + 4],
								val[i + 5], val[i + 4], val[i + 2]);
				}
				else if (ntype == DATA_MAT3SD) {
					fprintf(m_fp, "%s %s %s\n", "TENSORS", szname, "float");
					for (int i = 0, n = 0; i<val.size(); i += 3, ++n)
						if (tag[n] != 0)
							fprintf(m_fp, "%g %g %g\n%g %g %g\n%g %g %g\n\n",
								val[i], 0.f, 0.f,
								0.f, val[i + 1], 0.f,
								0.f, 0.f, val[i + 2]);
				}
				else if (ntype == DATA_ARRAY) {
					fprintf(m_fp, "FIELD %s %d\n", szname, data.GetArraySize());
					std::vector<string> arrayNames = data.GetArrayNames();
					for (int j = 0; j < data.GetArraySize(); ++j)
					{
						string name = arrayNames[j];
						strcpy(szname, name.c_str());
						Space2_(szname);
						int nsize = (int)val.size();
						fprintf(m_fp, "%s %d %d float\n", szname, 1, nsize);
						for (int i = 0; i < val.size(); ++i)
							if (tag[i] != 0)
							{
								float f = val[j * nodes + i];
								fprintf(m_fp, "%g\n", f);
							}
					}
				}
			}
		}

		// --- E L E M E N T   P O I N T   D A T A ---
		if (data.DataClass() == ELEM_DATA)
		{
			FEMeshData& meshData = ps->m_Data[n];
			DATA_FORMAT dfmt = meshData.GetFormat();
			if ((dfmt == DATA_NODE) || (dfmt == DATA_MULT)) {
				ModelDataField& data = *(*pd);
				char szname[256];
				strcpy(szname, data.GetName().c_str());
				Space2_(szname);

				// value array
				vector<float> val;
				if (FillElementNodeDataArray(val, meshData))
				{
					// write the value array
					int ntype = meshData.GetType();
					if (ntype == DATA_SCALAR) {
						fprintf(m_fp, "SCALARS %s float\n", szname);
						fprintf(m_fp, "LOOKUP_TABLE default\n");
						for (int i = 0; i<val.size(); ++i) 
							if (tag[i] != 0)
								fprintf(m_fp, "%g\n", val[i]);
					}
					else if (ntype == DATA_VEC3) {
						fprintf(m_fp, "VECTORS %s float\n", szname);
						for (int i = 0, n = 0; i<val.size(); i += 3, ++n)
							if (tag[n] != 0)
								fprintf(m_fp, "%g %g %g\n", val[i], val[i + 1], val[i + 2]);
					}
					else if (ntype == DATA_MAT3S) {
						fprintf(m_fp, "TENSORS %s float\n", szname);
						for (int i = 0, n = 0; i<val.size(); i += 6, ++n)
							if (tag[n] != 0)
								fprintf(m_fp, "%g %g %g\n%g %g %g\n%g %g %g\n\n",
									val[i], val[i + 3], val[i + 5],
									val[i + 3], val[i + 1], val[i + 4],
									val[i + 5], val[i + 4], val[i + 2]);
					}
					else if (ntype == DATA_MAT3SD) {
						fprintf(m_fp, "TENSORS %s float\n", szname);
						for (int i = 0, n = 0; i<val.size(); i += 3, ++n)
							if (tag[n] != 0)
								fprintf(m_fp, "%g %g %g\n%g %g %g\n%g %g %g\n\n",
									val[i], 0.f, 0.f,
									0.f, val[i + 1], 0.f,
									0.f, 0.f, val[i + 2]);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEVTKExport::WriteCellData(FEState* ps)
{
	FEPostModel& fem = *ps->GetFSModel();
    FEDataManager& DM = *fem.GetDataManager();
    FEDataFieldPtr pd = DM.FirstDataField();

	FSMesh& mesh = *ps->GetFEMesh();
	int NE = mesh.Elements();
	vector<int> tag(NE, 0);
	for (int i = 0; i < NE; ++i) tag[i] = (mesh.Element(i).m_ntag >= 0 ? 1 : 0);

    int NDATA = ps->m_Data.size();
    if ((NDATA > 0) || m_bwritePartIDs) fprintf(m_fp, "\nCELL_DATA %d\n" , m_elems);

	if (m_bwritePartIDs)
	{
		fprintf(m_fp, "SCALARS part_IDs int\n");
		fprintf(m_fp, "LOOKUP_TABLE default\n");
		for (int i = 0; i < NE; ++i)
			if (tag[i] != 0) fprintf(m_fp, "%d\n", mesh.Element(i).m_gid);
	}
            
    for (int n=0; n<NDATA; ++n, ++pd)
    {
        ModelDataField& data = *(*pd);
        if (data.DataClass() == ELEM_DATA)
        {
			FEMeshData& meshData = ps->m_Data[n];
            DATA_FORMAT dfmt = meshData.GetFormat();
            if (dfmt == DATA_ITEM) {
                ModelDataField& data = *(*pd);
                char szname[256];
				strcpy(szname, data.GetName().c_str());
                Space2_(szname);
                        
                // value array
                vector<float> val;
				if (FillElemDataArray(val, meshData))
				{
					// write the value array
					if (val.empty() == false) {
						int ntype = meshData.GetType();
						if (ntype == DATA_SCALAR) {
							fprintf(m_fp, "%s %s %s\n" ,"SCALARS",szname,"float");
							fprintf(m_fp, "%s %s\n","LOOKUP_TABLE","default");
							for (int i=0; i<val.size(); ++i) 
								if (tag[i] != 0) fprintf(m_fp,"%g\n",val[i]);
						}
						else if (ntype == DATA_VEC3) {
							fprintf(m_fp, "%s %s %s\n" ,"VECTORS",szname,"float");
							for (int i=0, n = 0; i<val.size(); i+=3, n++)
								if (tag[n] != 0) fprintf(m_fp,"%g %g %g\n",val[i],val[i+1],val[i+2]);
						}
						else if (ntype == DATA_MAT3S) {
							fprintf(m_fp, "%s %s %s\n" ,"TENSORS",szname,"float");
							for (int i=0, n = 0; i<val.size(); i+=6, n++)
								if (tag[n] != 0)
									fprintf(m_fp,"%g %g %g\n%g %g %g\n%g %g %g\n\n",
											val[i  ],val[i+3],val[i+5],
											val[i+3],val[i+1],val[i+4],
											val[i+5],val[i+4],val[i+2]);
						}
						else if (ntype == DATA_MAT3SD) {
							fprintf(m_fp, "%s %s %s\n" ,"TENSORS",szname,"float");
							for (int i=0, n = 0; i<val.size(); i+=3, ++n)
								if (tag[n] != 0)
									fprintf(m_fp,"%g %g %g\n%g %g %g\n%g %g %g\n\n",
											val[i  ],0.f,0.f,
											0.f,val[i+1],0.f,
											0.f,0.f,val[i+2]);
						}
						else if (ntype == DATA_ARRAY)
						{
							fprintf(m_fp, "FIELD %s %d\n", szname, data.GetArraySize());
							std::vector<string> arrayNames = data.GetArrayNames();
							for (int j = 0; j < data.GetArraySize(); ++j)
							{
								string name = arrayNames[j];
								strcpy(szname, name.c_str());
								Space2_(szname);
								fprintf(m_fp, "%s %d %d float\n", szname, 1, NE);
								for (int i = 0; i < NE; ++i)
									if (tag[i] != 0) {
										float f = val[j * NE + i];
										fprintf(m_fp, "%g\n", f);
								}
							}
						}
						else if (ntype == DATA_ARRAY_VEC3)
						{
							fprintf(m_fp, "FIELD %s %d\n", szname, data.GetArraySize());
							std::vector<string> arrayNames = data.GetArrayNames();
							for (int j = 0; j < data.GetArraySize(); ++j)
							{
								string name = arrayNames[j];
								strcpy(szname, name.c_str());
								Space2_(szname);
								fprintf(m_fp, "%s %d %d float\n", szname, 3, NE);
								for (int i = 0; i < NE; ++i)
									if (tag[i] != 0)
									{
										float f[3];
										f[0] = val[j * (3 * NE) + 3 * i    ];
										f[1] = val[j * (3 * NE) + 3 * i + 1];
										f[2] = val[j * (3 * NE) + 3 * i + 2];
										fprintf(m_fp, "%g %g %g\n", f[0], f[1], f[2]);
									}
							}
						}
					}
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
inline void write_data(vector<float>& val, int index, const vec3f& v)
{
	val[3 * index] = v.x;
	val[3 * index + 1] = v.y;
	val[3 * index + 2] = v.z;
}

inline void write_data(vector<float>& val, int index, const mat3fs& v)
{
	val[6 * index] = v.x;
	val[6 * index + 1] = v.y;
	val[6 * index + 2] = v.z;
	val[6 * index + 3] = v.xy;
	val[6 * index + 4] = v.yz;
	val[6 * index + 5] = v.xz;
}

inline void write_data(vector<float>& val, int index, const mat3fd& v)
{
	val[3 * index] = v.x;
	val[3 * index + 1] = v.y;
	val[3 * index + 2] = v.z;
}

//-----------------------------------------------------------------------------
bool FEVTKExport::FillNodeDataArray(vector<float>& val, Post::FEMeshData& meshData)
{
	FEPostModel& fem = *meshData.GetFSModel();
	FSMesh& mesh = *fem.GetFEMesh(0);

	int ntype = meshData.GetType();
	int NN = mesh.Nodes();

	if (ntype == DATA_SCALAR)
	{
		FENodeData<float>& data = dynamic_cast<FENodeData<float>&>(meshData);
		val.assign(NN, 0.f);
		for (int i=0; i<NN; ++i) val[i] = data[i];
	}
	else if (ntype == DATA_VEC3)
	{
		FENodeData<vec3f>& data = dynamic_cast<FENodeData<vec3f>&>(meshData);
		val.assign(NN*3, 0.f);
		for (int i=0; i<NN; ++i) write_data(val, i, data[i]);
	}
	else if (ntype == DATA_MAT3S)
	{
		FENodeData<mat3fs>& data = dynamic_cast<FENodeData<mat3fs>&>(meshData);
		val.assign(NN*6, 0.f);
		for (int i=0; i<NN; ++i) write_data(val, i, data[i]);
	}
	else if (ntype == DATA_MAT3SD)
	{
		FENodeData<mat3fd>& data = dynamic_cast<FENodeData<mat3fd>&>(meshData);
		val.assign(NN*3, 0.f);
		for (int i=0; i<NN; ++i) write_data(val, i, data[i]);
	}
	else if (ntype == DATA_ARRAY)
	{
		FENodeArrayData& data = dynamic_cast<FENodeArrayData&>(meshData);
		int nc = data.components();
		val.assign(NN * nc, 0.f);
		int m = 0;
		for (int n = 0; n < nc; ++n)
		{
			for (int i = 0; i < NN; ++i) val[m++] = data.eval(i, n);
		}
	}
	else return false;

	return true;
}

//-----------------------------------------------------------------------------
bool FEVTKExport::FillElementNodeDataArray(vector<float>& val, Post::FEMeshData& meshData)
{
	int ntype = meshData.GetType();
	int nfmt = meshData.GetFormat();

	if ((nfmt != DATA_NODE) && (nfmt != DATA_MULT)) return false;

	FSMesh& mesh = *meshData.GetFEMesh();
	int NN = mesh.Nodes();
	int nstride = 0;
	switch (ntype)
	{
	case DATA_SCALAR: nstride = 1; break;
	case DATA_VEC3: nstride = 3; break;
	case DATA_MAT3S: nstride = 6; break;
	case DATA_MAT3SD: nstride = 3; break;
	default:
		return false;
	}

	int nsize = NN*nstride;
	val.assign(nsize, 0.f);

	int NE = mesh.Elements();

	if (nfmt == DATA_NODE)
	{
		if (ntype == DATA_SCALAR)
		{
			FEElementData<float, DATA_NODE>& data = dynamic_cast<FEElementData<float, DATA_NODE>&>(meshData);
			float v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) val[el.m_node[j]] = v[j];
				}
			}
		}
		else if (ntype == DATA_VEC3)
		{
			FEElementData<vec3f, DATA_NODE>& data = dynamic_cast<FEElementData<vec3f, DATA_NODE>&>(meshData);
			vec3f v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) write_data(val, el.m_node[j], v[j]);
				}
			}
		}
		else if (ntype == DATA_MAT3S)
		{
			FEElementData<mat3fs, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fs, DATA_NODE>&>(meshData);
			val.assign(NN * 6, 0.f);

			mat3fs v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) write_data(val, el.m_node[j], v[j]);
				}
			}
		}
		else if (ntype == DATA_MAT3SD)
		{
			FEElementData<mat3fd, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fd, DATA_NODE>&>(meshData);
			val.assign(NN * 3, 0.f);

			mat3fd v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) write_data(val, el.m_node[j], v[j]);
				}
			}
		}
		else return false;
	}
	else if (nfmt == DATA_MULT)
	{
		if (ntype == DATA_SCALAR)
		{
			FEElementData<float, DATA_MULT>& data = dynamic_cast<FEElementData<float, DATA_MULT>&>(meshData);
			float v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) val[el.m_node[j]] = v[j];
				}
			}
		}
		else if (ntype == DATA_VEC3)
		{
			FEElementData<vec3f, DATA_MULT>& data = dynamic_cast<FEElementData<vec3f, DATA_MULT>&>(meshData);
			vec3f v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) write_data(val, el.m_node[j], v[j]);
				}
			}
		}
		else if (ntype == DATA_MAT3S)
		{
			FEElementData<mat3fs, DATA_MULT>& data = dynamic_cast<FEElementData<mat3fs, DATA_MULT>&>(meshData);
			val.assign(NN * 6, 0.f);

			mat3fs v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) write_data(val, el.m_node[j], v[j]);
				}
			}
		}
		else if (ntype == DATA_MAT3SD)
		{
			FEElementData<mat3fd, DATA_MULT>& data = dynamic_cast<FEElementData<mat3fd, DATA_MULT>&>(meshData);
			val.assign(NN * 3, 0.f);

			mat3fd v[FSElement::MAX_NODES];
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = mesh.ElementRef(i);
				if (data.active(i))
				{
					data.eval(i, v);
					int ne = el.Nodes();
					for (int j = 0; j < ne; ++j) write_data(val, el.m_node[j], v[j]);
				}
			}
		}
		else return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEVTKExport::FillElemDataArray(vector<float>& val, Post::FEMeshData& meshData)
{
	FEPostModel& fem = *meshData.GetFSModel();
	FSMesh& mesh = *fem.GetFEMesh(0);

	int ntype = meshData.GetType();
	int nfmt  = meshData.GetFormat();

	int NE = mesh.Elements();
	if (NE == 0) return false;

	// number of actually written values
	int nval = 0;

	if (nfmt == DATA_ITEM)
	{
		if (ntype == DATA_SCALAR)
		{
			FEElemData_T<float, DATA_ITEM>& data = dynamic_cast<FEElemData_T<float, DATA_ITEM>&>(meshData);
			val.assign(NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				if (data.active(i)) { data.eval(i, &val[i]); nval++; }
			}
		}
		else if (ntype == DATA_VEC3)
		{
			FEElemData_T<vec3f, DATA_ITEM>& data = dynamic_cast<FEElemData_T<vec3f, DATA_ITEM>&>(meshData);
			val.assign(3*NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				if (data.active(i))
				{
					vec3f v(0.f, 0.f, 0.f);
					data.eval(i, &v);
					write_data(val, i, v);
					nval++; 
				}
			}
		}
		else if (ntype == DATA_MAT3S)
		{
			FEElemData_T<mat3fs, DATA_ITEM>& data = dynamic_cast<FEElemData_T<mat3fs, DATA_ITEM>&>(meshData);
			val.assign(6*NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				if (data.active(i))
				{
					mat3fs v;
					data.eval(i, &v);
					write_data(val, i, v);

					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3SD)
		{
			FEElemData_T<mat3fd, DATA_ITEM>& data = dynamic_cast<FEElemData_T<mat3fd, DATA_ITEM>&>(meshData);
			val.assign(3*NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				if (data.active(i))
				{
					mat3fd v;
					data.eval(i, &v);
					write_data(val, i, v);

					nval++;
				}
			}
		}
		else if (ntype == DATA_ARRAY)
		{
			FEElemArrayDataItem& data = dynamic_cast<FEElemArrayDataItem&>(meshData);
			int nc = data.components();
			val.assign(NE * nc, 0.f);
			nval = 0;
			for (int n = 0; n < nc; ++n)
			{
				for (int i = 0; i < NE; ++i)
				{
					if (data.active(i))
					{
						float v = data.eval(i, n);
						val[nval++] = v;
					}
				}
			}
		}
		else if (ntype == DATA_ARRAY_VEC3)
		{
			FEElemArrayVec3Data& data = dynamic_cast<FEElemArrayVec3Data&>(meshData);
			int nc = data.components();
			val.assign(3 * NE * nc, 0.f);
			nval = 0;
			for (int n = 0; n < nc; ++n)
			{
				for (int i = 0; i < NE; ++i)
				{
					if (data.active(i))
					{
						vec3f v = data.eval(i, n);
						write_data(val, nval, v);
						nval++;
					}
				}
			}
		}
		else return false;
	}
/*	else if (nfmt == DATA_MULT)
	{
		if (ntype == DATA_SCALAR)
		{
			FEElemData_T<float, DATA_MULT>& data = dynamic_cast<FEElemData_T<float, DATA_MULT>&>(meshData);
			val.assign(NE*ne, 0.f);
			for (int i=0; i<NE; ++i)
			{
				float v[FSElement::MAX_NODES] = {0.f};
				if (data.active(i))
				{
					data.eval(i, v);
					for (int j=0; j<ne; ++j) val[i*ne + j] = v[j];
					nval++;
				}
			}
		}
		else if (ntype == DATA_VEC3)
		{
			FEElemData_T<vec3f, DATA_MULT>& data = dynamic_cast<FEElemData_T<vec3f, DATA_MULT>&>(meshData);
			val.assign(NE*ne*3, 0.f);
			for (int i=0; i<NE; ++i)
			{
				vec3f v[FSElement::MAX_NODES] = {vec3f(0.f,0.f,0.f)};
				if (data.active(i))
				{
					data.eval(i, v);
					for (int j=0; j<ne; ++j) write_data(val, i*ne + j, v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3S)
		{
			FEElemData_T<mat3fs, DATA_MULT>& data = dynamic_cast<FEElemData_T<mat3fs, DATA_MULT>&>(meshData);
			val.assign(NE*ne*6, 0.f);
			mat3fs v[FSElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				if (data.active(i))
				{
					data.eval(i, v);
					for (int j=0; j<ne; ++j) write_data(val, i*ne + j, v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3SD)
		{
			FEElemData_T<mat3fd, DATA_MULT>& data = dynamic_cast<FEElemData_T<mat3fd, DATA_MULT>&>(meshData);
			val.assign(NE*ne*3, 0.f);
			mat3fd v[FSElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				if (data.active(i))
				{
					data.eval(i, v);
					for (int j=0; j<ne; ++j) write_data(val, i*ne + j, v[j]);
					nval++;
				}
			}
		}
		else return false;
	}
	else if (nfmt == DATA_NODE)
	{
		vector<int> node, lnode;
		part.GetNodeList(node, lnode);
		int NN = (int) node.size();

		if (ntype == DATA_SCALAR)
		{
			FEElemData_T<float, DATA_NODE>& data = dynamic_cast<FEElemData_T<float, DATA_NODE>&>(meshData);
			val.assign(NN, 0.f);

			float v[FSElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid))
				{
					data.eval(eid, v);
					for (int j=0; j<ne; ++j) val[lnode[i*ne+j]] = v[j];
					nval++;
				}
			}
		}
		else if (ntype == DATA_VEC3)
		{
			FEElemData_T<vec3f, DATA_NODE>& data = dynamic_cast<FEElemData_T<vec3f, DATA_NODE>&>(meshData);
			val.assign(NN*3, 0.f);

			vec3f v[FSElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid))
				{
					data.eval(eid, v);
					for (int j=0; j<ne; ++j) write_data(val, lnode[i*ne+j], v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3S)
		{
			FEElemData_T<mat3fs, DATA_NODE>& data = dynamic_cast<FEElemData_T<mat3fs, DATA_NODE>&>(meshData);
			val.assign(NN*6, 0.f);

			mat3fs v[FSElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid))
				{
					data.eval(eid, v);
					for (int j=0; j<ne; ++j) write_data(val, lnode[i*ne+j], v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3SD)
		{
			FEElemData_T<mat3fd, DATA_NODE>& data = dynamic_cast<FEElemData_T<mat3fd, DATA_NODE>&>(meshData);
			val.assign(NN*3, 0.f);

			mat3fd v[FSElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid))
				{
					data.eval(eid, v);
					for (int j=0; j<ne; ++j) write_data(val, lnode[i*ne+j], v[j]);
					nval++;
				}
			}
		}
		else return false;
	}
*/	else return false;

	return true;
}

void FEVTKExport::WriteVTKSeriesFile(const char* szfile, std::vector<std::pair<std::string, float> >& series)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == nullptr) return;

	fprintf(fp, "{\n");
	fprintf(fp, "\t\"file-series-version\": \"1.0\",\n");
	fprintf(fp, "\t\"files\": [\n");

	for (size_t i = 0; i<series.size(); ++i)
	{
		auto& it = series[i];
		fprintf(fp, "\t{ \"name\": \"%s\", \"time\": %g }", it.first.c_str(), it.second);
		if (i != series.size() - 1) fprintf(fp, ",\n");
		else fprintf(fp, "\n");
	}

	fprintf(fp, "\t]\n");
	fprintf(fp, "}\n");

	fclose(fp);
}
