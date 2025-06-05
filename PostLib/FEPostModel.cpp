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

// FEModel.cpp: implementation of the FEModel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEPostModel.h"
#include "FEDataManager.h"
#include "constants.h"
#include "FEMeshData_T.h"
#include <MeshLib/MeshTools.h>
#include <stdio.h>
using namespace std;

extern int ET_HEX[12][2];

namespace Post {

FEPostModel* FEPostModel::m_pThis = 0;

FEPostModel::PlotObject::PlotObject() 
{ 
	AddColorParam(GLColor::White(), "Color");
	AddDoubleParam(1, "Scale")->SetFloatRange(0, 10, 0.1);
	m_tag = 0; m_id = -1; 
}

GLColor	FEPostModel::PlotObject::Color()
{
	return GetColorValue(0);
}

void FEPostModel::PlotObject::SetColor(const GLColor& c)
{
	SetColorValue(0, c);
}

double FEPostModel::PlotObject::Scale() const
{
	return GetFloatValue(1);
}

//=============================================================================
//								F E P O S T M O D E L
//=============================================================================
// constructor
FEPostModel::FEPostModel()
{
	m_ndisp = -1;
	m_pDM = new FEDataManager(this);

	m_nTime = 0;
	m_fTime = 0.f;

	m_pThis = this;
}

//-----------------------------------------------------------------------------
// desctructor
FEPostModel::~FEPostModel()
{
	Clear();
	delete m_pDM;
	if (m_pThis == this) m_pThis = 0;

	DeleteMeshes();

	ClearDependants();
}

void FEPostModel::DeleteMeshes()
{
	// delete all meshes
	for (size_t i = 0; i<m_mesh.size(); ++i) delete m_mesh[i];
	m_mesh.clear();

	for (size_t i = 0; i < m_RefState.size(); ++i) delete m_RefState[i];
	m_RefState.clear();
}

void FEPostModel::SetInstance(FEPostModel* fem)
{
	m_pThis = fem;
}

FEPostModel* FEPostModel::GetInstance()
{
	return m_pThis;
}

//-----------------------------------------------------------------------------
FEState* FEPostModel::CurrentState()
{
	return m_State[m_nTime];
}

//-----------------------------------------------------------------------------
void FEPostModel::SetCurrentTimeIndex(int ntime)
{
	m_nTime = ntime;
	m_fTime = GetTimeValue(m_nTime);
}

//-----------------------------------------------------------------------------
void FEPostModel::SetTimeValue(float ftime)
{
	m_nTime = GetClosestTime(ftime);
	m_fTime = ftime;
}

//------------------------------------------------------------------------------------------
// This returns the time step whose time value is closest but less than t
//
int FEPostModel::GetClosestTime(double t)
{
	FEState& s0 = *GetState(0);
	if (s0.m_time >= t) return 0;

	FEState& s1 = *GetState(GetStates() - 1);
	if (s1.m_time <= t) return GetStates() - 1;

	for (int i = 1; i<GetStates(); ++i)
	{
		FEState& s = *GetState(i);
		if (s.m_time >= t) return i - 1;
	}
	return GetStates() - 1;
}


//-----------------------------------------------------------------------------
float FEPostModel::GetTimeValue(int ntime)
{
	return GetState(ntime)->m_time;
}

//-----------------------------------------------------------------------------
void FEPostModel::AddMesh(FEPostMesh* mesh)
{
	m_mesh.push_back(mesh);

	// create a reference state for this mesh
	FERefState* ref = new FERefState(this);
	ref->m_Node.resize(mesh->Nodes());
	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		ref->m_Node[i].m_rt = to_vec3f(mesh->Node(i).r);
	}
	m_RefState.push_back(ref);
}

//-----------------------------------------------------------------------------
int FEPostModel::Meshes() const
{
	return (int) m_mesh.size();
}

//-----------------------------------------------------------------------------
FEPostMesh* FEPostModel::GetFEMesh(int n)
{
	if ((n>=0) && (n<m_mesh.size()))
		return m_mesh[n];
	else
		return nullptr;
}

//-----------------------------------------------------------------------------
// Clear the data of the model
void FEPostModel::Clear()
{
	ClearObjects();
	DeleteMeshes();

	m_Mat.clear();
	ClearStates();
	
	m_title.clear();
	m_name.clear();
}

//-----------------------------------------------------------------------------
void FEPostModel::SetTitle(const string& title)
{
	m_title = title;
}

//-----------------------------------------------------------------------------
const string& FEPostModel::GetTitle() const
{
	return m_title;
}

