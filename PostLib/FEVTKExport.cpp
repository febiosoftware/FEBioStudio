#include "FEVTKExport.h"
#include <stdio.h>
#include "FEModel.h"
#include "FEMeshData_T.h"

using namespace Post;

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
	VTK_QUADRATIC_WEDGE =        26
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
}

FEVTKExport::~FEVTKExport(void)
{
}

void FEVTKExport::ExportAllStates(bool b)
{
    m_bwriteAllStates = b;
}

bool FEVTKExport::Save(FEModel& fem, const char* szfile)
{
    int ns = fem.GetStates();
    if (ns == 0) return false;

	if (m_bwriteAllStates)
	{
		char szroot[256] = {0}, szname[256] = {0}, szext[16] = {0};
		const char* sz;
		sz = strrchr(szfile, '.');
		if (sz == 0) {
			strcpy(szroot, szfile);
			strcat(szroot,".");
			strcpy(szext,".vtk");
		}
		else {
			int l = sz - szfile + 1;
			strncpy(szroot, szfile, l);
			szroot[l] = 0;
			strcpy(szext, sz);
		}
    
		// save each state in a separate file
		int l0 = (int) log10((double)ns) + 1;
		for (int is=0; is<ns; ++is) 
		{
			if (sprintf(szname, "%st%0*d%s",szroot,l0,is,szext) < 0) return false;

			if (WriteState(szname, fem.GetState(is)) == false) return false;
		}

		return true;
	}
	else
	{
		FEState* state = fem.GetState(fem.currentTime());
		return WriteState(szfile, state);
	}
}        

bool FEVTKExport::WriteState(const char* szname, FEState* ps)
{
	FEMeshBase* pm = ps->GetFEMesh();
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
	FEMeshBase& m = *ps->GetFEMesh();
	int nodes = m.Nodes();
	fprintf(m_fp, "POINTS %d float\n", nodes);
	for (int j=0; j<nodes; j += 3)
	{
	    for (int k =0; k<3 && j+k<nodes;k++)
	    {
	        vec3f& r = ps->m_NODE[j+k].m_rt;
	        fprintf(m_fp, "%g %g %g ", r.x, r.y, r.z);
	    }
	    fprintf(m_fp, "\n");
	}
	fprintf(m_fp, "%s\n" ,"");
}

//-----------------------------------------------------------------------------
void FEVTKExport::WriteCells(FEState* ps)
{
	FEMeshBase&m = *ps->GetFEMesh();
	int NE = m.Elements();
    int nsize = 0;
	for (int j = 0; j<NE; ++j)
        nsize += m.Element(j).Nodes() + 1;

	// Write CELLS
    fprintf(m_fp, "CELLS %d %d\n", NE, nsize);
    for (int j=0; j<m.Elements(); ++j)
    {
        FEElement& el = m.Element(j);
        fprintf(m_fp, "%d ", el.Nodes());
        for (int k=0; k<el.Nodes(); ++k) fprintf(m_fp, "%d ", el.m_node[k]);
        fprintf(m_fp, "\n");
    }
        
	// Write CELL_TYPES
    fprintf(m_fp, "\nCELL_TYPES %d\n", NE);
	for (int j = 0; j<m.Elements(); ++j)
    {
        FEElement& el = m.Element(j);
        int vtk_type;
        switch (el.Type()) {
            case FE_HEX8   : vtk_type = VTK_HEXAHEDRON; break;
            case FE_TET4   : vtk_type = VTK_TETRA; break;
            case FE_PENTA6 : vtk_type = VTK_WEDGE; break;
            case FE_QUAD4  : vtk_type = VTK_QUAD; break;
            case FE_TRI3   : vtk_type = VTK_TRIANGLE; break;
            case FE_LINE2  : vtk_type = VTK_LINE; break;
            case FE_HEX20  : vtk_type = VTK_QUADRATIC_HEXAHEDRON; break;
            case FE_QUAD8  : vtk_type = VTK_QUADRATIC_QUAD; break;
            case FE_LINE3  : vtk_type = VTK_QUADRATIC_EDGE; break;
            case FE_TET10  : vtk_type = VTK_QUADRATIC_TETRA; break;
            case FE_TET15  : vtk_type = VTK_QUADRATIC_TETRA; break;
            case FE_PENTA15: vtk_type = VTK_QUADRATIC_WEDGE; break;
            case FE_HEX27  : vtk_type = VTK_QUADRATIC_HEXAHEDRON; break;
            case FE_TRI6   : vtk_type = VTK_QUADRATIC_TRIANGLE; break;
            case FE_QUAD9  : vtk_type = VTK_QUADRATIC_QUAD; break;
            default: vtk_type = -1; break;
        }
            
        fprintf(m_fp, "%d\n", vtk_type);
    }
}

