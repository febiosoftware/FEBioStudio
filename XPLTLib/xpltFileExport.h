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
#include "xpltArchive.h"

class FSElemSet;
class FSSurface;
class FSMeshPartition;
class FSMesh;

namespace Post {
//-----------------------------------------------------------------------------
class FEPostModel;
class ModelDataField;
class FEState;
class FEMeshData;

//-----------------------------------------------------------------------------
// Class for writing FEBio XPLT files.
class xpltFileExport
{
protected:
	// file version
	enum { PLT_VERSION = 0x0004 };

	// max nodes per facet
	enum { PLT_MAX_FACET_NODES = 9 };

	// file tags
	enum { 
		PLT_ROOT						= 0x01000000,
		PLT_HEADER						= 0x01010000,
			PLT_HDR_VERSION				= 0x01010001,
			PLT_HDR_NODES				= 0x01010002,
			PLT_HDR_MAX_FACET_NODES		= 0x01010003,
			PLT_HDR_COMPRESSION			= 0x01010004,	
		PLT_DICTIONARY					= 0x01020000,
			PLT_DIC_ITEM				= 0x01020001,
			PLT_DIC_ITEM_TYPE			= 0x01020002,
			PLT_DIC_ITEM_FMT			= 0x01020003,
			PLT_DIC_ITEM_NAME			= 0x01020004,
			PLT_DIC_GLOBAL				= 0x01021000,
			PLT_DIC_MATERIAL			= 0x01022000,
			PLT_DIC_NODAL				= 0x01023000,
			PLT_DIC_DOMAIN				= 0x01024000,
			PLT_DIC_SURFACE				= 0x01025000,
		PLT_MATERIALS					= 0x01030000,
			PLT_MATERIAL				= 0x01030001,
			PLT_MAT_ID					= 0x01030002,
			PLT_MAT_NAME				= 0x01030003,
		PLT_GEOMETRY					= 0x01040000,
			PLT_NODE_SECTION			= 0x01041000,
				PLT_NODE_COORDS			= 0x01041001,
			PLT_DOMAIN_SECTION			= 0x01042000,
				PLT_DOMAIN				= 0x01042100,
				PLT_DOMAIN_HDR			= 0x01042101,
					PLT_DOM_ELEM_TYPE	= 0x01042102,
					PLT_DOM_MAT_ID		= 0x01042103,
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
				PLT_FACE_LIST			= 0x01043200,
					PLT_FACE			= 0x01043201,
			PLT_NODESET_SECTION			= 0x01044000,
				PLT_NODESET				= 0x01044100,
				PLT_NODESET_HDR			= 0x01044101,
					PLT_NODESET_ID		= 0x01044102,
					PLT_NODESET_NAME	= 0x01044103,
					PLT_NODESET_SIZE	= 0x01044104,
				PLT_NODESET_LIST		= 0x01044200,
		PLT_STATE						= 0x02000000,
			PLT_STATE_HEADER			= 0x02010000,
				PLT_STATE_HDR_ID		= 0x02010001,
				PLT_STATE_HDR_TIME		= 0x02010002,
			PLT_STATE_DATA				= 0x02020000,
				PLT_STATE_VARIABLE		= 0x02020001,
				PLT_STATE_VAR_ID		= 0x02020002,
				PLT_STATE_VAR_DATA		= 0x02020003,
				PLT_GLOBAL_DATA			= 0x02020100,
				PLT_MATERIAL_DATA		= 0x02020200,
				PLT_NODE_DATA			= 0x02020300,
				PLT_ELEMENT_DATA		= 0x02020400,
				PLT_FACE_DATA			= 0x02020500
	};

	// variable types
	enum Var_Type { FLOAT, VEC3F, MAT3FS, MAT3FD, TENS4FS, MAT3F };

	// variable format
	enum Var_Fmt { FMT_NODE, FMT_ITEM, FMT_MULT, FMT_REGION };

	// element types
	enum Elem_Type { 
		PLT_ELEM_HEX8, 
		PLT_ELEM_PENTA, 
		PLT_ELEM_TET, 
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
        PLT_ELEM_PYRA13
	};

	// size of name variables
	enum { STR_SIZE = 64 };

public:
	xpltFileExport();

	// Save the model to file. 
	bool Save(FEPostModel& fem, const char* szfile);

	// get the error message (if any)
	const char* GetErrorMessage() const { return m_szerr; }

	// set the compression flag
	void SetCompression(bool b) { m_ncompress = (b ? 1 : 0); }

protected:
	bool WriteRoot(FEPostModel& fem);
	bool WriteHeader    (FEPostModel& fem);
	bool WriteDictionary(FEPostModel& fem);
	bool WriteMaterials (FEPostModel& fem);
	bool WriteGeometry  (FEPostModel& fem);
	bool WriteDataField (ModelDataField& data);

	bool WriteNodeSection   (FEPostModel& fem);
	bool WritePartSection   (FSMesh& m);
	bool WriteSurfaceSection(FSMesh& m);

	bool WritePart(FSMesh& m, FSMeshPartition& part);

	bool WriteState(FEPostModel& fem, FEState& state);
	bool WriteNodeData(FEPostModel& fem, FEState& state);
	bool WriteElemData(FEPostModel& fem, FEState& state);
	bool WriteFaceData(FEPostModel& fem, FEState& state);

	bool FillNodeDataArray(std::vector<float>& val, FEMeshData& data);
	bool FillElemDataArray(std::vector<float>& val, FEMeshData& data, FSMeshPartition& part);
	bool FillFaceDataArray(std::vector<float>& val, FEMeshData& data, FSSurface& part);

	bool error(const char* sz);

private:
	xpltArchive	m_ar;
	int			m_nodeData;
	int			m_elemData;
	int			m_faceData;
	int			m_ncompress;

	char		m_szerr[256];
};
}
