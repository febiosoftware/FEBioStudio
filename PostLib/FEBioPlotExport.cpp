#include "stdafx.h"
#include "FEBioPlotExport.h"
#include "FEModel.h"
#include "FEMeshData_T.h"
using namespace Post;

//=============================================================================
// MeshPartition
//=============================================================================

// helper class for creating partitions
class MeshPartition
{
public:
	MeshPartition(FEMeshBase& mesh, const vector<int>& elem, int matid, int ntype) : m_mesh(mesh), m_elem(elem) 
	{
		m_matid = matid;
		m_ntype = ntype;
	}

public:
	int		m_matid;
	int		m_ntype;
	FEMeshBase&		m_mesh;
	vector<int>	m_elem;
};

//=============================================================================
// FEBioPlotExport
//=============================================================================

FEBioPlotExport::FEBioPlotExport()
{
	m_szerr[0] = 0;
	m_ncompress = 0;
}

bool FEBioPlotExport::error(const char* sz)
{
	strcpy(m_szerr, sz);
	return false;
}

bool FEBioPlotExport::Save(FEModel& fem, const char* szfile)
{
	m_szerr[0] = 0;

	if (m_ar.Create(szfile) == false) return error("Failed creating archive");

	// write the root section (don't compress root)
	m_ar.SetCompression(0);
	if (WriteRoot(fem) == false)
	{
		m_ar.Close();
		return false;
	}

	// set compression option
	m_ar.SetCompression(m_ncompress);

	// write the state data
	int NS = fem.GetStates();
	for (int i=0; i<NS; ++i)
	{
		if (WriteState(fem, *fem.GetState(i)) == false)
		{
			m_ar.Close();
			return false;
		}
	}

	// don't forget to close
	m_ar.Close();

	return true;
}

//----------------------------------------------------------------------------
// Write the root section of the file
bool FEBioPlotExport::WriteRoot(FEModel& fem)
{
	m_ar.BeginChunk(PLT_ROOT);
	{
		// write header section
		m_ar.BeginChunk(PLT_HEADER);
		{
			if (WriteHeader(fem) == false) return false;
		}
		m_ar.EndChunk();

		// write dictionary section
		m_ar.BeginChunk(PLT_DICTIONARY);
		{
			if (WriteDictionary(fem) == false) return false;
		}
		m_ar.EndChunk();

		// write material section
		m_ar.BeginChunk(PLT_MATERIALS);
		{
			if (WriteMaterials(fem) == false) return false;
		}
		m_ar.EndChunk();

		// write geometry section
		m_ar.BeginChunk(PLT_GEOMETRY);
		{
			if (WriteGeometry(fem) == false) return false;
		}
		m_ar.EndChunk();
	}
	m_ar.EndChunk();

	return true;
}

//----------------------------------------------------------------------------
bool FEBioPlotExport::WriteHeader(FEModel& fem)
{
	// setup the header
	unsigned int nversion = PLT_VERSION;

	// output header
	m_ar.WriteChunk(PLT_HDR_VERSION, nversion);

	int N = fem.GetFEMesh(0)->Nodes();
	m_ar.WriteChunk(PLT_HDR_NODES, N);

	// max number of nodes per facet
	int n = (int) PLT_MAX_FACET_NODES;
	m_ar.WriteChunk(PLT_HDR_MAX_FACET_NODES, n);

	// compression flag
	m_ar.WriteChunk(PLT_HDR_COMPRESSION, m_ncompress);

	return true;
}

