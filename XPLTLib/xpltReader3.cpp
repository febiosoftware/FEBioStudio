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

#include "xpltReader3.h"
#include <MeshLib/FENodeFaceList.h>
#include <MeshLib/FENodeEdgeList.h>
#include <PostLib/FEDataManager.h>
#include <PostLib/FEMeshData_T.h>
#include <PostLib/FEState.h>
#include <PostLib/FEPostMesh.h>
#include <PostLib/FEPostModel.h>

using namespace Post;
using namespace std;

template <class Type> void ReadFaceData_REGION(xpltArchive& ar, Post::FEPostMesh& m, XpltReader3::Surface &s, Post::FEMeshData &data)
{
	int NF = s.nfaces;
	vector<int> face(NF);
	for (int i=0; i<(int) face.size(); ++i) face[i] = s.face[i].nid;

	FEFaceData<Type,DATA_REGION>& df = dynamic_cast<FEFaceData<Type,DATA_REGION>&>(data);
	Type a;
	ar.read(a);
	df.add(face, a);
}

template <class Type> void ReadEdgeData_REGION(xpltArchive& ar, Post::FEPostMesh& m, XpltReader3::Edge& e, Post::FEMeshData& data)
{
	int NL = e.nlines;
	vector<int> line(NL);
	for (int i = 0; i < (int)line.size(); ++i) line[i] = e.line[i].id;

	FEEdgeData<Type, DATA_REGION>& df = dynamic_cast<FEEdgeData<Type, DATA_REGION>&>(data);
	Type a;
	ar.read(a);
	df.add(line, a);
}

template <class Type> void ReadElemData_REGION(xpltArchive& ar, XpltReader3::Domain& dom, Post::FEMeshData& s, int ntype)
{
	int NE = dom.ne;
	vector<int> elem(NE);
	for (int i=0; i<NE; ++i) elem[i] = dom.elem[i].index;

	Type a;
	ar.read(a);
	Post::FEElementData<Type,DATA_REGION>& df = dynamic_cast<Post::FEElementData<Type,DATA_REGION>&>(s);
	df.add(elem, a);
}

//=================================================================================================

XpltReader3::DICT_ITEM::DICT_ITEM()
{
	szname[0] = 0;
	szunit[0] = 0;
}

XpltReader3::DICT_ITEM::DICT_ITEM(const XpltReader3::DICT_ITEM& item)
{
	ntype = item.ntype;
	nfmt = item.nfmt;
	strcpy(szname, item.szname);
	strcpy(szunit, item.szunit);

	index = item.index;

	arraySize = item.arraySize;
	arrayNames = item.arrayNames;
}

//=============================================================================
void XpltReader3::XMesh::Clear()
{
	m_Mat.clear();
	m_Node.clear();
	m_Dom.clear();
	m_Surf.clear();
	m_NodeSet.clear();
	m_ElemSet.clear();
	m_FacetSet.clear();
}

void XpltReader3::XMesh::addMaterial(MATERIAL& mat)
{
	m_Mat.push_back(mat);
}

void XpltReader3::XMesh::addNodes(std::vector<XpltReader3::NODE>& nodes)
{
	m_Node.insert(m_Node.end(), nodes.begin(), nodes.end());
}

void XpltReader3::XMesh::addDomain(XpltReader3::Domain& dom)
{
	m_Dom.push_back(dom);
}

void XpltReader3::XMesh::addSurface(XpltReader3::Surface& surf)
{
	m_Surf.push_back(surf);
}

void XpltReader3::XMesh::addEdge(XpltReader3::Edge& edge)
{
	m_Edge.push_back(edge);
}

void XpltReader3::XMesh::addNodeSet(XpltReader3::NodeSet& nset)
{
	m_NodeSet.push_back(nset);
}

void XpltReader3::XMesh::addElementSet(XpltReader3::ElemSet& eset)
{
	m_ElemSet.push_back(eset);
}

void XpltReader3::XMesh::addFacetSet(XpltReader3::Surface& surf)
{
	m_FacetSet.push_back(surf);
}

//=============================================================================

XpltReader3::XpltReader3(xpltFileReader* xplt) : xpltParser(xplt)
{
	m_pstate = 0;
	m_mesh = 0;
}

XpltReader3::~XpltReader3()
{
}

//-----------------------------------------------------------------------------
void XpltReader3::Clear()
{
	m_dic.Clear();
	m_xmesh.Clear();
	m_bHasDispl = false;
	m_bHasStress = false;
	m_bHasNodalStress = false;
	m_bHasShellThickness = false;
	m_bHasFluidPressure = false;
	m_bHasElasticity = false;
	m_nel = 0;
	m_pstate = 0;
}

//-----------------------------------------------------------------------------
bool XpltReader3::Load(FEPostModel& fem)
{
	// make sure all data is cleared
	Clear();

	// clear the model data
	fem.Clear();

	// read the root section (no compression for this section)
	if (ReadRootSection(fem) == false) return false;

	// Clear the end-flag of the root section
	m_ar.CloseChunk();
	if (m_ar.OpenChunk() != xpltArchive::IO_END) return false;

	// At this point we'll assume we can read the mesh in, so clear the model
	// clear the state data
	fem.ClearStates();

	// read the first Mesh section
	if (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() != PLT_MESH) return errf("Error while reading mesh section");
		if (ReadMesh(fem) == false) return false;
		m_ar.CloseChunk();
	}
	else return errf("Error while reading mesh section");

	// Clear the end-flag of the mesh section
	if (m_ar.OpenChunk() != xpltArchive::IO_END) return false;

	// read the state sections (these could be compressed)
	const xpltFileReader::HEADER& hdr = m_xplt->GetHeader();
	m_ar.SetCompression(hdr.ncompression);
	int read_state_flag = m_xplt->GetReadStateFlag();
	int nstate = 0;
	try{
		while (true)
		{
			if (m_ar.OpenChunk() != xpltArchive::IO_OK) break;

			if (m_ar.GetChunkID() == PLT_STATE)
			{
				if (m_pstate) { delete m_pstate; m_pstate = 0; }
				if (ReadStateSection(fem) == false) break;
				if (read_state_flag == XPLT_READ_ALL_STATES) { fem.AddState(m_pstate); m_pstate = 0; }
				else if (read_state_flag == XPLT_READ_ALL_CONVERGED_STATES) 
				{ 
					if (m_pstate->m_status == 0)
					{
						fem.AddState(m_pstate);
						m_pstate = 0;
					}
				}
				else if (read_state_flag == XPLT_READ_STATES_FROM_LIST)
				{
					vector<int> state_list = m_xplt->GetReadStates();
					int n = (int) state_list.size();
					for (int i=0; i<n; ++i)
					{
						if (state_list[i] == nstate)
						{
							fem.AddState(m_pstate); 
							m_pstate = 0;
							break;
						}
					}
				}
			}
			else if (m_ar.GetChunkID() == PLT_MESH)
			{
				if (ReadMesh(fem) == false) return errf("Error while reading mesh section.");
			}
			else errf("Error while reading state data.");
			m_ar.CloseChunk();
		
			// clear end-flag
			if (m_ar.OpenChunk() != xpltArchive::IO_END)
			{

				break;
			}

			++nstate;
		}
		if (read_state_flag == XPLT_READ_LAST_STATE_ONLY) { fem.AddState(m_pstate); m_pstate = 0; }
	}
	catch (...)
	{
		errf("An unknown exception has occurred.\nNot all data was read in.");
	}

	Clear();

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadRootSection(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		switch (nid)
		{
		case PLT_DICTIONARY: if (ReadDictionary(fem) == false) return false; break;
		default:
			return errf("Failed reading Root section");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadDictItem(DICT_ITEM& it)
{
	char szname[64] = {0};
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		switch(nid)
		{
		case PLT_DIC_ITEM_TYPE: m_ar.read(it.ntype); break;
		case PLT_DIC_ITEM_FMT : m_ar.read(it.nfmt ); break;
		case PLT_DIC_ITEM_ARRAYSIZE: m_ar.read(it.arraySize); break;
		case PLT_DIC_ITEM_ARRAYNAME:
		{
			char tmp[DI_NAME_SIZE] = { 0 };
			m_ar.read(tmp, DI_NAME_SIZE);
			it.arrayNames.push_back(tmp);
		}
		break;
		case PLT_DIC_ITEM_NAME:
			{
				m_ar.read(szname, DI_NAME_SIZE);
				char* sz = strchr(szname, '=');
				if (sz) *sz++ = 0; else sz = szname;
				strcpy(it.szname, sz);
			}
			break;
		case PLT_DIC_ITEM_UNITS: m_ar.read(it.szunit, DI_NAME_SIZE); break;
		default:
			return errf("Error while reading dictionary section");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadDictionary(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		switch (nid)
		{
		case PLT_DIC_GLOBAL   : ReadGlobalDicItems  (); break;
		case PLT_DIC_NODAL    : ReadNodeDicItems    (); break;
		case PLT_DIC_DOMAIN   : ReadElemDicItems    (); break;
		case PLT_DIC_SURFACE  : ReadFaceDicItems    (); break;
		case PLT_DIC_EDGE     : ReadEdgeDicItems    (); break;
		default:
			return errf("Error while reading Dictionary.");
		}
		m_ar.CloseChunk();
	}

	// clear data manager
	FEDataManager* pdm = fem.GetDataManager();
	pdm->Clear();

	// read global variables
	int nfields = 0;
	int i;
	int nv = (int)m_dic.m_Glb.size();
	for (i = 0; i < nv; ++i)
	{
		DICT_ITEM& it = m_dic.m_Glb[i];
		it.index = nfields++;

		// add global field
		Post::ModelDataField* pdf = nullptr;
		switch (it.ntype)
		{
		case FLOAT  : pdf = new FEDataField_T<Post::FEGlobalData_T<float  > >(&fem, EXPORT_DATA); break;
		case VEC3F  : pdf = new FEDataField_T<Post::FEGlobalData_T<vec3f  > >(&fem, EXPORT_DATA); break;
		case MAT3FS : pdf = new FEDataField_T<Post::FEGlobalData_T<mat3fs > >(&fem, EXPORT_DATA); break;
		case MAT3FD : pdf = new FEDataField_T<Post::FEGlobalData_T<mat3fd > >(&fem, EXPORT_DATA); break;
		case TENS4FS: pdf = new FEDataField_T<Post::FEGlobalData_T<tens4fs> >(&fem, EXPORT_DATA); break;
		case MAT3F  : pdf = new FEDataField_T<Post::FEGlobalData_T<mat3f  > >(&fem, EXPORT_DATA); break;
		case ARRAY:
		{
			FEArrayDataField* data = new FEArrayDataField(&fem, OBJECT_DATA, DATA_REGION, Post::EXPORT_DATA);
			data->SetArraySize(it.arraySize);
			data->SetArrayNames(it.arrayNames);
			pdf = data;
		}
		break;
		default:
			return errf("Error while reading dictionary.");
		}
		if (pdf == nullptr) return false;

		if (it.szunit[0]) pdf->SetUnits(it.szunit);

		pdm->AddDataField(pdf, it.szname);
	}

	// read nodal variables
	nv = (int)m_dic.m_Node.size();
	for (i=0; i<nv; ++i)
	{
		DICT_ITEM& it = m_dic.m_Node[i];
		it.index = nfields++;

		// add nodal field
		Post::ModelDataField* pdf = nullptr;
		switch (it.ntype)
		{
		case FLOAT  : pdf = new FEDataField_T<Post::FENodeData<float  > >(&fem, EXPORT_DATA); break;
		case VEC3F  : pdf = new FEDataField_T<Post::FENodeData<vec3f  > >(&fem, EXPORT_DATA); break;
		case MAT3FS : pdf = new FEDataField_T<Post::FENodeData<mat3fs > >(&fem, EXPORT_DATA); break;
		case MAT3FD : pdf = new FEDataField_T<Post::FENodeData<mat3fd > >(&fem, EXPORT_DATA); break;
        case TENS4FS: pdf = new FEDataField_T<Post::FENodeData<tens4fs> >(&fem, EXPORT_DATA); break;
		case MAT3F  : pdf = new FEDataField_T<Post::FENodeData<mat3f  > >(&fem, EXPORT_DATA); break;
		case ARRAY:
		{
			FEArrayDataField* data = new FEArrayDataField(&fem, NODE_DATA, DATA_ITEM, EXPORT_DATA);
			data->SetArraySize(it.arraySize);
			data->SetArrayNames(it.arrayNames);
			pdf = data;
		}
		break;
		default:
			return errf("Error while reading dictionary.");
		}
		if (pdf == nullptr) return false;

		if (it.szunit[0]) pdf->SetUnits(it.szunit);

		pdm->AddDataField(pdf, it.szname);
	}

	// read solid variables
	nv = (int) m_dic.m_Elem.size();
	for (i=0; i<nv; ++i)
	{
		DICT_ITEM& it = m_dic.m_Elem[i];
		it.index = nfields++;

		Post::ModelDataField* pdf = nullptr;
		switch (it.nfmt)
		{
		case FMT_NODE:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_NODE> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case ARRAY:
				{
					FEArrayDataField* data = new FEArrayDataField(&fem, ELEM_DATA, DATA_NODE, EXPORT_DATA);
					data->SetArraySize(it.arraySize);
					data->SetArrayNames(it.arrayNames);
					pdf = data;
				}
				break;
				default:
					assert(false);
					return false;
				}
			}
			break;
		case FMT_ITEM:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case ARRAY:
				{
					FEArrayDataField* data = new FEArrayDataField(&fem, ELEM_DATA, DATA_ITEM, EXPORT_DATA);
					data->SetArraySize(it.arraySize);
					data->SetArrayNames(it.arrayNames);
					pdf = data;
				}
				break;
				case ARRAY_VEC3F:
				{
					FEArrayVec3DataField* data = new FEArrayVec3DataField(&fem, ELEM_DATA, EXPORT_DATA);
					data->SetArraySize(it.arraySize);
					data->SetArrayNames(it.arrayNames);
					pdf = data;
				}
				break;
				default:
					assert(false);
					return false;
				}
			}
			break;
		case FMT_MULT:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_MULT> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
					return false;
				}
			}
			break;
		case FMT_REGION:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_REGION> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
					return false;
				}
			}
			break;
		default:
			assert(false);
			return errf("Error while reading dictionary");
		}
		if (pdf == nullptr) return false;
		if (it.szunit[0]) pdf->SetUnits(it.szunit);
		pdm->AddDataField(pdf, it.szname);
	}

	// read face variables
	nv = (int) m_dic.m_Face.size();
	for (i=0; i<nv; ++i)
	{
		DICT_ITEM& it = m_dic.m_Face[i];
		it.index = nfields++;

		Post::ModelDataField* pdf = nullptr;
		switch (it.nfmt)
		{
		case FMT_NODE:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_NODE> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_ITEM:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_MULT:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_MULT> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_REGION:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_REGION> >(&fem, EXPORT_DATA); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		default:
			assert(false);
			return errf("Error reading dictionary");
		}
		if (pdf == nullptr) return false;
		if (it.szunit[0]) pdf->SetUnits(it.szunit);
		pdm->AddDataField(pdf, it.szname);
	}

	// read edge variables
	nv = (int) m_dic.m_Edge.size();
	for (i=0; i<nv; ++i)
	{
		DICT_ITEM& it = m_dic.m_Edge[i];
		it.index = nfields++;

		Post::ModelDataField* pdf = nullptr;
		switch (it.nfmt)
		{
		case FMT_NODE:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEEdgeData<float  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEEdgeData<vec3f  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEEdgeData<mat3fs ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEEdgeData<mat3fd ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case TENS4FS: pdf = new FEDataField_T<FEEdgeData<tens4fs,DATA_NODE> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEEdgeData<mat3f  ,DATA_NODE> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_ITEM:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEEdgeData<float  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEEdgeData<vec3f  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEEdgeData<mat3fs ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEEdgeData<mat3fd ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case TENS4FS: pdf = new FEDataField_T<FEEdgeData<tens4fs,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEEdgeData<mat3f  ,DATA_ITEM> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_MULT:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEEdgeData<float  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEEdgeData<vec3f  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEEdgeData<mat3fs ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEEdgeData<mat3fd ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case TENS4FS: pdf = new FEDataField_T<FEEdgeData<tens4fs,DATA_MULT> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEEdgeData<mat3f  ,DATA_MULT> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_REGION:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEEdgeData<float  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case VEC3F  : pdf = new FEDataField_T<FEEdgeData<vec3f  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3FS : pdf = new FEDataField_T<FEEdgeData<mat3fs ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3FD : pdf = new FEDataField_T<FEEdgeData<mat3fd ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case TENS4FS: pdf = new FEDataField_T<FEEdgeData<tens4fs,DATA_REGION> >(&fem, EXPORT_DATA); break;
				case MAT3F  : pdf = new FEDataField_T<FEEdgeData<mat3f  ,DATA_REGION> >(&fem, EXPORT_DATA); break;
				default:
					assert(false);
				}
			}
			break;
		default:
			assert(false);
			return errf("Error reading dictionary");
		}
		if (pdf == nullptr) return false;
		if (it.szunit[0]) pdf->SetUnits(it.szunit);
		pdm->AddDataField(pdf, it.szname);
	}

	// add additional displacement fields
	if (m_bHasDispl) 
	{
		if (pdm->FindDataField("Lagrange strain") == -1)
			pdm->AddDataField(new StrainDataField(&fem, StrainDataField::LAGRANGE), "Lagrange strain");
		pdm->AddDataField(new FEDataField_T<NodePosition  >(&fem), "position"         , "L");
		pdm->AddDataField(new FEDataField_T<NodeInitPos   >(&fem), "initial position" , "L");
	}

	// add additional stress fields
	if (m_bHasStress)
	{
		pdm->AddDataField(new FEDataField_T<ElemPressure>(&fem), "pressure", "P");
		
		if (m_bHasFluidPressure) {
			// make sure the "solid stress" field was not added to the plot file
			if (pdm->FindDataField("solid stress") == -1)
				pdm->AddDataField(new FEDataField_T<SolidStress>(&fem), "solid stress", "P");
		}
	}

	// add additional stress fields
	if (m_bHasNodalStress)
	{
		pdm->AddDataField(new FEDataField_T<ElemNodalPressure>(&fem), "nodal pressure", "P");
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadGlobalDicItems()
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			DICT_ITEM it;
			ReadDictItem(it);
			m_dic.m_Glb.push_back(it);
		}
		else 
		{
			assert(false);
			return errf("Error reading Global section in Dictionary");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadMaterialDicItems()
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			DICT_ITEM it;
			ReadDictItem(it);
			m_dic.m_Mat.push_back(it);
		}
		else 
		{
			assert(false);
			return errf("Error reading Material section in Dictionary");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadNodeDicItems()
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			DICT_ITEM it;
			ReadDictItem(it);
			if (strcmp(it.szname, "displacement") == 0) m_bHasDispl = true;
			m_dic.m_Node.push_back(it);
		}
		else 
		{
			assert(false);
			return errf("Error reading Node section in Dictionary");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadElemDicItems()
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			DICT_ITEM it;
			ReadDictItem(it);
			if (strcmp(it.szname, "stress"         ) == 0) m_bHasStress         = true;
			if (strcmp(it.szname, "nodal stress"   ) == 0) m_bHasNodalStress    = true;
			if (strcmp(it.szname, "shell thickness") == 0) m_bHasShellThickness = true;
			if (strcmp(it.szname, "fluid pressure" ) == 0) m_bHasFluidPressure  = true;
			if (strcmp(it.szname, "elasticity"     ) == 0) m_bHasElasticity     = true;
			m_dic.m_Elem.push_back(it);
		}
		else 
		{
			assert(false);
			return errf("Error reading Element section in Dictionary");
		}
		m_ar.CloseChunk();
	}
	return true;
}