//-----------------------------------------------------------------------------
void FEVTKExport::WritePointData(FEState* ps)
{
	// make sure the state has data
	int NDATA = ps->m_Data.size();
	if (NDATA == 0) return;

	FEMeshBase& mesh = *ps->GetFEMesh();
	int nodes = mesh.Nodes();

	fprintf(m_fp, "\nPOINT_DATA %d\n", nodes);
	FEModel& fem = *ps->GetFEModel();
	FEDataManager& DM = *fem.GetDataManager();
	FEDataFieldPtr pd = DM.FirstDataField();
	for (int n = 0; n<NDATA; ++n, ++pd)
	{
		FEDataField& data = *(*pd);
		if ((data.DataClass() == CLASS_NODE) && (data.Flags() & EXPORT_DATA))
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
				if (ntype == DATA_FLOAT) {
					fprintf(m_fp, "%s %s %s\n", "SCALARS", szname, "float");
					fprintf(m_fp, "%s %s\n", "LOOKUP_TABLE", "default");
					for (int i = 0; i<val.size(); ++i) fprintf(m_fp, "%g\n", val[i]);
				}
				else if (ntype == DATA_VEC3F) {
					fprintf(m_fp, "%s %s %s\n", "VECTORS", szname, "float");
					for (int i = 0; i<val.size(); i += 3) fprintf(m_fp, "%g %g %g\n", val[i], val[i + 1], val[i + 2]);
				}
				else if (ntype == DATA_MAT3FS) {
					fprintf(m_fp, "%s %s %s\n", "TENSORS", szname, "float");
					for (int i = 0; i<val.size(); i += 6)
						fprintf(m_fp, "%g %g %g\n%g %g %g\n%g %g %g\n\n",
							val[i    ], val[i + 3], val[i + 5],
							val[i + 3], val[i + 1], val[i + 4],
							val[i + 5], val[i + 4], val[i + 2]);
				}
				else if (ntype == DATA_MAT3FD) {
					fprintf(m_fp, "%s %s %s\n", "TENSORS", szname, "float");
					for (int i = 0; i<val.size(); i += 3)
						fprintf(m_fp, "%g %g %g\n%g %g %g\n%g %g %g\n\n",
							val[i], 0.f, 0.f,
							0.f, val[i + 1], 0.f,
							0.f, 0.f, val[i + 2]);
				}
			}
		}

		// --- E L E M E N T   P O I N T   D A T A ---
		if ((data.DataClass() == CLASS_ELEM) && (data.Flags() & EXPORT_DATA))
		{
			FEMeshData& meshData = ps->m_Data[n];
			Data_Format dfmt = meshData.GetFormat();
			if (dfmt == DATA_NODE) {
				FEDataField& data = *(*pd);
				char szname[256];
				strcpy(szname, data.GetName().c_str());
				Space2_(szname);

				// value array
				vector<float> val;
				if (FillElementNodeDataArray(val, meshData))
				{
					// write the value array
					int ntype = meshData.GetType();
					if (ntype == DATA_FLOAT) {
						fprintf(m_fp, "SCALARS %s float\n", szname);
						fprintf(m_fp, "LOOKUP_TABLE default\n");
						for (int i = 0; i<val.size(); ++i) fprintf(m_fp, "%g\n", val[i]);
					}
					else if (ntype == DATA_VEC3F) {
						fprintf(m_fp, "VECTORS %s float\n", szname);
						for (int i = 0; i<val.size(); i += 3) fprintf(m_fp, "%g %g %g\n", val[i], val[i + 1], val[i + 2]);
					}
					else if (ntype == DATA_MAT3FS) {
						fprintf(m_fp, "TENSORS %s float\n", szname);
						for (int i = 0; i<val.size(); i += 6)
							fprintf(m_fp, "%g %g %g\n%g %g %g\n%g %g %g\n\n",
								val[i], val[i + 3], val[i + 5],
								val[i + 3], val[i + 1], val[i + 4],
								val[i + 5], val[i + 4], val[i + 2]);
					}
					else if (ntype == DATA_MAT3FD) {
						fprintf(m_fp, "TENSORS %s float\n", szname);
						for (int i = 0; i<val.size(); i += 3)
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
	FEModel& fem = *ps->GetFEModel();
    FEDataManager& DM = *fem.GetDataManager();
    FEDataFieldPtr pd = DM.FirstDataField();

	FEMeshBase& mesh = *ps->GetFEMesh();
            
    int NDATA = ps->m_Data.size();
    if (NDATA > 0) fprintf(m_fp, "\nCELL_DATA %d\n" , mesh.Elements());
            
    for (int n=0; n<NDATA; ++n, ++pd)
    {
        FEDataField& data = *(*pd);
        if ((data.DataClass() == CLASS_ELEM) && (data.Flags() & EXPORT_DATA))
        {
            FEMeshData& meshData = ps->m_Data[n];
            Data_Format dfmt = meshData.GetFormat();
            if (dfmt == DATA_ITEM) {
                FEDataField& data = *(*pd);
                char szname[256];
				strcpy(szname, data.GetName().c_str());
                Space2_(szname);
                        
                // value array
                vector<float> val;
                        
				bool first = true;
                int ND = mesh.Parts();
                for (int i=0; i<ND; ++i)
                {
                    FEPart& part = mesh.Part(i);
                            
                    if (FillElemDataArray(val, meshData, part))
                    {
						// write the value array
						if (val.empty() == false) {
							int ntype = meshData.GetType();
							if (ntype == DATA_FLOAT) {
								if (first) {
									fprintf(m_fp, "%s %s %s\n" ,"SCALARS",szname,"float");
									fprintf(m_fp, "%s %s\n","LOOKUP_TABLE","default");
								}
								for (int i=0; i<val.size(); ++i) fprintf(m_fp,"%g\n",val[i]);
							}
							else if (ntype == DATA_VEC3F) {
								if (first) {
									fprintf(m_fp, "%s %s %s\n" ,"VECTORS",szname,"float");
								}
								for (int i=0; i<val.size(); i+=3) fprintf(m_fp,"%g %g %g\n",val[i],val[i+1],val[i+2]);
							}
							else if (ntype == DATA_MAT3FS) {
								if (first) {
									fprintf(m_fp, "%s %s %s\n" ,"TENSORS",szname,"float");
								}
								for (int i=0; i<val.size(); i+=6)
									fprintf(m_fp,"%g %g %g\n%g %g %g\n%g %g %g\n\n",
											val[i  ],val[i+3],val[i+5],
											val[i+3],val[i+1],val[i+4],
											val[i+5],val[i+4],val[i+2]);
							}
							else if (ntype == DATA_MAT3FD) {
								if (first) {
									fprintf(m_fp, "%s %s %s\n" ,"TENSORS",szname,"float");
								}
								for (int i=0; i<val.size(); i+=3)
									fprintf(m_fp,"%g %g %g\n%g %g %g\n%g %g %g\n\n",
											val[i  ],0.f,0.f,
											0.f,val[i+1],0.f,
											0.f,0.f,val[i+2]);
							}

							first = false;
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
bool FEVTKExport::FillNodeDataArray(vector<float>& val, FEMeshData& meshData)
{
	FEModel& fem = *meshData.GetFEModel();
	FEMeshBase& mesh = *fem.GetFEMesh(0);

	int ntype = meshData.GetType();
	int NN = mesh.Nodes();

	if (ntype == DATA_FLOAT)
	{
		FENodeData<float>& data = dynamic_cast<FENodeData<float>&>(meshData);
		val.assign(NN, 0.f);
		for (int i=0; i<NN; ++i) val[i] = data[i];
	}
	else if (ntype == DATA_VEC3F)
	{
		FENodeData<vec3f>& data = dynamic_cast<FENodeData<vec3f>&>(meshData);
		val.assign(NN*3, 0.f);
		for (int i=0; i<NN; ++i) write_data(val, i, data[i]);
	}
	else if (ntype == DATA_MAT3FS)
	{
		FENodeData<mat3fs>& data = dynamic_cast<FENodeData<mat3fs>&>(meshData);
		val.assign(NN*6, 0.f);
		for (int i=0; i<NN; ++i) write_data(val, i, data[i]);
	}
	else if (ntype == DATA_MAT3FD)
	{
		FENodeData<mat3fd>& data = dynamic_cast<FENodeData<mat3fd>&>(meshData);
		val.assign(NN*3, 0.f);
		for (int i=0; i<NN; ++i) write_data(val, i, data[i]);
	}
	else return false;

	return true;
}

//-----------------------------------------------------------------------------
bool FEVTKExport::FillElementNodeDataArray(vector<float>& val, FEMeshData& meshData)
{
	int ntype = meshData.GetType();
	int nfmt = meshData.GetFormat();

	if (nfmt != DATA_NODE) return false;

	FEMeshBase& mesh = *meshData.GetFEMesh();
	int NN = mesh.Nodes();
	int nstride = 0;
	switch (ntype)
	{
	case DATA_FLOAT: nstride = 1; break;
	case DATA_VEC3F: nstride = 3; break;
	case DATA_MAT3FS: nstride = 6; break;
	case DATA_MAT3FD: nstride = 3; break;
	default:
		return false;
	}

	int nsize = NN*nstride;
	val.assign(nsize, 0.f);

	int NE = mesh.Elements();

	if (ntype == DATA_FLOAT)
	{
		FEElementData<float, DATA_NODE>& data = dynamic_cast<FEElementData<float, DATA_NODE>&>(meshData);
		float v[FEGenericElement::MAX_NODES];
		for (int i = 0; i<NE; ++i)
		{
			FEElement& el = mesh.Element(i);
			if (data.active(i))
			{
				data.eval(i, v);
				int ne = el.Nodes();
				for (int j = 0; j<ne; ++j) val[el.m_node[j]] = v[j];
			}
		}
	}
	else if (ntype == DATA_VEC3F)
	{
		FEElementData<vec3f, DATA_NODE>& data = dynamic_cast<FEElementData<vec3f, DATA_NODE>&>(meshData);
		vec3f v[FEGenericElement::MAX_NODES];
		for (int i = 0; i<NE; ++i)
		{
			FEElement& el = mesh.Element(i);
			if (data.active(i))
			{
				data.eval(i, v);
				int ne = el.Nodes();
				for (int j = 0; j<ne; ++j) write_data(val, el.m_node[j], v[j]);
			}
		}
	}
	else if (ntype == DATA_MAT3FS)
	{
		FEElementData<mat3fs, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fs, DATA_NODE>&>(meshData);
		val.assign(NN * 6, 0.f);

		mat3fs v[FEGenericElement::MAX_NODES];
		for (int i = 0; i<NE; ++i)
		{
			FEElement& el = mesh.Element(i);
			if (data.active(i))
			{
				data.eval(i, v);
				int ne = el.Nodes();
				for (int j = 0; j<ne; ++j) write_data(val, el.m_node[j], v[j]);
			}
		}
	}
	else if (ntype == DATA_MAT3FD)
	{
		FEElementData<mat3fd, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fd, DATA_NODE>&>(meshData);
		val.assign(NN * 3, 0.f);

		mat3fd v[FEGenericElement::MAX_NODES];
		for (int i = 0; i<NE; ++i)
		{
			FEElement& el = mesh.Element(i);
			if (data.active(i))
			{
				data.eval(i, v);
				int ne = el.Nodes();
				for (int j = 0; j<ne; ++j) write_data(val, el.m_node[j], v[j]);
			}
		}
	}
	else return false;

	return true;
}

//-----------------------------------------------------------------------------
bool FEVTKExport::FillElemDataArray(vector<float>& val, FEMeshData& meshData, FEPart& part)
{
	FEModel& fem = *meshData.GetFEModel();
	FEMeshBase& mesh = *fem.GetFEMesh(0);

	int ntype = meshData.GetType();
	int nfmt  = meshData.GetFormat();

	int NE = part.Size();
	if (NE == 0) return false;

	// number of nodes per element
	int ne = mesh.Element(part.m_Elem[0]).Nodes();

	// number of actually written values
	int nval = 0;

	if (nfmt == DATA_ITEM)
	{
		if (ntype == DATA_FLOAT)
		{
			FEElementData<float, DATA_ITEM>& data = dynamic_cast<FEElementData<float, DATA_ITEM>&>(meshData);
			val.assign(NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid)) { data.eval(eid, &val[i]); nval++; }
			}
		}
		else if (ntype == DATA_VEC3F)
		{
			FEElementData<vec3f, DATA_ITEM>& data = dynamic_cast<FEElementData<vec3f, DATA_ITEM>&>(meshData);
			val.assign(3*NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid))
				{
					vec3f v(0.f, 0.f, 0.f);
					data.eval(eid, &v);
					write_data(val, i, v);

					nval++; 
				}
			}
		}
		else if (ntype == DATA_MAT3FS)
		{
			FEElementData<mat3fs, DATA_ITEM>& data = dynamic_cast<FEElementData<mat3fs, DATA_ITEM>&>(meshData);
			val.assign(6*NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];

				if (data.active(eid))
				{
					mat3fs v;
					data.eval(eid, &v);
					write_data(val, i, v);

					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FD)
		{
			FEElementData<mat3fd, DATA_ITEM>& data = dynamic_cast<FEElementData<mat3fd, DATA_ITEM>&>(meshData);
			val.assign(3*NE, 0.f);
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];

				if (data.active(eid))
				{
					mat3fd v;
					data.eval(eid, &v);
					write_data(val, i, v);

					nval++;
				}
			}
		}
		else return false;
	}
	else if (nfmt == DATA_COMP)
	{
		if (ntype == DATA_FLOAT)
		{
			FEElementData<float, DATA_COMP>& data = dynamic_cast<FEElementData<float, DATA_COMP>&>(meshData);
			val.assign(NE*ne, 0.f);
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				float v[FEGenericElement::MAX_NODES] = {0.f};
				if (data.active(eid))
				{
					data.eval(eid, v);
					for (int j=0; j<ne; ++j) val[i*ne + j] = v[j];
					nval++;
				}
			}
		}
		else if (ntype == DATA_VEC3F)
		{
			FEElementData<vec3f, DATA_COMP>& data = dynamic_cast<FEElementData<vec3f, DATA_COMP>&>(meshData);
			val.assign(NE*ne*3, 0.f);
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				vec3f v[FEGenericElement::MAX_NODES] = {vec3f(0.f,0.f,0.f)};
				if (data.active(eid))
				{
					data.eval(eid, v);
					for (int j=0; j<ne; ++j) write_data(val, i*ne + j, v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FS)
		{
			FEElementData<mat3fs, DATA_COMP>& data = dynamic_cast<FEElementData<mat3fs, DATA_COMP>&>(meshData);
			val.assign(NE*ne*6, 0.f);
			mat3fs v[FEGenericElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid))
				{
					data.eval(eid, v);
					for (int j=0; j<ne; ++j) write_data(val, i*ne + j, v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FD)
		{
			FEElementData<mat3fd, DATA_COMP>& data = dynamic_cast<FEElementData<mat3fd, DATA_COMP>&>(meshData);
			val.assign(NE*ne*3, 0.f);
			mat3fd v[FEGenericElement::MAX_NODES];
			for (int i=0; i<NE; ++i)
			{
				int eid = part.m_Elem[i];
				if (data.active(eid))
				{
					data.eval(eid, v);
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

		if (ntype == DATA_FLOAT)
		{
			FEElementData<float, DATA_NODE>& data = dynamic_cast<FEElementData<float, DATA_NODE>&>(meshData);
			val.assign(NN, 0.f);

			float v[FEGenericElement::MAX_NODES];
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
		else if (ntype == DATA_VEC3F)
		{
			FEElementData<vec3f, DATA_NODE>& data = dynamic_cast<FEElementData<vec3f, DATA_NODE>&>(meshData);
			val.assign(NN*3, 0.f);

			vec3f v[FEGenericElement::MAX_NODES];
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
		else if (ntype == DATA_MAT3FS)
		{
			FEElementData<mat3fs, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fs, DATA_NODE>&>(meshData);
			val.assign(NN*6, 0.f);

			mat3fs v[FEGenericElement::MAX_NODES];
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
		else if (ntype == DATA_MAT3FD)
		{
			FEElementData<mat3fd, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fd, DATA_NODE>&>(meshData);
			val.assign(NN*3, 0.f);

			mat3fd v[FEGenericElement::MAX_NODES];
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
	else return false;

	return true;
}
