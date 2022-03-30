#pragma once
#include "FEBase.h"
#include <string>

class FSModel;
class FSLoadController;

//-----------------------------------------------------------------------------
// Base class for components of an FSModel
class FSModelComponent : public FSCoreBase
{
public:
	FSModelComponent(FSModel* fem = nullptr);

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

protected:
	FSModel*	m_fem;
	int			m_superClassID;		// super class ID (defined in FECore\fecore_enum.h)
};


void SaveClassMetaData(FSModelComponent* pc, OArchive& ar);
void LoadClassMetaData(FSModelComponent* pc, IArchive& ar);

void SaveFEBioProperties(FSModelComponent* pc, OArchive& ar);
void LoadFEBioProperties(FSModelComponent* pc, IArchive& ar);

class FSGenericClass : public FSModelComponent
{
public:
	FSGenericClass();

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
