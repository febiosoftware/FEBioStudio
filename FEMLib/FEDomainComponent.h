#pragma once
#include "FEStepComponent.h"
#include "IHasItemList.h"

class FSModel;
class FEItemListBuilder;

//-----------------------------------------------------------------------------
// Base class for anything applied to a partition of the model's geometry.
// This includes, boundary conditions, nodal loads, surface loads, body loads,
// constraints, etc.
class FSDomainComponent : public FSStepComponent, public IHasItemList
{
public:
	enum { NAME, PARAMS, LIST, STEP, SELECTION_TYPE };

public:
	FSDomainComponent(int ntype, FSModel* ps, int nstep = 0);
	FSDomainComponent(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0);

	virtual ~FSDomainComponent(void);

	int Type() { return m_ntype; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

public: // IHasItemList
	FEItemListBuilder* GetItemList() override;
	void SetItemList(FEItemListBuilder* pi) override;
	unsigned int GetMeshItemType() const override;
	void SetMeshItemType(unsigned int meshItem) override;

protected:
	unsigned int	m_itemType;	// the type of mesh item that can be assigned to this list

	int			m_ntype;	// type of boundary condition

	FEItemListBuilder*	m_pItem;	// list of item indices to apply the BC too
};

//---------------------------------------------------------------------------------------------
// This class is used to select mesh selections properties
class FSMeshSelection : public FSModelComponent, public IHasItemList
{
public:
	FSMeshSelection(FSModel* fem);

public: // IHasItemList
	FEItemListBuilder* GetItemList() override;
	void SetItemList(FEItemListBuilder* pi) override;

	unsigned int GetMeshItemType() const override;
	void SetMeshItemType(unsigned int meshItem) override;

private:
	unsigned int		m_itemType;
	FEItemListBuilder*	m_pItem;
};
