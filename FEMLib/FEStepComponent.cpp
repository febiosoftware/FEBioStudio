#include "stdafx.h"
#include "FEStepComponent.h"

FEStepComponent::FEStepComponent() 
{ 
	m_bActive = true; 
	m_nstepID = -1;
}

int FEStepComponent::GetStep() { return m_nstepID; }
void FEStepComponent::SetStep(int n) { m_nstepID = n; }

bool FEStepComponent::IsActiveInStep(int nstep)
{
	return ((nstep == -1) || (m_nstepID == 0) || (nstep == m_nstepID));
}

bool FEStepComponent::IsActive() const { return m_bActive; }
void FEStepComponent::Activate(bool bact) { m_bActive = bact; }

