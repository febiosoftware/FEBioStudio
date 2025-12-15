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

#include "xpltReader.h"
#include <MeshLib/FSNodeFaceList.h>
#include <PostLib/FEDataManager.h>
#include <PostLib/FEMeshData_T.h>
#include <PostLib/FEState.h>
#include <MeshLib/FSMesh.h>
#include <PostLib/FEPostModel.h>

using namespace Post;
using namespace std;

template <class Type> void ReadFaceData_REGION(xpltArchive& ar, FSMesh& m, XpltReader::Surface &s, Post::FEMeshData&data)
{
	int NF = s.nf;
	vector<int> face(NF);
	for (int i=0; i<(int) face.size(); ++i) face[i] = s.face[i].nid;

	FEFaceData<Type,DATA_REGION>& df = dynamic_cast<FEFaceData<Type,DATA_REGION>&>(data);
	Type a;
	ar.read(a);
	df.add(face, a);
}

template <class Type> void ReadElemData_REGION(xpltArchive& ar, XpltReader::Domain& dom, Post::FEMeshData& s, int ntype)
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

XpltReader::DICT_ITEM::DICT_ITEM()
{
}

XpltReader::DICT_ITEM::DICT_ITEM(const DICT_ITEM& item)
{
	ntype = item.ntype;
	nfmt  = item.nfmt;
	strcpy(szname, item.szname);

	index = item.index;

	arraySize = item.arraySize;
	arrayNames = item.arrayNames;
}

//=================================================================================================

//-----------------------------------------------------------------------------
XpltReader::XpltReader(xpltFileReader* xplt) : xpltParser(xplt)
{
	m_pstate = 0;
}

XpltReader::~XpltReader()
{
}

//-----------------------------------------------------------------------------
void XpltReader::Clear()
{
	m_dic.Clear();
	m_Mat.clear();
	m_Node.clear();
	m_Dom.clear();
	m_Surf.clear();
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
bool XpltReader::Load(FEPostModel& fem)
{
	// make sure all data is cleared
	Clear();

	// read the root section
	// (This section was already opened by xpltFileReader)
	if (ReadRootSection(fem) == false) return false;
	if (m_xplt->IsCanceled()) return false;

	// Clear the end-flag of the root section
	m_ar.CloseChunk();
	if (m_ar.OpenChunk() != xpltArchive::IO_END) return false;

	// Build the mesh
	if (BuildMesh(fem) == false) return false;
	if (m_xplt->IsCanceled()) return false;

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
				else if (read_state_flag == XPLT_READ_FIRST_AND_LAST)
				{
					if (nstate == 0)
					{
						fem.AddState(m_pstate);
						m_pstate = 0;
					}
				}
			}
			else 
				errf("Error while reading state data.");
			m_ar.CloseChunk();
		
			// clear end-flag
			if (m_ar.OpenChunk() != xpltArchive::IO_END)
			{
				break;
			}

			++nstate;

			if (m_xplt->IsCanceled())
			{
				break;
			}
		}
		if ((read_state_flag == XPLT_READ_LAST_STATE_ONLY) ||
			(read_state_flag == XPLT_READ_FIRST_AND_LAST)) { fem.AddState(m_pstate); m_pstate = 0; }
	}
	catch (...)
	{
		errf("An unknown exception has occurred.\nNot all data was read in.");
	}

	Clear();

	return true;
}