//-----------------------------------------------------------------------------
void FEPostModel::SetName(const std::string& name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
const string& FEPostModel::GetName() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
// add a material to the model
void FEPostModel::AddMaterial(Material& mat)
{ 
	static int n = 1;
	if (m_Mat.empty()) n = 1;

	if (mat.GetName()[0] == 0)
	{
		char sz[64];
		sprintf(sz, "Material%02d", n);
		n += 1;
		mat.SetName(sz);
	}
	m_Mat.push_back(mat); 
}

//-----------------------------------------------------------------------------
// clear the FE-states
void FEPostModel::ClearStates()
{
	for (int i=0; i<(int) m_State.size(); i++) delete m_State[i];
	m_State.clear();
	m_nTime = 0;
}

//-----------------------------------------------------------------------------
void FEPostModel::AddState(FEState* pFEState)
{
	pFEState->SetID((int) m_State.size());
	pFEState->m_ref = m_RefState[m_RefState.size() - 1];
	m_State.push_back(pFEState); 
}

//-----------------------------------------------------------------------------
// add a state
void FEPostModel::AddState(float ftime, int nstatus, bool interpolateData)
{
	FEState* psnew = nullptr;
	vector<FEState*>::iterator it = m_State.begin();
	for (it = m_State.begin(); it != m_State.end(); ++it)
		if ((*it)->m_time > ftime)
		{
			psnew = new FEState(ftime, this, (*it)->GetFEMesh());
			psnew->m_ref = (*it)->m_ref;
			psnew->m_status = nstatus;
			break;
		}

	// get last state
	if (psnew == nullptr)
	{
		FEState* ps = GetState(GetStates() - 1);
		ps->SetID((int)m_State.size());
		psnew = new FEState(ftime, this, ps->GetFEMesh());
		psnew->m_ref = ps->m_ref;
		psnew->m_status = nstatus;
	}

	assert(psnew);
	if (psnew)
	{
		InsertState(psnew, ftime);
		if (interpolateData) InterpolateStateData(psnew);
	}
}

//-----------------------------------------------------------------------------
template <typename T> void InterpolateNodeData(Post::FEMeshData& data, Post::FEMeshData& data0, Post::FEMeshData& data1, float w)
{
	FENodeData<T>* pf = dynamic_cast<FENodeData<T>*>(&data);
	if (pf)
	{
		FENodeData<T>& d  = dynamic_cast<FENodeData<T>&>(data );
		FENodeData<T>& s0 = dynamic_cast<FENodeData<T>&>(data0);
		FENodeData<T>& s1 = dynamic_cast<FENodeData<T>&>(data1);

		float w0 = 1.f - w;
		float w1 = w;
		int N = d.size();
		for (int i = 0; i < N; ++i)
		{
			d[i] = s0[i] * w0 + s1[i] * w1;
		}
	}
}

//-----------------------------------------------------------------------------
template <typename T, DATA_FORMAT F> void InterpolateFaceData(Post::FEMeshData& data, Post::FEMeshData& data0, Post::FEMeshData& data1, float w)
{
	float w0 = 1.f - w;
	float w1 = w;

	FEFaceData<T, F>* pf = dynamic_cast<FEFaceData<T, F>*>(&data);
	if (pf)
	{
		FEFaceData<T, F>& d  = dynamic_cast<FEFaceData<T, F>&>(data);
		FEFaceData<T, F>& s0 = dynamic_cast<FEFaceData<T, F>&>(data0);
		FEFaceData<T, F>& s1 = dynamic_cast<FEFaceData<T, F>&>(data1);

		// first copy the data from s0 to make sure all data arrays are initialized
		d.copy(s0);

		// now, interpolate data
		int N = s0.size();
		for (int i = 0; i < N; ++i) d[i] = s0[i] * w0 + s1[i] * w1;
	}
}

//-----------------------------------------------------------------------------
template <typename T, DATA_FORMAT F> void InterpolateElementData(Post::FEMeshData& data, Post::FEMeshData& data0, Post::FEMeshData& data1, float w)
{
	float w0 = 1.f - w;
	float w1 = w;

	FEElementData<T, F>* pf = dynamic_cast<FEElementData<T, F>*>(&data);
	if (pf)
	{
		FEElementData<T, F>& d  = dynamic_cast<FEElementData<T, F>&>(data);
		FEElementData<T, F>& s0 = dynamic_cast<FEElementData<T, F>&>(data0);
		FEElementData<T, F>& s1 = dynamic_cast<FEElementData<T, F>&>(data1);

		// first copy the data from s0 to make sure all data arrays are initialized
		d.copy(s0);

		// now, interpolate data
		int N = s0.size();
		for (int i = 0; i < N; ++i) d[i] = s0[i] * w0 + s1[i] * w1;
	}
}

//-----------------------------------------------------------------------------
void InterpolateMeshData(Post::FEMeshData& data, Post::FEMeshData& data0, Post::FEMeshData& data1, float w)
{
	if (dynamic_cast<FENodeItemData*>(&data))
	{
		switch (data.GetType())
		{
		case DATA_SCALAR : InterpolateNodeData<float >(data, data0, data1, w); break;
		case DATA_VEC3 : InterpolateNodeData<vec3f >(data, data0, data1, w); break;
		case DATA_MAT3 : InterpolateNodeData<mat3f >(data, data0, data1, w); break;
		case DATA_MAT3S: InterpolateNodeData<mat3fs>(data, data0, data1, w); break;
		}
	}
	else if (dynamic_cast<FEFaceItemData*>(&data))
	{
		if (data.GetFormat() == DATA_ITEM)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : InterpolateFaceData<float , DATA_ITEM>(data, data0, data1, w); break;
			case DATA_VEC3 : InterpolateFaceData<vec3f , DATA_ITEM>(data, data0, data1, w); break;
			case DATA_MAT3 : InterpolateFaceData<mat3f , DATA_ITEM>(data, data0, data1, w); break;
			case DATA_MAT3S: InterpolateFaceData<mat3fs, DATA_ITEM>(data, data0, data1, w); break;
			}
		}
		else if (data.GetFormat() == DATA_MULT)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : InterpolateFaceData<float , DATA_MULT>(data, data0, data1, w); break;
			case DATA_VEC3 : InterpolateFaceData<vec3f , DATA_MULT>(data, data0, data1, w); break;
			case DATA_MAT3 : InterpolateFaceData<mat3f , DATA_MULT>(data, data0, data1, w); break;
			case DATA_MAT3S: InterpolateFaceData<mat3fs, DATA_MULT>(data, data0, data1, w); break;
			}
		}
		else if (data.GetFormat() == DATA_NODE)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : InterpolateFaceData<float , DATA_NODE>(data, data0, data1, w); break;
			case DATA_VEC3 : InterpolateFaceData<vec3f , DATA_NODE>(data, data0, data1, w); break;
			case DATA_MAT3 : InterpolateFaceData<mat3f , DATA_NODE>(data, data0, data1, w); break;
			case DATA_MAT3S: InterpolateFaceData<mat3fs, DATA_NODE>(data, data0, data1, w); break;
			}
		}
		else if (data.GetFormat() == DATA_REGION)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR: InterpolateFaceData<float , DATA_REGION>(data, data0, data1, w); break;
			case DATA_VEC3  : InterpolateFaceData<vec3f , DATA_REGION>(data, data0, data1, w); break;
			case DATA_MAT3  : InterpolateFaceData<mat3f , DATA_REGION>(data, data0, data1, w); break;
			case DATA_MAT3S : InterpolateFaceData<mat3fs, DATA_REGION>(data, data0, data1, w); break;
			}
		}
	}
	else if (dynamic_cast<FEElemItemData*>(&data))
	{
		if (data.GetFormat() == DATA_ITEM)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : InterpolateElementData<float , DATA_ITEM>(data, data0, data1, w); break;
			case DATA_VEC3 : InterpolateElementData<vec3f , DATA_ITEM>(data, data0, data1, w); break;
			case DATA_MAT3 : InterpolateElementData<mat3f , DATA_ITEM>(data, data0, data1, w); break;
			case DATA_MAT3S: InterpolateElementData<mat3fs, DATA_ITEM>(data, data0, data1, w); break;
			}
		}
		else if (data.GetFormat() == DATA_MULT)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : InterpolateElementData<float , DATA_MULT>(data, data0, data1, w); break;
			case DATA_VEC3 : InterpolateElementData<vec3f , DATA_MULT>(data, data0, data1, w); break;
			case DATA_MAT3 : InterpolateElementData<mat3f , DATA_MULT>(data, data0, data1, w); break;
			case DATA_MAT3S: InterpolateElementData<mat3fs, DATA_MULT>(data, data0, data1, w); break;
			}
		}
		else if (data.GetFormat() == DATA_NODE)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : InterpolateElementData<float , DATA_NODE>(data, data0, data1, w); break;
			case DATA_VEC3 : InterpolateElementData<vec3f , DATA_NODE>(data, data0, data1, w); break;
			case DATA_MAT3 : InterpolateElementData<mat3f , DATA_NODE>(data, data0, data1, w); break;
			case DATA_MAT3S: InterpolateElementData<mat3fs, DATA_NODE>(data, data0, data1, w); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
template <typename T> void CopyNodeData(Post::FEMeshData& data, Post::FEMeshData& src)
{
	FENodeData<T>* pf = dynamic_cast<FENodeData<T>*>(&data);
	if (pf)
	{
		FENodeData<T>* pfs = dynamic_cast<FENodeData<T>*>(&src);
		pf->copy(*pfs);
	}
}

