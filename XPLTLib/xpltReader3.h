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

#pragma once
#include "xpltFileReader.h"
#include <MeshLib/FEElement.h>

class FSMesh;

namespace Post {
	class FEState;
	class FEMeshData;
}

//-----------------------------------------------------------------------------
// This class reads the XPLT file, version 3.2, 3.3, 3.4
// 3.3: node IDs are stored in Node section
// 3.4: Added support for beam elements
class XpltReader3 : public xpltParser
{
public:
	// file tags
	enum { 
		PLT_ROOT						= 0x01000000,
		PLT_HEADER						= 0x01010000,
			PLT_HDR_VERSION				= 0x01010001,
//			PLT_HDR_NODES				= 0x01010002,
//			PLT_HDR_MAX_FACET_NODES		= 0x01010003,	// removed (redefined in seach SURFACE section)
			PLT_HDR_COMPRESSION			= 0x01010004,
			PLT_HDR_AUTHOR				= 0x01010005,	// new in 2.0
			PLT_HDR_SOFTWARE			= 0x01010006,	// new in 2.0
			PLT_HDR_UNITS				= 0x01010007,	// new in 4.0
		PLT_DICTIONARY					= 0x01020000,
			PLT_DIC_ITEM				= 0x01020001,
			PLT_DIC_ITEM_TYPE			= 0x01020002,
			PLT_DIC_ITEM_FMT			= 0x01020003,
			PLT_DIC_ITEM_NAME			= 0x01020004,
			PLT_DIC_ITEM_ARRAYSIZE		= 0x01020005,	// added in version 0x05
			PLT_DIC_ITEM_ARRAYNAME		= 0x01020006,	// added in version 0x05
			PLT_DIC_ITEM_UNITS			= 0x01020007,	// added in version 4.0
			PLT_DIC_GLOBAL				= 0x01021000,
//			PLT_DIC_MATERIAL			= 0x01022000,	// this was removed
			PLT_DIC_NODAL				= 0x01023000,
			PLT_DIC_DOMAIN				= 0x01024000,
			PLT_DIC_SURFACE				= 0x01025000,
//		PLT_MATERIALS					= 0x01030000,		// This was removed
//			PLT_MATERIAL				= 0x01030001,
//			PLT_MAT_ID					= 0x01030002,
//			PLT_MAT_NAME				= 0x01030003,
		PLT_MESH						= 0x01040000,		// this was PLT_GEOMETRY
			PLT_NODE_SECTION			= 0x01041000,
				PLT_NODE_HEADER			= 0x01041100,		// new in 2.0
					PLT_NODE_SIZE		= 0x01041101,		// new in 2.0
					PLT_NODE_DIM		= 0x01041102,		// new in 2.0
					PLT_NODE_NAME		= 0x01041103,		// new in 2.0
				PLT_NODE_COORDS			= 0x01041200,		// new in 2.0
			PLT_DOMAIN_SECTION			= 0x01042000,
				PLT_DOMAIN				= 0x01042100,
				PLT_DOMAIN_HDR			= 0x01042101,
					PLT_DOM_ELEM_TYPE	= 0x01042102,
					PLT_DOM_PART_ID		= 0x01042103,		// this was PLT_DOM_MAT_ID
					PLT_DOM_ELEMS		= 0x01032104,
					PLT_DOM_NAME		= 0x01032105,
				PLT_DOM_ELEM_LIST		= 0x01042200,
					PLT_ELEMENT			= 0x01042201,
			PLT_SURFACE_SECTION			= 0x01043000,
				PLT_SURFACE				= 0x01043100,
				PLT_SURFACE_HDR			= 0x01043101,
					PLT_SURFACE_ID		= 0x01043102,
					PLT_SURFACE_FACES	= 0x01043103,
					PLT_SURFACE_NAME	= 0x01043104,
					PLT_SURFACE_MAX_FACET_NODES = 0x01043105,	// new in 2.0 (max number of nodes per facet)
				PLT_FACE_LIST			= 0x01043200,
					PLT_FACE			= 0x01043201,
			PLT_NODESET_SECTION			= 0x01044000,
				PLT_NODESET				= 0x01044100,
				PLT_NODESET_HDR			= 0x01044101,
					PLT_NODESET_ID		= 0x01044102,
					PLT_NODESET_NAME	= 0x01044103,
					PLT_NODESET_SIZE	= 0x01044104,
				PLT_NODESET_LIST		= 0x01044200,
			PLT_PARTS_SECTION			= 0x01045000,		// new in 2.0
				PLT_PART				= 0x01045100,
				PLT_PART_ID				= 0x01045101,
				PLT_PART_NAME			= 0x01045102,