//----------------------------------------------------------------------------
bool FEBioPlotExport::WriteDictionary(FEModel& fem)
{
	// get the data manager
	FEDataManager& DM = *fem.GetDataManager();

	int NDATA = DM.DataFields();
	if (NDATA == 0) return false;

	// store nodal variables
	m_nodeData = 0;
	FEDataFieldPtr pd = DM.FirstDataField();
	for (int i=0; i<NDATA; ++i, ++pd)
	{
		FEDataField& data = *(*pd);
		if ((data.DataClass() == CLASS_NODE) && (data.Flags() & EXPORT_DATA)) m_nodeData++;
	}

	if (m_nodeData > 0)
	{
		m_ar.BeginChunk(PLT_DIC_NODAL);
		{
			FEDataFieldPtr pd = DM.FirstDataField();
			for (int i=0; i<NDATA; ++i, ++pd)
			{
				FEDataField& data = *(*pd);
				if ((data.DataClass() == CLASS_NODE) && (data.Flags() & EXPORT_DATA))
					if (WriteDataField(data) == false) return false;
			}
		}
		m_ar.EndChunk();
	}

	// store element variables
	m_elemData = 0;
	pd = DM.FirstDataField();
	for (int i=0; i<NDATA; ++i, ++pd)
	{
		FEDataField& data = *(*pd);
		if ((data.DataClass() == CLASS_ELEM) && (data.Flags() & EXPORT_DATA)) m_elemData++;
	}

	if (m_elemData > 0)
	{
		m_ar.BeginChunk(PLT_DIC_DOMAIN);
		{
			FEDataFieldPtr pd = DM.FirstDataField();
			for (int i=0; i<NDATA; ++i, ++pd)
			{
				FEDataField& data = *(*pd);
				if ((data.DataClass() == CLASS_ELEM) && (data.Flags() & EXPORT_DATA))
					if (WriteDataField(data) == false) return false;
			}
		}
		m_ar.EndChunk();
	}

	// store surface data
	m_faceData = 0;
	pd = DM.FirstDataField();
	for (int i=0; i<NDATA; ++i, ++pd)
	{
		FEDataField& data = *(*pd);
		if ((data.DataClass() == CLASS_FACE) && (data.Flags() & EXPORT_DATA)) m_faceData++;
	}

	if (m_faceData > 0)
	{
		m_ar.BeginChunk(PLT_DIC_SURFACE);
		{
			FEDataFieldPtr pd = DM.FirstDataField();
			for (int i=0; i<NDATA; ++i, ++pd)
			{
				FEDataField& data = *(*pd);
				if ((data.DataClass() == CLASS_FACE) && (data.Flags() & EXPORT_DATA))
					if (WriteDataField(data) == false) return false;
			}
		}
		m_ar.EndChunk();
	}

	return true;
}

//----------------------------------------------------------------------------
bool FEBioPlotExport::WriteDataField(FEDataField& data)
{
	// get the data type
	unsigned int ntype = 0;
	switch (data.Type())
	{
	case DATA_FLOAT  : ntype = FLOAT; break;
	case DATA_VEC3F  : ntype = VEC3F; break;
	case DATA_MAT3FS : ntype = MAT3FS; break;
	case DATA_MAT3FD : ntype = MAT3FD; break;
	case DATA_TENS4FS: ntype = TENS4FS; break;
	case DATA_MAT3F  : ntype = MAT3F; break;
	case DATA_MAT3D  : ntype = MAT3F; break;
	default:
		return error("Unknown data type in WriteDataField");
	}

	// get the data format
	unsigned int nfmt = 0;
	switch (data.Format())
	{
	case DATA_NODE  : nfmt = FMT_NODE; break;
	case DATA_ITEM  : nfmt = FMT_ITEM; break;
	case DATA_COMP  : nfmt = FMT_MULT; break;
	case DATA_REGION: nfmt = FMT_REGION; break;
	default:
		return error("Unknown data format in WriteDataField");
	}

	// we need to cast away the const on the name
	char* szname = const_cast<char*>(data.GetName().c_str());

	// store the dictionary entry
	m_ar.BeginChunk(PLT_DIC_ITEM);
	{
		m_ar.WriteChunk(PLT_DIC_ITEM_TYPE, ntype);
		m_ar.WriteChunk(PLT_DIC_ITEM_FMT , nfmt);
		m_ar.WriteChunk(PLT_DIC_ITEM_NAME, szname, STR_SIZE);
	}
	m_ar.EndChunk();

	return true;
}

