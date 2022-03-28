#include "stdafx.h"
#include "FEMeshDataGenerator.h"
#include <MeshTools/GGroup.h>

int FSMeshDataGenerator::m_nref = 1;

FSMeshDataGenerator::FSMeshDataGenerator(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
	m_nUID = m_nref++;
	m_pItem = nullptr;
	m_itemType = 0;
}

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

int FSMeshDataGenerator::Type() const
{
	return m_ntype;
}

int FSMeshDataGenerator::GetID() const
{
	return m_nUID;
}
	
void FSMeshDataGenerator::SetID(int nid)
{
	m_nUID = nid;
	if (nid >= m_nref) m_nref = nid + 1;
}

void FSMeshDataGenerator::ResetCounter()
{
	m_nref = 1;
}

void FSMeshDataGenerator::SetCounter(int n)
{
	m_nref = n;
}
	
int FSMeshDataGenerator::GetCounter()
{
	return m_nref;
}

//=============================================================================
FEBioMeshDataGenerator::FEBioMeshDataGenerator(FSModel* fem) : FSMeshDataGenerator(fem, FE_FEBIO_MESHDATA_GENERATOR)
{
	SetSuperClassID(FEMESHDATAGENERATOR_ID);
	SetMeshItemType(FE_ALL_FLAGS);
}

void FEBioMeshDataGenerator::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSMeshDataGenerator::Save(ar);
	}
	ar.EndChunk();
}

void FEBioMeshDataGenerator::Load(IArchive& ar)
{
	TRACE("FEBioMeshDataGenerator::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSMeshDataGenerator::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}
