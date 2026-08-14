#include "FSCoreStudy.h"
#include <FECore/fecore_enum.h>

FSCoreStudy::FSCoreStudy(FSModel* fem) : FSModelComponent(fem) 
{
	SetSuperClassID(FETASK_ID);
}

void FSCoreStudy::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSModelComponent::Save(ar);
	}
	ar.EndChunk();

	if (Properties() > 0)
	{
		ar.BeginChunk(CID_PROPERTY_LIST);
		{
			SaveFEBioProperties(this, ar);
		}
		ar.EndChunk();
	}
}

void FSCoreStudy::Load(IArchive& ar)
{
	TRACE("FSCoreStudy::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSModelComponent::Load(ar); break;
		case CID_PROPERTY_LIST: LoadFEBioProperties(this, ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}

	// map parameters to FEBio class
	UpdateData(true);
}