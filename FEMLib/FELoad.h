#pragma once
#include "FEDomainComponent.h"
#include <MeshLib/FSItemListBuilder.h>

//=============================================================================
// Base class for all nodal, edge, surface, and body loads
class FSLoad : public FSDomainComponent
{
public:
	FSLoad(int ntype, FSModel* fem) : FSDomainComponent(ntype, fem) { m_superClassID = FELOAD_ID; }
	FSLoad(int ntype, FSModel* ps, FSItemListBuilder* pi, int nstep) : FSDomainComponent(ntype, ps, pi, nstep) { m_superClassID = FELOAD_ID; }
};

//=============================================================================
// NODAL LOADS
//=============================================================================

class FSNodalLoad : public FSLoad
{
public:
	FSNodalLoad(int ntype, FSModel* fem) : FSLoad(ntype, fem) 
	{
		SetMeshItemType(FE_ALL_FLAGS);
	}
	FSNodalLoad(int ntype, FSModel* ps, FSItemListBuilder* pi, int nstep) : FSLoad(ntype, ps, pi, nstep) 
	{
		SetMeshItemType(FE_ALL_FLAGS);
	}
};

class FSNodalDOFLoad : public FSNodalLoad
{
public:
	enum { DOF, LOAD };

public:
	FSNodalDOFLoad(FSModel* ps);
	FSNodalDOFLoad(FSModel* ps, FSItemListBuilder* pi, int bc, double f, int nstep = 0);

	int GetDOF() { return GetIntValue(DOF); }
	void SetDOF(int n) { SetIntValue(DOF, n); }

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }
};

class FEBioNodalLoad : public FSNodalLoad
{
public:
	FEBioNodalLoad(FSModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
