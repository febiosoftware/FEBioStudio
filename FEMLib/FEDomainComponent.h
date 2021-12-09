#pragma once
#include "FEStepComponent.h"

class FSModel;
class FEItemListBuilder;

//-----------------------------------------------------------------------------
// Base class for anything applied to a partition of the model's geometry.
// This includes, boundary conditions, nodal loads, surface loads, body loads,
// constraints, etc.
class FEDomainComponent : public FEStepComponent
{
public:
	enum { NAME, PARAMS, LIST, STEP };

public:
	FEDomainComponent(int ntype, FSModel* ps, int nstep = 0);
	FEDomainComponent(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0);

	virtual ~FEDomainComponent(void);

	int Type() { return m_ntype; }

	FEItemListBuilder* GetItemList() { return m_pItem; }
	void SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	FSModel* GetFSModel() { return m_ps; }

	unsigned int GetMeshItemType() const;

	void SetMeshItemType(unsigned int meshItem);

protected:
	unsigned int	m_itemType;	// the type of mesh item that can be assigned to this list

	int			m_ntype;	// type of boundary condition
	FSModel*	m_ps;		// pointer to model

	FEItemListBuilder*	m_pItem;	// list of item indices to apply the BC too
};