//-----------------------------------------------------------------------------
template <typename T, DATA_FORMAT F> void CopyFaceData(Post::FEMeshData& data, Post::FEMeshData& src)
{
	FEFaceData<T, F>* pf = dynamic_cast<FEFaceData<T, F>*>(&data);
	if (pf)
	{
		FEFaceData<T, F>* pfs = dynamic_cast<FEFaceData<T, F>*>(&src);
		pf->copy(*pfs);
	}
}

//-----------------------------------------------------------------------------
template <typename T, DATA_FORMAT F> void CopyElementData(Post::FEMeshData& data, Post::FEMeshData& src)
{
	FEElementData<T, F>* pf = dynamic_cast<FEElementData<T, F>*>(&data);
	if (pf)
	{
		FEElementData<T, F>* pfs = dynamic_cast<FEElementData<T, F>*>(&src);
		pf->copy(*pfs);
	}
}

//-----------------------------------------------------------------------------
void CopyMeshData(Post::FEMeshData& data, Post::FEMeshData& src)
{
	if (dynamic_cast<FENodeItemData*>(&data))
	{
		switch (data.GetType())
		{
		case DATA_SCALAR : CopyNodeData<float >(data, src); break;
		case DATA_VEC3 : CopyNodeData<vec3f >(data, src); break;
		case DATA_MAT3 : CopyNodeData<mat3f >(data, src); break;
		case DATA_MAT3S: CopyNodeData<mat3fs>(data, src); break;
		}
	}
	else if (dynamic_cast<FEFaceItemData*>(&data))
	{
		if (data.GetFormat() == DATA_ITEM)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : CopyFaceData<float , DATA_ITEM>(data, src); break;
			case DATA_VEC3 : CopyFaceData<vec3f , DATA_ITEM>(data, src); break;
			case DATA_MAT3 : CopyFaceData<mat3f , DATA_ITEM>(data, src); break;
			case DATA_MAT3S: CopyFaceData<mat3fs, DATA_ITEM>(data, src); break;
			}
		}
		else if (data.GetFormat() == DATA_MULT)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : CopyFaceData<float , DATA_MULT>(data, src); break;
			case DATA_VEC3 : CopyFaceData<vec3f , DATA_MULT>(data, src); break;
			case DATA_MAT3 : CopyFaceData<mat3f , DATA_MULT>(data, src); break;
			case DATA_MAT3S: CopyFaceData<mat3fs, DATA_MULT>(data, src); break;
			}
		}
		else if (data.GetFormat() == DATA_NODE)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : CopyFaceData<float , DATA_NODE>(data, src); break;
			case DATA_VEC3 : CopyFaceData<vec3f , DATA_NODE>(data, src); break;
			case DATA_MAT3 : CopyFaceData<mat3f , DATA_NODE>(data, src); break;
			case DATA_MAT3S: CopyFaceData<mat3fs, DATA_NODE>(data, src); break;
			}
		}
	}
	else if (dynamic_cast<FEElemItemData*>(&data))
	{
		if (data.GetFormat() == DATA_ITEM)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : CopyElementData<float , DATA_ITEM>(data, src); break;
			case DATA_VEC3 : CopyElementData<vec3f , DATA_ITEM>(data, src); break;
			case DATA_MAT3 : CopyElementData<mat3f , DATA_ITEM>(data, src); break;
			case DATA_MAT3S: CopyElementData<mat3fs, DATA_ITEM>(data, src); break;
			}
		}
		else if (data.GetFormat() == DATA_MULT)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : CopyElementData<float , DATA_MULT>(data, src); break;
			case DATA_VEC3 : CopyElementData<vec3f , DATA_MULT>(data, src); break;
			case DATA_MAT3 : CopyElementData<mat3f , DATA_MULT>(data, src); break;
			case DATA_MAT3S: CopyElementData<mat3fs, DATA_MULT>(data, src); break;
			}
		}
		else if (data.GetFormat() == DATA_NODE)
		{
			switch (data.GetType())
			{
			case DATA_SCALAR : CopyElementData<float , DATA_NODE>(data, src); break;
			case DATA_VEC3 : CopyElementData<vec3f , DATA_NODE>(data, src); break;
			case DATA_MAT3 : CopyElementData<mat3f , DATA_NODE>(data, src); break;
			case DATA_MAT3S: CopyElementData<mat3fs, DATA_NODE>(data, src); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEPostModel::InterpolateStateData(FEState* ps)
{
	if (ps == nullptr) return;

	FEState& sd = *ps;
	int n = ps->GetID();
	if (n == 0)
	{
		// first state, so copy from next
		FEState& s1 = *GetState(n + 1);
		Post::FEMeshDataList& dataList1 = s1.m_Data;
		Post::FEMeshDataList& dataList = sd.m_Data;

		for (int i = 0; i < dataList.size(); ++i)
		{
			Post::FEMeshData& data1 = dataList1[i];
			Post::FEMeshData& data = dataList[i];
			CopyMeshData(data, data1);
		}
	}
	else if (n == GetStates() - 1)
	{
		// last state, so copy from prev
		FEState& s0 = *GetState(n - 1);

		Post::FEMeshDataList& dataList0 = s0.m_Data;
		Post::FEMeshDataList& dataList = sd.m_Data;

		for (int i = 0; i < dataList.size(); ++i)
		{
			Post::FEMeshData& data0 = dataList0[i];
			Post::FEMeshData& data = dataList[i];
			CopyMeshData(data, data0);
		}
	}
	else
	{
		// genuine interpolation
		FEState& s0 = *GetState(n - 1);
		FEState& s1 = *GetState(n + 1);

		float t0 = s0.m_time;
		float t1 = s1.m_time;
		float Dt = t1 - t0; if (Dt == 0.f) Dt = 1.f;
		float t = sd.m_time;
		float w = (t - t0) / Dt;

		Post::FEMeshDataList& dataList0 = s0.m_Data;
		Post::FEMeshDataList& dataList1 = s1.m_Data;
		Post::FEMeshDataList& dataList  = sd.m_Data;

		for (int i = 0; i < dataList.size(); ++i)
		{
			Post::FEMeshData& data0 = dataList0[i];
			Post::FEMeshData& data1 = dataList1[i];
			Post::FEMeshData& data  = dataList [i];

			InterpolateMeshData(data, data0, data1, w);
		}
	}
}

//-----------------------------------------------------------------------------
// delete a state
void FEPostModel::DeleteState(int n)
{
	vector<FEState*>::iterator it = m_State.begin();
	int N = m_State.size();
	assert((n>=0) && (n<N));
	for (int i=0; i<n; ++i) ++it;
	m_State.erase(it);

	// reindex the states
	for (int i=0; i<(int)m_State.size(); ++i) m_State[i]->SetID(i);
}

//-----------------------------------------------------------------------------
// insert a state a time f
void FEPostModel::InsertState(FEState *ps, float f)
{
	vector<FEState*>::iterator it = m_State.begin();
	for (it=m_State.begin(); it != m_State.end(); ++it)
		if ((*it)->m_time > f) 
		{
			break;
		}
	m_State.insert(it, ps);

	// reindex the states
	for (int i = 0; i<(int)m_State.size(); ++i) m_State[i]->SetID(i);
}

//-----------------------------------------------------------------------------
template <typename Type> void copy_node_data(FEMeshData& d, FEMeshData& s)
{
	FENodeData<Type>& dt = dynamic_cast<FENodeData<Type>&>(d);
	FENodeData<Type>& st = dynamic_cast<FENodeData<Type>&>(s);
	dt.copy(st);
}

//-----------------------------------------------------------------------------
template <typename Type, DATA_FORMAT Fmt> void copy_elem_data(FEMeshData& d, FEMeshData& s)
{
	FEElementData<Type, Fmt>& dt = dynamic_cast<FEElementData<Type, Fmt>&>(d);
	FEElementData<Type, Fmt>& st = dynamic_cast<FEElementData<Type, Fmt>&>(s);
	dt.copy(st);
}

//-----------------------------------------------------------------------------
template <typename Type, DATA_FORMAT Fmt> void copy_face_data(FEMeshData& d, FEMeshData& s)
{
	FEFaceData<Type, Fmt>& dt = dynamic_cast<FEFaceData<Type, Fmt>&>(d);
	FEFaceData<Type, Fmt>& st = dynamic_cast<FEFaceData<Type, Fmt>&>(s);
	dt.copy(st);
}

//-----------------------------------------------------------------------------
// Copy a data field
ModelDataField* FEPostModel::CopyDataField(ModelDataField* pd, const char* sznewname)
{
	// Clone the data field
	ModelDataField* pdcopy = pd->Clone();

	// create a new name
	if (sznewname == 0)
	{
		char szname[256] = {0};
		sprintf(szname, "%s_copy", pd->GetName().c_str());
		pdcopy->SetName(szname);
	}
	else pdcopy->SetName(sznewname);

	// Add it to the model
	AddDataField(pdcopy);

	int ndst = FIELD_CODE(pdcopy->GetFieldID());
	int nsrc = FIELD_CODE(pd    ->GetFieldID());

	int nstates = GetStates();
	for (int i=0; i<nstates; ++i)
	{
		FEState& state = *GetState(i);
		FEMeshDataList& DL = state.m_Data;

		FEMeshData& dst = DL[ndst];
		FEMeshData& src = DL[nsrc];

		if (IS_NODE_FIELD(pd->GetFieldID()))
		{
			assert(pd->Format() == DATA_ITEM);
			if      (pd->Type() == DATA_SCALAR ) copy_node_data<float >(dst, src);
			else if (pd->Type() == DATA_VEC3 ) copy_node_data<vec3f >(dst, src);
			else if (pd->Type() == DATA_MAT3S) copy_node_data<mat3fs>(dst, src);
			else if (pd->Type() == DATA_MAT3SD) copy_node_data<mat3fd>(dst, src);
			else if (pd->Type() == DATA_MAT3 ) copy_node_data<mat3f >(dst, src);
		}
		else if (IS_FACE_FIELD(pd->GetFieldID()))
		{
			switch (pd->Format())
			{
			case DATA_ITEM:
				{
					if      (pd->Type() == DATA_SCALAR ) copy_face_data<float , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_VEC3 ) copy_face_data<vec3f , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_MAT3S) copy_face_data<mat3fs, DATA_ITEM>(dst, src);
				}
				break;
			case DATA_NODE:
				{
					if      (pd->Type() == DATA_SCALAR ) copy_face_data<float , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_VEC3 ) copy_face_data<vec3f , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_MAT3S) copy_face_data<mat3fs, DATA_NODE>(dst, src);
				}
				break;
			case DATA_MULT:
				{
					if      (pd->Type() == DATA_SCALAR ) copy_face_data<float , DATA_MULT>(dst, src);
					else if (pd->Type() == DATA_VEC3 ) copy_face_data<vec3f , DATA_MULT>(dst, src);
					else if (pd->Type() == DATA_MAT3S) copy_face_data<mat3fs, DATA_MULT>(dst, src);
				}
				break;
			}
		}
		else if (IS_ELEM_FIELD(pd->GetFieldID()))
		{
			switch (pd->Format())
			{
			case DATA_ITEM:
				{
					if      (pd->Type() == DATA_SCALAR ) copy_elem_data<float , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_VEC3 ) copy_elem_data<vec3f , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_MAT3S) copy_elem_data<mat3fs, DATA_ITEM>(dst, src);
				}
				break;
			case DATA_NODE:
				{
					if      (pd->Type() == DATA_SCALAR ) copy_elem_data<float , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_VEC3 ) copy_elem_data<vec3f , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_MAT3S) copy_elem_data<mat3fs, DATA_NODE>(dst, src);
				}
				break;
			case DATA_MULT:
				{
					if      (pd->Type() == DATA_SCALAR ) copy_elem_data<float , DATA_MULT>(dst, src);
					else if (pd->Type() == DATA_VEC3 ) copy_elem_data<vec3f , DATA_MULT>(dst, src);
					else if (pd->Type() == DATA_MAT3S) copy_elem_data<mat3fs, DATA_MULT>(dst, src);
				}
				break;
			}
		}
	}

	return pdcopy;
}

