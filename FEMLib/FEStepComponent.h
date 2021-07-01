#pragma once
#include "FEModelComponent.h"
#include "enums.h"

//-----------------------------------------------------------------------------
// Base class for all objects that are assigned to a step
class FEStepComponent : public FEModelComponent
{
public:
	FEStepComponent();

	int GetStep();
	void SetStep(int n);

	bool IsActiveInStep(int nstep = -1);
	bool IsActive() const;
	void Activate(bool bact = true);

protected:
	int		m_nstepID;		// step ID to which this component belongs
	bool	m_bActive;		// is this active
};
