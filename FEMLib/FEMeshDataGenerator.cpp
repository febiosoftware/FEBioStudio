#include "stdafx.h"
#include "FEMeshDataGenerator.h"
#include <GeomLib/GGroup.h>
#include "FSModel.h"

FSMeshDataGenerator::FSMeshDataGenerator(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
}

int FSMeshDataGenerator::Type() const { return m_ntype; }

void FSMeshDataGenerator::Save(OArchive& ar)
{
	// write list ID
	FSItemListBuilder* pl = GetItemList();
	if (pl) ar.WriteChunk(LIST_ID, pl->GetID());

	ar.BeginChunk(PARAMS);
	{
		FSModelComponent::Save(ar);
	}
	ar.EndChunk();
}

void FSMeshDataGenerator::Load(IArchive& ar)
{
	TRACE("FSMeshDataGenerator::Load");

	FSModel* fem = GetFSModel();
	GModel* pgm = &fem->GetModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case LIST_ID:
		{
			int nid = 0;
			ar.read(nid);
			FSItemListBuilder* pItem = pgm->FindNamedSelection(nid);
			SetItemList(pItem);
		}
		break;
		case PARAMS: FSModelComponent::Load(ar);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
int FSNodeDataGenerator::m_nref = 1;

FSNodeDataGenerator::FSNodeDataGenerator(FSModel* fem, int ntype) : FSMeshDataGenerator(fem, ntype)
{
	m_nUID = m_nref++;
}

int FSNodeDataGenerator::GetID() const { return m_nUID; }
void FSNodeDataGenerator::SetID(int nid) { m_nUID = nid; if (nid >= m_nref) m_nref = nid + 1; }
void FSNodeDataGenerator::ResetCounter() { m_nref = 1; }
void FSNodeDataGenerator::SetCounter(int n) { m_nref = n; }
int FSNodeDataGenerator::GetCounter() { return m_nref; }

//=============================================================================
FEBioNodeDataGenerator::FEBioNodeDataGenerator(FSModel* fem) : FSNodeDataGenerator(fem, FE_FEBIO_NODEDATA_GENERATOR)
{
	SetSuperClassID(FEMESHDATAGENERATOR_ID);
	SetMeshItemType(FE_ALL_FLAGS);
}

void FEBioNodeDataGenerator::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSNodeDataGenerator::Save(ar);
	}
	ar.EndChunk();
}

void FEBioNodeDataGenerator::Load(IArchive& ar)
{
	TRACE("FEBioNodeDataGenerator::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSNodeDataGenerator::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

//=============================================================================
int FSEdgeDataGenerator::m_nref = 1;

FSEdgeDataGenerator::FSEdgeDataGenerator(FSModel* fem, int ntype) : FSMeshDataGenerator(fem, ntype)
{
	m_nUID = m_nref++;
}

int FSEdgeDataGenerator::GetID() const { return m_nUID; }
void FSEdgeDataGenerator::SetID(int nid) { m_nUID = nid; if (nid >= m_nref) m_nref = nid + 1; }
void FSEdgeDataGenerator::ResetCounter() { m_nref = 1; }
void FSEdgeDataGenerator::SetCounter(int n) { m_nref = n; }
int FSEdgeDataGenerator::GetCounter() { return m_nref; }

//=============================================================================
FEBioEdgeDataGenerator::FEBioEdgeDataGenerator(FSModel* fem) : FSEdgeDataGenerator(fem, FE_FEBIO_EDGEDATA_GENERATOR)
{
	SetSuperClassID(FEMESHDATAGENERATOR_ID);
	SetMeshItemType(FE_EDGE_FLAG);
}

void FEBioEdgeDataGenerator::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSEdgeDataGenerator::Save(ar);
	}
	ar.EndChunk();
}

