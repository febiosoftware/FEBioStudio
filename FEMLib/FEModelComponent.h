#pragma once
#include "FEBase.h"
#include "FEDataMap.h"
#include <string>

//-----------------------------------------------------------------------------
// Base class for components of an FSModel
class FSModelComponent : public FSCoreBase
{
public:
	FSModelComponent();

	int GetSuperClassID() const;

	// Set the super class ID. This should not be called directly,
	// but now some material classes need to set their super class to FEMATERIALPROP_ID
	void SetSuperClassID(int superClassID);

protected:
	int			m_superClassID;		// super class ID (defined in FECore\fecore_enum.h)
};


void SaveClassMetaData(FSModelComponent* pc, OArchive& ar);
void LoadClassMetaData(FSModelComponent* pc, IArchive& ar);
