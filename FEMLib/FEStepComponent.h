#pragma once
#include "FEComponent.h"

//-----------------------------------------------------------------------------
// Base class for all objects that are assigned to a step
class FEStepComponent : public FEComponent
{
public:
	FEStepComponent() { m_bActive = true; }

	int GetStep() { return m_nstepID; }
	void SetStep(int n) { m_nstepID = n; }

	bool IsActiveInStep(int nstep = -1)
	{
		return ((nstep == -1) || (m_nstepID == 0) || (nstep == m_nstepID));
	}

	bool IsActive() const { return m_bActive; }
	void Activate(bool bact = true) { m_bActive = bact; }

public:
	void SetTypeString(const char* sztype) { m_sztype = sztype; }
	const char* GetTypeString() { return m_sztype; }

protected:
	int		m_nstepID;		// step ID to which this component belongs
	bool	m_bActive;		// is this active
	const char*	m_sztype;	// type string
};