bool XpltReader3::ReadFaceDicItems()
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			DICT_ITEM it;
			ReadDictItem(it);
			m_dic.m_Face.push_back(it);
		}
		else 
		{
			assert(false);
			return errf("Error reading Face section in Dictionary");
		}
		m_ar.CloseChunk();
	}
	return true;
}

bool XpltReader3::ReadEdgeDicItems()
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_DIC_ITEM)
		{
			DICT_ITEM it;
			ReadDictItem(it);
			m_dic.m_Edge.push_back(it);
		}
		else
		{
			assert(false);
			return errf("Error reading Edge section in Dictionary");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadPartsSection(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_PART)
		{
			MATERIAL m;
			char sz[DI_NAME_SIZE] = {0};
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				switch (m_ar.GetChunkID())
				{
				case PLT_PART_ID  : m_ar.read(m.nid); break;
				case PLT_PART_NAME: m_ar.read(sz, DI_NAME_SIZE); break;
				}
				m_ar.CloseChunk();
			}
			strcpy(m.szname, sz);
			m_xmesh.addMaterial(m);
		}
		else
		{
			assert(false);
			return errf("Error while reading parts");
		}
		m_ar.CloseChunk();
	}
	CreateMaterials(fem);
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadObjectsSection(Post::FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_POINT_OBJECT)
		{
			Post::FEPostModel::PointObject*  ob = new Post::FEPostModel::PointObject;

			char sz[DI_NAME_SIZE] = { 0 };
			float r[3], q[4];
			int ntag = 0;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				switch (m_ar.GetChunkID())
				{
				case PLT_OBJECT_NAME: m_ar.read(sz); break;
				case PLT_OBJECT_TAG : m_ar.read(ntag); break;
				case PLT_OBJECT_POS : m_ar.read(r, 3); break;
				case PLT_OBJECT_ROT : m_ar.read(q, 4); break;
				case PLT_OBJECT_DATA:
				{
					char szdata[DI_NAME_SIZE] = { 0 };
					int ndataType = -1;
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch (m_ar.GetChunkID())
						{
						case PLT_DIC_ITEM_NAME: m_ar.read(szdata, DI_NAME_SIZE); break;
						case PLT_DIC_ITEM_TYPE: m_ar.read(ndataType); break;
						}
						m_ar.CloseChunk();
					}

					DATA_TYPE dataType = (DATA_TYPE) ndataType;

					PlotObjectData* data = new PlotObjectData(&fem, dataType);
					data->SetName(szdata);
					ob->m_data.push_back(data);
				}
				break;
				}
				m_ar.CloseChunk();
			}

			GLColor c(255, 255, 0);
			switch (ntag)
			{
			case 3: c = GLColor(0, 255, 0); break;
			case 4: c = GLColor(0, 0, 255); break;
			case 5: c = GLColor(255, 0, 255); break;
			}

			ob->SetName(sz);
			ob->m_tag = ntag;
			ob->m_pos = vec3d(r[0], r[1], r[2]);
			ob->m_rot = quatd(q[0], q[1], q[2], q[3]);
			ob->SetColor(c);
			fem.AddPointObject(ob);
		}
		else if (nid == PLT_LINE_OBJECT)
		{
			Post::FEPostModel::LineObject*  ob = new Post::FEPostModel::LineObject;

			char sz[DI_NAME_SIZE] = { 0 };
			float r[6] = { 0.f };
			int ntag = 0;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				switch (m_ar.GetChunkID())
				{
				case PLT_OBJECT_NAME: m_ar.read(sz); break;
				case PLT_OBJECT_TAG: m_ar.read(ntag); break;
				case PLT_LINE_COORDS: m_ar.read(r, 6); break;
				case PLT_OBJECT_DATA:
				{
					char szdata[DI_NAME_SIZE] = { 0 };
					int ndataType = -1;
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch (m_ar.GetChunkID())
						{
						case PLT_DIC_ITEM_NAME: m_ar.read(szdata, DI_NAME_SIZE); break;
						case PLT_DIC_ITEM_TYPE: m_ar.read(ndataType); break;
						}
						m_ar.CloseChunk();
					}

					DATA_TYPE dataType = (DATA_TYPE) ndataType;

					PlotObjectData* data = new PlotObjectData(&fem, dataType);
					data->SetName(szdata);
					ob->m_data.push_back(data);
				}
				break;
				}
				m_ar.CloseChunk();
			}

			ob->SetName(sz);
			ob->m_tag = ntag;
			ob->m_r1 = ob->m_r01 = vec3d(r[0], r[1], r[2]);
			ob->m_r2 = ob->m_r02 = vec3d(r[3], r[4], r[5]);
			ob->SetColor(GLColor(255, 0, 0));
			fem.AddLineObject(ob);
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
void XpltReader3::CreateMaterials(FEPostModel& fem)
{
	// initialize material properties
	fem.ClearMaterials();
	int nmat = m_xmesh.materials();
	for (int i=0; i<nmat; i++)
	{
		MATERIAL& xm = m_xmesh.material(i);
		Material m;
		m.SetName(xm.szname);
		fem.AddMaterial(m);
	}
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadMesh(FEPostModel &fem)
{
	// clear the current XMesh
	m_xmesh.Clear();

	// read the mesh in
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		switch (m_ar.GetChunkID())
		{
		case PLT_NODE_SECTION      : if (ReadNodeSection      (fem) == false) return false; break;
		case PLT_DOMAIN_SECTION    : if (ReadDomainSection    (fem) == false) return false; break;
		case PLT_SURFACE_SECTION   : if (ReadSurfaceSection   (fem) == false) return false; break;
		case PLT_EDGE_SECTION      : if (ReadEdgeSection      (fem) == false) return false; break;
		case PLT_NODESET_SECTION   : if (ReadNodeSetSection   (fem) == false) return false; break;
		case PLT_ELEMENTSET_SECTION: if (ReadElementSetSection(fem) == false) return false; break;
		case PLT_FACETSET_SECTION  : if (ReadFacetSetSection  (fem) == false) return false; break;
		case PLT_PARTS_SECTION     : if (ReadPartsSection     (fem) == false) return false; break;
		case PLT_OBJECTS_SECTION   : if (ReadObjectsSection   (fem) == false) return false; break;
		}
		m_ar.CloseChunk();
	}

	// create a FE mesh
	if (BuildMesh(fem) == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadNodeSection(FEPostModel &fem)
{
	int nodes = 0;
	int dim = 0;
	char szname[DI_NAME_SIZE] = {0};
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		switch(m_ar.GetChunkID())
		{
		case PLT_NODE_HEADER:
			{
				while (m_ar.OpenChunk() == xpltArchive::IO_OK)
				{
					int nid = m_ar.GetChunkID();
					if      (nid == PLT_NODE_SIZE) m_ar.read(nodes); 
					else if (nid == PLT_NODE_DIM ) m_ar.read(dim);
					else if (nid == PLT_NODE_NAME) m_ar.sread(szname, DI_NAME_SIZE);
					m_ar.CloseChunk();
				}
			}
			break;
		case PLT_NODE_COORDS:
			{
				if (nodes == 0) return errf("Missing or invalid node header section");
				if (dim   == 0) return errf("Missing or invalid node header section");

				vector<NODE> allNodes;
				NODE node = {-1, 0.f, 0.f, 0.f};
				for (int i=0; i<nodes; ++i)
				{
					m_ar.read(node.id);
					m_ar.read(node.x, dim);

					allNodes.push_back(node);
				}
				m_xmesh.addNodes(allNodes);
			}
			break;
		default:
			assert(false);
			return errf("Error while reading Node section");
		}
		m_ar.CloseChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadDomainSection(FEPostModel &fem)
{
	int nd = 0, index = 0;
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_DOMAIN)
		{
			Domain D;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_DOMAIN_HDR)
				{
					// read the domain header
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch (m_ar.GetChunkID())
						{
						case PLT_DOM_ELEM_TYPE: m_ar.read(D.etype); break;
						case PLT_DOM_PART_ID  : m_ar.read(D.mid); break;
						case PLT_DOM_ELEMS    : m_ar.read(D.ne); break;
						case PLT_DOM_NAME     : m_ar.sread(D.szname, DI_NAME_SIZE); break;
						default:
							assert(false);
							return errf("Error while reading Domain section");
						}
						m_ar.CloseChunk();
					}
				}
				else if (nid == PLT_DOM_ELEM_LIST)
				{
					assert(D.ne > 0);
					D.elem.reserve(D.ne);
					D.elist.reserve(D.ne);
					int ne = 0;
					switch (D.etype)
					{
					case PLT_ELEM_HEX8   : ne =  8; break;
					case PLT_ELEM_PENTA  : ne =  6; break;
                    case PLT_ELEM_PENTA15: ne =  15; break;
                    case PLT_ELEM_TET4   : ne =  4; break;
					case PLT_ELEM_TET5   : ne =  5; break;
					case PLT_ELEM_QUAD   : ne =  4; break;
					case PLT_ELEM_TRI    : ne =  3; break;
					case PLT_ELEM_TRUSS  : ne =  2; break;
					case PLT_ELEM_HEX20  : ne = 20; break;
					case PLT_ELEM_HEX27  : ne = 27; break;
					case PLT_ELEM_TET10  : ne = 10; break;
					case PLT_ELEM_TET15  : ne = 15; break;
					case PLT_ELEM_TET20  : ne = 20; break;
                    case PLT_ELEM_TRI6   : ne =  6; break;
                    case PLT_ELEM_QUAD8  : ne =  8; break;
                    case PLT_ELEM_QUAD9  : ne =  9; break;
					case PLT_ELEM_PYRA5  : ne =  5; break;
                    case PLT_ELEM_PYRA13 : ne = 13; break;
                    case PLT_ELEM_LINE3  : ne =  3; break;
					default:
						assert(false);
						return errf("Error while reading Domain section");
					}
					assert((ne > 0)&&(ne <= FSElement::MAX_NODES));
					int n[FSElement::MAX_NODES + 1];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						if (m_ar.GetChunkID() == PLT_ELEMENT)
						{
							ELEM e;
							m_ar.read(n, ne+1);
							e.index = index++;
							e.eid = n[0];
							for (int i=0; i<ne; ++i) { e.node[i] = n[i+1]; assert(e.node[i] < m_xmesh.nodes()); }
							D.elem.push_back(e);
							D.elist.push_back(e.index);
						}
						else
						{
							assert(false);
							return errf("Error while reading Domain section");
						}
						m_ar.CloseChunk();
					}
				}
				else
				{
					assert(false);
					return errf("Error while reading Domain section");
				}
				m_ar.CloseChunk();
			}
			assert(D.ne == D.elem.size());
			D.nid = nd++;
			m_xmesh.addDomain(D);
		}
		else
		{
			assert(false);
			return errf("Error while reading Domain section");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadSurfaceSection(FEPostModel &fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_SURFACE)
		{
			Surface S;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_SURFACE_HDR)
				{
					// read the surface header
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch(m_ar.GetChunkID())
						{
						case PLT_SURFACE_ID             : m_ar.read(S.sid); break;
						case PLT_SURFACE_FACES          : m_ar.read(S.nfaces); break;
						case PLT_SURFACE_NAME           : m_ar.sread(S.szname, DI_NAME_SIZE); break;
						case PLT_SURFACE_MAX_FACET_NODES: m_ar.read(S.maxNodes); break;
						default:
							assert(false);
							return errf("Error while reading Surface section");
						}
						m_ar.CloseChunk();
					}
				}
				else if (nid == PLT_FACE_LIST)
				{
					assert(S.nfaces > 0);
					S.face.reserve(S.nfaces);
					int n[12];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						if (m_ar.GetChunkID() == PLT_FACE)
						{
							m_ar.read(n, S.maxNodes+2);
							FACE f;
							f.nid = n[0];
							f.nn = n[1];
							for (int i=0; i<f.nn; ++i) f.node[i] = n[2+i];
							S.face.push_back(f);
						}
						else 
						{
							assert(false);
							return errf("Error while reading Surface section");
						}
						m_ar.CloseChunk();
					}
				}
				m_ar.CloseChunk();
			}
			assert(S.nfaces == S.face.size());
			m_xmesh.addSurface(S);
		}
		else
		{
			assert(false);
			return errf("Error while reading Surface section");
		}
		m_ar.CloseChunk();
	}
	return true;
}

