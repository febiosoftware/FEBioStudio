// FEModel.cpp: implementation of the FEModel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEModel.h"
#include "FEDataManager.h"
#include "constants.h"
#include "FEMeshData_T.h"
#include <stdio.h>

extern int ET_HEX[12][2];

namespace Post {

FEModel* FEModel::m_pThis = 0;

//=============================================================================
//								F E M O D E L
//=============================================================================
// constructor
FEModel::FEModel()
{
	m_ndisp = 0;
	m_pDM = new FEDataManager(this);
	m_ntime = 0;

	m_pThis = this;
}

//-----------------------------------------------------------------------------
// desctructor
FEModel::~FEModel()
{
	Clear();
	delete m_pDM;
	if (m_pThis == this) m_pThis = 0;

	DeleteMeshes();

	ClearDependants();
}

void FEModel::DeleteMeshes()
{
	// delete all meshes
	for (size_t i = 0; i<m_mesh.size(); ++i) delete m_mesh[i];
	m_mesh.clear();
}

void FEModel::SetInstance(FEModel* fem)
{
	m_pThis = fem;
}

FEModel* FEModel::GetInstance()
{
	return m_pThis;
}

//-----------------------------------------------------------------------------
void FEModel::AddMesh(FEMeshBase* mesh)
{
	m_mesh.push_back(mesh);
}

//-----------------------------------------------------------------------------
int FEModel::Meshes() const
{
	return (int) m_mesh.size();
}

//-----------------------------------------------------------------------------
FEMeshBase* FEModel::GetFEMesh(int n)
{
	return m_mesh[n];
}

//-----------------------------------------------------------------------------
// Clear the data of the model
// TODO: This does not delete the mesh. Should I?
void FEModel::Clear()
{
	DeleteMeshes();

	m_Mat.clear();
	ClearStates();
	
	m_ntime = 0;
	m_title.clear();
	m_name.clear();
}

//-----------------------------------------------------------------------------
void FEModel::SetTitle(const string& title)
{
	m_title = title;
}

//-----------------------------------------------------------------------------
const string& FEModel::GetTitle() const
{
	return m_title;
}

//-----------------------------------------------------------------------------
void FEModel::SetName(const std::string& name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
const string& FEModel::GetName() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
// add a material to the model
void FEModel::AddMaterial(FEMaterial& mat)
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
void FEModel::ClearStates()
{
	for (int i=0; i<(int) m_State.size(); i++) delete m_State[i];
	m_State.clear();
}

//-----------------------------------------------------------------------------
void FEModel::AddState(FEState* pFEState)
{
	pFEState->SetID((int) m_State.size());
	m_State.push_back(pFEState); 
}

//-----------------------------------------------------------------------------
// add a state
void FEModel::AddState(float ftime)
{
	vector<FEState*>::iterator it = m_State.begin();
	for (it = m_State.begin(); it != m_State.end(); ++it)
		if ((*it)->m_time > ftime)
		{
			m_State.insert(it, new FEState(ftime, this, (*it)->GetFEMesh()));
			return;
		}

	// get last state
	FEState* ps = GetState(GetStates()-1);
	ps->SetID((int)m_State.size());
	m_State.push_back(new FEState(ftime, this, ps->GetFEMesh()));
}

//-----------------------------------------------------------------------------
// delete a state
void FEModel::DeleteState(int n)
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
void FEModel::InsertState(FEState *ps, float f)
{
	vector<FEState*>::iterator it = m_State.begin();
	for (it=m_State.begin(); it != m_State.end(); ++it)
		if ((*it)->m_time > f) 
		{
			m_State.insert(it, ps);
			return;
		}
	m_State.push_back(ps);

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
template <typename Type, Data_Format Fmt> void copy_elem_data(FEMeshData& d, FEMeshData& s)
{
	FEElementData<Type, Fmt>& dt = dynamic_cast<FEElementData<Type, Fmt>&>(d);
	FEElementData<Type, Fmt>& st = dynamic_cast<FEElementData<Type, Fmt>&>(s);
	dt.copy(st);
}

//-----------------------------------------------------------------------------
template <typename Type, Data_Format Fmt> void copy_face_data(FEMeshData& d, FEMeshData& s)
{
	FEFaceData<Type, Fmt>& dt = dynamic_cast<FEFaceData<Type, Fmt>&>(d);
	FEFaceData<Type, Fmt>& st = dynamic_cast<FEFaceData<Type, Fmt>&>(s);
	dt.copy(st);
}

//-----------------------------------------------------------------------------
// Copy a data field
FEDataField* FEModel::CopyDataField(FEDataField* pd, const char* sznewname)
{
	// Clone the data field
	FEDataField* pdcopy = pd->Clone();

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
			if      (pd->Type() == DATA_FLOAT ) copy_node_data<float >(dst, src);
			else if (pd->Type() == DATA_VEC3F ) copy_node_data<vec3f >(dst, src);
			else if (pd->Type() == DATA_MAT3FS) copy_node_data<mat3fs>(dst, src);
			else if (pd->Type() == DATA_MAT3FD) copy_node_data<mat3fd>(dst, src);
			else if (pd->Type() == DATA_MAT3F ) copy_node_data<mat3f >(dst, src);
			else if (pd->Type() == DATA_MAT3D ) copy_node_data<Mat3d >(dst, src);
		}
		else if (IS_FACE_FIELD(pd->GetFieldID()))
		{
			switch (pd->Format())
			{
			case DATA_ITEM:
				{
					if      (pd->Type() == DATA_FLOAT ) copy_face_data<float , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_VEC3F ) copy_face_data<vec3f , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_MAT3FS) copy_face_data<mat3fs, DATA_ITEM>(dst, src);
				}
				break;
			case DATA_NODE:
				{
					if      (pd->Type() == DATA_FLOAT ) copy_face_data<float , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_VEC3F ) copy_face_data<vec3f , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_MAT3FS) copy_face_data<mat3fs, DATA_NODE>(dst, src);
				}
				break;
			case DATA_COMP:
				{
					if      (pd->Type() == DATA_FLOAT ) copy_face_data<float , DATA_COMP>(dst, src);
					else if (pd->Type() == DATA_VEC3F ) copy_face_data<vec3f , DATA_COMP>(dst, src);
					else if (pd->Type() == DATA_MAT3FS) copy_face_data<mat3fs, DATA_COMP>(dst, src);
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
					if      (pd->Type() == DATA_FLOAT ) copy_elem_data<float , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_VEC3F ) copy_elem_data<vec3f , DATA_ITEM>(dst, src);
					else if (pd->Type() == DATA_MAT3FS) copy_elem_data<mat3fs, DATA_ITEM>(dst, src);
				}
				break;
			case DATA_NODE:
				{
					if      (pd->Type() == DATA_FLOAT ) copy_elem_data<float , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_VEC3F ) copy_elem_data<vec3f , DATA_NODE>(dst, src);
					else if (pd->Type() == DATA_MAT3FS) copy_elem_data<mat3fs, DATA_NODE>(dst, src);
				}
				break;
			case DATA_COMP:
				{
					if      (pd->Type() == DATA_FLOAT ) copy_elem_data<float , DATA_COMP>(dst, src);
					else if (pd->Type() == DATA_VEC3F ) copy_elem_data<vec3f , DATA_COMP>(dst, src);
					else if (pd->Type() == DATA_MAT3FS) copy_elem_data<mat3fs, DATA_COMP>(dst, src);
				}
				break;
			}
		}
	}

	return pdcopy;
}