//-----------------------------------------------------------------------------
ModelDataField* createCachedDataField(ModelDataField* pd)
{
	Post::FEPostModel* fem = pd->GetModel();

	DATA_CLASS nclass = pd->DataClass();
	DATA_TYPE ntype = pd->Type();
	DATA_FORMAT nfmt = pd->Format();

	ModelDataField* newField = 0;
	if (nclass == NODE_DATA)
	{
		if      (ntype == DATA_SCALAR ) newField = new FEDataField_T<FENodeData<float > >(fem);
		else if (ntype == DATA_VEC3  ) newField = new FEDataField_T<FENodeData<vec3f > >(fem);
		else if (ntype == DATA_MAT3  ) newField = new FEDataField_T<FENodeData<mat3f > >(fem);
		else if (ntype == DATA_MAT3S ) newField = new FEDataField_T<FENodeData<mat3fs> >(fem);
		else if (ntype == DATA_MAT3SD) newField = new FEDataField_T<FENodeData<mat3fd> >(fem);
		else assert(false);
	}
	else if (nclass == ELEM_DATA)
	{
		if (ntype == DATA_SCALAR)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEElementData<float, DATA_NODE  > >(fem);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEElementData<float, DATA_ITEM  > >(fem);
			else if (nfmt == DATA_MULT  ) newField = new FEDataField_T<FEElementData<float, DATA_MULT  > >(fem);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEElementData<float, DATA_REGION> >(fem);
			else assert(false);
		}
		else if (ntype == DATA_VEC3)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEElementData<vec3f, DATA_NODE  > >(fem);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEElementData<vec3f, DATA_ITEM  > >(fem);
			else if (nfmt == DATA_MULT  ) newField = new FEDataField_T<FEElementData<vec3f, DATA_MULT  > >(fem);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEElementData<vec3f, DATA_REGION> >(fem);
			else assert(false);
		}
		else if (ntype == DATA_MAT3S)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEElementData<mat3fs, DATA_NODE  > >(fem);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEElementData<mat3fs, DATA_ITEM  > >(fem);
			else if (nfmt == DATA_MULT  ) newField = new FEDataField_T<FEElementData<mat3fs, DATA_MULT  > >(fem);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEElementData<mat3fs, DATA_REGION> >(fem);
			else assert(false);
		}
		else assert(false);
	}
	else if (nclass == FACE_DATA)
	{
		if (ntype == DATA_SCALAR)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEFaceData<float, DATA_NODE  > >(fem);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEFaceData<float, DATA_ITEM  > >(fem);
			else if (nfmt == DATA_MULT  ) newField = new FEDataField_T<FEFaceData<float, DATA_MULT  > >(fem);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEFaceData<float, DATA_REGION> >(fem);
			else assert(false);
		}
		else if (ntype == DATA_VEC3)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEFaceData<vec3f, DATA_NODE  > >(fem);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEFaceData<vec3f, DATA_ITEM  > >(fem);
			else if (nfmt == DATA_MULT  ) newField = new FEDataField_T<FEFaceData<vec3f, DATA_MULT  > >(fem);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEFaceData<vec3f, DATA_REGION> >(fem);
			else assert(false);
		}
		else if (ntype == DATA_MAT3S)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_NODE  > >(fem);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_ITEM  > >(fem);
			else if (nfmt == DATA_MULT  ) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_MULT  > >(fem);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_REGION> >(fem);
			else assert(false);
		}
		else assert(false);
	}

	assert(newField);

	return newField;
}

