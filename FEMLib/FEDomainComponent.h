#pragma once
#include "FEStepComponent.h"
#include <MeshLib/IHasItemList.h>

class FSModel;
class FEItemListBuilder;

//-----------------------------------------------------------------------------
// Base class for anything applied to a partition of the model's geometry.
// This includes, boundary conditions, nodal loads, surface loads, body loads,
// constraints, etc.
class FSDomainComponent : public FSStepComponent, public FSHasOneItemList
{
public:
	enum { NAME, PARAMS, LIST, STEP, SELECTION_TYPE, LIST_ID };

public:
	FSDomainComponent(int ntype, FSModel* ps, int nstep = 0);
	FSDomainComponent(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0);

	virtual ~FSDomainComponent(void);

	int Type() { return m_ntype; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

protected:
	int			m_ntype;	// type of boundary condition
};

//---------------------------------------------------------------------------------------------
// This class is used to select mesh selections properties
class FSMeshSelection : public FSModelComponent, public FSHasOneItemList
{
	enum {LIST_ID};

public:
	FSMeshSelection(FSModel* fem);

	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
