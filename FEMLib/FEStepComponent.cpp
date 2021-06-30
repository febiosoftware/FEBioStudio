#include "stdafx.h"
#include "FEStepComponent.h"
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>

FEStepComponent::FEStepComponent() 
{ 
	m_bActive = true; 
	m_nstepID = -1;
	m_sztype = nullptr;
	m_superClassID = -1;
}

int FEStepComponent::GetStep() { return m_nstepID; }
void FEStepComponent::SetStep(int n) { m_nstepID = n; }

bool FEStepComponent::IsActiveInStep(int nstep)
{
	return ((nstep == -1) || (m_nstepID == 0) || (nstep == m_nstepID));
}

bool FEStepComponent::IsActive() const { return m_bActive; }
void FEStepComponent::Activate(bool bact) { m_bActive = bact; }

void FEStepComponent::SetTypeString(const char* sztype) { m_sztype = sztype; }
const char* FEStepComponent::GetTypeString() { return m_sztype; }

int FEStepComponent::GetSuperClassID() const { return m_superClassID; }

//==============================================================================
void SaveClassMetaData(FEStepComponent* pc, OArchive& ar)
{
	string typeStr(pc->GetTypeString());
	int superClassId = pc->GetSuperClassID(); assert(superClassId > 0);
	ar.WriteChunk(CID_FEBIO_SUPER_CLASS, superClassId);
	ar.WriteChunk(CID_FEBIO_TYPE_STRING, typeStr);
}

void LoadClassMetaData(FEStepComponent* pc, IArchive& ar)
{
	TRACE("LoadClassMetaData");
	int superClassId = -1;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_SUPER_CLASS: ar.read(superClassId); break;
		case CID_FEBIO_TYPE_STRING:
		{
			string typeStr;
			ar.read(typeStr);
			assert(superClassId != -1);
			FEBio::CreateStepComponent(superClassId, typeStr, pc);
		}
		break;
		}
		ar.CloseChunk();
	}
}
