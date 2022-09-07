#pragma once
#include "FEMaterial.h"

//===================================================================
// Materials used by discrete element sets
class FSDiscreteMaterial : public FSMaterial
{
public:
	FSDiscreteMaterial(int ntype, FSModel* fem);
};

//===================================================================
// Linear spring
class FSLinearSpringMaterial : public FSDiscreteMaterial
{
public:
	FSLinearSpringMaterial(FSModel* fem);

	void SetSpringConstant(double E);

	DECLARE_REGISTERED(FSLinearSpringMaterial);
};

//===================================================================
// Nonlinear spring
class FSNonLinearSpringMaterial : public FSDiscreteMaterial
{
public:
	FSNonLinearSpringMaterial(FSModel* fem);
	DECLARE_REGISTERED(FSNonLinearSpringMaterial);
};

//===================================================================
// Hill-contractile
class FSHillContractileMaterial : public FSDiscreteMaterial
{
public:
	FSHillContractileMaterial(FSModel* fem);
	DECLARE_REGISTERED(FSHillContractileMaterial);
};

//===================================================================
// This is not a material, but currently there is no mechanism to define 
// properties in anything except a material, so here we are. 
class FS1DFunction : public FSMaterial
{
public:
	FS1DFunction(int ntype, FSModel* fem);
};

class FS1DPointFunction : public FS1DFunction
{
public:
	FS1DPointFunction(FSModel* fem);
	LoadCurve* GetPointCurve();
	const LoadCurve* GetPointCurve() const;
	void SetPointCurve(LoadCurve& lc);
	DECLARE_REGISTERED(FS1DPointFunction);

private:
	LoadCurve	m_lc;
};

class FS1DMathFunction : public FS1DFunction
{
public:
	FS1DMathFunction(FSModel* fem);
	DECLARE_REGISTERED(FS1DMathFunction);
};

//===================================================================
class FEBioDiscreteMaterial : public FSDiscreteMaterial
{
public:
	FEBioDiscreteMaterial(FSModel* fem);
	~FEBioDiscreteMaterial();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	bool UpdateData(bool bsave) override;

	// return a string for the material type
	const char* GetTypeString() const override;
	void SetTypeString(const std::string& s) override;

	DECLARE_REGISTERED(FEBioDiscreteMaterial);
};
