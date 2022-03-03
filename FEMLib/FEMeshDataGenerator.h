#pragma once
#include "FEModelComponent.h"

class FEItemListBuilder;

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

public:
	FEItemListBuilder* GetItemList() { return m_pItem; }
	void SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

	unsigned int GetMeshItemType() const;

protected:
	void SetMeshItemType(unsigned int meshItem);

private:
	int	m_ntype;

private:
	int			m_nUID;	//!< unique ID

	unsigned int	m_itemType;	// the type of mesh item that can be assigned to this list
	FEItemListBuilder* m_pItem;	// list of item indices to apply the BC too

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