bool XpltReader3::ReadEdgeSection(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_EDGE)
		{
			Edge E;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_EDGE_HDR)
				{
					// read the header
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch (m_ar.GetChunkID())
						{
						case PLT_EDGE_ID: m_ar.read(E.eid); break;
						case PLT_EDGE_LINES: m_ar.read(E.nlines); break;
						case PLT_EDGE_NAME: m_ar.sread(E.szname, DI_NAME_SIZE); break;
						case PLT_EDGE_MAX_NODES: m_ar.read(E.maxNodes); break;
						default:
							assert(false);
							return errf("Error while reading Edge section");
						}
						m_ar.CloseChunk();
					}
				}
				else if (nid == PLT_EDGE_LIST)
				{
					assert(E.nlines > 0);
					E.line.reserve(E.nlines);
					int n[12];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						if (m_ar.GetChunkID() == PLT_LINE)
						{
							m_ar.read(n, E.maxNodes + 2);
							LINE l;
							l.id = n[0];
							l.nn = n[1];
							for (int i = 0; i < l.nn; ++i) l.node[i] = n[2 + i];
							E.line.push_back(l);
						}
						else
						{
							assert(false);
							return errf("Error while reading Edge section");
						}
						m_ar.CloseChunk();
					}
				}
				m_ar.CloseChunk();
			}
			assert(E.nlines == E.line.size());
			m_xmesh.addEdge(E);
		}
		else
		{
			assert(false);
			return errf("Error while reading Edge section");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadNodeSetSection(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_NODESET)
		{
			NodeSet S;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_NODESET_HDR)
				{
					// read the nodeset header
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch(m_ar.GetChunkID())
						{
						case PLT_NODESET_ID   : m_ar.read(S.nid); break;
						case PLT_NODESET_SIZE : m_ar.read(S.nn); break;
						case PLT_NODESET_NAME : m_ar.sread(S.szname, DI_NAME_SIZE); break;
						default:
							assert(false);
							return errf("Error while reading NodeSet section");
						}
						m_ar.CloseChunk();
					}
				}
				else if (nid == PLT_NODESET_LIST)
				{
					S.node.assign(S.nn, 0);
					m_ar.read(S.node);
				}
				else
				{
					assert(false);
					return errf("Error while reading NodeSet section");
				}
				m_ar.CloseChunk();
			}
			m_xmesh.addNodeSet(S);
		}
		else
		{
			assert(false);
			return errf("Error while reading NodeSet section");
		}
		m_ar.CloseChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadElementSetSection(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_ELEMENTSET)
		{
			ElemSet S;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_ELEMENTSET_HDR)
				{
					// read the nodeset header
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch (m_ar.GetChunkID())
						{
						case PLT_ELEMENTSET_ID  : m_ar.read(S.nid); break;
						case PLT_ELEMENTSET_SIZE: m_ar.read(S.ne); break;
						case PLT_ELEMENTSET_NAME: m_ar.sread(S.szname, DI_NAME_SIZE); break;
						default:
							assert(false);
							return errf("Error while reading ElementSet section");
						}
						m_ar.CloseChunk();
					}
				}
				else if (nid == PLT_ELEMENTSET_LIST)
				{
					S.elem.assign(S.ne, 0);
					m_ar.read(S.elem);
				}
				else
				{
					assert(false);
					return errf("Error while reading ElementSet section");
				}
				m_ar.CloseChunk();
			}
			m_xmesh.addElementSet(S);
		}
		else
		{
			assert(false);
			return errf("Error while reading NodeSet section");
		}
		m_ar.CloseChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadFacetSetSection(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_FACETSET)
		{
			Surface S;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_FACETSET_HDR)
				{
					// read the header
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						switch (m_ar.GetChunkID())
						{
						case PLT_FACETSET_ID: m_ar.read(S.sid); break;
						case PLT_FACETSET_SIZE: m_ar.read(S.nfaces); break;
						case PLT_FACETSET_NAME: m_ar.sread(S.szname, DI_NAME_SIZE); break;
						case PLT_FACETSET_MAXNODES: m_ar.read(S.maxNodes); break;
						default:
							assert(false);
							return errf("Error while reading FacetSet section");
						}
						m_ar.CloseChunk();
					}
				}
				else if (nid == PLT_FACETSET_LIST)
				{
					assert(S.nfaces > 0);
					S.face.reserve(S.nfaces);
					int n[12];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						if (m_ar.GetChunkID() == PLT_FACET)
						{
							m_ar.read(n, S.maxNodes + 2);
							FACE f;
							f.nid = n[0];
							f.nn = n[1];
							for (int i = 0; i < f.nn; ++i) f.node[i] = n[2 + i];
							S.face.push_back(f);
						}
						else
						{
							assert(false);
							return errf("Error while reading FacetSet section");
						}
						m_ar.CloseChunk();
					}
				}
				m_ar.CloseChunk();
			}
			assert(S.nfaces == S.face.size());
			m_xmesh.addFacetSet(S);
		}
		else
		{
			assert(false);
			return errf("Error while reading Surface section");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::BuildMesh(FEPostModel &fem)
{
	// count all nodes
	int NN = m_xmesh.nodes();

	// count all elements
	int ND = m_xmesh.domains();
	int NE = 0;
	for (int i=0; i<ND; ++i) NE += m_xmesh.domain(i).ne;

	// find the element type
	int ntype = m_xmesh.domain(0).etype;
	bool blinear = true;	// all linear elements flag
	for (int i=0; i<ND; ++i)
	{
		int domType = m_xmesh.domain(i).etype;
		if (domType != ntype) ntype = -1;
		if ((domType != PLT_ELEM_TRUSS) && 
			(domType != PLT_ELEM_TRI) && 
			(domType != PLT_ELEM_QUAD) && 
			(domType != PLT_ELEM_TET4) && 
			(domType != PLT_ELEM_PENTA) && 
			(domType != PLT_ELEM_HEX8) &&
			(domType != PLT_ELEM_PYRA5)) blinear = false;
	}

	Post::FEPostMesh* pmesh = new Post::FEPostMesh;
	pmesh->Create(NN, NE);

	// read the element connectivity
	int nmat = fem.Materials();
	for (int i=0; i<ND; i++)
	{
		Domain& D = m_xmesh.domain(i);
		for (int j=0; j<D.ne; ++j)
		{
			ELEM& E = D.elem[j];
			FSElement& el = pmesh->Element(E.index);
			el.m_MatID = D.mid - 1;
			el.m_gid = i;
			el.SetID(E.eid);

			FEElementType srcType;
			switch (D.etype)
			{
			case PLT_ELEM_HEX8   : srcType = FE_HEX8  ; break;
			case PLT_ELEM_PENTA  : srcType = FE_PENTA6; break;
			case PLT_ELEM_PENTA15: srcType = FE_PENTA15; break;
			case PLT_ELEM_TET4   : srcType = FE_TET4  ; break;
			case PLT_ELEM_TET5   : srcType = FE_TET5  ; break;
			case PLT_ELEM_QUAD   : srcType = FE_QUAD4 ; break;
			case PLT_ELEM_TRI    : srcType = FE_TRI3  ; break;
			case PLT_ELEM_TRUSS  : srcType = FE_BEAM2 ; break;
			case PLT_ELEM_HEX20  : srcType = FE_HEX20 ; break;
			case PLT_ELEM_HEX27  : srcType = FE_HEX27 ; break;
			case PLT_ELEM_TET10  : srcType = FE_TET10 ; break;
			case PLT_ELEM_TET15  : srcType = FE_TET15 ; break;
			case PLT_ELEM_TET20  : srcType = FE_TET20 ; break;
			case PLT_ELEM_TRI6   : srcType = FE_TRI6  ; break;
			case PLT_ELEM_QUAD8  : srcType = FE_QUAD8 ; break;
			case PLT_ELEM_QUAD9  : srcType = FE_QUAD9 ; break;
			case PLT_ELEM_PYRA5  : srcType = FE_PYRA5 ; break;
			case PLT_ELEM_PYRA13 : srcType = FE_PYRA13; break;
			case PLT_ELEM_LINE3  : srcType = FE_BEAM3; break;
			}

			// check for degenerate elements
			int dstType = srcType;
			if ((srcType == FE_QUAD4 ) && (E.node[2] == E.node[3])) dstType = FE_TRI3;
			if ((srcType == FE_PENTA6) && (E.node[5] == E.node[4]) && (E.node[4] == E.node[3])) dstType = FE_TET4;
			if ((srcType == FE_HEX8  ) && (E.node[2] == E.node[3]) && (E.node[7] == E.node[6]) && (E.node[6]==E.node[5]) && (E.node[5] == E.node[4])) dstType = FE_TET4;
			if ((srcType == FE_HEX8  ) && (E.node[2] == E.node[3]) && (E.node[6] == E.node[7])) dstType = FE_PENTA6;

			el.SetType(dstType);
			if (dstType == srcType)
			{
				int ne = el.Nodes();
				for (int k = 0; k < ne; ++k) el.m_node[k] = E.node[k];
			}
			else
			{
				if (dstType == FE_TRI3)
				{
					assert(srcType == FE_QUAD4);
					el.m_node[0] = E.node[0];
					el.m_node[1] = E.node[1];
					el.m_node[2] = E.node[2];
				}
				else if (dstType == FE_PENTA6)
				{
					assert(srcType == FE_HEX8);
					el.m_node[0] = E.node[0];
					el.m_node[1] = E.node[1];
					el.m_node[2] = E.node[2];
					el.m_node[3] = E.node[4];
					el.m_node[4] = E.node[5];
					el.m_node[5] = E.node[6];
				}
				else if (dstType == FE_TET4)
				{
					if (srcType == FE_PENTA6)
					{
						el.m_node[0] = E.node[0];
						el.m_node[1] = E.node[1];
						el.m_node[2] = E.node[2];
						el.m_node[3] = E.node[3];
					}
					else if (srcType == FE_HEX8)
					{
						el.m_node[0] = E.node[0];
						el.m_node[1] = E.node[1];
						el.m_node[2] = E.node[2];
						el.m_node[3] = E.node[4];
					}
					else assert(false);
				}
				else assert(false);
			}
		}
	}

	NN = m_xmesh.nodes();
	// read the nodal coordinates
	if (FileVersion() < 0x0033)
	{
		for (int i = 0; i < NN; i++)
		{
			FSNode& n = pmesh->Node(i);
			NODE& N = m_xmesh.node(i);
			n.m_nid = i + 1;
			n.r = vec3d(N.x[0], N.x[1], N.x[2]);
		}
	}
	else
	{
		for (int i = 0; i < NN; i++)
		{
			FSNode& n = pmesh->Node(i);
			NODE& N = m_xmesh.node(i);
			n.m_nid = N.id;
			n.r = vec3d(N.x[0], N.x[1], N.x[2]);
		}
	}

	// set the enabled-ness of the elements and the nodes
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = pmesh->ElementRef(i);
		Material* pm = fem.GetMaterial(el.m_MatID);
		if (pm->benable) el.Enable(); else el.Disable();
	}

	for (int i=0; i<NN; ++i) pmesh->Node(i).Disable();
	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = pmesh->ElementRef(i);
		if (el.IsEnabled())
		{
			int n = el.Nodes();
			for (int j=0; j<n; ++j) pmesh->Node(el.m_node[j]).Enable();
		}
	}

	// Update the mesh
	// This will also build the faces
	pmesh->BuildMesh();

	// Next, we'll build a Node-Face lookup table
	FSNodeFaceList NFT; NFT.Build(pmesh);

	// next, we reindex the surfaces and facet sets
	for (int n=0; n< m_xmesh.surfaces(); ++n)
	{
		Surface& s = m_xmesh.surface(n);
		for (int i=0; i<s.nfaces; ++i)
		{
			FACE& f = s.face[i];
			f.nid = NFT.FindFace(f.node[0], f.node, f.nn);
//			assert(f.nid >= 0);
		}
	}
	for (int n = 0; n < m_xmesh.facetSets(); ++n)
	{
		Surface& s = m_xmesh.facetSet(n);
		for (int i = 0; i < s.nfaces; ++i)
		{
			FACE& f = s.face[i];
			f.nid = NFT.FindFace(f.node[0], f.node, f.nn);
//			assert(f.nid >= 0);
		}
	}

	// do the same for the edges
	if (m_xmesh.edges() > 0)
	{
		FSNodeEdgeList NEL; NEL.Build(pmesh);

		for (int n = 0; n < m_xmesh.edges(); ++n)
		{
			Edge& e = m_xmesh.edge(n);
			for (int i = 0; i < e.nlines; ++i)
			{
				LINE& l = e.line[i];
				l.id = -1;
				const std::vector<NodeEdgeRef>& edgeList = NEL.EdgeList(l.node[0]);
				for (NodeEdgeRef e : edgeList)
				{
					const FSEdge& edge = *e.pe;
					if (((edge.n[0] == l.node[0]) && (edge.n[1] == l.node[1])) ||
						((edge.n[0] == l.node[1]) && (edge.n[1] == l.node[0])))
					{
						l.id = e.eid;
						break;
					}
				}
				assert(l.id != -1);
			}
		}
	}

	// let's create the nodesets
	char szname[128]={0};
	for (int n=0; n< m_xmesh.nodeSets(); ++n)
	{
		NodeSet& s = m_xmesh.nodeSet(n);
		Post::FSNodeSet* ps = new Post::FSNodeSet(pmesh);
		if (s.szname[0]==0) { sprintf(szname, "nodeset%02d",n+1); ps->SetName(szname); }
		else ps->SetName(s.szname);
		ps->m_Node = s.node;
		pmesh->AddNodeSet(ps);
	}

	// let's create the FE surfaces
	// This is no longer necessary for newer files (>= 3.2) since all
	// facets sets are stored in the PLT_FACETSET_SECTION
	if (m_xplt->GetHeader().nversion < 0x0032)
	{
		for (int n=0; n< m_xmesh.surfaces(); ++n)
		{
			Surface& s = m_xmesh.surface(n);
			Post::FSSurface* ps = new Post::FSSurface(pmesh);
			if (s.szname[0]==0) { sprintf(szname, "surface%02d",n+1); ps->SetName(szname); }
			else ps->SetName(s.szname);
			ps->m_Face.reserve(s.nfaces);
			for (int i=0; i<s.nfaces; ++i) ps->m_Face.push_back(s.face[i].nid);
			pmesh->AddSurface(ps);
		}
	}
	else
	{
		for (int n = 0; n < m_xmesh.facetSets(); ++n)
		{
			Surface& s = m_xmesh.facetSet(n);
			Post::FSSurface* ps = new Post::FSSurface(pmesh);
			if (s.szname[0] == 0) { sprintf(szname, "surface%02d", n + 1); ps->SetName(szname); }
			else ps->SetName(s.szname);
			ps->m_Face.reserve(s.nfaces);
			for (int i = 0; i < s.nfaces; ++i) ps->m_Face.push_back(s.face[i].nid);
			pmesh->AddSurface(ps);
		}

		for (int n = 0; n < m_xmesh.edges(); ++n)
		{
			Edge& e = m_xmesh.edge(n);
			FSEdgeSet* pe = new FSEdgeSet(pmesh);
			if (e.szname[0] == 0) { sprintf(szname, "surface%02d", n + 1); pe->SetName(szname); }
			else pe->SetName(e.szname);
			for (int i = 0; i < e.nlines; ++i)
			{
				if (e.line[i].id != -1) pe->add(e.line[i].id);
			}
			pmesh->AddFEEdgeSet(pe);
		}
	}

	if (m_xmesh.elementSets() > 0)
	{
		// let's create element sets
		// the element IDs stored are global IDs, but we need local indices for the Post::FSElemSet.
		// so let's build a lookup table
		int minId = -1, maxId = -1;
		for (int i = 0; i < pmesh->Elements(); ++i)
		{
			FSElement& el = pmesh->Element(i);
			if ((minId == -1) || (el.m_nid < minId)) minId = el.m_nid;
			if ((maxId == -1) || (el.m_nid > maxId)) maxId = el.m_nid;
		}
		int nsize = maxId - minId + 1;
		vector<int> lut(nsize, -1);
		for (int i = 0; i < pmesh->Elements(); ++i)
		{
			FSElement& el = pmesh->Element(i);
			lut[el.m_nid - minId] = i;
		}

		for (int n = 0; n < m_xmesh.elementSets(); ++n)
		{
			ElemSet& e = m_xmesh.elementSet(n);
			Post::FSElemSet* pg = new Post::FSElemSet(pmesh);
			if (e.szname[0] == 0) { sprintf(szname, "ElementSet%02d", n + 1); pg->SetName(szname); }
			else pg->SetName(e.szname);
			pg->m_Elem.resize(e.ne);
			for (int i = 0; i < e.elem.size(); ++i)
			{
				pg->m_Elem[i] = lut[e.elem[i] - minId];
			}
			pmesh->AddElemSet(pg);
		}
	}

	// let's create the parts
	// This is no longer necessary for newer files (>= 3.2) since all
	// element sets are stored in the PLT_ELEMENTSET_SECTION for all domains
	if (m_xplt->GetHeader().nversion < 0x0032)
	{
		for (int n = 0; n < m_xmesh.domains(); ++n)
		{
			Domain& s = m_xmesh.domain(n);
			Post::FSElemSet* pg = new Post::FSElemSet(pmesh);
			if (s.szname[0] == 0) { sprintf(szname, "part%02d", n + 1); pg->SetName(szname); }
			else pg->SetName(s.szname);
			pg->m_Elem.resize(s.ne);
			pg->m_Elem = s.elist;
			pmesh->AddElemSet(pg);
		}
	}

	// store the current mesh
	fem.AddMesh(pmesh);
	m_mesh = pmesh;

	fem.UpdateBoundingBox();

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadStateSection(FEPostModel& fem)
{
	// get the mesh
	Post::FEPostMesh& mesh = *GetCurrentMesh();

	// add a state
	FEState* ps = 0;
	
	try 
	{
		ps = m_pstate = new FEState(0.f, &fem, &mesh);
	}
	catch (...)
	{
		m_pstate = 0;
		return errf("Error allocating memory for state data");
	}

	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_STATE_HEADER)
		{
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_STATE_HDR_TIME) m_ar.read(ps->m_time);
				if (nid == PLT_STATE_STATUS  ) m_ar.read(ps->m_status);
				m_ar.CloseChunk();
			}
		}
		else if (nid == PLT_STATE_DATA)
		{
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				switch (m_ar.GetChunkID())
				{
				case PLT_GLOBAL_DATA  : if (ReadGlobalData  (fem, ps) == false) return false; break;
				case PLT_NODE_DATA    : if (ReadNodeData    (fem, ps) == false) return false; break;
				case PLT_ELEMENT_DATA : if (ReadElemData    (fem, ps) == false) return false; break;
				case PLT_FACE_DATA    : if (ReadFaceData    (fem, ps) == false) return false; break;
				case PLT_EDGE_DATA    : if (ReadEdgeData    (fem, ps) == false) return false; break;
				default:
					assert(false);
					return errf("Invalid chunk ID");
				}
				m_ar.CloseChunk();
			}
		}
		else if (nid == PLT_MESH_STATE)
		{
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_NODE_COORDS)
				{
					const int dim = 3;
					int nodes = mesh.Nodes();
					NODE node = { -1, 0.f, 0.f, 0.f };
					for (int i = 0; i < nodes; ++i)
					{
						m_ar.read(node.id);
						m_ar.read(node.x, dim);

						ps->m_NODE[i].m_rt = vec3f(node.x[0], node.x[1], node.x[2]);
					}
				}
				else if (m_ar.GetChunkID() == PLT_ELEMENT_STATE)
				{
					int NE = mesh.Elements();
					vector<unsigned int> flags(NE, 0);
					m_ar.read(flags);

					for (int i = 0; i < NE; ++i)
					{
						if (flags[i] == 1)
							ps->m_ELEM[i].m_state = StatusFlags::VISIBLE;
						else
							ps->m_ELEM[i].m_state = 0;
					}
				}
				m_ar.CloseChunk();
			}
		}
		else if (nid == PLT_OBJECTS_STATE)
		{
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int chunkId = m_ar.GetChunkID();
				if (chunkId == PLT_POINT_OBJECT)
				{
					int objId = -1;

					FEPostModel::PlotObject* po = nullptr;
					Post::OBJ_POINT_DATA* pd = nullptr;

					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int nid = m_ar.GetChunkID();

						if (nid == PLT_OBJECT_ID)
						{
							m_ar.read(objId);
							objId -= 1;

							po = fem.GetPointObject(objId);
							if ((objId >= 0) && (objId < ps->m_objPt.size()))
								pd = &(ps->m_objPt[objId]);
						}
						else if (nid == PLT_OBJECT_POS)
						{
							assert(objId != -1);
							float r[3];
							m_ar.read(r, 3);
							if (pd) pd->pos = vec3d(r[0], r[1], r[2]);
						}
						else if (nid == PLT_OBJECT_ROT)
						{
							assert(objId != -1);
							float q[4];
							m_ar.read(q, 4);
							if (pd) pd->rot = quatd(q[0], q[1], q[2], q[3]);
						}
						else if (nid == PLT_POINT_COORD)
						{
							assert(objId != -1);
							float r[3];
							m_ar.read(r, 3);
							if (pd) pd->m_rt = vec3d(r[0], r[1], r[2]);
						}
						else if (nid == PLT_OBJECT_DATA)
						{
							while (m_ar.OpenChunk() == xpltArchive::IO_OK)
							{
								int nv = m_ar.GetChunkID();
								if (po)
								{
									assert((nv >= 0) && (nv < po->m_data.size()));

									ObjectData* pd = m_pstate->m_objPt[objId].data;

									switch (po->m_data[nv]->Type())
									{
									case DATA_SCALAR: { float v; m_ar.read(v); pd->set(nv, v); } break;
									case DATA_VEC3 : { vec3f v; m_ar.read(v); pd->set(nv, v); } break;
									}
								}

								m_ar.CloseChunk();
							}
						}

						m_ar.CloseChunk();
					}
				}
				else if (chunkId == PLT_LINE_OBJECT)
				{
					int objId = -1;
					FEPostModel::PlotObject* po = nullptr;
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int nid = m_ar.GetChunkID();

						if (nid == PLT_OBJECT_ID)
						{
							m_ar.read(objId);
							objId -= 1;
							po = fem.GetLineObject(objId);
						}
						else if (nid == PLT_OBJECT_POS)
						{
							assert(objId != -1);
							float r[3];
							m_ar.read(r, 3);
							ps->m_objLn[objId].pos = vec3d(r[0], r[1], r[2]);
						}
						else if (nid == PLT_OBJECT_ROT)
						{
							assert(objId != -1);
							float q[4];
							m_ar.read(q, 4);
							ps->m_objLn[objId].rot = quatd(q[0], q[1], q[2], q[3]);
						}
						else if (nid == PLT_LINE_COORDS)
						{
							assert(objId != -1);
							float r[6];
							m_ar.read(r, 6);
							ps->m_objLn[objId].m_r1 = vec3d(r[0], r[1], r[2]);
							ps->m_objLn[objId].m_r2 = vec3d(r[3], r[4], r[5]);
						}
						else if (nid == PLT_OBJECT_DATA)
						{
							while (m_ar.OpenChunk() == xpltArchive::IO_OK)
							{
								int nv = m_ar.GetChunkID();

								assert((nv >= 0) && (nv < po->m_data.size()));

								ObjectData* pd = m_pstate->m_objLn[objId].data;

								switch (po->m_data[nv]->Type())
								{
								case DATA_SCALAR: { float v; m_ar.read(v); pd->set(nv, v); } break;
								case DATA_VEC3 : { vec3f v; m_ar.read(v); pd->set(nv, v); } break;
								}

								m_ar.CloseChunk();
							}
						}
						m_ar.CloseChunk();
					}
				}
				m_ar.CloseChunk();
			}
		}
		else return errf("Invalid chunk ID");
		m_ar.CloseChunk();
	}

	// Assign shell thicknesses
	if (m_bHasShellThickness)
	{
		FEDataManager& dm = *fem.GetDataManager();
		int n = dm.FindDataField("shell thickness");
		Post::FEElementData<float,DATA_MULT>& df = dynamic_cast<Post::FEElementData<float,DATA_MULT>&>(ps->m_Data[n]);
		Post::FEPostMesh& mesh = *GetCurrentMesh();
		int NE = mesh.Elements();
		float h[FSElement::MAX_NODES] = {0.f};
		for (int i=0; i<NE; ++i)
		{
			ELEMDATA& d = ps->m_ELEM[i];
			if (df.active(i))
			{
				df.eval(i, h);
				int n = mesh.ElementRef(i).Nodes();
				for (int j=0; j<n; ++j) d.m_h[j] = h[j];
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadGlobalData(FEPostModel& fem, FEState* pstate)
{
	FEDataManager& dm = *fem.GetDataManager();
	Post::FEPostMesh& mesh = *GetCurrentMesh();
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_STATE_VARIABLE)
		{
			int nv = -1;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_STATE_VAR_ID) m_ar.read(nv);
				else if (nid == PLT_STATE_VAR_DATA)
				{
					nv--;
					assert((nv >= 0) && (nv < (int)m_dic.m_Glb.size()));
					if ((nv < 0) || (nv >= (int)m_dic.m_Glb.size())) return errf("Failed reading global data");

					DICT_ITEM it = m_dic.m_Glb[nv];
					int nfield = dm.FindDataField(it.szname);
					int ndata = 0;
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int ns = m_ar.GetChunkID();
						assert(ns == 0);

						if (it.ntype == FLOAT)
						{
							float a;
							m_ar.read(a);

							Post::FEGlobalData_T<float>& df = dynamic_cast<Post::FEGlobalData_T<float>&>(pstate->m_Data[nfield]);
							df.setValue(a);
						}
						else if (it.ntype == VEC3F)
						{
							vec3f a;
							m_ar.read(a);

							Post::FEGlobalData_T<vec3f>& dv = dynamic_cast<Post::FEGlobalData_T<vec3f>&>(pstate->m_Data[nfield]);
							dv.setValue(a);
						}
						else if (it.ntype == MAT3FS)
						{
							mat3fs a;
							m_ar.read(a);
							Post::FEGlobalData_T<mat3fs>& dv = dynamic_cast<Post::FEGlobalData_T<mat3fs>&>(pstate->m_Data[nfield]);
							dv.setValue(a);
						}
						else if (it.ntype == TENS4FS)
						{
							tens4fs a;
							m_ar.read(a);
							Post::FEGlobalData_T<tens4fs>& dv = dynamic_cast<Post::FEGlobalData_T<tens4fs>&>(pstate->m_Data[nfield]);
							dv.setValue(a);
						}
						else if (it.ntype == MAT3F)
						{
							mat3f a;
							m_ar.read(a);
							Post::FEGlobalData_T<mat3f>& dv = dynamic_cast<Post::FEGlobalData_T<mat3f>&>(pstate->m_Data[nfield]);
							dv.setValue(a);
						}
						else if (it.ntype == ARRAY)
						{
							Post::FEGlobalArrayData& dv = dynamic_cast<Post::FEGlobalArrayData&>(pstate->m_Data[nfield]);
							int n = dv.components();
							vector<float> a(n);
							m_ar.read(a);
							dv.setData(a);
						}
						else
						{
							assert(false);
							return errf("Error while reading node data");
						}
						m_ar.CloseChunk();
					}
				}
				else
				{
					assert(false);
					return errf("Error while reading node data");
				}
				m_ar.CloseChunk();
			}
		}
		else
		{
			assert(false);
			return errf("Error while reading node data");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadMaterialData(FEPostModel& fem, FEState* pstate)
{
	return false;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadNodeData(FEPostModel& fem, FEState* pstate)
{
	FEDataManager& dm = *fem.GetDataManager();
	Post::FEPostMesh& mesh = *GetCurrentMesh();
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_STATE_VARIABLE)
		{
			int nv = -1;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_STATE_VAR_ID) m_ar.read(nv);
				else if (nid ==	PLT_STATE_VAR_DATA)
				{
					nv--;
					assert((nv>=0)&&(nv<(int)m_dic.m_Node.size()));
					if ((nv<0) || (nv >= (int)m_dic.m_Node.size())) return errf("Failed reading node data");

					DICT_ITEM it = m_dic.m_Node[nv];
					int nfield = dm.FindDataField(it.szname);
					int ndata = 0;
					int NN = mesh.Nodes();
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int ns = m_ar.GetChunkID();
						assert(ns == 0);

						if (it.ntype == FLOAT)
						{
							vector<float> a(NN);
							m_ar.read(a);

							Post::FENodeData<float>& df = dynamic_cast<Post::FENodeData<float>&>(pstate->m_Data[nfield]);
							for (int j=0; j<NN; ++j) df[j] = a[j];
						}
						else if (it.ntype == VEC3F)
						{
							vector<vec3f> a(NN);
							m_ar.read(a);

							Post::FENodeData<vec3f>& dv = dynamic_cast<Post::FENodeData<vec3f>&>(pstate->m_Data[nfield]);
							for (int j=0; j<NN; ++j) dv[j] = a[j];
						}
						else if (it.ntype == MAT3FS)
						{
							vector<mat3fs> a(NN);
							m_ar.read(a);
							Post::FENodeData<mat3fs>& dv = dynamic_cast<Post::FENodeData<mat3fs>&>(pstate->m_Data[nfield]);
							for (int j=0; j<NN; ++j) dv[j] = a[j];
						}
						else if (it.ntype == TENS4FS)
						{
							vector<tens4fs> a(NN);
							m_ar.read(a);
							Post::FENodeData<tens4fs>& dv = dynamic_cast<Post::FENodeData<tens4fs>&>(pstate->m_Data[nfield]);
							for (int j=0; j<NN; ++j) dv[j] = a[j];
						}
						else if (it.ntype == MAT3F)
						{
							vector<mat3f> a(NN);
							m_ar.read(a);
							Post::FENodeData<mat3f>& dv = dynamic_cast<Post::FENodeData<mat3f>&>(pstate->m_Data[nfield]);
							for (int j=0; j<NN; ++j) dv[j] = a[j];
						}
						else if (it.ntype == ARRAY)
						{
							int D = it.arraySize; assert(D != 0);
							vector<float> a(NN*D);
							m_ar.read(a);
							FENodeArrayData& dv = dynamic_cast<FENodeArrayData&>(pstate->m_Data[nfield]);
							dv.setData(a);
						}
						else
						{
							assert(false);
							return errf("Error while reading node data");;
						}
						m_ar.CloseChunk();
					}
				}
				else
				{
					assert(false);
					return errf("Error while reading node data");
				}
				m_ar.CloseChunk();
			}
		}
		else
		{
			assert(false);
			return errf("Error while reading node data");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadElemData(FEPostModel &fem, FEState* pstate)
{
	Post::FEPostMesh& mesh = *GetCurrentMesh();
	FEDataManager& dm = *fem.GetDataManager();
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_STATE_VARIABLE)
		{
			int nv = -1;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_STATE_VAR_ID) m_ar.read(nv);
				else if (nid ==	PLT_STATE_VAR_DATA)
				{
					nv--;
					assert((nv>=0)&&(nv<(int)m_dic.m_Elem.size()));
					if ((nv < 0) || (nv >= (int) m_dic.m_Elem.size())) return errf("Failed reading all state data");
					DICT_ITEM it = m_dic.m_Elem[nv];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int nd = m_ar.GetChunkID() - 1;
						assert((nd >= 0)&&(nd < m_xmesh.domains()));
						if ((nd < 0) || (nd >= (int)m_xmesh.domains())) return errf("Failed reading all state data");

						int nfield = dm.FindDataField(it.szname);

						Domain& dom = m_xmesh.domain(nd);
						FEElemItemData& ed = dynamic_cast<FEElemItemData&>(pstate->m_Data[nfield]);
						switch (it.nfmt)
						{
						case FMT_NODE: ReadElemData_NODE(mesh, dom, ed, it.ntype, it.arraySize); break;
						case FMT_ITEM: ReadElemData_ITEM(dom, ed, it.ntype, it.arraySize); break;
						case FMT_MULT: ReadElemData_MULT(dom, ed, it.ntype); break;
						case FMT_REGION: 
							switch (it.ntype)
							{
							case FLOAT  : ReadElemData_REGION<float  >(m_ar, dom, ed, it.ntype); break;
							case VEC3F  : ReadElemData_REGION<vec3f  >(m_ar, dom, ed, it.ntype); break;
							case MAT3FS : ReadElemData_REGION<mat3fs >(m_ar, dom, ed, it.ntype); break;
							case MAT3FD : ReadElemData_REGION<mat3fd >(m_ar, dom, ed, it.ntype); break;
							case TENS4FS: ReadElemData_REGION<tens4fs>(m_ar, dom, ed, it.ntype); break;
							case MAT3F  : ReadElemData_REGION<mat3f  >(m_ar, dom, ed, it.ntype); break;
							default:
								assert(false);
								return errf("Error reading element data");
							}
							break;
						default:
							assert(false);
							return errf("Error reading element data");
						}
						m_ar.CloseChunk();
					}
				}
				else
				{
					assert(false);
					return errf("Error while reading element data");
				}
				m_ar.CloseChunk();
			}
		}
		else
		{
			assert(false);
			return errf("Error while reading element data");
		}
		m_ar.CloseChunk();
	}
	return true;
}


