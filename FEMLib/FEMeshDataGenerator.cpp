#include "stdafx.h"
#include "FEMeshDataGenerator.h"

int FSMeshDataGenerator::m_nref = 1;

FSMeshDataGenerator::FSMeshDataGenerator(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
	m_nUID = m_nref++;
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

}
