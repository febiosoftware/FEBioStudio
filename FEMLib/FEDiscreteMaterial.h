#pragma once
#include "FEMaterial.h"

//===================================================================
// Materials used by discrete element sets
class FSDiscreteMaterial : public FSMaterial
{
public:
	FSDiscreteMaterial(int ntype);
};

//===================================================================
// Linear spring
class FSLinearSpringMaterial : public FSDiscreteMaterial
{
public:
	FSLinearSpringMaterial();

	void SetSpringConstant(double E);

	DECLARE_REGISTERED(FSLinearSpringMaterial);
};

//===================================================================
// Nonlinear spring
class FSNonLinearSpringMaterial : public FSDiscreteMaterial
{
public:
	FSNonLinearSpringMaterial();
	DECLARE_REGISTERED(FSNonLinearSpringMaterial);
};

//===================================================================
// Hill-contractile
class FSHillContractileMaterial : public FSDiscreteMaterial
{
public:
	FSHillContractileMaterial();
	DECLARE_REGISTERED(FSHillContractileMaterial);
};

//===================================================================
// This is not a material, but currently there is no mechanism to define 
// properties in anything except a material, so here we are. 
class FS1DFunction : public FSMaterial
{
public:
	FS1DFunction(int ntype) : FSMaterial(ntype) {}
};

class FS1DPointFunction : public FS1DFunction
{
public:
	FS1DPointFunction();
	LoadCurve* GetPointCurve();
	void SetPointCurve(LoadCurve& lc);
	DECLARE_REGISTERED(FS1DPointFunction);

private:
	LoadCurve	m_lc;
};

//===================================================================
class FEBioDiscreteMaterial : public FSDiscreteMaterial
{
public:
	FEBioDiscreteMaterial();
	~FEBioDiscreteMaterial();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	bool UpdateData(bool bsave) override;

	DECLARE_REGISTERED(FEBioDiscreteMaterial);
};