//-----------------------------------------------------------------------------
// Header section is already read by xpltFileReader
bool XpltReader::ReadRootSection(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		switch (nid)
		{
		case PLT_DICTIONARY: if (ReadDictionary(fem) == false) return false; break;
		case PLT_MATERIALS : if (ReadMaterials (fem) == false) return false; break;
		case PLT_GEOMETRY  : if (ReadMesh      (fem) == false) return false; break;
		default:
			return errf("Failed reading Root section");
		}
		m_ar.CloseChunk();
	}
	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadDictItem(DICT_ITEM& it)
{
	char szname[64] = {0};
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		switch(nid)
		{
		case PLT_DIC_ITEM_TYPE      : m_ar.read(it.ntype); break;
		case PLT_DIC_ITEM_FMT       : m_ar.read(it.nfmt ); break;
		case PLT_DIC_ITEM_ARRAYSIZE : m_ar.read(it.arraySize); break;
		case PLT_DIC_ITEM_ARRAYNAME :
			{
				char tmp[DI_NAME_SIZE] = {0};
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
		default:
			return errf("Error while reading dictionary section");
		}
		m_ar.CloseChunk();
	}

	assert((it.arrayNames.size() == 0) || (it.arrayNames.size() == it.arraySize));

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadDictionary(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		switch (nid)
		{
		case PLT_DIC_GLOBAL   : ReadGlobalDicItems  (); break;
		case PLT_DIC_MATERIAL : ReadMaterialDicItems(); break;
		case PLT_DIC_NODAL    : ReadNodeDicItems    (); break;
		case PLT_DIC_DOMAIN   : ReadElemDicItems    (); break;
		case PLT_DIC_SURFACE  : ReadFaceDicItems    (); break;
		default:
			return errf("Error while reading Dictionary.");
		}
		m_ar.CloseChunk();
	}

	// clear data manager
	FEDataManager* pdm = fem.GetDataManager();
	pdm->Clear();

	// read nodal variables
	int nfields = 0;
	int i;
	int nv = (int)m_dic.m_Node.size();
	for (i=0; i<nv; ++i)
	{
		DICT_ITEM& it = m_dic.m_Node[i];
		it.index = nfields++;

		// add nodal field
		Post::ModelDataField* pdf = nullptr;
		switch (it.ntype)
		{
		case FLOAT  : pdf = new FEDataField_T<Post::FENodeData<float  > >(&fem); break;
		case VEC3F  : pdf = new FEDataField_T<Post::FENodeData<vec3f  > >(&fem); break;
		case MAT3FS : pdf = new FEDataField_T<Post::FENodeData<mat3fs > >(&fem); break;
		case MAT3FD : pdf = new FEDataField_T<Post::FENodeData<mat3fd > >(&fem); break;
        case TENS4FS: pdf = new FEDataField_T<Post::FENodeData<tens4fs> >(&fem); break;
		case MAT3F  : pdf = new FEDataField_T<Post::FENodeData<mat3f  > >(&fem); break;
		case ARRAY  : 
			{
				FEArrayDataField* data = new FEArrayDataField(&fem, NODE_DATA, DATA_ITEM);
				data->SetArraySize(it.arraySize);
				data->SetArrayNames(it.arrayNames);
				pdf = data;
			}
			break;
		default:
			return errf("Error while reading dictionary");
		}
		pdf->SetName(it.szname);
		pdm->AddDataField(pdf);
	}

	// read solid variables
	nv = (int)m_dic.m_Elem.size();
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
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_NODE> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_NODE> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_NODE> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_NODE> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_NODE> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_NODE> >(&fem); break;
				case ARRAY  :
					{
						FEArrayDataField* data = new FEArrayDataField(&fem, ELEM_DATA, DATA_NODE);
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
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_ITEM> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_ITEM> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_ITEM> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_ITEM> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_ITEM> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_ITEM> >(&fem); break;
				case ARRAY  :
					{
						FEArrayDataField* data = new FEArrayDataField(&fem, ELEM_DATA, DATA_ITEM);
						data->SetArraySize(it.arraySize);
						data->SetArrayNames(it.arrayNames);
						pdf = data;
					}
					break;
				case ARRAY_VEC3F:
					{
						FEArrayVec3DataField* data = new FEArrayVec3DataField(&fem, ELEM_DATA);
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
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_MULT> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_MULT> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_MULT> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_MULT> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_MULT> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_MULT> >(&fem); break;
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
				case FLOAT  : pdf = new FEDataField_T<Post::FEElementData<float  ,DATA_REGION> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<Post::FEElementData<vec3f  ,DATA_REGION> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<Post::FEElementData<mat3fs ,DATA_REGION> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<Post::FEElementData<mat3fd ,DATA_REGION> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<Post::FEElementData<tens4fs,DATA_REGION> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<Post::FEElementData<mat3f  ,DATA_REGION> >(&fem); break;
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
		pdf->SetName(it.szname);
		pdm->AddDataField(pdf);
	}

	// read face variables
	nv = (int)m_dic.m_Face.size();
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
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_NODE> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_NODE> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_NODE> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_NODE> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_NODE> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_NODE> >(&fem); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_ITEM:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_ITEM> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_ITEM> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_ITEM> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_ITEM> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_ITEM> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_ITEM> >(&fem); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_MULT:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_MULT> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_MULT> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_MULT> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_MULT> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_MULT> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_MULT> >(&fem); break;
				default:
					assert(false);
				}
			}
			break;
		case FMT_REGION:
			{
				switch (it.ntype)
				{
				case FLOAT  : pdf = new FEDataField_T<FEFaceData<float  ,DATA_REGION> >(&fem); break;
				case VEC3F  : pdf = new FEDataField_T<FEFaceData<vec3f  ,DATA_REGION> >(&fem); break;
				case MAT3FS : pdf = new FEDataField_T<FEFaceData<mat3fs ,DATA_REGION> >(&fem); break;
				case MAT3FD : pdf = new FEDataField_T<FEFaceData<mat3fd ,DATA_REGION> >(&fem); break;
                case TENS4FS: pdf = new FEDataField_T<FEFaceData<tens4fs,DATA_REGION> >(&fem); break;
				case MAT3F  : pdf = new FEDataField_T<FEFaceData<mat3f  ,DATA_REGION> >(&fem); break;
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
		fem.AddDataField(pdf, it.szname);
	}

	// add additional displacement fields
	if (m_bHasDispl) 
	{
		pdm->AddDataField(new StrainDataField(&fem, StrainDataField::LAGRANGE), "Lagrange strain");
		pdm->AddDataField(new FEDataField_T<NodePosition  >(&fem, IMPLICIT_DATA), "position");
		pdm->AddDataField(new FEDataField_T<NodeInitPos   >(&fem, IMPLICIT_DATA), "initial position" );
	}

	// add additional stress fields
	if (m_bHasStress)
	{
		pdm->AddDataField(new FEDataField_T<ElemPressure>(&fem, IMPLICIT_DATA), "pressure");

		if (m_bHasFluidPressure) {
			// make sure the "solid stress" field was not added to the plot file
			if (pdm->FindDataField("solid stress") == -1)
				pdm->AddDataField(new FEDataField_T<SolidStress>(&fem, IMPLICIT_DATA), "solid stress");
		}
	}

	// add additional stress fields
	if (m_bHasNodalStress)
	{
		pdm->AddDataField(new FEDataField_T<ElemNodalPressure>(&fem, IMPLICIT_DATA), "nodal pressure");
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadGlobalDicItems()
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
bool XpltReader::ReadMaterialDicItems()
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
bool XpltReader::ReadNodeDicItems()
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
bool XpltReader::ReadElemDicItems()
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

//-----------------------------------------------------------------------------
bool XpltReader::ReadFaceDicItems()
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

//-----------------------------------------------------------------------------
bool XpltReader::ReadMaterials(FEPostModel& fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		if (nid == PLT_MATERIAL)
		{
			MATERIAL m;
			char sz[DI_NAME_SIZE] = {0};
			while (m_ar.OpenChunk() == xpltArchive::IO_OK)
			{
				switch (m_ar.GetChunkID())
				{
				case PLT_MAT_ID  : m_ar.read(m.nid); break;
				case PLT_MAT_NAME: m_ar.read(sz, DI_NAME_SIZE); break;
				}
				m_ar.CloseChunk();
			}
			strcpy(m.szname, sz);
			m_Mat.push_back(m);
		}
		else
		{
			assert(false);
			return errf("Error while reading materials");
		}
		m_ar.CloseChunk();
	}
	CreateMaterials(fem);
	return true;
}

//-----------------------------------------------------------------------------
void XpltReader::CreateMaterials(FEPostModel& fem)
{
	// initialize material properties
	fem.ClearMaterials();
	int nmat = (int)m_Mat.size();
	for (int i=0; i<nmat; i++)
	{
		Material m;
		m.SetName(m_Mat[i].szname);
		fem.AddMaterial(m);
	}
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadMesh(FEPostModel &fem)
{
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		switch (m_ar.GetChunkID())
		{
		case PLT_NODE_SECTION   : if (ReadNodeSection   (fem) == false) return false; break;
		case PLT_DOMAIN_SECTION : if (ReadDomainSection (fem) == false) return false; break;
		case PLT_SURFACE_SECTION: if (ReadSurfaceSection(fem) == false) return false; break;
		case PLT_NODESET_SECTION: if (ReadNodeSetSection(fem) == false) return false; break;
		default:
			assert(false);
			return errf("Error while reading mesh");
		}
		m_ar.CloseChunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadNodeSection(FEPostModel &fem)
{
	const xpltFileReader::HEADER& hdr = m_xplt->GetHeader();
	vector<float> a(3 * hdr.nn);
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		if (m_ar.GetChunkID() == PLT_NODE_COORDS) m_ar.read(a);
		else
		{
			assert(false);
			return errf("Error while reading Node section");
		}
		m_ar.CloseChunk();
	}

	m_Node.resize(hdr.nn);
	for (int i=0; i<hdr.nn; ++i)
	{
		NODE& n = m_Node[i];
		n.r.x = a[3*i  ];
		n.r.y = a[3*i+1];
		n.r.z = a[3*i+2];
	}

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadDomainSection(FEPostModel &fem)
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
						case PLT_DOM_MAT_ID   : m_ar.read(D.mid); break;
						case PLT_DOM_ELEMS    : m_ar.read(D.ne); break;
						case PLT_DOM_NAME     : m_ar.read(D.szname); break;
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
							for (int i=0; i<ne; ++i) { e.node[i] = n[i+1]; assert(e.node[i] < (int)m_Node.size()); }
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
			m_Dom.push_back(D);
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
bool XpltReader::ReadSurfaceSection(FEPostModel &fem)
{
	const xpltFileReader::HEADER& hdr = m_xplt->GetHeader();
	int nodes_per_facet = hdr.nmax_facet_nodes;

	// in previous versions there was a bug in the number
	// of nodes written so we need to make an adjustment.
	if (hdr.nversion < 0x04) nodes_per_facet -= 2;

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
						case PLT_SURFACE_ID   : m_ar.read(S.sid); break;
						case PLT_SURFACE_FACES: m_ar.read(S.nf); break;
						case PLT_SURFACE_NAME : m_ar.read(S.szname, 64); break;
						default:
							assert(false);
							return errf("Error while reading Surface section");
						}
						m_ar.CloseChunk();
					}
				}
				else if (nid == PLT_FACE_LIST)
				{
					if (hdr.nversion == 1)
					{
						assert(S.nf > 0);
						S.face.reserve(S.nf);
						int n[5];
						while (m_ar.OpenChunk() == xpltArchive::IO_OK)
						{
							if (m_ar.GetChunkID() == PLT_FACE)
							{
								m_ar.read(n, 5);
								FACE f;
								f.nid = n[0];
								f.node[0] = n[1];
								f.node[1] = n[2];
								f.node[2] = n[3];
								f.node[3] = n[4];
								f.nn = (f.node[3] == f.node[2] ? 3 : 4);
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
					else
					{
						assert(S.nf > 0);
						S.face.reserve(S.nf);
						int n[12];
						assert(hdr.nmax_facet_nodes <= 10);
						while (m_ar.OpenChunk() == xpltArchive::IO_OK)
						{
							if (m_ar.GetChunkID() == PLT_FACE)
							{
								m_ar.read(n, nodes_per_facet+2);
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
				}
				m_ar.CloseChunk();
			}
			assert(S.nf == S.face.size());
			m_Surf.push_back(S);
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
bool XpltReader::ReadNodeSetSection(FEPostModel& fem)
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
						case PLT_NODESET_NAME : m_ar.read(S.szname); break;
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
			m_NodeSet.push_back(S);
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
bool XpltReader::BuildMesh(FEPostModel &fem)
{
	// clear the state data
	fem.ClearStates();
	fem.DeleteMeshes();

	// count all nodes
	const xpltFileReader::HEADER& hdr = m_xplt->GetHeader();
	int NN = hdr.nn;

	// count all elements
	int ND = (int)m_Dom.size();
	int NE = 0;
	for (int i=0; i<ND; ++i) NE += m_Dom[i].ne;

	// find the element type
	int ntype = m_Dom[0].etype;
	bool blinear = true;	// all linear elements flag
	for (int i=0; i<ND; ++i)
	{
		int domType = m_Dom[i].etype;
		if (domType != ntype) ntype = -1;
		if ((domType != PLT_ELEM_TRUSS) && 
			(domType != PLT_ELEM_TRI) && 
			(domType != PLT_ELEM_QUAD) && 
			(domType != PLT_ELEM_TET4) && 
			(domType != PLT_ELEM_PENTA) && 
			(domType != PLT_ELEM_HEX8) &&
			(domType != PLT_ELEM_PYRA5)) blinear = false;
	}

	FSMesh* pmesh = new FSMesh;
	pmesh->Create(NN, NE);

	// read the element connectivity
	int nmat = fem.Materials();
	for (int i=0; i<ND; i++)
	{
		Domain& D = m_Dom[i];
		for (int j=0; j<D.ne; ++j)
		{
			ELEM& E = D.elem[j];
			FSElement& el = static_cast<FSElement&>(pmesh->ElementRef(E.index));
			el.m_MatID = D.mid - 1;
			el.m_gid = i;
			el.SetID(E.eid);

			FSElementType etype;
			switch (D.etype)
			{
			case PLT_ELEM_HEX8   : etype = FE_HEX8  ; break;
			case PLT_ELEM_PENTA  : etype = FE_PENTA6; break;
            case PLT_ELEM_PENTA15: etype = FE_PENTA15; break;
            case PLT_ELEM_TET4   : etype = FE_TET4  ; break;
			case PLT_ELEM_TET5   : etype = FE_TET5; break;
			case PLT_ELEM_QUAD   : etype = FE_QUAD4 ; break;
			case PLT_ELEM_TRI    : etype = FE_TRI3  ; break;
			case PLT_ELEM_TRUSS  : etype = FE_BEAM2 ; break;
			case PLT_ELEM_HEX20  : etype = FE_HEX20 ; break;
			case PLT_ELEM_HEX27  : etype = FE_HEX27 ; break;
			case PLT_ELEM_TET10  : etype = FE_TET10 ; break;
			case PLT_ELEM_TET15  : etype = FE_TET15 ; break;
			case PLT_ELEM_TET20  : etype = FE_TET20 ; break;
			case PLT_ELEM_TRI6   : etype = FE_TRI6  ; break;
			case PLT_ELEM_QUAD8  : etype = FE_QUAD8 ; break;
			case PLT_ELEM_QUAD9  : etype = FE_QUAD9 ; break;
			case PLT_ELEM_PYRA5  : etype = FE_PYRA5 ; break;
            case PLT_ELEM_PYRA13 : etype = FE_PYRA13; break;
			}
			el.SetType(etype);
			int ne = el.Nodes();
			for (int k=0; k<ne; ++k) el.m_node[k] = E.node[k];
		}
	}

	// read the nodal coordinates
	for (int i=0; i<hdr.nn; i++)
	{
		FSNode& n = pmesh->Node(i);
		NODE& N = m_Node[i];

		// assign coordinates
		n.r = to_vec3d(N.r);
	}

	fem.AddMesh(pmesh);

	// set the enabled-ness of the elements and the nodes
	for (int i=0; i<NE; ++i)
	{
		FSElement_& el = pmesh->ElementRef(i);
		Material* pm = fem.GetMaterial(el.m_MatID);
		if (pm->benable) el.Enable(); else el.Disable();
	}

	for (int i=0; i<NN; ++i) pmesh->Node(i).Disable();
	for (int i=0; i<NE; ++i)
	{
		FSElement_& el = pmesh->ElementRef(i);
		if (el.IsEnabled())
		{
			int n = el.Nodes();
			for (int j=0; j<n; ++j) pmesh->Node(el.m_node[j]).Enable();
		}
	}

	// Update the mesh
	// This will also build the faces
	pmesh->RebuildMesh();

	// Next, we'll build a Node-Face lookup table
	FSNodeFaceList NFT; NFT.Build(pmesh);

	// next, we reindex the surfaces
	for (int n=0; n<(int) m_Surf.size(); ++n)
	{
		Surface& s = m_Surf[n];
		for (int i=0; i<s.nf; ++i)
		{
			FACE& f = s.face[i];
			f.nid = NFT.FindFace(f.node[0], f.node, f.nn);
//			assert(f.nid >= 0);
		}
	}

	// let's create the nodesets
	char szname[128]={0};
	for (int n=0; n<(int)m_NodeSet.size(); ++n)
	{
		NodeSet& s = m_NodeSet[n];
		FSNodeSet* ps = new FSNodeSet(pmesh, s.node);
		if (s.szname[0]==0) { sprintf(szname, "nodeset%02d",n+1); ps->SetName(szname); }
		else ps->SetName(s.szname);
		pmesh->AddFENodeSet(ps);
	}

	// let's create the FE surfaces
	for (int n=0; n<(int) m_Surf.size(); ++n)
	{
		Surface& s = m_Surf[n];
		FSSurface* ps = new FSSurface(pmesh);
		if (s.szname[0]==0) { sprintf(szname, "surface%02d",n+1); ps->SetName(szname); }
		else ps->SetName(s.szname);
		ps->reserve(s.nf);
		for (int i=0; i<s.nf; ++i) ps->add(s.face[i].nid);
		pmesh->AddFESurface(ps);
	}

	// let's create the parts
	for (int n=0; n<(int) m_Dom.size(); ++n)
	{
		Domain& s = m_Dom[n];
		FSElemSet* pg = new FSElemSet(pmesh, s.elist);
		if (s.szname[0]==0) { sprintf(szname, "part%02d",n+1); pg->SetName(szname); }
		else pg->SetName(s.szname);
		pmesh->AddFEElemSet(pg);
	}

	fem.UpdateBoundingBox();

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadStateSection(FEPostModel& fem)
{
	// get the mesh
	FSMesh& mesh = *fem.GetFEMesh(0);

	// add a state
	FEState* ps = 0;
	
	try 
	{
		ps = m_pstate = new FEState(0.f, &fem, &mesh);

		// we give it a temporary ID
		ps->SetID(fem.GetStates());
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
				if (m_ar.GetChunkID() == PLT_STATE_HDR_TIME) m_ar.read(ps->m_time);
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
				case PLT_MATERIAL_DATA: if (ReadMaterialData(fem, ps) == false) return false; break;
				case PLT_NODE_DATA    : if (ReadNodeData    (fem, ps) == false) return false; break;
				case PLT_ELEMENT_DATA : if (ReadElemData    (fem, ps) == false) return false; break;
				case PLT_FACE_DATA    : if (ReadFaceData    (fem, ps) == false) return false; break;
				default:
					assert(false);
					return errf("Invalid chunk ID");
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
bool XpltReader::ReadGlobalData(FEPostModel& fem, FEState* pstate)
{
	return false;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadMaterialData(FEPostModel& fem, FEState* pstate)
{
	return false;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadNodeData(FEPostModel& fem, FEState* pstate)
{
	FSMesh& mesh = *fem.GetFEMesh(0);
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
					assert((nv>=0)&&(nv<(int)m_dic.m_Node.size()));
					if ((nv<0) || (nv >= (int)m_dic.m_Node.size())) return errf("Failed reading node data");

					DICT_ITEM it = m_dic.m_Node[nv];
					int nfield = it.index;
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
bool XpltReader::ReadElemData(FEPostModel &fem, FEState* pstate)
{
	FSMesh& mesh = *fem.GetFEMesh(0);
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
						assert((nd >= 0)&&(nd < (int)m_Dom.size()));
						if ((nd < 0) || (nd >= (int) m_Dom.size())) return errf("Failed reading all state data");

						int nfield = it.index;

						Domain& dom = m_Dom[nd];
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
bool XpltReader::ReadElemData_NODE(FSMesh& m, XpltReader::Domain &d, Post::FEMeshData&data, int ntype, int arrSize)
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
bool XpltReader::ReadElemData_ITEM(XpltReader::Domain& dom, Post::FEMeshData& s, int ntype, int arrSize)
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
			m_ar.read(a);
			FEElemArrayDataItem& dm = dynamic_cast<FEElemArrayDataItem&>(s);

			vector<int> elem(NE);
			for (int i=0; i<NE; ++i) elem[i] = dom.elem[i].index;

			dm.setData(a, elem);
		}
		break;
	case ARRAY_VEC3F:
		{
			vector<float> a(NE*arrSize*3);
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
bool XpltReader::ReadElemData_MULT(XpltReader::Domain& dom, Post::FEMeshData& s, int ntype)
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

//-----------------------------------------------------------------------------
bool XpltReader::ReadFaceData(FEPostModel& fem, FEState* pstate)
{
	FSMesh& mesh = *fem.GetFEMesh(0);
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
					DICT_ITEM it = m_dic.m_Face[nv];
					while (m_ar.OpenChunk() == xpltArchive::IO_OK)
					{
						int ns = m_ar.GetChunkID() - 1;
						assert((ns >= 0)&&(ns < (int)m_Surf.size()));
						if ((ns < 0) || (ns >= (int)m_Surf.size())) return errf("Failed reading all state data");

						int nfield = it.index;

						Surface& s = m_Surf[ns];
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

//-----------------------------------------------------------------------------
bool XpltReader::ReadFaceData_MULT(FSMesh& m, XpltReader::Surface &s, Post::FEMeshData&data, int ntype)
{
	// It is possible that the node ordering of the FACE's are different than the FSFace's
	// so we setup up an array to unscramble the nodal values
	int NF = s.nf;
	vector<int> tag;
	tag.assign(m.Nodes(), -1);
	const xpltFileReader::HEADER& hdr = m_xplt->GetHeader();
	const int NFM = hdr.nmax_facet_nodes;
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
bool XpltReader::ReadFaceData_ITEM(XpltReader::Surface &s, Post::FEMeshData&data, int ntype)
{
	int NF = s.nf;
	bool bok = true;
	switch (ntype)
	{
	case FLOAT:
		{
			FEFaceData<float,DATA_ITEM>& df = dynamic_cast<FEFaceData<float,DATA_ITEM>&>(data);
			vector<float> a(NF);
			m_ar.read(a);
			for (int i=0; i<NF; ++i) bok &= df.add(s.face[i].nid, a[i]);
		}
		break;
	case VEC3F:
		{
			vector<vec3f> a(NF);
			m_ar.read(a);
			FEFaceData<vec3f,DATA_ITEM>& dv = dynamic_cast<FEFaceData<vec3f,DATA_ITEM>&>(data);
			for (int i = 0; i<NF; ++i) bok &= dv.add(s.face[i].nid, a[i]);
		}
		break;
	case MAT3FS:
		{
			vector<mat3fs> a(NF);
			m_ar.read(a);
			FEFaceData<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEFaceData<mat3fs,DATA_ITEM>&>(data);
			for (int i = 0; i<NF; ++i) bok &= dm.add(s.face[i].nid, a[i]);
		}
		break;
	case MAT3F:
		{
			vector<mat3f> a(NF);
			m_ar.read(a);
			FEFaceData<mat3f,DATA_ITEM>& dm = dynamic_cast<FEFaceData<mat3f,DATA_ITEM>&>(data);
			for (int i = 0; i<NF; ++i) bok &= dm.add(s.face[i].nid, a[i]);
		}
		break;
	case MAT3FD:
		{
			vector<mat3fd> a(NF);
			m_ar.read(a);
			FEFaceData<mat3fd,DATA_ITEM>& dm = dynamic_cast<FEFaceData<mat3fd,DATA_ITEM>&>(data);
			for (int i = 0; i<NF; ++i) bok &= dm.add(s.face[i].nid, a[i]);
		}
		break;
    case TENS4FS:
		{
			vector<tens4fs> a(NF);
			m_ar.read(a);
			FEFaceData<tens4fs,DATA_ITEM>& dm = dynamic_cast<FEFaceData<tens4fs,DATA_ITEM>&>(data);
			for (int i = 0; i<NF; ++i) bok &= dm.add(s.face[i].nid, a[i]);
		}
        break;
	default:
		return errf("Failed reading face data");
	}

	if (bok == false) addWarning(XPLT_READ_DUPLICATE_FACES);

	return true;
}

//-----------------------------------------------------------------------------
bool XpltReader::ReadFaceData_NODE(FSMesh& m, XpltReader::Surface &s, Post::FEMeshData&data, int ntype)
{
	// set nodal tags to local node number
	int NN = m.Nodes();
	for (int i=0; i<NN; ++i) m.Node(i).m_ntag = -1;

	int n = 0;
	for (int i=0; i<s.nf; ++i)
	{
		FACE& f = s.face[i];
		int nf = f.nn;
		for (int j=0; j<nf; ++j)
			if (m.Node(f.node[j]).m_ntag == -1) m.Node(f.node[j]).m_ntag = n++;
	}

	// create the face list
	vector<int> f(s.nf);
	for (int i=0; i<s.nf; ++i) f[i] = s.face[i].nid;

	// create vector that stores the number of nodes for each facet
	vector<int> fn(s.nf, 0);
	for (int i=0; i<s.nf; ++i) fn[i] = s.face[i].nn;

	// create the local node index list
	vector<int> l; l.reserve(s.nf*FSFace::MAX_NODES);
	for (int i=0; i<s.nf; ++i)
	{
		FSFace& f = m.Face(s.face[i].nid);
		int nn = f.Nodes();
		for (int j=0; j<nn; ++j) 
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
			FEFaceData<float,DATA_NODE>& df = dynamic_cast<FEFaceData<float,DATA_NODE>&>(data);
			vector<float> a(n);
			m_ar.read(a);
			df.add(a, f, l, fn);
		}
		break;
	case VEC3F:
		{
			FEFaceData<vec3f,DATA_NODE>& df = dynamic_cast<FEFaceData<vec3f,DATA_NODE>&>(data);
			vector<vec3f> a(n);
			m_ar.read(a);
			df.add(a, f, l, fn);
		}
		break;
	case MAT3FS:
		{
			FEFaceData<mat3fs,DATA_NODE>& df = dynamic_cast<FEFaceData<mat3fs,DATA_NODE>&>(data);
			vector<mat3fs> a(n);
			m_ar.read(a);
			df.add(a, f, l, fn);
		}
		break;
	case MAT3F:
		{
			FEFaceData<mat3f,DATA_NODE>& df = dynamic_cast<FEFaceData<mat3f,DATA_NODE>&>(data);
			vector<mat3f> a(n);
			m_ar.read(a);
			df.add(a, f, l, fn);
		}
		break;
	case MAT3FD:
		{
			FEFaceData<mat3fd,DATA_NODE>& df = dynamic_cast<FEFaceData<mat3fd,DATA_NODE>&>(data);
			vector<mat3fd> a(n);
			m_ar.read(a);
			df.add(a, f, l, fn);
		}
		break;
    case TENS4FS:
		{
			FEFaceData<tens4fs,DATA_NODE>& df = dynamic_cast<FEFaceData<tens4fs,DATA_NODE>&>(data);
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
