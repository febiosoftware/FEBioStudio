#include "stdafx.h"
#include "FEMeshDataGenerator.h"
#include <MeshTools/GGroup.h>

FSMeshDataGenerator::FSMeshDataGenerator(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
	m_pItem = nullptr;
	m_itemType = 0;
}

int FSMeshDataGenerator::Type() const { return m_ntype; }

FEItemListBuilder* FSMeshDataGenerator::GetItemList() { return m_pItem; }
void FSMeshDataGenerator::SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

unsigned int FSMeshDataGenerator::GetMeshItemType() const
{
	return m_itemType;
}

void FSMeshDataGenerator::SetMeshItemType(unsigned int meshItem)
{
	m_itemType = meshItem;
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
	SetSuperClassID(FENODEDATAGENERATOR_ID);
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
	SetSuperClassID(FEEDGEDATAGENERATOR_ID);
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
FEBioFaceDataGenerator::FEBioFaceDataGenerator(FSModel* fem) : FSFaceDataGenerator(fem, FE_FEBIO_FACEDATA_GENERATOR)
{
	SetSuperClassID(FEFACEDATAGENERATOR_ID);
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
	SetSuperClassID(FEELEMDATAGENERATOR_ID);
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
}

void FEBioElemDataGenerator::Load(IArchive& ar)
{
	TRACE("FEBioFaceDataGenerator::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSElemDataGenerator::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}
