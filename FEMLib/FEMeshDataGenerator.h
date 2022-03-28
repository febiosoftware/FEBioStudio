#pragma once
#include "FEModelComponent.h"
#include "IHasItemList.h"

class FEItemListBuilder;

enum MeshDataGeneratorType
{
	FE_FEBIO_MESHDATA_GENERATOR = 1,
};

class FSMeshDataGenerator : public FSModelComponent, public IHasItemList
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
	FEItemListBuilder* GetItemList() override;
	void SetItemList(FEItemListBuilder* pi) override;

	unsigned int GetMeshItemType() const override;

	void SetMeshItemType(unsigned int meshItem) override;

private:
	int	m_ntype;

private:
	int			m_nUID;	//!< unique ID

	unsigned int	m_itemType;	// the type of mesh item that can be assigned to this list
	FEItemListBuilder* m_pItem;	// list of item indices to apply the BC too

	static	int	m_nref;
};

class FEBioMeshDataGenerator : public FSMeshDataGenerator
{
public:
	FEBioMeshDataGenerator(FSModel* fem = nullptr);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};
