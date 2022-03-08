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
FEBioNodeDataGenerator::FEBioNodeDataGenerator(FSModel* fem) : FSMeshDataGenerator(fem, FE_FEBIO_NODEDATA_GENERATOR)
{
	SetMeshItemType(FE_ALL_FLAGS);
}

//=============================================================================
FEBioFaceDataGenerator::FEBioFaceDataGenerator(FSModel* fem) : FSMeshDataGenerator(fem, FE_FEBIO_FACEDATA_GENERATOR)
{
	SetMeshItemType(FE_FACE_FLAG);
}

//=============================================================================
FEBioElemDataGenerator::FEBioElemDataGenerator(FSModel* fem) : FSMeshDataGenerator(fem, FE_FEBIO_ELEMDATA_GENERATOR)
{
	SetMeshItemType(FE_ELEM_FLAG);
}