			// element set section was added in 4.1
			PLT_ELEMENTSET_SECTION		= 0x01046000,
				PLT_ELEMENTSET			= 0x01046100,
				PLT_ELEMENTSET_HDR		= 0x01046101,
					PLT_ELEMENTSET_ID	= 0x01046102,
					PLT_ELEMENTSET_NAME	= 0x01046103,
					PLT_ELEMENTSET_SIZE	= 0x01046104,
				PLT_ELEMENTSET_LIST		= 0x01046200,

			// facet set section was added in 4.1
			PLT_FACETSET_SECTION			= 0x01047000,
				PLT_FACETSET				= 0x01047100,
				PLT_FACETSET_HDR			= 0x01047101,
					PLT_FACETSET_ID			= 0x01047102,
					PLT_FACETSET_NAME		= 0x01047103,
					PLT_FACETSET_SIZE		= 0x01047104,
					PLT_FACETSET_MAXNODES	= 0x01047105,
				PLT_FACETSET_LIST			= 0x01047200,
					PLT_FACET				= 0x01047201,

			PLT_OBJECTS_SECTION			= 0x01050000,
					PLT_OBJECT_ID		= 0x01050001,
					PLT_OBJECT_NAME		= 0x01050002,
					PLT_OBJECT_TAG		= 0x01050003,
					PLT_OBJECT_POS		= 0x01050004,
					PLT_OBJECT_ROT		= 0x01050005,
					PLT_OBJECT_DATA		= 0x01050006,
				PLT_POINT_OBJECT		= 0x01051000,
					PLT_POINT_COORD		= 0x01051001,
				PLT_LINE_OBJECT			= 0x01052000,
					PLT_LINE_COORDS		= 0x01052001,


		PLT_STATE						= 0x02000000,
			PLT_STATE_HEADER			= 0x02010000,
				PLT_STATE_HDR_ID		= 0x02010001,
				PLT_STATE_HDR_TIME		= 0x02010002,
				PLT_STATE_STATUS        = 0x02010003,	// new in 3.1
			PLT_STATE_DATA				= 0x02020000,
				PLT_STATE_VARIABLE		= 0x02020001,
				PLT_STATE_VAR_ID		= 0x02020002,
				PLT_STATE_VAR_DATA		= 0x02020003,
				PLT_GLOBAL_DATA			= 0x02020100,
//				PLT_MATERIAL_DATA		= 0x02020200,		// this was removed
				PLT_NODE_DATA			= 0x02020300,
				PLT_ELEMENT_DATA		= 0x02020400,
				PLT_FACE_DATA			= 0x02020500,
			PLT_MESH_STATE				= 0x02030000,
				PLT_ELEMENT_STATE		= 0x02030001,
			PLT_OBJECTS_STATE			= 0x02040000
	};

public:
	// FEBio tag
	enum {FEBIO_TAG = 0x00464542 };

	// variable types
	enum Var_Type { FLOAT, VEC3F, MAT3FS, MAT3FD, TENS4FS, MAT3F, ARRAY, ARRAY_VEC3F };

	// variable format
	enum Var_Fmt { FMT_NODE, FMT_ITEM, FMT_MULT, FMT_REGION };

	// element types
	enum Elem_Type { 
		PLT_ELEM_HEX8, 
		PLT_ELEM_PENTA, 
		PLT_ELEM_TET4, 
		PLT_ELEM_QUAD, 
		PLT_ELEM_TRI, 
		PLT_ELEM_TRUSS, 
		PLT_ELEM_HEX20, 
		PLT_ELEM_TET10, 
		PLT_ELEM_TET15, 
		PLT_ELEM_HEX27, 
		PLT_ELEM_TRI6, 
		PLT_ELEM_QUAD8, 
		PLT_ELEM_QUAD9, 
        PLT_ELEM_PENTA15,
        PLT_ELEM_TET20,
		PLT_ELEM_TRI10,
		PLT_ELEM_PYRA5,
		PLT_ELEM_TET5,
        PLT_ELEM_PYRA13,
		PLT_ELEM_LINE3			// new in 3.4
	};

