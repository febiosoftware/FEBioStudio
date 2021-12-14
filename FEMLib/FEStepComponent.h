#pragma once
#include "FEModelComponent.h"
#include <FECore/fecore_enum.h>

//-----------------------------------------------------------------------------
// Base class for all objects that are assigned to a step
class FSStepComponent : public FSModelComponent
{
public:
	FSStepComponent();

	int GetStep();
	void SetStep(int n);

	bool IsActiveInStep(int nstep = -1);
	bool IsActive() const;
	void Activate(bool bact = true);

protected:
	int		m_nstepID;		// step ID to which this component belongs
	bool	m_bActive;		// is this active
};