//-----------------------------------------------------------------------------
bool XpltReader3::ReadElemData_NODE(Post::FEPostMesh& m, XpltReader3::Domain &d, Post::FEMeshData &data, int ntype, int arrSize)
{
	int ne = 0;
	switch (d.etype)
	{
	case PLT_ELEM_HEX8   : ne =  8; break;
	case PLT_ELEM_PENTA  : ne =  6; break;
    case PLT_ELEM_PENTA15: ne =  15; break;
    case PLT_ELEM_TET4   : ne =  4; break;
	case PLT_ELEM_TET5   : ne =  5; break;
	case PLT_ELEM_QUAD   : ne =  4; break;
	case PLT_ELEM_TRI    : ne =  3; break;
	case PLT_ELEM_TRUSS  : ne =  2; break;
	case PLT_ELEM_HEX20  : ne = 20; break;
	case PLT_ELEM_HEX27  : ne = 27; break;
	case PLT_ELEM_TET10  : ne = 10; break;
	case PLT_ELEM_TET15  : ne = 15; break;
	case PLT_ELEM_TET20  : ne = 20; break;
    case PLT_ELEM_TRI6   : ne =  6; break;
    case PLT_ELEM_QUAD8  : ne =  8; break;
    case PLT_ELEM_QUAD9  : ne =  9; break;
	case PLT_ELEM_PYRA5  : ne =  5; break;
    case PLT_ELEM_PYRA13 : ne = 13; break;
	default:
		assert(false);
		return errf("Error while reading element data");
	}

	int i, j;

	// set nodal tags to local node number
	int NN = m.Nodes();
	for (i=0; i<NN; ++i) m.Node(i).m_ntag = -1;

	int n = 0;
	for (i=0; i<d.ne; ++i)
	{
		ELEM& e = d.elem[i];
		for (j=0; j<ne; ++j)
			if (m.Node(e.node[j]).m_ntag == -1) m.Node(e.node[j]).m_ntag = n++;
	}

	// create the element list
	vector<int> e(d.ne);
	for (i=0; i<d.ne; ++i) e[i] = d.elem[i].index;

	// create the local node index list
	vector<int> l(ne*d.ne);
	for (i=0; i<d.ne; ++i)
	{
		ELEM& e = d.elem[i];
		for (j=0; j<ne; ++j) l[ne*i+j] = m.Node(e.node[j]).m_ntag;
	}

	// get the data
	switch (ntype)
	{
	case FLOAT:
		{
			Post::FEElementData<float,DATA_NODE>& df = dynamic_cast<Post::FEElementData<float,DATA_NODE>&>(data);
			vector<float> a(n);
			m_ar.read(a);
			df.add(a, e, l, ne);
		}
		break;
	case VEC3F:
		{
			Post::FEElementData<vec3f,DATA_NODE>& df = dynamic_cast<Post::FEElementData<vec3f,DATA_NODE>&>(data);
			vector<vec3f> a(n);
			m_ar.read(a);
			df.add(a, e, l, ne);
		}
		break;
	case MAT3F:
		{
			Post::FEElementData<mat3f,DATA_NODE>& df = dynamic_cast<Post::FEElementData<mat3f,DATA_NODE>&>(data);
			vector<mat3f> a(n);
			m_ar.read(a);
			df.add(a, e, l, ne);
		}
		break;
	case MAT3FS:
		{
			Post::FEElementData<mat3fs,DATA_NODE>& df = dynamic_cast<Post::FEElementData<mat3fs,DATA_NODE>&>(data);
			vector<mat3fs> a(n);
			m_ar.read(a);
			df.add(a, e, l, ne);
		}
		break;
	case MAT3FD:
		{
			Post::FEElementData<mat3fd,DATA_NODE>& df = dynamic_cast<Post::FEElementData<mat3fd,DATA_NODE>&>(data);
			vector<mat3fd> a(n);
			m_ar.read(a);
			df.add(a, e, l, ne);
		}
		break;
    case TENS4FS:
		{
			Post::FEElementData<tens4fs,DATA_NODE>& df = dynamic_cast<Post::FEElementData<tens4fs,DATA_NODE>&>(data);
			vector<tens4fs> a(n);
			m_ar.read(a);
			df.add(a, e, l, ne);
		}
        break;
	case ARRAY:
	{
		FEElemArrayDataNode& df = dynamic_cast<FEElemArrayDataNode&>(data);
		vector<float> a(n*arrSize);
		m_ar.read(a);
		df.add(a, e, l, ne);
	}
	break;
	default:
		assert(false);
		return errf("Error while reading element data");
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadElemData_ITEM(XpltReader3::Domain& dom, Post::FEMeshData& s, int ntype, int arrSize)
{
	int NE = dom.ne;
	switch (ntype)
	{
	case FLOAT:
		{
			vector<float> a(NE);
			m_ar.read(a);
			Post::FEElementData<float,DATA_ITEM>& df = dynamic_cast<Post::FEElementData<float,DATA_ITEM>&>(s);
			for (int i=0; i<NE; ++i) df.add(dom.elem[i].index, a[i]);
		}
		break;
	case VEC3F:
		{
			vector<vec3f> a(NE);
			m_ar.read(a);
			Post::FEElementData<vec3f,DATA_ITEM>& dv = dynamic_cast<Post::FEElementData<vec3f,DATA_ITEM>&>(s);
			for (int i=0; i<NE; ++i) dv.add(dom.elem[i].index, a[i]);
		}
		break;
	case MAT3FS:
		{
			vector<mat3fs> a(NE);
			m_ar.read(a);
			Post::FEElementData<mat3fs,DATA_ITEM>& dm = dynamic_cast<Post::FEElementData<mat3fs,DATA_ITEM>&>(s);
			for (int i=0; i<NE; ++i) dm.add(dom.elem[i].index, a[i]);
		}
		break;
	case MAT3FD:
		{
			vector<mat3fd> a(NE);
			m_ar.read(a);
			Post::FEElementData<mat3fd,DATA_ITEM>& dm = dynamic_cast<Post::FEElementData<mat3fd,DATA_ITEM>&>(s);
			for (int i=0; i<NE; ++i) dm.add(dom.elem[i].index, a[i]);
		}
		break;
    case TENS4FS:
		{
			vector<tens4fs> a(NE);
			m_ar.read(a);
			Post::FEElementData<tens4fs,DATA_ITEM>& dm = dynamic_cast<Post::FEElementData<tens4fs,DATA_ITEM>&>(s);
			for (int i=0; i<NE; ++i) dm.add(dom.elem[i].index, a[i]);
		}
        break;
	case MAT3F:
		{
			vector<mat3f> a(NE);
			m_ar.read(a);
			Post::FEElementData<mat3f,DATA_ITEM>& dm = dynamic_cast<Post::FEElementData<mat3f,DATA_ITEM>&>(s);
			for (int i=0; i<NE; ++i) dm.add(dom.elem[i].index, a[i]);
		}
		break;
	case ARRAY:
	{
		vector<float> a(NE*arrSize);
		if (arrSize > 0)
		{
			m_ar.read(a);
			FEElemArrayDataItem& dm = dynamic_cast<FEElemArrayDataItem&>(s);

			vector<int> elem(NE);
			for (int i = 0; i < NE; ++i) elem[i] = dom.elem[i].index;

			dm.setData(a, elem);
		}
	}
	break;
	case ARRAY_VEC3F:
	{
		vector<float> a(NE*arrSize * 3);
		m_ar.read(a);
		FEElemArrayVec3Data& dm = dynamic_cast<FEElemArrayVec3Data&>(s);

		vector<int> elem(NE);
		for (int i = 0; i<NE; ++i) elem[i] = dom.elem[i].index;

		dm.setData(a, elem);
	}
	break;
	default:
		assert(false);
		return errf("Error while reading element data");
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadElemData_MULT(XpltReader3::Domain& dom, Post::FEMeshData& s, int ntype)
{
	int NE = dom.ne;
	int ne = 0;
	switch (dom.etype)
	{
	case PLT_ELEM_HEX8   : ne =  8; break;
	case PLT_ELEM_PENTA  : ne =  6; break;
    case PLT_ELEM_PENTA15: ne =  15; break;
    case PLT_ELEM_TET4   : ne =  4; break;
	case PLT_ELEM_TET5   : ne =  5; break;
	case PLT_ELEM_QUAD   : ne =  4; break;
	case PLT_ELEM_TRI    : ne =  3; break;
	case PLT_ELEM_TRUSS  : ne =  2; break;
	case PLT_ELEM_HEX20  : ne = 20; break;
	case PLT_ELEM_HEX27  : ne = 27; break;
	case PLT_ELEM_TET10  : ne = 10; break;
	case PLT_ELEM_TET15  : ne = 15; break;
	case PLT_ELEM_TET20  : ne = 20; break;
    case PLT_ELEM_TRI6   : ne =  6; break;
    case PLT_ELEM_QUAD8  : ne =  8; break;
    case PLT_ELEM_QUAD9  : ne =  9; break;
	case PLT_ELEM_PYRA5  : ne =  5; break;
    case PLT_ELEM_PYRA13 : ne = 13; break;
	default:
		assert(false);
		return errf("Error while reading element data");
	}

	int nsize = NE*ne;

	switch (ntype)
	{
	case FLOAT:
		{
			vector<float> a(nsize), d(NE);
			m_ar.read(a);

			Post::FEElementData<float,DATA_MULT>& df = dynamic_cast<Post::FEElementData<float,DATA_MULT>&>(s);
			for (int i=0; i<NE; ++i) df.add(dom.elem[i].index, ne, &a[i*ne]);
		}
		break;
	case VEC3F:
		{
			vector<vec3f> a(nsize), d(NE);
			m_ar.read(a);

			Post::FEElementData<vec3f,DATA_MULT>& df = dynamic_cast<Post::FEElementData<vec3f,DATA_MULT>&>(s);
			for (int i=0; i<NE; ++i) df.add(dom.elem[i].index, ne, &a[i*ne]);
		};
		break;
	case MAT3FS:
		{
			vector<mat3fs> a(nsize), d(NE);
			m_ar.read(a);

			Post::FEElementData<mat3fs,DATA_MULT>& df = dynamic_cast<Post::FEElementData<mat3fs,DATA_MULT>&>(s);
			for (int i=0; i<NE; ++i) df.add(dom.elem[i].index, ne, &a[i*ne]);
		};
		break;
	case MAT3FD:
		{
			vector<mat3fd> a(nsize), d(NE);
			m_ar.read(a);

			Post::FEElementData<mat3fd,DATA_MULT>& df = dynamic_cast<Post::FEElementData<mat3fd,DATA_MULT>&>(s);
			for (int i=0; i<NE; ++i) df.add(dom.elem[i].index, ne, &a[i*ne]);
		};
		break;
	case MAT3F:
		{
			vector<mat3f> a(nsize), d(NE);
			m_ar.read(a);

			Post::FEElementData<mat3f,DATA_MULT>& df = dynamic_cast<Post::FEElementData<mat3f,DATA_MULT>&>(s);
			for (int i=0; i<NE; ++i) df.add(dom.elem[i].index, ne, &a[i*ne]);
		};
		break;
    case TENS4FS:
		{
			vector<tens4fs> a(nsize), d(NE);
			m_ar.read(a);
            
			Post::FEElementData<tens4fs,DATA_MULT>& df = dynamic_cast<Post::FEElementData<tens4fs,DATA_MULT>&>(s);
			for (int i=0; i<NE; ++i) df.add(dom.elem[i].index, ne, &a[i*ne]);
		};
        break;
	default:
		assert(false);
		return errf("Error while reading element data");
	}

	return true;
}

bool XpltReader3::ReadFaceData(FEPostModel& fem, FEState* pstate)
{
	Post::FEPostMesh& mesh = *GetCurrentMesh();
	FEDataManager& dm = *fem.GetDataManager();
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_STATE_VARIABLE)
		{
			int nv = -1;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_STATE_VAR_ID) m_ar.read(nv);
				else if (nid ==	PLT_STATE_VAR_DATA)
				{
					nv--;
					assert((nv>=0)&&(nv<(int)m_dic.m_Face.size()));
					if ((nv < 0) || (nv >= (int)m_dic.m_Face.size())) return errf("Failed reading all state data");
					const DICT_ITEM& it = m_dic.m_Face[nv];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int ns = m_ar.GetChunkID() - 1;
						assert((ns >= 0)&&(ns < m_xmesh.surfaces()));
						if ((ns < 0) || (ns >= m_xmesh.surfaces())) return errf("Failed reading all state data");

//						int nfield = dm.FindDataField(it.szname);
						int nfield = it.index;

						Surface& s = m_xmesh.surface(ns);
						switch (it.nfmt)
						{
						case FMT_NODE  : if (ReadFaceData_NODE  (mesh, s, pstate->m_Data[nfield], it.ntype) == false) return errf("Failed reading face data"); break;
						case FMT_ITEM  : if (ReadFaceData_ITEM  (s, pstate->m_Data[nfield], it.ntype   ) == false) return errf("Failed reading face data"); break;
						case FMT_MULT  : if (ReadFaceData_MULT  (mesh, s, pstate->m_Data[nfield], it.ntype) == false) return errf("Failed reading face data"); break;
						case FMT_REGION: 
							switch (it.ntype)
							{
								case FLOAT  : ReadFaceData_REGION<float  >(m_ar, mesh, s, pstate->m_Data[nfield]); break;
								case VEC3F  : ReadFaceData_REGION<vec3f  >(m_ar, mesh, s, pstate->m_Data[nfield]); break;
								case MAT3FS : ReadFaceData_REGION<mat3fs >(m_ar, mesh, s, pstate->m_Data[nfield]); break;
								case MAT3FD : ReadFaceData_REGION<mat3fd >(m_ar, mesh, s, pstate->m_Data[nfield]); break;
								case TENS4FS: ReadFaceData_REGION<tens4fs>(m_ar, mesh, s, pstate->m_Data[nfield]); break;
								case MAT3F  : ReadFaceData_REGION<mat3f  >(m_ar, mesh, s, pstate->m_Data[nfield]); break;
								default:
									return errf("Failed reading face data");
							}
							break;
						default:
							return errf("Failed reading face data");
						}
						m_ar.CloseChunk();
					}
				}
				else
				{
					return errf("Failed reading face data");
				}
				m_ar.CloseChunk();
			}
		}
		else 
		{
			return errf("Failed reading face data");
		}
		m_ar.CloseChunk();
	}
	return true;
}