	// size of name variables
	enum { DI_NAME_SIZE = 64 };

public:
	class DICT_ITEM
	{
	public:
		DICT_ITEM();
		DICT_ITEM(const DICT_ITEM& item);

	public:
		unsigned int	ntype;
		unsigned int	nfmt;
		char			szname[DI_NAME_SIZE];
		char			szunit[DI_NAME_SIZE];

		unsigned int	index;	// index into data manager list

		unsigned int	arraySize;	// only used for array variables (plt version 0x05)
		std::vector<string>	arrayNames;	// (optional) names of array components
	};

	class Dictionary
	{
	public:
		std::vector<DICT_ITEM>	m_Glb;
		std::vector<DICT_ITEM>	m_Mat;
		std::vector<DICT_ITEM>	m_Node;
		std::vector<DICT_ITEM>	m_Elem;
		std::vector<DICT_ITEM>	m_Face;

	public:
		void Clear()
		{
			m_Glb.clear();
			m_Mat.clear();
			m_Node.clear();
			m_Elem.clear();
			m_Face.clear();
		}
	};

	struct MATERIAL
	{
		int		nid;
		char	szname[DI_NAME_SIZE];
	};

	struct NODE
	{
		int		id;
		float	x[3];
	};

	struct ELEM
	{
		int		eid;
		int		index;
		int		node[FSElement::MAX_NODES];
	};

	struct FACE
	{
		int		nid;
		int		nn;
		int		node[9];
	};

	class Domain
	{
	public:
		int		etype;
		int		mid;
		int		ne;
		int		nid;	// domain ID
		char	szname[DI_NAME_SIZE];
		std::vector<int>		elist;
		std::vector<ELEM>	elem;

	public:
		Domain() { ne = 0; szname[0] = 0; }
		Domain(const Domain& d) { nid = d.nid; etype = d.etype; mid = d.mid; ne = d.ne; elem = d.elem; elist = d.elist; strncpy(szname, d.szname, 64); }
		void operator = (const Domain& d) { nid = d.nid; etype = d.etype; mid = d.mid; ne = d.ne; elem = d.elem; elist = d.elist; strncpy(szname, d.szname, 64);  }
	};

	class Surface
	{
	public:
		int				sid;
		int				nfaces;		// number of faces
		int				maxNodes;	// max nr of nodes
		std::vector<FACE>	face;
		char			szname[DI_NAME_SIZE];

	public:
		Surface() { nfaces = 0; maxNodes = 0; szname[0] = 0; }
		Surface(const Surface& s) { nfaces = s.nfaces; maxNodes = s.maxNodes; face = s.face; strncpy(szname, s.szname, 64); }
		void operator = (const Surface& s) { nfaces = s.nfaces; maxNodes = s.maxNodes; face = s.face; }
	};

	class NodeSet
	{
	public:
		int		nid;
		int		nn;
		char	szname[DI_NAME_SIZE];
		std::vector<int>	node;

	public:
		NodeSet() { nn = 0; szname[0] = 0; }
		NodeSet(const NodeSet& s) { nn = s.nn; node = s.node; strncpy(szname, s.szname, 64); }
	};

	class ElemSet
	{
	public:
		int		nid;
		int		ne;
		char	szname[DI_NAME_SIZE];
		std::vector<int>	elem;

	public:
		ElemSet() { nid = -1; ne = 0; szname[0] = 0; }
		ElemSet(const ElemSet& s) 
		{ 
			nid = s.nid; 
			ne = s.ne; 
			elem = s.elem; 
			strncpy(szname, s.szname, DI_NAME_SIZE);
		}
	};

	class XMesh
	{
	public:
		std::vector<MATERIAL>	m_Mat;
		std::vector<NODE>		m_Node;
		std::vector<Domain>		m_Dom;
		std::vector<Surface>	m_Surf;
		std::vector<NodeSet>	m_NodeSet;
		std::vector<ElemSet>	m_ElemSet;
		std::vector<Surface>	m_FacetSet;

	public:
		void Clear();

		int materials() const { return (int)m_Mat.size(); }
		void addMaterial(MATERIAL& mat);
		MATERIAL& material(int i) { return m_Mat[i]; }

		int nodes() const { return (int)m_Node.size(); }
		NODE& node(int i) { return m_Node[i]; }
		void addNodes(std::vector<NODE>& nodes);