//-----------------------------------------------------------------------------
FEDataField* createCachedDataField(FEDataField* pd, const char* sznewname)
{
	Data_Class nclass = pd->DataClass();
	Data_Type ntype = pd->Type();
	Data_Format nfmt = pd->Format();

	FEDataField* newField = 0;
	if (nclass == CLASS_NODE)
	{
		if      (ntype == DATA_FLOAT ) newField = new FEDataField_T<FENodeData<float > >(sznewname);
		else if (ntype == DATA_VEC3F ) newField = new FEDataField_T<FENodeData<vec3f > >(sznewname);
		else if (ntype == DATA_MAT3FS) newField = new FEDataField_T<FENodeData<mat3fs> >(sznewname);
		else if (ntype == DATA_MAT3FD) newField = new FEDataField_T<FENodeData<mat3fd> >(sznewname);
		else if (ntype == DATA_MAT3D ) newField = new FEDataField_T<FENodeData<Mat3d > >(sznewname);
		else if (ntype == DATA_MAT3F ) newField = new FEDataField_T<FENodeData<mat3f > >(sznewname);
		else assert(false);
	}
	else if (nclass == CLASS_ELEM)
	{
		if (ntype == DATA_FLOAT)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEElementData<float, DATA_NODE  > >(sznewname);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEElementData<float, DATA_ITEM  > >(sznewname);
			else if (nfmt == DATA_COMP  ) newField = new FEDataField_T<FEElementData<float, DATA_COMP  > >(sznewname);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEElementData<float, DATA_REGION> >(sznewname);
			else assert(false);
		}
		else if (ntype == DATA_VEC3F)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEElementData<vec3f, DATA_NODE  > >(sznewname);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEElementData<vec3f, DATA_ITEM  > >(sznewname);
			else if (nfmt == DATA_COMP  ) newField = new FEDataField_T<FEElementData<vec3f, DATA_COMP  > >(sznewname);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEElementData<vec3f, DATA_REGION> >(sznewname);
			else assert(false);
		}
		else if (ntype == DATA_MAT3FS)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEElementData<mat3fs, DATA_NODE  > >(sznewname);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEElementData<mat3fs, DATA_ITEM  > >(sznewname);
			else if (nfmt == DATA_COMP  ) newField = new FEDataField_T<FEElementData<mat3fs, DATA_COMP  > >(sznewname);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEElementData<mat3fs, DATA_REGION> >(sznewname);
			else assert(false);
		}
		else assert(false);
	}
	else if (nclass == CLASS_FACE)
	{
		if (ntype == DATA_FLOAT)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEFaceData<float, DATA_NODE  > >(sznewname);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEFaceData<float, DATA_ITEM  > >(sznewname);
			else if (nfmt == DATA_COMP  ) newField = new FEDataField_T<FEFaceData<float, DATA_COMP  > >(sznewname);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEFaceData<float, DATA_REGION> >(sznewname);
			else assert(false);
		}
		else if (ntype == DATA_VEC3F)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEFaceData<vec3f, DATA_NODE  > >(sznewname);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEFaceData<vec3f, DATA_ITEM  > >(sznewname);
			else if (nfmt == DATA_COMP  ) newField = new FEDataField_T<FEFaceData<vec3f, DATA_COMP  > >(sznewname);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEFaceData<vec3f, DATA_REGION> >(sznewname);
			else assert(false);
		}
		else if (ntype == DATA_MAT3FS)
		{
			if      (nfmt == DATA_NODE  ) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_NODE  > >(sznewname);
			else if (nfmt == DATA_ITEM  ) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_ITEM  > >(sznewname);
			else if (nfmt == DATA_COMP  ) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_COMP  > >(sznewname);
			else if (nfmt == DATA_REGION) newField = new FEDataField_T<FEFaceData<mat3fs, DATA_REGION> >(sznewname);
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

template <typename T> void cached_copy_face_data_COMP(FEMeshData& dst, FEMeshData& src, FEMeshBase& mesh)
{
	FEFaceData<T, DATA_COMP>& d = dynamic_cast<FEFaceData<T, DATA_COMP>&>(dst);
	FEFaceData_T<T, DATA_COMP>& s = dynamic_cast<FEFaceData_T<T, DATA_COMP>&>(src);

	int NF = mesh.Faces();
	T f[FEFace::MAX_NODES];
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = mesh.Face(i);
		if (s.active(i))
		{
			int nf = face.Nodes();
			s.eval(i, f);
			d.add(i, f, nf);
		}
	}
}

