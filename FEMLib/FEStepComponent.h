#pragma once
#include "FEComponent.h"
#include "enums.h"

//-----------------------------------------------------------------------------
// Base class for all objects that are assigned to a step
class FEStepComponent : public FEComponent
{
public:
	FEStepComponent();

	int GetStep();
	void SetStep(int n);

	bool IsActiveInStep(int nstep = -1);
	bool IsActive() const;
	void Activate(bool bact = true);

public:
	void SetTypeString(const char* sztype);
	const char* GetTypeString();

	int GetSuperClassID() const;

protected:
	int		m_nstepID;		// step ID to which this component belongs
	int		m_superClassID;
	bool	m_bActive;		// is this active
	const char*	m_sztype;	// type string
};


void SaveClassMetaData(FEStepComponent* pc, OArchive& ar);
void LoadClassMetaData(FEStepComponent* pc, IArchive& ar);