//-----------------------------------------------------------------------------
template <typename T> void cached_copy_node_data(FEMeshData& dst, FEMeshData& src, int NN)
{
	FENodeData<T>& d = dynamic_cast<FENodeData<T>&>(dst);
	FENodeData_T<T>& s = dynamic_cast<FENodeData_T<T>&>(src);
	for (int i = 0; i<NN; ++i) s.eval(i, &d[i]);
}

template <typename T> void cached_copy_face_data_ITEM(FEMeshData& dst, FEMeshData& src, int NF)
{
	FEFaceData<T, DATA_ITEM>& d = dynamic_cast<FEFaceData<T, DATA_ITEM>&>(dst);
	FEFaceData_T<T, DATA_ITEM>& s = dynamic_cast<FEFaceData_T<T, DATA_ITEM>&>(src);

	T f;
	for (int i = 0; i<NF; ++i)
	{
		if (s.active(i))
		{
			s.eval(i, &f);
			d.add(i, f);
		}
	}
}

template <typename T> void cached_copy_face_data_COMP(FEMeshData& dst, FEMeshData& src, FEPostMesh& mesh)
{
	FEFaceData<T, DATA_MULT>& d = dynamic_cast<FEFaceData<T, DATA_MULT>&>(dst);
	FEFaceData_T<T, DATA_MULT>& s = dynamic_cast<FEFaceData_T<T, DATA_MULT>&>(src);

	int NF = mesh.Faces();
	T f[FSFace::MAX_NODES];
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = mesh.Face(i);
		if (s.active(i))
		{
			int nf = face.Nodes();
			s.eval(i, f);
			d.add(i, f, nf);
		}
	}
}

template <typename T> void cached_copy_face_data_NODE(FEMeshData& dst, FEMeshData& src, FEPostMesh& mesh)
{
	FEFaceData<T, DATA_NODE>& d = dynamic_cast<FEFaceData<T, DATA_NODE>&>(dst);
	FEFaceData_T<T, DATA_NODE>& s = dynamic_cast<FEFaceData_T<T, DATA_NODE>&>(src);

	int NF = mesh.Faces();
	T f[FSFace::MAX_NODES];
	vector<T> vf;
	vector<int> faceList(1);
	vector<int> index;
	vector<int> faceSize(1);
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = mesh.Face(i);
		if (s.active(i))
		{
			int nf = face.Nodes();
			s.eval(i, f);

			// we need to convert data to vectors
			faceList[0] = i;
			vf.resize(nf);
			for (int j = 0; j<nf; ++j) vf[j] = f[j];
			index.resize(nf);
			for (int j = 0; j<nf; ++j) index[j] = j;
			faceSize[0] = nf;

			// NOTE: This actually is equivalent to COMP format. 
			d.add(vf, faceList, index, faceSize);
		}
	}
}

template <typename T> void cached_copy_elem_data_ITEM(FEMeshData& dst, FEMeshData& src, int NE)
{
	FEElementData<T, DATA_ITEM>& d = dynamic_cast<FEElementData<T, DATA_ITEM>&>(dst);
	FEElemData_T<T, DATA_ITEM>& s = dynamic_cast<FEElemData_T<T, DATA_ITEM>&>(src);

	T f;
	for (int i = 0; i<NE; ++i)
	{
		if (s.active(i))
		{
			s.eval(i, &f);
			d.add(i, f);
		}
	}
}

template <typename T> void cached_copy_elem_data_REGION(FEMeshData& dst, FEMeshData& src, int NE)
{
	FEElementData<T, DATA_REGION>& d = dynamic_cast<FEElementData<T, DATA_REGION>&>(dst);
	FEElemData_T<T, DATA_REGION>& s = dynamic_cast<FEElemData_T<T, DATA_REGION>&>(src);

	T f;
	for (int i = 0; i<NE; ++i)
	{
		if (s.active(i))
		{
			s.eval(i, &f);
			d.add(i, f);
		}
	}
}

template <typename T> void cached_copy_elem_data_COMP(FEMeshData& dst, FEMeshData& src, FEPostMesh& mesh)
{
	FEElementData<T, DATA_MULT>& d = dynamic_cast<FEElementData<T, DATA_MULT>&>(dst);
	FEElemData_T<T, DATA_MULT>& s = dynamic_cast<FEElemData_T<T, DATA_MULT>&>(src);

	int NE = mesh.Elements();
	T f[FSElement::MAX_NODES];
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (s.active(i))
		{
			int ne = el.Nodes();
			s.eval(i, f);
			d.add(i, ne, f);
		}
	}
}

template <typename T> void cached_copy_elem_data_NODE(FEMeshData& dst, FEMeshData& src, FEPostMesh& mesh)
{
	FEElementData<T, DATA_NODE>& d = dynamic_cast<FEElementData<T, DATA_NODE>&>(dst);
	FEElemData_T<T, DATA_NODE>& s = dynamic_cast<FEElemData_T<T, DATA_NODE>&>(src);

	int NE = mesh.Elements();
	T f[FSElement::MAX_NODES];
	vector<T> vf;
	vector<int> elem(1);
	vector<int> index;
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (s.active(i))
		{
			int ne = el.Nodes();
			s.eval(i, f);

			// we need to convert data to vectors
			elem[0] = i;
			vf.resize(ne);
			for (int j=0; j<ne; ++j) vf[j] = f[j];
			index.resize(ne);
			for (int j=0; j<ne; ++j) index[j] = j;

			// NOTE: This actually is equivalent to COMP format. 
			d.add(vf, elem, index, ne);
		}
	}
}