bool XpltReader3::ReadEdgeData(FEPostModel& fem, FEState* pstate)
{
	Post::FEPostMesh& mesh = *GetCurrentMesh();
	FEDataManager& dm = *fem.GetDataManager();
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_STATE_VARIABLE)
		{
			int nv = -1;
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				int nid = m_ar.GetChunkID();
				if (nid == PLT_STATE_VAR_ID) m_ar.read(nv);
				else if (nid ==	PLT_STATE_VAR_DATA)
				{
					nv--;
					assert((nv>=0)&&(nv<(int)m_dic.m_Edge.size()));
					if ((nv < 0) || (nv >= (int)m_dic.m_Edge.size())) return errf("Failed reading all state data");
					const DICT_ITEM& it = m_dic.m_Edge[nv];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int ns = m_ar.GetChunkID() - 1;
						assert((ns >= 0)&&(ns < m_xmesh.edges()));
						if ((ns < 0) || (ns >= m_xmesh.edges())) return errf("Failed reading all state data");

//						int nfield = dm.FindDataField(it.szname);
						int nfield = it.index;

						Edge& e = m_xmesh.edge(ns);
						switch (it.nfmt)
						{
						case FMT_NODE  : if (ReadEdgeData_NODE  (mesh, e, pstate->m_Data[nfield], it.ntype) == false) return errf("Failed reading edge data"); break;
						case FMT_ITEM  : if (ReadEdgeData_ITEM  (e, pstate->m_Data[nfield], it.ntype   ) == false) return errf("Failed reading edge data"); break;
						case FMT_MULT  : if (ReadEdgeData_MULT  (mesh, e, pstate->m_Data[nfield], it.ntype) == false) return errf("Failed reading edge data"); break;
						case FMT_REGION: 
							switch (it.ntype)
							{
								case FLOAT  : ReadEdgeData_REGION<float  >(m_ar, mesh, e, pstate->m_Data[nfield]); break;
								case VEC3F  : ReadEdgeData_REGION<vec3f  >(m_ar, mesh, e, pstate->m_Data[nfield]); break;
								case MAT3FS : ReadEdgeData_REGION<mat3fs >(m_ar, mesh, e, pstate->m_Data[nfield]); break;
								case MAT3FD : ReadEdgeData_REGION<mat3fd >(m_ar, mesh, e, pstate->m_Data[nfield]); break;
								case TENS4FS: ReadEdgeData_REGION<tens4fs>(m_ar, mesh, e, pstate->m_Data[nfield]); break;
								case MAT3F  : ReadEdgeData_REGION<mat3f  >(m_ar, mesh, e, pstate->m_Data[nfield]); break;
								default:
									return errf("Failed reading edge data");
							}
							break;
						default:
							return errf("Failed reading edge data");
						}
						m_ar.CloseChunk();
					}
				}
				else
				{
					return errf("Failed reading edge data");
				}
				m_ar.CloseChunk();
			}
		}
		else 
		{
			return errf("Failed reading edge data");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadFaceData_MULT(Post::FEPostMesh& m, XpltReader3::Surface &s, Post::FEMeshData &data, int ntype)
{
	// It is possible that the node ordering of the FACE's are different than the FSFace's
	// so we setup up an array to unscramble the nodal values
	int NF = s.nfaces;
	vector<int> tag;
	tag.assign(m.Nodes(), -1);
	const int NFM = s.maxNodes;
	vector<vector<int> > l(NF, vector<int>(NFM));
	for (int i=0; i<NF; ++i)
	{
		FACE& f = s.face[i];
		if (f.nid >= 0)
		{
			FSFace& fm = m.Face(f.nid);
			for (int j=0; j<f.nn; ++j) tag[f.node[j]] = j;
			for (int j=0; j<f.nn; ++j) l[i][j] = tag[fm.n[j]];
		}
	}

	bool bok = true;

	switch (ntype)
	{
	case FLOAT:
		{
			FEFaceData<float,DATA_MULT>& df = dynamic_cast<FEFaceData<float,DATA_MULT>&>(data);
			vector<float> a(NFM*NF);
			m_ar.read(a);
			float v[10];
			for (int i=0; i<NF; ++i)
			{
				FACE& f = s.face[i];
				vector<int>& li = l[i];
				for (int j=0; j<f.nn; ++j) v[j] = a[NFM*i + li[j]];
				if (f.nid >= 0) bok &= df.add(f.nid, v, f.nn);
			}
		}
		break;
	case VEC3F:
		{
			FEFaceData<vec3f,DATA_MULT>& df = dynamic_cast<FEFaceData<vec3f,DATA_MULT>&>(data);
			vector<vec3f> a(NFM*NF);
			m_ar.read(a);
			vec3f v[10];
			for (int i=0; i<NF; ++i)
			{
				FACE& f = s.face[i];
				vector<int>& li = l[i];
				for (int j=0; j<f.nn; ++j) v[j] = a[NFM*i + li[j]];
				if (f.nid >= 0) bok &= df.add(f.nid, v, f.nn);
			}
		}
		break;
	case MAT3FS:
		{
			FEFaceData<mat3fs,DATA_MULT>& df = dynamic_cast<FEFaceData<mat3fs,DATA_MULT>&>(data);
			vector<mat3fs> a(4*NF);
			m_ar.read(a);
			mat3fs v[10];
			for (int i=0; i<NF; ++i)
			{
				FACE& f = s.face[i];
				vector<int>& li = l[i];
				for (int j=0; j<f.nn; ++j) v[j] = a[NFM*i + li[j]];
				if (f.nid >= 0) bok &= df.add(f.nid, v, f.nn);
			}
		}
		break;
	case MAT3F:
		{
			FEFaceData<mat3f,DATA_MULT>& df = dynamic_cast<FEFaceData<mat3f,DATA_MULT>&>(data);
			vector<mat3f> a(4*NF);
			m_ar.read(a);
			mat3f v[10];
			for (int i=0; i<NF; ++i)
			{
				FACE& f = s.face[i];
				vector<int>& li = l[i];
				for (int j=0; j<f.nn; ++j) v[j] = a[NFM*i + li[j]];
				if (f.nid >= 0) bok &= df.add(f.nid, v, f.nn);
			}
		}
		break;
	case MAT3FD:
		{
			FEFaceData<mat3fd,DATA_MULT>& df = dynamic_cast<FEFaceData<mat3fd,DATA_MULT>&>(data);
			vector<mat3fd> a(4*NF);
			m_ar.read(a);
			mat3fd v[10];
			for (int i=0; i<NF; ++i)
			{
				FACE& f = s.face[i];
				vector<int>& li = l[i];
				for (int j=0; j<f.nn; ++j) v[j] = a[NFM*i + li[j]];
				if (f.nid >= 0) bok &= df.add(f.nid, v, f.nn);
			}
		}
		break;	
    case TENS4FS:
		{
			FEFaceData<tens4fs,DATA_MULT>& df = dynamic_cast<FEFaceData<tens4fs,DATA_MULT>&>(data);
			vector<tens4fs> a(4*NF);
			m_ar.read(a);
			tens4fs v[10];
			for (int i=0; i<NF; ++i)
			{
				FACE& f = s.face[i];
				vector<int>& li = l[i];
				for (int j=0; j<f.nn; ++j) v[j] = a[NFM*i + li[j]];
				if (f.nid >= 0) bok &= df.add(f.nid, v, f.nn);
			}
		}
        break;
	default:
		return errf("Failed reading face data");
	}

	if (bok == false) addWarning(XPLT_READ_DUPLICATE_FACES);

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadFaceData_ITEM(XpltReader3::Surface &s, Post::FEMeshData &data, int ntype)
{
	int NF = s.nfaces;
	switch (ntype)
	{
	case FLOAT:
		{
			FEFaceData<float,DATA_ITEM>& df = dynamic_cast<FEFaceData<float,DATA_ITEM>&>(data);
			vector<float> a(NF);
			m_ar.read(a);
			for (int i=0; i<NF; ++i) df.add(s.face[i].nid, a[i]);
		}
		break;
	case VEC3F:
		{
			vector<vec3f> a(NF);
			m_ar.read(a);
			FEFaceData<vec3f,DATA_ITEM>& dv = dynamic_cast<FEFaceData<vec3f,DATA_ITEM>&>(data);
			for (int i=0; i<NF; ++i) dv.add(s.face[i].nid, a[i]);
		}
		break;
	case MAT3FS:
		{
			vector<mat3fs> a(NF);
			m_ar.read(a);
			FEFaceData<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEFaceData<mat3fs,DATA_ITEM>&>(data);
			for (int i=0; i<NF; ++i) dm.add(s.face[i].nid, a[i]);
		}
		break;
	case MAT3F:
		{
			vector<mat3f> a(NF);
			m_ar.read(a);
			FEFaceData<mat3f,DATA_ITEM>& dm = dynamic_cast<FEFaceData<mat3f,DATA_ITEM>&>(data);
			for (int i=0; i<NF; ++i) dm.add(s.face[i].nid, a[i]);
		}
		break;
	case MAT3FD:
		{
			vector<mat3fd> a(NF);
			m_ar.read(a);
			FEFaceData<mat3fd,DATA_ITEM>& dm = dynamic_cast<FEFaceData<mat3fd,DATA_ITEM>&>(data);
			for (int i=0; i<NF; ++i) dm.add(s.face[i].nid, a[i]);
		}
		break;
    case TENS4FS:
		{
			vector<tens4fs> a(NF);
			m_ar.read(a);
			FEFaceData<tens4fs,DATA_ITEM>& dm = dynamic_cast<FEFaceData<tens4fs,DATA_ITEM>&>(data);
			for (int i=0; i<NF; ++i) dm.add(s.face[i].nid, a[i]);
		}
        break;
	default:
		return errf("Failed reading face data");
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadFaceData_NODE(Post::FEPostMesh& m, XpltReader3::Surface &s, Post::FEMeshData &data, int ntype)
{
	// set nodal tags to local node number
	int NN = m.Nodes();
	for (int i = 0; i<NN; ++i) m.Node(i).m_ntag = -1;

	int n = 0;
	for (int i = 0; i<s.nfaces; ++i)
	{
		FACE& f = s.face[i];
		int nf = f.nn;
		for (int j = 0; j<nf; ++j)
		if (m.Node(f.node[j]).m_ntag == -1) m.Node(f.node[j]).m_ntag = n++;
	}

	// create the face list
	vector<int> f(s.nfaces);
	for (int i = 0; i<s.nfaces; ++i) f[i] = s.face[i].nid;

	// create vector that stores the number of nodes for each facet
	vector<int> fn(s.nfaces, 0);
	for (int i = 0; i<s.nfaces; ++i) fn[i] = s.face[i].nn;

	// create the local node index list
	vector<int> l; l.resize(s.nfaces*FSFace::MAX_NODES);
	for (int i = 0; i<s.nfaces; ++i)
	{
		FSFace& f = m.Face(s.face[i].nid);
		int nn = f.Nodes();
		for (int j = 0; j<nn; ++j)
		{
			int n = m.Node(f.n[j]).m_ntag; assert(n >= 0);
			l.push_back(n);
		}
	}

	// get the data
	switch (ntype)
	{
	case FLOAT:
	{
				  FEFaceData<float, DATA_NODE>& df = dynamic_cast<FEFaceData<float, DATA_NODE>&>(data);
				  vector<float> a(n);
				  m_ar.read(a);
				  df.add(a, f, l, fn);
	}
		break;
	case VEC3F:
	{
				  FEFaceData<vec3f, DATA_NODE>& df = dynamic_cast<FEFaceData<vec3f, DATA_NODE>&>(data);
				  vector<vec3f> a(n);
				  m_ar.read(a);
				  df.add(a, f, l, fn);
	}
		break;
	case MAT3FS:
	{
				   FEFaceData<mat3fs, DATA_NODE>& df = dynamic_cast<FEFaceData<mat3fs, DATA_NODE>&>(data);
				   vector<mat3fs> a(n);
				   m_ar.read(a);
				   df.add(a, f, l, fn);
	}
		break;
	case MAT3F:
	{
				  FEFaceData<mat3f, DATA_NODE>& df = dynamic_cast<FEFaceData<mat3f, DATA_NODE>&>(data);
				  vector<mat3f> a(n);
				  m_ar.read(a);
				  df.add(a, f, l, fn);
	}
		break;
	case MAT3FD:
	{
				   FEFaceData<mat3fd, DATA_NODE>& df = dynamic_cast<FEFaceData<mat3fd, DATA_NODE>&>(data);
				   vector<mat3fd> a(n);
				   m_ar.read(a);
				   df.add(a, f, l, fn);
	}
		break;
	case TENS4FS:
	{
					FEFaceData<tens4fs, DATA_NODE>& df = dynamic_cast<FEFaceData<tens4fs, DATA_NODE>&>(data);
					vector<tens4fs> a(n);
					m_ar.read(a);
					df.add(a, f, l, fn);
	}
		break;
	default:
		return errf("Failed reading face data");
	}

	return true;
}

bool XpltReader3::ReadEdgeData_MULT(Post::FEPostMesh& m, XpltReader3::Edge& e, Post::FEMeshData& data, int ntype)
{
	// It is possible that the node ordering of the FACE's are different than the FSFace's
	// so we setup up an array to unscramble the nodal values
	int NL = e.nlines;
	vector<int> tag;
	tag.assign(m.Nodes(), -1);
	const int NFM = e.maxNodes;
	vector<vector<int> > lut(NL, vector<int>(NFM));
	for (int i = 0; i < NL; ++i)
	{
		LINE& l = e.line[i];
		if (l.id >= 0)
		{
			FSEdge& em = m.Edge(l.id);
			for (int j = 0; j < l.nn; ++j) tag[l.node[j]] = j;
			for (int j = 0; j < l.nn; ++j) lut[i][j] = tag[em.n[j]];
		}
	}

	bool bok = true;

	switch (ntype)
	{
	case FLOAT:
	{
		FEEdgeData<float, DATA_MULT>& df = dynamic_cast<FEEdgeData<float, DATA_MULT>&>(data);
		vector<float> a(NFM * NL);
		m_ar.read(a);
		float v[4];
		for (int i = 0; i < NL; ++i)
		{
			LINE& l = e.line[i];
			vector<int>& li = lut[i];
			for (int j = 0; j < l.nn; ++j) v[j] = a[NFM * i + li[j]];
			if (l.id >= 0) bok &= df.add(l.id, v, l.nn);
		}
	}
	break;
	case VEC3F:
	{
		FEEdgeData<vec3f, DATA_MULT>& df = dynamic_cast<FEEdgeData<vec3f, DATA_MULT>&>(data);
		vector<vec3f> a(NFM * NL);
		m_ar.read(a);
		vec3f v[4];
		for (int i = 0; i < NL; ++i)
		{
			LINE& l = e.line[i];
			vector<int>& li = lut[i];
			for (int j = 0; j < l.nn; ++j) v[j] = a[NFM * i + li[j]];
			if (l.id >= 0) bok &= df.add(l.id, v, l.nn);
		}
	}
	break;
	case MAT3FS:
	{
		FEEdgeData<mat3fs, DATA_MULT>& df = dynamic_cast<FEEdgeData<mat3fs, DATA_MULT>&>(data);
		vector<mat3fs> a(NFM * NL);
		m_ar.read(a);
		mat3fs v[4];
		for (int i = 0; i < NL; ++i)
		{
			LINE& l = e.line[i];
			vector<int>& li = lut[i];
			for (int j = 0; j < l.nn; ++j) v[j] = a[NFM * i + li[j]];
			if (l.id >= 0) bok &= df.add(l.id, v, l.nn);
		}
	}
	break;
	case MAT3F:
	{
		FEEdgeData<mat3f, DATA_MULT>& df = dynamic_cast<FEEdgeData<mat3f, DATA_MULT>&>(data);
		vector<mat3f> a(NFM * NL);
		m_ar.read(a);
		mat3f v[4];
		for (int i = 0; i < NL; ++i)
		{
			LINE& l = e.line[i];
			vector<int>& li = lut[i];
			for (int j = 0; j < l.nn; ++j) v[j] = a[NFM * i + li[j]];
			if (l.id >= 0) bok &= df.add(l.id, v, l.nn);
		}
	}
	break;
	case MAT3FD:
	{
		FEEdgeData<mat3fd, DATA_MULT>& df = dynamic_cast<FEEdgeData<mat3fd, DATA_MULT>&>(data);
		vector<mat3fd> a(NFM * NL);
		m_ar.read(a);
		mat3fd v[4];
		for (int i = 0; i < NL; ++i)
		{
			LINE& l = e.line[i];
			vector<int>& li = lut[i];
			for (int j = 0; j < l.nn; ++j) v[j] = a[NFM * i + li[j]];
			if (l.id >= 0) bok &= df.add(l.id, v, l.nn);
		}
	}
	break;
	case TENS4FS:
	{
		FEEdgeData<tens4fs, DATA_MULT>& df = dynamic_cast<FEEdgeData<tens4fs, DATA_MULT>&>(data);
		vector<tens4fs> a(NFM * NL);
		m_ar.read(a);
		tens4fs v[4];
		for (int i = 0; i < NL; ++i)
		{
			LINE& l = e.line[i];
			vector<int>& li = lut[i];
			for (int j = 0; j < l.nn; ++j) v[j] = a[NFM * i + li[j]];
			if (l.id >= 0) bok &= df.add(l.id, v, l.nn);
		}
	}
	break;
	default:
		return errf("Failed reading edge data");
	}

	if (bok == false) addWarning(XPLT_READ_DUPLICATE_EDGES);

	return true;
}

bool XpltReader3::ReadEdgeData_ITEM(XpltReader3::Edge& e, Post::FEMeshData& data, int ntype)
{
	int NL = e.nlines;
	switch (ntype)
	{
	case FLOAT:
	{
		FEEdgeData<float, DATA_ITEM>& df = dynamic_cast<FEEdgeData<float, DATA_ITEM>&>(data);
		vector<float> a(NL);
		m_ar.read(a);
		for (int i = 0; i < NL; ++i) df.add(e.line[i].id, a[i]);
	}
	break;
	case VEC3F:
	{
		vector<vec3f> a(NL);
		m_ar.read(a);
		FEEdgeData<vec3f, DATA_ITEM>& dv = dynamic_cast<FEEdgeData<vec3f, DATA_ITEM>&>(data);
		for (int i = 0; i < NL; ++i) dv.add(e.line[i].id, a[i]);
	}
	break;
	case MAT3FS:
	{
		vector<mat3fs> a(NL);
		m_ar.read(a);
		FEEdgeData<mat3fs, DATA_ITEM>& dm = dynamic_cast<FEEdgeData<mat3fs, DATA_ITEM>&>(data);
		for (int i = 0; i < NL; ++i) dm.add(e.line[i].id, a[i]);
	}
	break;
	case MAT3F:
	{
		vector<mat3f> a(NL);
		m_ar.read(a);
		FEEdgeData<mat3f, DATA_ITEM>& dm = dynamic_cast<FEEdgeData<mat3f, DATA_ITEM>&>(data);
		for (int i = 0; i < NL; ++i) dm.add(e.line[i].id, a[i]);
	}
	break;
	case MAT3FD:
	{
		vector<mat3fd> a(NL);
		m_ar.read(a);
		FEEdgeData<mat3fd, DATA_ITEM>& dm = dynamic_cast<FEEdgeData<mat3fd, DATA_ITEM>&>(data);
		for (int i = 0; i < NL; ++i) dm.add(e.line[i].id, a[i]);
	}
	break;
	case TENS4FS:
	{
		vector<tens4fs> a(NL);
		m_ar.read(a);
		FEEdgeData<tens4fs, DATA_ITEM>& dm = dynamic_cast<FEEdgeData<tens4fs, DATA_ITEM>&>(data);
		for (int i = 0; i < NL; ++i) dm.add(e.line[i].id, a[i]);
	}
	break;
	default:
		return errf("Failed reading edge data");
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader3::ReadEdgeData_NODE(Post::FEPostMesh& m, XpltReader3::Edge& e, Post::FEMeshData& data, int ntype)
{
	// set nodal tags to local node number
	int NN = m.Nodes();
	for (int i = 0; i < NN; ++i) m.Node(i).m_ntag = -1;

	int n = 0;
	for (int i = 0; i < e.nlines; ++i)
	{
		LINE& l = e.line[i];
		int nf = l.nn;
		for (int j = 0; j < nf; ++j)
			if (m.Node(l.node[j]).m_ntag == -1) m.Node(l.node[j]).m_ntag = n++;
	}

	// create the edge list
	vector<int> f(e.nlines);
	for (int i = 0; i < e.nlines; ++i) f[i] = e.line[i].id;

	// create vector that stores the number of nodes for each facet
	vector<int> fn(e.nlines, 0);
	for (int i = 0; i < e.nlines; ++i) fn[i] = e.line[i].nn;

	// create the local node index list
	vector<int> l; l.reserve(e.nlines * FSEdge::MAX_NODES);
	for (int i = 0; i < e.nlines; ++i)
	{
		FSEdge& f = m.Edge(e.line[i].id);
		int nn = f.Nodes();
		for (int j = 0; j < nn; ++j)
		{
			int n = m.Node(f.n[j]).m_ntag; assert(n >= 0);
			l.push_back(n);
		}
	}

	// get the data
	switch (ntype)
	{
	case FLOAT:
	{
		FEEdgeData<float, DATA_NODE>& df = dynamic_cast<FEEdgeData<float, DATA_NODE>&>(data);
		vector<float> a(n);
		m_ar.read(a);
		df.add(a, f, l, fn);
	}
	break;
	case VEC3F:
	{
		FEEdgeData<vec3f, DATA_NODE>& df = dynamic_cast<FEEdgeData<vec3f, DATA_NODE>&>(data);
		vector<vec3f> a(n);
		m_ar.read(a);
		df.add(a, f, l, fn);
	}
	break;
	case MAT3FS:
	{
		FEEdgeData<mat3fs, DATA_NODE>& df = dynamic_cast<FEEdgeData<mat3fs, DATA_NODE>&>(data);
		vector<mat3fs> a(n);
		m_ar.read(a);
		df.add(a, f, l, fn);
	}
	break;
	case MAT3F:
	{
		FEEdgeData<mat3f, DATA_NODE>& df = dynamic_cast<FEEdgeData<mat3f, DATA_NODE>&>(data);
		vector<mat3f> a(n);
		m_ar.read(a);
		df.add(a, f, l, fn);
	}
	break;
	case MAT3FD:
	{
		FEEdgeData<mat3fd, DATA_NODE>& df = dynamic_cast<FEEdgeData<mat3fd, DATA_NODE>&>(data);
		vector<mat3fd> a(n);
		m_ar.read(a);
		df.add(a, f, l, fn);
	}
	break;
	case TENS4FS:
	{
		FEEdgeData<tens4fs, DATA_NODE>& df = dynamic_cast<FEEdgeData<tens4fs, DATA_NODE>&>(data);
		vector<tens4fs> a(n);
		m_ar.read(a);
		df.add(a, f, l, fn);
	}
	break;
	default:
		return errf("Failed reading edge data");
	}

	return true;
}