		int domains() const { return (int)m_Dom.size(); }
		void addDomain(Domain& dom);
		Domain& domain(int i) { return m_Dom[i]; }

		int surfaces() const { return (int)m_Surf.size(); }
		void addSurface(Surface& surf);
		Surface& surface(int i) { return m_Surf[i]; }

		int nodeSets() const { return (int)m_NodeSet.size(); }
		void addNodeSet(NodeSet& nset);
		NodeSet& nodeSet(int i) { return m_NodeSet[i]; }

		int elementSets() const { return (int)m_ElemSet.size(); }
		void addElementSet(ElemSet& eset);
		ElemSet& elementSet(int i) { return m_ElemSet[i]; }

		int facetSets() const { return (int)m_FacetSet.size(); }
		void addFacetSet(Surface& fset);
		Surface& facetSet(int i) { return m_FacetSet[i]; }
	};

public:
	XpltReader3(xpltFileReader* xplt);
	~XpltReader3();

	bool Load(Post::FEPostModel& fem);

protected:
	bool ReadRootSection(Post::FEPostModel& fem);
	bool ReadStateSection(Post::FEPostModel& fem);

	bool ReadDictionary(Post::FEPostModel& fem);
	bool ReadMesh(Post::FEPostModel& fem);

	bool ReadDictItem(DICT_ITEM& it);

	void CreateMaterials(Post::FEPostModel& fem);

	bool BuildMesh(Post::FEPostModel& fem);

protected:
	bool ReadGlobalDicItems  ();
	bool ReadMaterialDicItems();
	bool ReadNodeDicItems    ();
	bool ReadElemDicItems    ();
	bool ReadFaceDicItems    ();

	bool ReadNodeSection      (Post::FEPostModel& fem);
	bool ReadDomainSection    (Post::FEPostModel& fem);
	bool ReadSurfaceSection   (Post::FEPostModel& fem);
	bool ReadNodeSetSection   (Post::FEPostModel& fem);
	bool ReadElementSetSection(Post::FEPostModel& fem);
	bool ReadFacetSetSection  (Post::FEPostModel& fem);
	bool ReadPartsSection     (Post::FEPostModel& fem);
	bool ReadObjectsSection   (Post::FEPostModel& fem);

	bool ReadGlobalData  (Post::FEPostModel& fem, Post::FEState* pstate);
	bool ReadMaterialData(Post::FEPostModel& fem, Post::FEState* pstate);
	bool ReadNodeData    (Post::FEPostModel& fem, Post::FEState* pstate);
	bool ReadElemData    (Post::FEPostModel& fem, Post::FEState* pstate);
	bool ReadFaceData    (Post::FEPostModel& fem, Post::FEState* pstate);

	bool ReadElemData_NODE(FSMesh& m, Domain& d, Post::FEMeshData& s, int ntype, int arrSize);
	bool ReadElemData_ITEM(Domain& d, Post::FEMeshData& s, int ntype, int arrSize);
	bool ReadElemData_MULT(Domain& d, Post::FEMeshData& s, int ntype);

	bool ReadFaceData_NODE(FSMesh& m, Surface& s, Post::FEMeshData& data, int ntype);
	bool ReadFaceData_ITEM(Surface& s, Post::FEMeshData& data, int ntype);
	bool ReadFaceData_MULT(FSMesh& m, Surface& s, Post::FEMeshData& data, int ntype);

	void Clear();

protected:
	FSMesh* GetCurrentMesh() { return m_mesh; }

protected:
	Dictionary			m_dic;
	XMesh				m_xmesh;

	bool	m_bHasDispl;			// has displacement field
	bool	m_bHasStress;			// has stress field
	bool	m_bHasNodalStress;		// has nodal stress field
	bool	m_bHasShellThickness;	// has shell thicknesses
	bool	m_bHasFluidPressure;	// has fluid pressure field
	bool	m_bHasElasticity;		// has elasticity field

	int		m_ngvsize;	// size of all global variables
	int		m_nnvsize;	// size of all nodal variables
	int		m_n3dsize;	// size of all solid variables
	int		m_n2dsize;	// size of all shell variables
	int		m_n1dsize;	// size of all beam variables

	int		m_nel;

	Post::FEState*	m_pstate;	//!< last read state section
	FSMesh*	m_mesh;		//!< current mesh
};