//-----------------------------------------------------------------------------
//! Create a cached copy of a data field
ModelDataField* FEPostModel::CreateCachedCopy(ModelDataField* pd, const char* sznewname)
{
	// create a new data field that will store a cached copy
	ModelDataField* pdcopy = createCachedDataField(pd);
	if (pdcopy == 0) return 0;

	// Add it to the model
	AddDataField(pdcopy, sznewname);

	// get the field ID codes
	int ndst = FIELD_CODE(pdcopy->GetFieldID());
	int nsrc = FIELD_CODE(pd->GetFieldID());

	// get the data info
	DATA_CLASS nclass = pd->DataClass();
	DATA_TYPE ntype = pd->Type();
	DATA_FORMAT nfmt = pd->Format();

	// loop over all the states
	FEPostMesh& mesh = *GetFEMesh(0);
	int nstates = GetStates();
	for (int i = 0; i<nstates; ++i)
	{
		FEState& state = *GetState(i);
		FEMeshDataList& DL = state.m_Data;

		// get source and destination fields
		FEMeshData& dst = DL[ndst];
		FEMeshData& src = DL[nsrc];

		// copy data
		if (nclass == NODE_DATA)
		{
			int NN = mesh.Nodes();
			if      (ntype == DATA_SCALAR ) cached_copy_node_data<float >(dst, src, NN);
			else if (ntype == DATA_VEC3 ) cached_copy_node_data<vec3f >(dst, src, NN);
			else if (ntype == DATA_MAT3S) cached_copy_node_data<mat3fs>(dst, src, NN);
			else if (ntype == DATA_MAT3SD) cached_copy_node_data<mat3fd>(dst, src, NN);
			else assert(false);
		}
		else if (nclass == FACE_DATA)
		{
			int NF = mesh.Faces();
			if (nfmt == DATA_ITEM)
			{
				if      (ntype == DATA_SCALAR ) cached_copy_face_data_ITEM<float >(dst, src, NF);
				else if (ntype == DATA_VEC3 ) cached_copy_face_data_ITEM<vec3f >(dst, src, NF);
				else if (ntype == DATA_MAT3S) cached_copy_face_data_ITEM<mat3fs>(dst, src, NF);
				else if (ntype == DATA_MAT3SD) cached_copy_face_data_ITEM<mat3fd>(dst, src, NF);
				else assert(false);
			}
			else if (nfmt == DATA_MULT)
			{
				if      (ntype == DATA_SCALAR ) cached_copy_face_data_COMP<float >(dst, src, mesh);
				else if (ntype == DATA_SCALAR ) cached_copy_face_data_COMP<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3S) cached_copy_face_data_COMP<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3SD) cached_copy_face_data_COMP<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else if (nfmt == DATA_NODE)
			{
				if      (ntype == DATA_SCALAR ) cached_copy_face_data_NODE<float >(dst, src, mesh);
				else if (ntype == DATA_SCALAR ) cached_copy_face_data_NODE<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3S) cached_copy_face_data_NODE<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3SD) cached_copy_face_data_NODE<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else assert(false);
		}
		else if (nclass == ELEM_DATA)
		{
			int NE = mesh.Elements();
			if (nfmt == DATA_ITEM)
			{
				if      (ntype == DATA_SCALAR ) cached_copy_elem_data_ITEM<float >(dst, src, NE);
				else if (ntype == DATA_VEC3 ) cached_copy_elem_data_ITEM<vec3f >(dst, src, NE);
				else if (ntype == DATA_MAT3S) cached_copy_elem_data_ITEM<mat3fs>(dst, src, NE);
				else if (ntype == DATA_MAT3SD) cached_copy_elem_data_ITEM<mat3fd>(dst, src, NE);
				else assert(false);
			}
			else if (nfmt == DATA_MULT)
			{
				if      (ntype == DATA_SCALAR ) cached_copy_elem_data_COMP<float >(dst, src, mesh);
				else if (ntype == DATA_SCALAR ) cached_copy_elem_data_COMP<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3S) cached_copy_elem_data_COMP<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3SD) cached_copy_elem_data_COMP<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else if (nfmt == DATA_NODE)
			{
				if      (ntype == DATA_SCALAR ) cached_copy_elem_data_NODE<float >(dst, src, mesh);
				else if (ntype == DATA_VEC3 ) cached_copy_elem_data_NODE<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3S) cached_copy_elem_data_NODE<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3SD) cached_copy_elem_data_NODE<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else if (nfmt == DATA_REGION)
			{
				if      (ntype == DATA_SCALAR ) cached_copy_elem_data_REGION<float >(dst, src, NE);
				else if (ntype == DATA_VEC3 ) cached_copy_elem_data_REGION<vec3f >(dst, src, NE);
				else if (ntype == DATA_MAT3S) cached_copy_elem_data_REGION<mat3fs>(dst, src, NE);
				else if (ntype == DATA_MAT3SD) cached_copy_elem_data_REGION<mat3fd>(dst, src, NE);
				else assert(false);
			}
			else assert(false);
		}
		else assert(false);
	}

	return pdcopy;
}

//-----------------------------------------------------------------------------
// Get the field variable name
std::string FEPostModel::getDataString(int ndata, Data_Tensor_Type ntype)
{
	FEDataManager& dm = *GetDataManager();
	return dm.getDataString(ndata, ntype);
}

//-----------------------------------------------------------------------------
// Delete a data field
void FEPostModel::DeleteDataField(ModelDataField* pd)
{
	// find out which data field this is
	FEDataFieldPtr it = m_pDM->FirstDataField();
	int NDF = m_pDM->DataFields(), m = -1;
	for (int i=0; i<NDF; ++i, ++it)
	{
		if (*it == pd)
		{
			m = i;
			break;
		}
	}
	if (m == -1) { assert(false); return; }

	// remove this field from all states
	int NS = GetStates();
	for (int i=0; i<NS; ++i)
	{
		FEState* ps = GetState(i);
		ps->m_Data.erase(m);
	}
	m_pDM->DeleteDataField(pd);

	// Inform all dependants
	UpdateDependants();
}

//-----------------------------------------------------------------------------
// Add a data field to all states of the model
void FEPostModel::AddDataField(ModelDataField* pd, const std::string& name)
{
	// add the data field to the data manager
	m_pDM->AddDataField(pd, name);

	// now add new data for each of the states
	vector<FEState*>::iterator it;
	for (it=m_State.begin(); it != m_State.end(); ++it)
	{
		(*it)->m_Data.push_back(pd->CreateData(*it));
	}

	// update all dependants
	UpdateDependants();
}

//-----------------------------------------------------------------------------
// Add an data field to all states of the model
void FEPostModel::AddDataField(ModelDataField* pd, vector<int>& L)
{
	assert(pd->DataClass() == FACE_DATA);

	// add the data field to the data manager
	m_pDM->AddDataField(pd);

	// now add new meshdata for each of the states
	vector<FEState*>::iterator it;
	for (it=m_State.begin(); it != m_State.end(); ++it)
	{
		FEFaceItemData* pmd = dynamic_cast<FEFaceItemData*>(pd->CreateData(*it));
		(*it)->m_Data.push_back(pmd);
		if (dynamic_cast<Curvature*>(pmd))
		{
			Curvature* pcrv = dynamic_cast<Curvature*>(pmd);
			pcrv->set_facelist(L);
		}
		if (dynamic_cast<SurfaceCongruency*>(pmd))
		{
			SurfaceCongruency* pcon = dynamic_cast<SurfaceCongruency*>(pmd);
			pcon->set_facelist(L);
		}
	}

	// update all dependants
	UpdateDependants();
}