void FEBioEdgeDataGenerator::Load(IArchive& ar)
{
	TRACE("FEBioEdgeDataGenerator::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSEdgeDataGenerator::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

//=============================================================================
int FSFaceDataGenerator::m_nref = 1;

FSFaceDataGenerator::FSFaceDataGenerator(FSModel* fem, int ntype) : FSMeshDataGenerator(fem, ntype)
{
	m_nUID = m_nref++;
}

int FSFaceDataGenerator::GetID() const { return m_nUID; }
void FSFaceDataGenerator::SetID(int nid) { m_nUID = nid; if (nid >= m_nref) m_nref = nid + 1; }
void FSFaceDataGenerator::ResetCounter() { m_nref = 1; }
void FSFaceDataGenerator::SetCounter(int n) { m_nref = n; }
int FSFaceDataGenerator::GetCounter() { return m_nref; }

//=============================================================================
FSConstFaceDataGenerator::FSConstFaceDataGenerator(FSModel* fem) : FSFaceDataGenerator(fem, FE_CONST_FACEDATA_GENERATOR)
{
	SetTypeString("const");
	SetMeshItemType(FE_FACE_FLAG);
}

FSConstFaceDataGenerator::FSConstFaceDataGenerator(FSModel* fem, DATA_TYPE dataType) : FSFaceDataGenerator(fem, FE_CONST_FACEDATA_GENERATOR)
{
	SetTypeString("const");
	SetMeshItemType(FE_FACE_FLAG);

	BuildParameterList((int)dataType);
}

void FSConstFaceDataGenerator::BuildParameterList(int dataType)
{
	Param* p = AddIntParam((int)dataType, "data_type");
	p->SetFlags(FS_PARAM_ATTRIBUTE);
	p->SetVisible(false);
	p->SetEnumNames("scalar\0vec3\0mat3\0");

	switch (dataType)
	{
	case DATA_SCALAR: AddDoubleParam(0.0, "value"); break;
	case DATA_VEC3  : AddVecParam(vec3d(0, 0, 0), "value"); break;
	case DATA_MAT3  : AddMat3dParam(mat3d(), "value"); break;
	default:
		assert(false);
	}
}

void FSConstFaceDataGenerator::Save(OArchive& ar)
{
	int dataType = GetIntValue(0);
	ar.WriteChunk(0, dataType);
	ar.BeginChunk(1);
	{
		FSFaceDataGenerator::Save(ar);
	}
	ar.EndChunk();
}

void FSConstFaceDataGenerator::Load(IArchive& ar)
{
	TRACE("FSConstFaceDataGenerator::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case 0: {
			int dataType = 0;
			ar.read(dataType);
			BuildParameterList(dataType);
		}
		break;
		case 1: FSFaceDataGenerator::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FEBioFaceDataGenerator::FEBioFaceDataGenerator(FSModel* fem) : FSFaceDataGenerator(fem, FE_FEBIO_FACEDATA_GENERATOR)
{
	SetSuperClassID(FEMESHDATAGENERATOR_ID);
	SetMeshItemType(FE_FACE_FLAG);
}

void FEBioFaceDataGenerator::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSFaceDataGenerator::Save(ar);
	}
	ar.EndChunk();
}

void FEBioFaceDataGenerator::Load(IArchive& ar)
{
	TRACE("FEBioFaceDataGenerator::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSFaceDataGenerator::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

//=============================================================================
int FSElemDataGenerator::m_nref = 1;

FSElemDataGenerator::FSElemDataGenerator(FSModel* fem, int ntype) : FSMeshDataGenerator(fem, ntype)
{
	m_nUID = m_nref++;
}

int FSElemDataGenerator::GetID() const { return m_nUID; }
void FSElemDataGenerator::SetID(int nid) { m_nUID = nid; if (nid >= m_nref) m_nref = nid + 1; }
void FSElemDataGenerator::ResetCounter() { m_nref = 1; }
void FSElemDataGenerator::SetCounter(int n) { m_nref = n; }
int FSElemDataGenerator::GetCounter() { return m_nref; }

//=============================================================================
FEBioElemDataGenerator::FEBioElemDataGenerator(FSModel* fem) : FSElemDataGenerator(fem, FE_FEBIO_ELEMDATA_GENERATOR)
{
	SetSuperClassID(FEMESHDATAGENERATOR_ID);
	SetMeshItemType(FE_ELEM_FLAG);
}

void FEBioElemDataGenerator::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSElemDataGenerator::Save(ar);
	}
	ar.EndChunk();

	if (Properties() > 0)
	{
		ar.BeginChunk(CID_PROPERTY_LIST);
		{
			SaveFEBioProperties(this, ar);
		}
		ar.EndChunk();
	}
}

void FEBioElemDataGenerator::Load(IArchive& ar)
{
	TRACE("FEBioElemDataGenerator::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSElemDataGenerator::Load(ar); break;
		case CID_PROPERTY_LIST  : LoadFEBioProperties(this, ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}
