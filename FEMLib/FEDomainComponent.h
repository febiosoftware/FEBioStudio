#pragma once
#include "FEStepComponent.h"

class FSModel;
class FEItemListBuilder;

//-----------------------------------------------------------------------------
// Base class for anything applied to a partition of the model's geometry.
// This includes, boundary conditions, nodal loads, surface loads, body loads,
// constraints, etc.
class FSDomainComponent : public FSStepComponent
{
public:
	enum { NAME, PARAMS, LIST, STEP };

public:
	FSDomainComponent(int ntype, FSModel* ps, int nstep = 0);
	FSDomainComponent(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0);

	virtual ~FSDomainComponent(void);

	int Type() { return m_ntype; }

	FEItemListBuilder* GetItemList() { return m_pItem; }
	void SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	unsigned int GetMeshItemType() const;

	void SetMeshItemType(unsigned int meshItem);

protected:
	unsigned int	m_itemType;	// the type of mesh item that can be assigned to this list

	int			m_ntype;	// type of boundary condition

	FEItemListBuilder*	m_pItem;	// list of item indices to apply the BC too
};