//-----------------------------------------------------------------------------
// This function calculates the position of a node based on the selected
// displacement field.
vec3f FEPostModel::NodePosition(int n, int ntime)
{
	vec3f r;
	if (ntime >= 0)
	{
		FEState* state = GetState(ntime);
		FERefState& ref = *state->m_ref;
		FEPostMesh* mesh = state->GetFEMesh();
		r = ref.m_Node[n].m_rt;
		if (m_ndisp >= 0) r += EvaluateNodeVector(n, ntime, m_ndisp);
	}
	else
	{
		FEPostMesh* mesh = GetFEMesh(0);
		r = to_vec3f(mesh->Node(n).r);
	}

	return r;
}

//-----------------------------------------------------------------------------
vec3f FEPostModel::NodePosition(const vec3f& r, int ntime)
{
	FEPostMesh* mesh = GetState(ntime)->GetFEMesh();

	// find the element in which this node lies
	int iel = -1; double iso[3] = {0};
	if (FindElementInReferenceFrame(*mesh, r, iel, iso))
	{
		vec3f x[FSElement::MAX_NODES];
		GetElementCoords(iel, ntime, x);

		// evaluate 
		FEElement_& el = mesh->ElementRef(iel);

		vec3f xt = el.eval(x, iso[0], iso[1], iso[2]);

		return xt;
	}
	else 
	{
		assert(false);
		return r;
	}
}

//-----------------------------------------------------------------------------
vec3f FEPostModel::FaceNormal(FSFace& f, int ntime)
{
	vec3f r0 = NodePosition(f.n[0], ntime);
	vec3f r1 = NodePosition(f.n[1], ntime);
	vec3f r2 = NodePosition(f.n[2], ntime);
	vec3f fn = (r1 - r0)^(r2 - r0);
	fn.Normalize();
	return fn;
}

//-----------------------------------------------------------------------------
// get the nodal coordinates of an element at time n
void FEPostModel::GetElementCoords(int iel, int ntime, vec3f* r)
{
	FEPostMesh* mesh = GetState(ntime)->GetFEMesh();
	FEElement_& elem = mesh->ElementRef(iel);
	NODEDATA* pn = &m_State[ntime]->m_NODE[0];

	for (int i=0; i<elem.Nodes(); i++)
		r[i] = pn[ elem.m_node[i] ].m_rt;
}

//-----------------------------------------------------------------------------
// Update the bounding box of the mesh. Note that this box bounds the reference
// configuration, not the current configuration
void FEPostModel::UpdateBoundingBox()
{
	FEPostMesh* mesh = GetFEMesh(0);
	if (mesh == nullptr)
	{
		m_bbox = BOX(vec3d(0, 0, 0), vec3d(1, 1, 1));
		return;
	}

	FSNode& n = mesh->Node(0);
	m_bbox.x0 = m_bbox.x1 = n.r.x;
	m_bbox.y0 = m_bbox.y1 = n.r.y;
	m_bbox.z0 = m_bbox.z1 = n.r.z;

	int N = mesh->Nodes();
	for (int i=0; i<N; i++)
	{
		FSNode& n = mesh->Node(i);
		if (n.r.x < m_bbox.x0) m_bbox.x0 = n.r.x;
		if (n.r.y < m_bbox.y0) m_bbox.y0 = n.r.y;
		if (n.r.z < m_bbox.z0) m_bbox.z0 = n.r.z;

		if (n.r.x > m_bbox.x1) m_bbox.x1 = n.r.x;
		if (n.r.y > m_bbox.y1) m_bbox.y1 = n.r.y;
		if (n.r.z > m_bbox.z1) m_bbox.z1 = n.r.z;
	}
}

//-----------------------------------------------------------------------------
void FEPostModel::AddDependant(FEModelDependant* pc)
{
	// make sure we have not added this dependant yet
	if (m_Dependants.empty() == false)
	{
		for (size_t i=0; i<m_Dependants.size(); ++i)
		{
			if (m_Dependants[i] == pc) return;
		}
	}

	// if we get here, the depedant was not added yet, so add it
	m_Dependants.push_back(pc);
}

//-----------------------------------------------------------------------------
void FEPostModel::UpdateDependants()
{
	int N = m_Dependants.size();
	for (int i=0; i<N; ++i) m_Dependants[i]->Update(this);
}

