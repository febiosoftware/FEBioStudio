#pragma once
#include "FEMaterial.h"

//===================================================================
// Materials used by discrete element sets
class FEDiscreteMaterial : public FEMaterial
{
public:
	FEDiscreteMaterial(int ntype);
};

//===================================================================
// Linear spring
class FELinearSpringMaterial : public FEDiscreteMaterial
{
public:
	FELinearSpringMaterial();

	void SetSpringConstant(double E);

	DECLARE_REGISTERED(FELinearSpringMaterial);
};

//===================================================================
// Nonlinear spring
class FENonLinearSpringMaterial : public FEDiscreteMaterial
{
public:
	FENonLinearSpringMaterial();
	DECLARE_REGISTERED(FENonLinearSpringMaterial);
};

//===================================================================
// Hill-contractile
class FEHillContractileMaterial : public FEDiscreteMaterial
{
public:
	FEHillContractileMaterial();
	DECLARE_REGISTERED(FEHillContractileMaterial);
};

//===================================================================
// This is not a material, but currently there is no mechanism to define 
// properties in anything except a material, so here we are. 
class FE1DFunction : public FEMaterial
{
public:
	FE1DFunction(int ntype) : FEMaterial(ntype) {}
};

class FE1DPointFunction : public FE1DFunction
{
public:
	FE1DPointFunction();
	FELoadCurve* GetPointCurve();
	void SetPointCurve(FELoadCurve& lc);
	DECLARE_REGISTERED(FE1DPointFunction);
};

//===================================================================
class FEBioDiscreteMaterial : public FEDiscreteMaterial
{
public:
	FEBioDiscreteMaterial();
	~FEBioDiscreteMaterial();

	void SetTypeString(const char* sz) override;
	const char* GetTypeString() override;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void SetFEBioMaterial(FEBio::FEBioClass* febClass);
	FEBio::FEBioClass* GetFEBioMaterial();

	bool UpdateData(bool bsave) override;

	DECLARE_REGISTERED(FEBioDiscreteMaterial);

private:
	std::string	m_stype;

	FEBio::FEBioClass* m_febClass;	// pointer to FEBio interface class.
};
