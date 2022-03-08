#include "FEElementFormulation.h"
#include <FECore/fecore_enum.h>

FEElementFormulation::FEElementFormulation(FSModel* fem) : FSModelComponent(fem) {}

FESolidFormulation::FESolidFormulation(FSModel* fem) : FEElementFormulation(fem) 
{
	m_superClassID = FESOLIDDOMAIN_ID;
}

void FESolidFormulation::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FEElementFormulation::Save(ar);
	}
	ar.EndChunk();
}

void FESolidFormulation::Load(IArchive& ar)
{
	TRACE("FESolidFormulation::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FEElementFormulation::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

FEShellFormulation::FEShellFormulation(FSModel* fem) : FEElementFormulation(fem) 
{
	m_superClassID = FESHELLDOMAIN_ID;
}

void FEShellFormulation::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FEElementFormulation::Save(ar);
	}
	ar.EndChunk();
}

void FEShellFormulation::Load(IArchive& ar)
{
	TRACE("FEShellFormulation::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FEElementFormulation::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}