//-----------------------------------------------------------------------------
void FEPostModel::RemoveDependant(FEModelDependant* pc)
{
	int N = m_Dependants.size();
	if (N > 0)
	{
		vector<FEModelDependant*>::iterator it = m_Dependants.begin();
		for (int i=0; i<N; ++i, it++) 
		{
			if (m_Dependants[i] == pc) 
			{
				m_Dependants.erase(it);
				return;
			}
		}
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEPostModel::ClearDependants()
{
	int N = m_Dependants.size();
	if (N > 0)
	{
		// inform the dependents that the model is about to be deleted
		vector<FEModelDependant*>::iterator it = m_Dependants.begin();
		for (int i = 0; i<N; ++i) m_Dependants[i]->Update(0);
		m_Dependants.clear();
	}
}

//-----------------------------------------------------------------------------
void FEPostModel::UpdateMeshState(int ntime)
{
	FEState& state = *GetState(ntime);

	FEPostMesh* mesh = state.GetFEMesh();
	int NE = mesh->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& el = mesh->ElementRef(i);
		ELEMDATA& data = state.m_ELEM[i];

		if (el.IsShell())
		{
			int n = el.Nodes();
			for (int j = 0; j < n; ++j) el.m_h[j] = data.m_h[j];
		}

		if ((data.m_state & StatusFlags::VISIBLE) == 0)
		{
			el.SetEroded(true);
		}
		else el.SetEroded(false);
	}

	// update plot objects
	for (int i = 0; i < PointObjects(); ++i)
	{
		Post::FEPostModel::PointObject& po = *GetPointObject(i);
		OBJ_POINT_DATA& di = state.m_objPt[i];

		po.m_pos = di.pos;
		po.m_rot = di.rot;

		po.m_rt = di.m_rt;
	}

	for (int i = 0; i < LineObjects(); ++i)
	{
		Post::FEPostModel::LineObject& po = *GetLineObject(i);
		OBJ_LINE_DATA& di = state.m_objLn[i];

		po.m_pos = di.pos;
		po.m_rot = di.rot;

		po.m_r1 = di.m_r1;
		po.m_r2 = di.m_r2;
	}
}

//-----------------------------------------------------------------------------
int FEPostModel::PlotObjects() const
{
	return PointObjects() + LineObjects();
}

//-----------------------------------------------------------------------------
FEPostModel::PlotObject* FEPostModel::GetPlotObject(int n)
{
	if (n < PointObjects()) return GetPointObject(n);
	else return GetLineObject(n - PointObjects());
}

//-----------------------------------------------------------------------------
int FEPostModel::PointObjects() const
{
	return m_Points.size();
}

void FEPostModel::AddPointObject(FEPostModel::PointObject* ob)
{
	ob->m_id = (int)m_Points.size();
	m_Points.push_back(ob);

	for (int i = 0; i < m_State.size(); ++i)
	{
		m_State[i]->AddPointObjectData();
	}
}

FEPostModel::PointObject* FEPostModel::GetPointObject(int i)
{
	if ((i < 0) || (i >= m_Points.size())) return nullptr;
	return m_Points[i];
}

//-----------------------------------------------------------------------------
int FEPostModel::LineObjects() const
{
	return (int)m_Lines.size();
}

void FEPostModel::AddLineObject(LineObject* ob)
{
	m_Lines.push_back(ob);
}

FEPostModel::LineObject* FEPostModel::GetLineObject(int i)
{
	return m_Lines[i];
}

//-----------------------------------------------------------------------------
void FEPostModel::ClearObjects()
{
	m_Points.clear();
	m_Lines.clear();
}

//-----------------------------------------------------------------------------
bool FEPostModel::Merge(FEPostModel* fem)
{
	// for now, only works with one mesh
	if ((Meshes() != 1) || (fem->Meshes() != 1)) return false;

	// for now, does not work with data
	if ((m_pDM->DataFields() > 0) || (fem->GetDataManager()->DataFields() > 0)) return false;

	// only single state models
	if ((GetStates() > 1) || (fem->GetStates() > 1)) return false;
	if ((m_RefState.size() > 1) || (fem->m_RefState.size() > 1)) return false;

	// merge materials
	int NMAT0 = Materials();
	int NMAT1 = fem->Materials();
	for (int i = 0; i < NMAT1; ++i) m_Mat.push_back(*fem->GetMaterial(i));

	// get the meshes
	FSMesh& mesh0 = *GetFEMesh(0);
	FSMesh& mesh1 = *fem->GetFEMesh(0);

	int NN0 = mesh0.Nodes();
	int NN1 = mesh1.Nodes();

	int NE0 = mesh0.Elements();
	int NE1 = mesh1.Elements();

	int NN = NN0 + NN1;
	int NE = NE0 + NE1;

	// create new mesh
	mesh0.Create(NN, NE);
	
	// copy new nodes
	for (int i = 0; i < NN1; ++i)
	{
		mesh0.Node(NN0 + i) = mesh1.Node(i);
		mesh0.Node(NN0 + i).SetID(NN0 + i + 1);
	}

	// copy new elements
	for (int i = 0; i < NE1; ++i)
	{
		FSElement& el = mesh0.Element(NE0 + i);
		el = mesh1.Element(i);
		for (int j = 0; j < el.Nodes(); ++j) el.m_node[j] += NN0;
		el.m_gid += NMAT0;
		el.m_MatID += NMAT0;
		el.SetID(NE0 + i + 1);
	}

	mesh0.BuildMesh();

	// update reference state
	FERefState& ref0 = *m_RefState[0];
	FERefState& ref1 = *fem->m_RefState[0];
	ref0.m_Node.resize(NN);
	for (int i = 0; i < NN1; ++i) ref0.m_Node[NN0 + i] = ref1.m_Node[i];

	// update state
	FEState& s0 = *GetState(0);
	s0.RebuildData();

	return true;
}

int FEPostModel::ProjectToMesh(int nstate, const vec3f& r0, vec3d& rt, bool bfollow)
{
	Post::FEState* state = GetState(nstate);
	Post::FERefState* ps = state->m_ref;
	Post::FEPostMesh& mesh = *state->GetFEMesh();

	rt = to_vec3d(r0);

	int nelem = -1;
	vec3f x0[FSElement::MAX_NODES];
	vec3f xt[FSElement::MAX_NODES];
	int nmin = -1;
	double L2min = 0.0;
	vec3f rmin;
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.IsSolid())
		{
			int ne = el.Nodes();
			for (int j = 0; j < el.Nodes(); ++j)
			{
				x0[j] = ps->m_Node[el.m_node[j]].m_rt;
				xt[j] = to_vec3f(mesh.Node(el.m_node[j]).r);
			}

			if (bfollow)
			{
				vec3f q;
				if (ProjectToElement(el, r0, x0, xt, q))
				{
					rt = to_vec3d(q);
					nelem = i;
					break;
				}
			}
			else
			{
				vec3f q;
				if (ProjectToElement(el, r0, x0, x0, q))
				{
					rt = to_vec3d(q);
					nelem = i;
					break;
				}
			}
		}
		else if (el.IsShell() && bfollow)
		{
			int ne = el.Nodes();
			vec3f ri(0, 0, 0);
			for (int j = 0; j < ne; ++j)
			{
				vec3f rj = NodePosition(el.m_node[j], 0);
				ri += rj;
			}
			ri /= ne;

			// get the distance
			double L2 = (ri - r0).SqrLength();

			if ((nmin == -1) || (L2 < L2min))
			{
				nmin = i;
				L2min = L2;
				rmin = ri;
			}
		}
	}

	if ((nelem == -1) && (nmin != -1))
	{
		nelem = nmin;
		vec3d dr = to_vec3d(r0 - rmin);

		FSElement& e = mesh.Element(nmin);
		vec3d a0 = to_vec3d(NodePosition(e.m_node[0], 0));
		vec3d a1 = to_vec3d(NodePosition(e.m_node[1], 0));
		vec3d a2 = to_vec3d(NodePosition(e.m_node[2], 0));

		vec3d e1 = a1 - a0; e1.Normalize();
		vec3d e2 = a2 - a0; e2.Normalize();
		vec3d e3 = e1 ^ e2; e3.Normalize();
		e2 = e3 ^ e1; e2.Normalize();

		mat3d QT(\
			e1.x, e1.y, e1.z, \
			e2.x, e2.y, e2.z, \
			e3.x, e3.y, e3.z	\
		);

		vec3d qr = QT * dr;

		// calculate current position of origin
		vec3d ri(0, 0, 0);
		for (int j = 0; j < e.Nodes(); ++j)
		{
			FSNode& nj = mesh.Node(e.m_node[j]);
			vec3d rj = to_vec3d(NodePosition(e.m_node[j], nstate));
			ri += rj;
		}
		ri /= e.Nodes();

		a0 = to_vec3d(NodePosition(e.m_node[0], nstate));
		a1 = to_vec3d(NodePosition(e.m_node[1], nstate));
		a2 = to_vec3d(NodePosition(e.m_node[2], nstate));

		e1 = a1 - a0; e1.Normalize();
		e2 = a2 - a0; e2.Normalize();
		e3 = e1 ^ e2; e3.Normalize();
		e2 = e3 ^ e1; e2.Normalize();

		mat3d Q(\
			e1.x, e2.x, e3.x, \
			e1.y, e2.y, e3.y, \
			e1.z, e2.z, e3.z	\
		);

		dr = Q * qr;

		rt = ri + dr;
	}

	return nelem;
}


} // namespace Post
