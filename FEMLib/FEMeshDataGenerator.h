#pragma once
#include "FEModelComponent.h"

enum MeshDataGeneratorType
{
	FE_FEBIO_MESHDATA_GENERATOR = 1
};

class FSMeshDataGenerator : public FSModelComponent
{
public:
	FSMeshDataGenerator(FSModel* fem, int ntype);

	int Type() const;

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

private:
	int	m_ntype;

private:
	int			m_nUID;	//!< unique ID
	static	int	m_nref;
};

class FEBioMeshDataGenerator : public FSMeshDataGenerator
{
public:
	FEBioMeshDataGenerator(FSModel* fem = nullptr);
};