#pragma once
#include "FEModelComponent.h"

enum MeshDataGeneratorType
{
	FE_FEBIO_NODEDATA_GENERATOR = 1,
	FE_FEBIO_FACEDATA_GENERATOR = 2,
	FE_FEBIO_ELEMDATA_GENERATOR = 3
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

class FEBioNodeDataGenerator : public FSMeshDataGenerator
{
public:
	FEBioNodeDataGenerator(FSModel* fem = nullptr);
};

class FEBioFaceDataGenerator : public FSMeshDataGenerator
{
public:
	FEBioFaceDataGenerator(FSModel* fem = nullptr);
};

class FEBioElemDataGenerator : public FSMeshDataGenerator
{
public:
	FEBioElemDataGenerator(FSModel* fem = nullptr);
};
