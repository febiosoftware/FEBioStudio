#include "stdafx.h"
#include "FEMeshAdaptor.h"
#include <MeshLib/FSItemListBuilder.h>

FSMeshAdaptor::FSMeshAdaptor(FSModel* fem, int ntype) : FSDomainComponent(ntype, fem)
{
	m_ntype = ntype;
	SetSuperClassID(FEMESHADAPTOR_ID);
	SetMeshItemType(FE_ELEM_FLAG);
}

FEBioMeshAdaptor::FEBioMeshAdaptor(FSModel* fem) : FSMeshAdaptor(fem, FE_FEBIO_MESH_ADAPTOR)
{

}

void FEBioMeshAdaptor::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSMeshAdaptor::Save(ar);
	}
	ar.EndChunk();
}

void FEBioMeshAdaptor::Load(IArchive& ar)
{
	TRACE("FEBioLoadController::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSMeshAdaptor::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}
