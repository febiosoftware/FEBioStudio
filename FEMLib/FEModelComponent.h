#pragma once
#include "FEStepComponent.h"

class FEModel;
class FEItemListBuilder;

//-----------------------------------------------------------------------------
// Base class for anything applied to a partition of the model's geometry.
// This includes, boundary conditions, nodal loads, surface loads, body loads,
// constraints, etc.
class FEModelComponent : public FEStepComponent
{
public:
	enum { NAME, PARAMS, LIST, STEP };

public:
	FEModelComponent(int ntype, FEModel* ps, int nstep = 0);
	FEModelComponent(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep = 0);

	virtual ~FEModelComponent(void);

	int Type() { return m_ntype; }

	FEItemListBuilder* GetItemList() { return m_pItem; }
	void SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	FEModel* GetFEModel() { return m_ps; }

protected:
	int			m_ntype;	// type of boundary condition
	FEModel*	m_ps;		// pointer to model

	FEItemListBuilder*	m_pItem;	// list of item indices to apply the BC too
};
