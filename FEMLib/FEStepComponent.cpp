#include "stdafx.h"
#include "FEStepComponent.h"

FSStepComponent::FSStepComponent() 
{ 
	m_bActive = true; 
	m_nstepID = -1;
}

int FSStepComponent::GetStep() { return m_nstepID; }
void FSStepComponent::SetStep(int n) { m_nstepID = n; }

bool FSStepComponent::IsActiveInStep(int nstep)
{
	return ((nstep == -1) || (m_nstepID == 0) || (nstep == m_nstepID));
}

bool FSStepComponent::IsActive() const { return m_bActive; }
void FSStepComponent::Activate(bool bact) { m_bActive = bact; }