template <typename T> void cached_copy_face_data_NODE(FEMeshData& dst, FEMeshData& src, FEMeshBase& mesh)
{
	FEFaceData<T, DATA_NODE>& d = dynamic_cast<FEFaceData<T, DATA_NODE>&>(dst);
	FEFaceData_T<T, DATA_NODE>& s = dynamic_cast<FEFaceData_T<T, DATA_NODE>&>(src);

	int NF = mesh.Faces();
	T f[FEFace::MAX_NODES];
	vector<T> vf;
	vector<int> faceList(1);
	vector<int> index;
	vector<int> faceSize(1);
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = mesh.Face(i);
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

template <typename T> void cached_copy_elem_data_COMP(FEMeshData& dst, FEMeshData& src, FEMeshBase& mesh)
{
	FEElementData<T, DATA_COMP>& d = dynamic_cast<FEElementData<T, DATA_COMP>&>(dst);
	FEElemData_T<T, DATA_COMP>& s = dynamic_cast<FEElemData_T<T, DATA_COMP>&>(src);

	int NE = mesh.Elements();
	T f[FEGenericElement::MAX_NODES];
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		if (s.active(i))
		{
			int ne = el.Nodes();
			s.eval(i, f);
			d.add(i, ne, f);
		}
	}
}

template <typename T> void cached_copy_elem_data_NODE(FEMeshData& dst, FEMeshData& src, FEMeshBase& mesh)
{
	FEElementData<T, DATA_NODE>& d = dynamic_cast<FEElementData<T, DATA_NODE>&>(dst);
	FEElemData_T<T, DATA_NODE>& s = dynamic_cast<FEElemData_T<T, DATA_NODE>&>(src);

	int NE = mesh.Elements();
	T f[FEGenericElement::MAX_NODES];
	vector<T> vf;
	vector<int> elem(1);
	vector<int> index;
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
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
FEDataField* FEModel::CreateCachedCopy(FEDataField* pd, const char* sznewname)
{
	// create a new data field that will store a cached copy
	FEDataField* pdcopy = createCachedDataField(pd, sznewname);
	if (pdcopy == 0) return 0;

	// Add it to the model
	AddDataField(pdcopy);

	// get the field ID codes
	int ndst = FIELD_CODE(pdcopy->GetFieldID());
	int nsrc = FIELD_CODE(pd->GetFieldID());

	// get the data info
	Data_Class nclass = pd->DataClass();
	Data_Type ntype = pd->Type();
	Data_Format nfmt = pd->Format();

	// loop over all the states
	FEMeshBase& mesh = *GetFEMesh(0);
	int nstates = GetStates();
	for (int i = 0; i<nstates; ++i)
	{
		FEState& state = *GetState(i);
		FEMeshDataList& DL = state.m_Data;

		// get source and destination fields
		FEMeshData& dst = DL[ndst];
		FEMeshData& src = DL[nsrc];

		// copy data
		if (nclass == CLASS_NODE)
		{
			int NN = mesh.Nodes();
			if      (ntype == DATA_FLOAT ) cached_copy_node_data<float >(dst, src, NN);
			else if (ntype == DATA_VEC3F ) cached_copy_node_data<vec3f >(dst, src, NN);
			else if (ntype == DATA_MAT3FS) cached_copy_node_data<mat3fs>(dst, src, NN);
			else if (ntype == DATA_MAT3FD) cached_copy_node_data<mat3fd>(dst, src, NN);
			else assert(false);
		}
		else if (nclass == CLASS_FACE)
		{
			int NF = mesh.Faces();
			if (nfmt == DATA_ITEM)
			{
				if      (ntype == DATA_FLOAT ) cached_copy_face_data_ITEM<float >(dst, src, NF);
				else if (ntype == DATA_VEC3F ) cached_copy_face_data_ITEM<vec3f >(dst, src, NF);
				else if (ntype == DATA_MAT3FS) cached_copy_face_data_ITEM<mat3fs>(dst, src, NF);
				else if (ntype == DATA_MAT3FD) cached_copy_face_data_ITEM<mat3fd>(dst, src, NF);
				else assert(false);
			}
			else if (nfmt == DATA_COMP)
			{
				if      (ntype == DATA_FLOAT ) cached_copy_face_data_COMP<float >(dst, src, mesh);
				else if (ntype == DATA_FLOAT ) cached_copy_face_data_COMP<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3FS) cached_copy_face_data_COMP<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3FD) cached_copy_face_data_COMP<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else if (nfmt == DATA_NODE)
			{
				if      (ntype == DATA_FLOAT ) cached_copy_face_data_NODE<float >(dst, src, mesh);
				else if (ntype == DATA_FLOAT ) cached_copy_face_data_NODE<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3FS) cached_copy_face_data_NODE<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3FD) cached_copy_face_data_NODE<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else assert(false);
		}
		else if (nclass == CLASS_ELEM)
		{
			int NE = mesh.Elements();
			if (nfmt == DATA_ITEM)
			{
				if      (ntype == DATA_FLOAT ) cached_copy_elem_data_ITEM<float >(dst, src, NE);
				else if (ntype == DATA_VEC3F ) cached_copy_elem_data_ITEM<vec3f >(dst, src, NE);
				else if (ntype == DATA_MAT3FS) cached_copy_elem_data_ITEM<mat3fs>(dst, src, NE);
				else if (ntype == DATA_MAT3FD) cached_copy_elem_data_ITEM<mat3fd>(dst, src, NE);
				else assert(false);
			}
			else if (nfmt == DATA_COMP)
			{
				if      (ntype == DATA_FLOAT ) cached_copy_elem_data_COMP<float >(dst, src, mesh);
				else if (ntype == DATA_FLOAT ) cached_copy_elem_data_COMP<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3FS) cached_copy_elem_data_COMP<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3FD) cached_copy_elem_data_COMP<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else if (nfmt == DATA_NODE)
			{
				if      (ntype == DATA_FLOAT ) cached_copy_elem_data_NODE<float >(dst, src, mesh);
				else if (ntype == DATA_VEC3F ) cached_copy_elem_data_NODE<vec3f >(dst, src, mesh);
				else if (ntype == DATA_MAT3FS) cached_copy_elem_data_NODE<mat3fs>(dst, src, mesh);
				else if (ntype == DATA_MAT3FD) cached_copy_elem_data_NODE<mat3fd>(dst, src, mesh);
				else assert(false);
			}
			else if (nfmt == DATA_REGION)
			{
				if      (ntype == DATA_FLOAT ) cached_copy_elem_data_REGION<float >(dst, src, NE);
				else if (ntype == DATA_VEC3F ) cached_copy_elem_data_REGION<vec3f >(dst, src, NE);
				else if (ntype == DATA_MAT3FS) cached_copy_elem_data_REGION<mat3fs>(dst, src, NE);
				else if (ntype == DATA_MAT3FD) cached_copy_elem_data_REGION<mat3fd>(dst, src, NE);
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
std::string FEModel::getDataString(int ndata, Data_Tensor_Type ntype)
{
	FEDataManager& dm = *GetDataManager();
	return dm.getDataString(ndata, ntype);
}

//-----------------------------------------------------------------------------
// Delete a data field
void FEModel::DeleteDataField(FEDataField* pd)
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
void FEModel::AddDataField(FEDataField* pd)
{
	// add the data field to the data manager
	m_pDM->AddDataField(pd);

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
void FEModel::AddDataField(FEDataField* pd, vector<int>& L)
{
	assert(pd->DataClass() == CLASS_FACE);

	// add the data field to the data manager
	m_pDM->AddDataField(pd);

	// now add new meshdata for each of the states
	vector<FEState*>::iterator it;
	for (it=m_State.begin(); it != m_State.end(); ++it)
	{
		FEFaceItemData* pmd = dynamic_cast<FEFaceItemData*>(pd->CreateData(*it));
		(*it)->m_Data.push_back(pmd);
		if (dynamic_cast<FECurvature*>(pmd))
		{
			FECurvature* pcrv = dynamic_cast<FECurvature*>(pmd);
			pcrv->set_facelist(L);
		}
		if (dynamic_cast<FECongruency*>(pmd))
		{
			FECongruency* pcon = dynamic_cast<FECongruency*>(pmd);
			pcon->set_facelist(L);
		}
	}

	// update all dependants
	UpdateDependants();
}

//-----------------------------------------------------------------------------
// This function calculates the position of a node based on the selected
// displacement field.
vec3f FEModel::NodePosition(int n, int ntime)
{
	vec3f r;
	if (ntime >= 0)
	{
		FEMeshBase* mesh = GetState(ntime)->GetFEMesh();
		r = mesh->Node(n).m_r0;
		if (m_ndisp) r += EvaluateNodeVector(n, ntime, m_ndisp);
	}
	else
	{
		FEMeshBase* mesh = GetFEMesh(0);
		r = mesh->Node(n).m_r0;
	}

	return r;
}

//-----------------------------------------------------------------------------
vec3f FEModel::NodePosition(const vec3f& r, int ntime)
{
	FEMeshBase* mesh = GetState(ntime)->GetFEMesh();

	// find the element in which this node lies
	int iel = -1; double iso[3] = {0};
	if (FindElementInReferenceFrame(*mesh, r, iel, iso))
	{
		vec3f x[FEGenericElement::MAX_NODES];
		GetElementCoords(iel, ntime, x);

		// evaluate 
		FEElement& el = mesh->Element(iel);

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
vec3f FEModel::FaceNormal(FEFace& f, int ntime)
{
	vec3f r0 = NodePosition(f.node[0], ntime);
	vec3f r1 = NodePosition(f.node[1], ntime);
	vec3f r2 = NodePosition(f.node[2], ntime);
	vec3f fn = (r1 - r0)^(r2 - r0);
	fn.Normalize();
	return fn;
}

//-----------------------------------------------------------------------------
// get the nodal coordinates of an element at time n
void FEModel::GetElementCoords(int iel, int ntime, vec3f* r)
{
	FEMeshBase* mesh = GetState(ntime)->GetFEMesh();
	FEElement& elem = mesh->Element(iel);
	NODEDATA* pn = &m_State[ntime]->m_NODE[0];

	for (int i=0; i<elem.Nodes(); i++)
		r[i] = pn[ elem.m_node[i] ].m_rt;
}

//-----------------------------------------------------------------------------
// Update the bounding box of the mesh. Note that this box bounds the reference
// configuration, not the current configuration
void FEModel::UpdateBoundingBox()
{
	FEMeshBase* mesh = GetFEMesh(0);
	FENode& n = mesh->Node(0);
	m_bbox.x0 = m_bbox.x1 = n.m_r0.x;
	m_bbox.y0 = m_bbox.y1 = n.m_r0.y;
	m_bbox.z0 = m_bbox.z1 = n.m_r0.z;

	int N = mesh->Nodes();
	for (int i=0; i<N; i++)
	{
		FENode& n = mesh->Node(i);
		if (n.m_r0.x < m_bbox.x0) m_bbox.x0 = n.m_r0.x;
		if (n.m_r0.y < m_bbox.y0) m_bbox.y0 = n.m_r0.y;
		if (n.m_r0.z < m_bbox.z0) m_bbox.z0 = n.m_r0.z;

		if (n.m_r0.x > m_bbox.x1) m_bbox.x1 = n.m_r0.x;
		if (n.m_r0.y > m_bbox.y1) m_bbox.y1 = n.m_r0.y;
		if (n.m_r0.z > m_bbox.z1) m_bbox.z1 = n.m_r0.z;
	}
}

//-----------------------------------------------------------------------------
void FEModel::AddDependant(FEModelDependant* pc)
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
void FEModel::UpdateDependants()
{
	int N = m_Dependants.size();
	for (int i=0; i<N; ++i) m_Dependants[i]->Update(this);
}

//-----------------------------------------------------------------------------
void FEModel::RemoveDependant(FEModelDependant* pc)
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
void FEModel::ClearDependants()
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
void FEModel::UpdateMeshState(int ntime)
{
	FEState& state = *GetState(ntime);

	FEMeshBase* mesh = state.GetFEMesh();
	int NE = mesh->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FEElement& el = mesh->Element(i);
		ELEMDATA& data = state.m_ELEM[i];

		if ((data.m_state & StatusFlags::VISIBLE) == 0)
		{
			el.SetEroded(true);
		}
		else el.SetEroded(false);
	}
}
}
