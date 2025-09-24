#pragma once
#include "FEBase.h"
#include <string>

class FSModel;
class FSLoadController;
class FEElementRef;

//-----------------------------------------------------------------------------
// Base class for components of an FSModel
class FSModelComponent : public FSCoreBase
{
public:
	FSModelComponent(FSModel* fem = nullptr);
	~FSModelComponent();

	int GetSuperClassID() const;

	// Set the super class ID. This should not be called directly,
	// but now some material classes need to set their super class to FEMATERIALPROP_ID
	void SetSuperClassID(int superClassID);

	FSModel* GetFSModel();

public:
	// helper function for retrieving the load controller assigned to a parameter
	FSLoadController* GetLoadController(int n);

	// overridden from ParamContainer to help processing load curves from old fsm files
	void AssignLoadCurve(Param& p, LoadCurve& lc) override;

public:
	void SetFEBioClass(void* pc);
	void* GetFEBioClass();
	bool UpdateData(bool bsave) override;

protected:
	void SaveProperties(OArchive& ar);
	void LoadProperties(IArchive& ar);

protected:
	FSModel*	m_fem;
	int			m_superClassID;		// super class ID (defined in FECore\fecore_enum.h)
	void*		m_febClass;			// The FEBio class
};


void SaveClassMetaData(FSModelComponent* pc, OArchive& ar);
void LoadClassMetaData(FSModelComponent* pc, IArchive& ar);

void SaveFEBioProperties(FSModelComponent* pc, OArchive& ar);
void LoadFEBioProperties(FSModelComponent* pc, IArchive& ar);

class FSGenericClass : public FSModelComponent
{
public:
	FSGenericClass(FSModel* fem);

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

class FSVec3dValuator : public FSGenericClass
{
public:
	FSVec3dValuator(FSModel* fem);

	vec3d GetFiberVector(const FEElementRef& el);

	bool UpdateData(bool bsave) override;

private:
	int		m_naopt;
	int		m_n[2];
	std::string	m_map;
};

class FSMat3dValuator : public FSGenericClass
{
public:
	FSMat3dValuator(FSModel* fem);

	mat3d GetMatAxis(const FEElementRef& el) const;

	bool UpdateData(bool bsave) override;

private:
	int		m_naopt;
	int		m_n[3];
};