//----------------------------------------------------------------------------
bool FEBioPlotExport::WriteMaterials(FEModel& fem)
{
	int NMAT = fem.Materials();
	for (int i=0; i<NMAT; ++i)
	{
		FEMaterial* pm = fem.GetMaterial(i);
		m_ar.BeginChunk(PLT_MATERIAL);
		{
			unsigned int nid = i+1;
			char szname[STR_SIZE] = {0};

			// Make sure that the material name fits in the buffer
			const char* sz = pm->GetName();
			int l = (int)strlen(sz);
			if (l >= STR_SIZE) l = STR_SIZE - 1;
			strncpy(szname, sz, l);

			// write the material data
			m_ar.WriteChunk(PLT_MAT_ID, nid);
			m_ar.WriteChunk(PLT_MAT_NAME, szname, STR_SIZE);
		}
		m_ar.EndChunk();
	}
	return true;
}

//----------------------------------------------------------------------------
bool FEBioPlotExport::WriteGeometry(FEModel& fem)
{
	// get the mesh
	FEMeshBase& m = *fem.GetFEMesh(0);

	// node section
	m_ar.BeginChunk(PLT_NODE_SECTION);
	{
		if (WriteNodeSection(m) == false) return false;
	}
	m_ar.EndChunk();

	// domain section
	m_ar.BeginChunk(PLT_DOMAIN_SECTION);
	{
		if (WritePartSection(m) == false) return false;
	}
	m_ar.EndChunk();

	// surface section
	if (m.Surfaces() > 0)
	{
		m_ar.BeginChunk(PLT_SURFACE_SECTION);
		{
			if (WriteSurfaceSection(m) == false) return false;
		}
		m_ar.EndChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WriteNodeSection(FEMeshBase& m)
{
	// write the reference coordinates
	int NN = m.Nodes();
	vector<float> X(3*NN);
	for (int i=0; i<m.Nodes(); ++i)
	{
		FENode& node = m.Node(i);
		X[3*i  ] = (float) node.m_r0.x;
		X[3*i+1] = (float) node.m_r0.y;
		X[3*i+2] = (float) node.m_r0.z;
	}

	m_ar.WriteChunk(PLT_NODE_COORDS, X);

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WritePartSection(FEMeshBase& mesh)
{
	// make sure there are parts
	int NP = mesh.Parts();
	if (NP == 0) return false;

	// write all partitions
	for (int nd=0; nd < NP; ++nd)
	{
		m_ar.BeginChunk(PLT_DOMAIN);
		{
			if (WritePart(mesh.Part(nd)) == false) return false;
		}
		m_ar.EndChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WritePart(FEPart& part)
{
	// number of elements
	int NE = part.Size();
	if (NE == 0) return false;

	// figure out element type
	FEMeshBase& mesh = *part.GetMesh();
	FEElement& e0 = mesh.Element(part.m_Elem[0]);
	int matid = e0.m_MatID + 1;

	int ne = 0;
	int dtype = 0;
	switch (e0.Type())
	{
	case FE_HEX8   : ne =  8; dtype = PLT_ELEM_HEX8; break;
	case FE_PENTA6 : ne =  6; dtype = PLT_ELEM_PENTA; break;
    case FE_PENTA15: ne = 15; dtype = PLT_ELEM_PENTA15; break;
    case FE_TET4   : ne =  4; dtype = PLT_ELEM_TET; break;
	case FE_TET10  : ne = 10; dtype = PLT_ELEM_TET10; break;
	case FE_TET15  : ne = 15; dtype = PLT_ELEM_TET15; break;
	case FE_TET20  : ne = 20; dtype = PLT_ELEM_TET20; break;
	case FE_HEX20  : ne = 20; dtype = PLT_ELEM_HEX20; break;
	case FE_HEX27  : ne = 27; dtype = PLT_ELEM_HEX27; break;
	case FE_QUAD4  : ne =  4; dtype = PLT_ELEM_QUAD; break;
	case FE_QUAD8  : ne =  8; dtype = PLT_ELEM_QUAD8; break;
	case FE_QUAD9  : ne =  9; dtype = PLT_ELEM_QUAD9; break;
	case FE_TRI3   : ne =  3; dtype = PLT_ELEM_TRI; break;
	case FE_TRI6   : ne =  6; dtype = PLT_ELEM_TRI6; break;
	case FE_LINE2  : ne =  2; dtype = PLT_ELEM_TRUSS; break;
	case FE_PYRA5  : ne =  5; dtype = PLT_ELEM_PYRA5; break;
	default:
		assert(false);
		return error("Unknown element type in WritePart");
	}

	// write the header
	m_ar.BeginChunk(PLT_DOMAIN_HDR);
	{
		m_ar.WriteChunk(PLT_DOM_ELEM_TYPE, dtype);
		m_ar.WriteChunk(PLT_DOM_MAT_ID   , matid);
		m_ar.WriteChunk(PLT_DOM_ELEMS    ,    NE);
		m_ar.WriteChunk(PLT_DOM_NAME     , "part");
	}
	m_ar.EndChunk();

	// write the element list
	int n[FEGenericElement::MAX_NODES + 1];
	m_ar.BeginChunk(PLT_DOM_ELEM_LIST);
	{
		for (int i=0; i<NE; ++i)
		{
			FEElement& el = mesh.Element(part.m_Elem[i]);
			n[0] = el.GetID();
			for (int j=0; j<ne; ++j) n[j+1] = el.m_node[j];
			m_ar.WriteChunk(PLT_ELEMENT, n, ne+1);
		}
	}
	m_ar.EndChunk();

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WriteSurfaceSection(FEMeshBase& mesh)
{
	int NS = mesh.Surfaces();
	for (int n=0; n<NS; ++n)
	{
		FESurface& s = mesh.Surface(n);
		int NF = s.Size();

		m_ar.BeginChunk(PLT_SURFACE);
		{
			m_ar.BeginChunk(PLT_SURFACE_HDR);
			{
				int sid = n+1;
				m_ar.WriteChunk(PLT_SURFACE_ID, sid);
				m_ar.WriteChunk(PLT_SURFACE_FACES, NF);
				m_ar.WriteChunk(PLT_SURFACE_NAME, s.GetName());
			}
			m_ar.EndChunk();

			m_ar.BeginChunk(PLT_FACE_LIST);
			{
				int n[PLT_MAX_FACET_NODES + 2];
				for (int i=0; i<NF; ++i)
				{
					FEFace& f = mesh.Face(s.m_Face[i]);
					int nf = f.Nodes();
					n[0] = i+1;
					n[1] = nf;
					for (int i=0; i<nf; ++i) n[i+2] = f.node[i];
					m_ar.WriteChunk(PLT_FACE, n, PLT_MAX_FACET_NODES+2);
				}
			}
			m_ar.EndChunk();
		}
		m_ar.EndChunk();	
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WriteState(FEModel& fem, FEState& state)
{
	m_ar.BeginChunk(PLT_STATE);
	{
		// state header
		m_ar.BeginChunk(PLT_STATE_HEADER);
		{
			float f = (float) state.m_time;
			m_ar.WriteChunk(PLT_STATE_HDR_TIME, f);
		}
		m_ar.EndChunk();

		m_ar.BeginChunk(PLT_STATE_DATA);
		{
			// Node Data
			if (m_nodeData)
			{
				m_ar.BeginChunk(PLT_NODE_DATA);
				{
					if (WriteNodeData(fem, state) == false) return false;
				}
				m_ar.EndChunk();
			}

			// Element Data
			if (m_elemData)
			{
				m_ar.BeginChunk(PLT_ELEMENT_DATA);
				{
					if (WriteElemData(fem, state) == false) return false;
				}
				m_ar.EndChunk();
			}

			// surface data
			if (m_faceData)
			{
				m_ar.BeginChunk(PLT_FACE_DATA);
				{
					if (WriteFaceData(fem, state) == false) return false;
				}
				m_ar.EndChunk();
			}
		}
		m_ar.EndChunk();
	}
	m_ar.EndChunk();

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WriteNodeData(FEModel& fem, FEState& state)
{
	FEDataManager& DM = *fem.GetDataManager();
	FEDataFieldPtr pd = DM.FirstDataField();

	FEMeshBase& mesh = *fem.GetFEMesh(0);
	int NN = mesh.Nodes();
	int NDATA = state.m_Data.size();
	unsigned int nid = 1;
	for (int n=0; n<NDATA; ++n, ++pd)
	{
		FEDataField& data = *(*pd);
		if ((data.DataClass() == CLASS_NODE) && (data.Flags() & EXPORT_DATA))
		{
			FEMeshData& meshData = state.m_Data[n];
			m_ar.BeginChunk(PLT_STATE_VARIABLE);
			{
				m_ar.WriteChunk(PLT_STATE_VAR_ID, nid); nid++;

				m_ar.BeginChunk(PLT_STATE_VAR_DATA);
				{
					// value array
					vector<float> val;
					if (FillNodeDataArray(val, meshData) == false) return false;

					// write the value array
					if (val.empty() == false) m_ar.WriteChunk(0, val);
				}
				m_ar.EndChunk();
			}
			m_ar.EndChunk();
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WriteElemData(FEModel& fem, FEState& state)
{
	FEDataManager& DM = *fem.GetDataManager();
	FEDataFieldPtr pd = DM.FirstDataField();

	FEMeshBase& mesh = *fem.GetFEMesh(0);
	int NDATA = state.m_Data.size();
	unsigned int nid = 1;
	for (int n=0; n<NDATA; ++n, ++pd)
	{
		FEDataField& data = *(*pd);
		if ((data.DataClass() == CLASS_ELEM) && (data.Flags() & EXPORT_DATA))
		{
			FEMeshData& data = state.m_Data[n];
			m_ar.BeginChunk(PLT_STATE_VARIABLE);
			{
				m_ar.WriteChunk(PLT_STATE_VAR_ID, nid); nid++;

				vector<float> val;
				m_ar.BeginChunk(PLT_STATE_VAR_DATA);
				{
					int ND = mesh.Parts();
					for (int i=0; i<ND; ++i)
					{
						FEPart& part = mesh.Part(i);

						if (FillElemDataArray(val, data, part) == false) return false;

						if (val.empty() == false) m_ar.WriteData(i+1, val);
					}
				}
				m_ar.EndChunk();
			}
			m_ar.EndChunk();
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::WriteFaceData(FEModel& fem, FEState& state)
{
	FEDataManager& DM = *fem.GetDataManager();
	FEDataFieldPtr pd = DM.FirstDataField();

	FEMeshBase& mesh = *fem.GetFEMesh(0);
	int NDATA = state.m_Data.size();
	unsigned int nid = 1;
	for (int n=0; n<NDATA; ++n, ++pd)
	{
		FEDataField& data = *(*pd);
		if ((data.DataClass() == CLASS_FACE) && (data.Flags() & EXPORT_DATA))
		{
			FEMeshData& data = state.m_Data[n];
			m_ar.BeginChunk(PLT_STATE_VARIABLE);
			{
				m_ar.WriteChunk(PLT_STATE_VAR_ID, nid); nid++;

				vector<float> val;
				m_ar.BeginChunk(PLT_STATE_VAR_DATA);
				{
					int NS = mesh.Surfaces();
					for (int i=0; i<NS; ++i)
					{
						FESurface& surf = mesh.Surface(i);

						if (FillFaceDataArray(val, data, surf) == false) return false;

						if (val.empty() == false) m_ar.WriteData(i+1, val);
					}
				}
				m_ar.EndChunk();
			}
			m_ar.EndChunk();
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
inline void write_data(vector<float>& val, int index, const vec3f& v)
{
	val[3*index  ] = v.x;
	val[3*index+1] = v.y;
	val[3*index+2] = v.z;
}

inline void write_data(vector<float>& val, int index, const mat3fs& v)
{
	val[6*index  ] = v.x;
	val[6*index+1] = v.y;
	val[6*index+2] = v.z;
	val[6*index+3] = v.xy;
	val[6*index+4] = v.yz;
	val[6*index+5] = v.xz;
}

inline void write_data(vector<float>& val, int index, const mat3fd& v)
{
	val[3*index  ] = v.x;
	val[3*index+1] = v.y;
	val[3*index+2] = v.z;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::FillNodeDataArray(vector<float>& val, FEMeshData& meshData)
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
	else return error("Unknown data type in FillNodeDataArray");

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::FillElemDataArray(vector<float>& val, FEMeshData& meshData, FEPart& part)
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
		else return error("Unknown data type in FillElemDataArray");
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
		else return error("Unknown data type in FillElemDataArray");
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
		else return error("Unknown data type in FillElemDataArray");
	}
	else return error("Unknown data format in FillElemDataArray");

	// if no values were written, clear the data array
	// This will skip the data export for this domain
	if (nval == 0) val.clear();

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioPlotExport::FillFaceDataArray(vector<float>& val, FEMeshData& meshData, FESurface& surf)
{
	FEModel& fem = *meshData.GetFEModel();
	FEMeshBase& mesh = *fem.GetFEMesh(0);

	int ntype = meshData.GetType();
	int nfmt  = meshData.GetFormat();

	int NF = surf.Size();
	int nval = 0;
	if (nfmt == DATA_ITEM)
	{
		if (ntype == DATA_FLOAT)
		{
			FEFaceData<float, DATA_ITEM>& data = dynamic_cast<FEFaceData<float, DATA_ITEM>&>(meshData);
			val.assign(NF, 0.f);
			for (int i=0; i<NF; ++i) 
			{
				int fid = surf.m_Face[i];
				if ((fid >=0) && data.active(fid)) { data.eval(fid, &val[i]); nval++; }
			}
		}
		else if (ntype == DATA_VEC3F)
		{
			FEFaceData<vec3f, DATA_ITEM>& data = dynamic_cast<FEFaceData<vec3f, DATA_ITEM>&>(meshData);
			val.assign(3*NF, 0.f);
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				if ((fid >= 0) && data.active(fid))
				{
					vec3f v(0.f, 0.f, 0.f);
					data.eval(fid, &v);
					write_data(val, i, v);

					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FS)
		{
			FEFaceData<mat3fs, DATA_ITEM>& data = dynamic_cast<FEFaceData<mat3fs, DATA_ITEM>&>(meshData);
			val.assign(6*NF, 0.f);
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				if ((fid >= 0) && data.active(fid))
				{
					mat3fs v;
					data.eval(fid, &v);
					write_data(val, i, v);

					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FD)
		{
			FEFaceData<mat3fd, DATA_ITEM>& data = dynamic_cast<FEFaceData<mat3fd, DATA_ITEM>&>(meshData);
			val.assign(3*NF, 0.f);
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				if ((fid >= 0) && data.active(fid))
				{
					mat3fd v;
					data.eval(fid, &v);
					write_data(val, i, v);

					nval++;
				}
			}
		}
		else return error("Unknown data type in FillFaceDataArray");
	}
	else if (nfmt == DATA_COMP)
	{
		if (ntype == DATA_FLOAT)
		{
			FEFaceData<float, DATA_COMP>& data = dynamic_cast<FEFaceData<float, DATA_COMP>&>(meshData);
			val.assign(NF*PLT_MAX_FACET_NODES, 0.f);
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				float v[PLT_MAX_FACET_NODES] = {0.f};
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);

					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) val[i*PLT_MAX_FACET_NODES + j] = v[j];

					nval++;
				}
			}
		}
		else if (ntype == DATA_VEC3F)
		{
			FEFaceData<vec3f, DATA_COMP>& data = dynamic_cast<FEFaceData<vec3f, DATA_COMP>&>(meshData);
			val.assign(NF*PLT_MAX_FACET_NODES*3, 0.f);
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				vec3f v[PLT_MAX_FACET_NODES] = {vec3f(0.f,0.f,0.f)};
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);

					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) write_data(val, i*PLT_MAX_FACET_NODES + j, v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FS)
		{
			FEFaceData<mat3fs, DATA_COMP>& data = dynamic_cast<FEFaceData<mat3fs, DATA_COMP>&>(meshData);
			val.assign(NF*PLT_MAX_FACET_NODES*6, 0.f);
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				mat3fs v[PLT_MAX_FACET_NODES];
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);

					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) write_data(val, i*PLT_MAX_FACET_NODES + j, v[j]);
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FD)
		{
			FEFaceData<mat3fd, DATA_COMP>& data = dynamic_cast<FEFaceData<mat3fd, DATA_COMP>&>(meshData);
			val.assign(NF*PLT_MAX_FACET_NODES*3, 0.f);
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				mat3fd v[PLT_MAX_FACET_NODES];
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);

					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) write_data(val, i*PLT_MAX_FACET_NODES + j, v[j]);
					nval++;
				}
			}
		}
		else return error("Unknown data type in FillFaceDataArray");
	}
	else if (nfmt == DATA_NODE)
	{
		vector<int> node, lnode;
		surf.GetNodeList(node, lnode);
		int NN = (int) node.size();

		if (ntype == DATA_FLOAT)
		{
			FEFaceData<float, DATA_NODE>& data = dynamic_cast<FEFaceData<float, DATA_NODE>&>(meshData);
			val.assign(NN, 0.f);

			float v[FEGenericElement::MAX_NODES];
			int nnf = 0;
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);
					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) val[lnode[nnf + j]] = v[j];
					nnf += nf;
					nval++;
				}
			}
		}
		else if (ntype == DATA_VEC3F)
		{
			FEFaceData<vec3f, DATA_NODE>& data = dynamic_cast<FEFaceData<vec3f, DATA_NODE>&>(meshData);
			val.assign(NN*3, 0.f);

			vec3f v[FEGenericElement::MAX_NODES];
			int nnf = 0;
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);
					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) write_data(val, lnode[nnf + j], v[j]);
					nnf += nf;
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FS)
		{
			FEElementData<mat3fs, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fs, DATA_NODE>&>(meshData);
			val.assign(NN*6, 0.f);

			mat3fs v[FEGenericElement::MAX_NODES];
			int nnf = 0;
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);
					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) write_data(val, lnode[nnf + j], v[j]);
					nnf += nf;
					nval++;
				}
			}
		}
		else if (ntype == DATA_MAT3FD)
		{
			FEElementData<mat3fd, DATA_NODE>& data = dynamic_cast<FEElementData<mat3fd, DATA_NODE>&>(meshData);
			val.assign(NN*3, 0.f);

			mat3fd v[FEGenericElement::MAX_NODES];
			int nnf = 0;
			for (int i=0; i<NF; ++i)
			{
				int fid = surf.m_Face[i];
				if ((fid >= 0) && data.active(fid))
				{
					data.eval(fid, v);
					int nf = mesh.Face(fid).Nodes();
					for (int j=0; j<nf; ++j) write_data(val, lnode[nnf + j], v[j]);
					nnf += nf;
					nval++;
				}
			}
		}
		else return error("Unknown data type in FillFaceDataArray");
	}
	else return error("Unknown data format in FillFaceDataArray");

	if (nval == 0) val.clear();

	return true;
}
